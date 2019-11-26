#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/pit.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/malloc.h>
#include <kernel/system/elf.h>
#include "time.h"
#include "task.h"
#include "tss.h"
#include <kernel/fs/vfs.h>

#define KERNEL_THREAD_SIZE 0x2000

#define get_next_thread(p) list_entry((p)->threads.next, struct thread, sibling)
#define get_prev_thread(p) list_entry((p)->threads.prev, struct thread, sibling)

extern void enter_usermode(uint32_t eip, uint32_t esp);
extern void irq_schedule_handler(interrupt_registers *regs);

volatile thread *current_thread;
volatile process *current_process;
static uint32_t next_pid = 0;
static uint32_t next_tid = 0;

void thread_main_entry(thread *t, void *flow())
{
  flow();
  schedule();
}

thread *create_kernel_thread(process *parent, uint32_t eip, enum thread_state state)
{
  thread *t = malloc(sizeof(thread));
  t->tid = next_tid++;
  t->policy = KERNEL_POLICY;
  t->priority = 0;
  t->kernel_stack = malloc(KERNEL_THREAD_SIZE) + KERNEL_THREAD_SIZE;
  t->parent = parent;
  t->state = state;
  t->esp = t->kernel_stack - sizeof(trap_frame);

  trap_frame *frame = (trap_frame *)t->esp;
  memset(frame, 0, sizeof(trap_frame));

  frame->parameter2 = eip;
  frame->parameter1 = t;
  frame->return_address = 0xFFFFFFFF;
  frame->eip = thread_main_entry;

  frame->eax = 0;
  frame->ecx = 0;
  frame->edx = 0;
  frame->ebx = 0;
  frame->esp = 0;
  frame->ebp = 0;
  frame->esi = 0;
  frame->edi = 0;

  list_add_tail(&t->sibling, &parent->threads);
  return t;
}

process *create_process(process *parent, const char *name, pdirectory *pdir)
{
  process *p = malloc(sizeof(process));
  p->pid = next_pid++;
  p->name = strdup(name);
  if (pdir)
    p->pdir = create_address_space(pdir);
  else
    p->pdir = vmm_get_directory();
  p->parent = parent;

  if (parent)
  {
    p->files = parent->files;
    p->fs = parent->fs;
    list_add_tail(&p->sibling, &parent->children);
  }
  else
  {
    p->files = malloc(sizeof(files_struct));
    p->fs = malloc(sizeof(fs_struct));
  }

  INIT_LIST_HEAD(&p->children);
  INIT_LIST_HEAD(&p->threads);

  return p;
}

void setup_swapper_process()
{
  current_process = create_process(NULL, "swapper", NULL);
  current_thread = create_kernel_thread(current_process, 0, RUNNING);
}

void task_init(void *func)
{
  setup_swapper_process();
  // FIXME: Enable it
  // register_pit_handler(irq_schedule_handler);

  process *init = create_process(current_process, "init", current_process->pdir);
  thread *nt = create_kernel_thread(init, func, READY_TO_RUN);

  switch_thread(nt);
}

/*
- fork current process
  - create new page directory
  - copy kernel page tables
- schedule process/thread with setup function (setup_uspace)
- setup_uspace is called
  - load elf into userspace
  - jump to elf entry (usermode)
*/

void user_thread_entry(thread *t, const char *path)
{
  char *buf = vfs_read(path);
  Elf32_Layout *elf_layout = elf_load(buf, t->parent->pdir);
  t->user_stack = elf_layout->stack;
  tss_set_stack(0x10, t->kernel_stack);
  enter_usermode(elf_layout->stack, elf_layout->entry);
}

thread *create_user_thread(process *parent, const char *path)
{
  thread *t = malloc(sizeof(thread));
  t->tid = next_tid++;
  t->parent = parent;
  t->policy = KERNEL_POLICY;
  t->priority = 0;
  t->state = READY_TO_RUN;
  t->kernel_stack = malloc(KERNEL_THREAD_SIZE) + KERNEL_THREAD_SIZE;
  t->esp = t->kernel_stack - sizeof(trap_frame);

  trap_frame *frame = (trap_frame *)t->esp;
  memset(frame, 0, sizeof(trap_frame));

  frame->parameter2 = path;
  frame->parameter1 = t;
  frame->return_address = 0xFFFFFFFF;
  frame->eip = user_thread_entry;

  frame->eax = 0;
  frame->ecx = 0;
  frame->edx = 0;
  frame->ebx = 0;
  frame->esp = 0;
  frame->ebp = 0;
  frame->esi = 0;
  frame->edi = 0;

  list_add_tail(&t->sibling, &parent->threads);
  return t;
}

void process_load(const char *pname, const char *path)
{
  process *p = create_process(current_process, pname, current_process->pdir);
  thread *t = create_user_thread(p, path);
  switch_thread(t);
}