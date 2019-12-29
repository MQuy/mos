#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/pit.h>
#include <kernel/cpu/tss.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/malloc.h>
#include <kernel/proc/elf.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/time.h>
#include "task.h"

#define KERNEL_THREAD_SIZE 0x2000

extern void enter_usermode(uint32_t eip, uint32_t esp);
extern void return_usermode(uint32_t uregs);
extern void irq_schedule_handler(interrupt_registers *regs);
extern void thread_page_fault(interrupt_registers *regs);

volatile thread *current_thread;
volatile process *current_process;
static uint32_t next_pid = 0;
static uint32_t next_tid = 0;

files_struct *clone_file_descriptor_table(process *parent)
{
  files_struct *files = malloc(sizeof(files_struct));

  if (parent)
  {
    memcpy(files, parent->files, sizeof(files_struct));
    // NOTE: MQ 2019-12-30 Increasing file description usage when forking because child refers to the same one
    for (uint32_t i = 0; i < MAX_FD; ++i)
      if (parent->files->fd[i])
        parent->files->fd[i]->f_count++;
  }
  sema_init(&files->lock, 1);
  return files;
}

void kernel_thread_entry(thread *t, void *flow())
{
  flow();
  schedule();
}

thread *create_kernel_thread(process *parent, uint32_t eip, thread_state state, int priority)
{
  disable_interrupts();

  thread *t = malloc(sizeof(thread));
  t->tid = next_tid++;
  t->kernel_stack = (uint32_t)(malloc(KERNEL_THREAD_SIZE) + KERNEL_THREAD_SIZE);
  t->parent = parent;
  t->state = state;
  t->esp = t->kernel_stack - sizeof(trap_frame);
  plist_node_init(&t->sched_sibling, priority);

  trap_frame *frame = (trap_frame *)t->esp;
  memset(frame, 0, sizeof(trap_frame));

  frame->parameter2 = eip;
  frame->parameter1 = (uint32_t)t;
  frame->return_address = PROCESS_TRAPPED_PAGE_FAULT;
  frame->eip = (uint32_t)kernel_thread_entry;

  frame->eax = 0;
  frame->ecx = 0;
  frame->edx = 0;
  frame->ebx = 0;
  frame->esp = 0;
  frame->ebp = 0;
  frame->esi = 0;
  frame->edi = 0;

  list_add_tail(&t->sibling, &parent->threads);

  enable_interrupts();

  return t;
}

process *create_process(process *parent, const char *name, pdirectory *pdir)
{
  disable_interrupts();

  process *p = malloc(sizeof(process));
  p->pid = next_pid++;
  p->name = strdup(name);
  if (pdir)
    p->pdir = vmm_create_address_space(pdir);
  else
    p->pdir = vmm_get_directory();
  p->parent = parent;
  p->files = clone_file_descriptor_table(parent);
  p->fs = malloc(sizeof(fs_struct));

  if (parent)
  {
    memcpy(p->fs, parent->fs, sizeof(fs_struct));
    list_add_tail(&p->sibling, &parent->children);
  }

  INIT_LIST_HEAD(&p->children);
  INIT_LIST_HEAD(&p->threads);

  enable_interrupts();

  return p;
}

void setup_swapper_process()
{
  current_process = create_process(NULL, "swapper", NULL);
  current_thread = create_kernel_thread(current_process, 0, RUNNING, 0);

  current_process->active_thread = current_thread;
}

void task_init(void *func)
{
  sched_init();
  setup_swapper_process();
  register_interrupt_handler(IRQ0, irq_schedule_handler);
  register_interrupt_handler(14, thread_page_fault);

  process *init = create_process(current_process, "init", current_process->pdir);
  thread *nt = create_kernel_thread(init, func, READY_TO_RUN, 0);

  switch_thread(nt);
}

void user_thread_entry(thread *t)
{
  tss_set_stack(0x10, t->kernel_stack);
  return_usermode(&t->uregs);
}

void user_thread_elf_entry(thread *t, const char *path)
{
  char *buf = vfs_read(path);
  Elf32_Layout *elf_layout = elf_load(buf, t->parent->pdir);
  t->user_stack = elf_layout->stack;
  tss_set_stack(0x10, t->kernel_stack);
  enter_usermode(elf_layout->stack, elf_layout->entry);
}

thread *create_user_thread(process *parent, const char *path, thread_state state, thread_policy policy, int priority)
{
  disable_interrupts();

  thread *t = malloc(sizeof(thread));
  t->tid = next_tid++;
  t->parent = parent;
  t->state = state;
  t->kernel_stack = (uint32_t)(malloc(KERNEL_THREAD_SIZE) + KERNEL_THREAD_SIZE);
  t->esp = t->kernel_stack - sizeof(trap_frame);
  plist_node_init(&t->sched_sibling, priority);

  trap_frame *frame = (trap_frame *)t->esp;
  memset(frame, 0, sizeof(trap_frame));

  frame->parameter2 = (uint32_t)path;
  frame->parameter1 = (uint32_t)t;
  frame->return_address = PROCESS_TRAPPED_PAGE_FAULT;
  frame->eip = (uint32_t)user_thread_elf_entry;

  frame->eax = 0;
  frame->ecx = 0;
  frame->edx = 0;
  frame->ebx = 0;
  frame->esp = 0;
  frame->ebp = 0;
  frame->esi = 0;
  frame->edi = 0;

  list_add_tail(&t->sibling, &parent->threads);

  enable_interrupts();

  return t;
}

void process_load(const char *pname, const char *path)
{
  process *p = create_process(current_process, pname, current_process->pdir);
  thread *t = create_user_thread(p, path, READY_TO_RUN, SYSTEM_POLICY, 0);
  queue_thread(t);
}

process *process_fork(process *parent)
{
  disable_interrupts();

  // fork process
  process *p = malloc(sizeof(process));
  p->pid = next_pid++;
  p->gid = parent->pid;
  p->name = strdup(parent->name);
  p->parent = parent;
  INIT_LIST_HEAD(&p->children);
  INIT_LIST_HEAD(&p->threads);

  list_add_tail(&p->sibling, &parent->children);

  p->fs = malloc(sizeof(fs_struct));
  memcpy(p->fs, parent->fs, sizeof(fs_struct));

  p->files = clone_file_descriptor_table(parent);
  p->pdir = vmm_fork(parent->pdir);

  // copy active parent's thread
  thread *parent_thread = parent->active_thread;
  thread *t = malloc(sizeof(thread));
  t->tid = next_tid++;
  t->state = READY_TO_RUN;
  t->time_used = 0;
  t->parent = p;
  t->kernel_stack = (uint32_t)(malloc(KERNEL_THREAD_SIZE) + KERNEL_THREAD_SIZE);
  t->user_stack = parent_thread->user_stack;
  // NOTE: MQ 2019-12-18 Setup trap frame
  t->esp = t->kernel_stack - sizeof(trap_frame);
  plist_node_init(&t->sched_sibling, parent_thread->sched_sibling.prio);

  memcpy(&t->uregs, &parent_thread->uregs, sizeof(interrupt_registers));
  t->uregs.eax = 0;

  trap_frame *frame = (trap_frame *)t->esp;
  frame->parameter1 = (uint32_t)t;
  frame->return_address = PROCESS_TRAPPED_PAGE_FAULT;
  frame->eip = (uint32_t)user_thread_entry;

  frame->eax = 0;
  frame->ecx = 0;
  frame->edx = 0;
  frame->ebx = 0;
  frame->esp = 0;
  frame->ebp = 0;
  frame->esi = 0;
  frame->edi = 0;

  list_add_tail(&t->sibling, &p->threads);

  enable_interrupts();

  return p;
}