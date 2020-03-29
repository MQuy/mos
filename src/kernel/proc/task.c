#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/pit.h>
#include <kernel/cpu/tss.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/elf.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/time.h>
#include <kernel/utils/hashmap.h>
#include "task.h"

extern void enter_usermode(uint32_t eip, uint32_t esp, uint32_t failed_address);
extern void return_usermode(uint32_t uregs);
extern void irq_schedule_handler(interrupt_registers *regs);
extern void thread_page_fault(interrupt_registers *regs);

volatile thread *current_thread;
volatile process *current_process;
static uint32_t next_pid = 0;
static uint32_t next_tid = 0;
struct hashmap mprocess;

files_struct *clone_file_descriptor_table(process *parent)
{
  files_struct *files = kcalloc(1, sizeof(struct files_struct));

  if (parent)
  {
    memcpy(files, parent->files, sizeof(struct files_struct));
    // NOTE: MQ 2019-12-30 Increasing file description usage when forking because child refers to the same one
    for (uint32_t i = 0; i < MAX_FD; ++i)
      if (parent->files->fd[i])
        parent->files->fd[i]->f_count++;
  }
  sema_init(&files->lock, 1);
  return files;
}

mm_struct *clone_mm_struct(process *parent)
{
  mm_struct *mm = kcalloc(1, sizeof(struct mm_struct));
  memcpy(mm, parent->mm, sizeof(struct mm_struct));
  INIT_LIST_HEAD(&mm->mmap);

  vm_area_struct *iter = NULL;
  list_for_each_entry(iter, &parent->mm->mmap, vm_sibling)
  {
    vm_area_struct *clone = kcalloc(1, sizeof(struct vm_area_struct));
    clone->vm_start = iter->vm_start;
    clone->vm_end = iter->vm_end;
    clone->vm_file = iter->vm_file;
    clone->vm_flags = iter->vm_flags;
    clone->vm_mm = mm;
    list_add_tail(&clone->vm_sibling, &mm->mmap);
  }

  return mm;
}

void kernel_thread_entry(thread *t, void *flow())
{
  flow();
  schedule();
}

thread *create_kernel_thread(process *parent, uint32_t eip, thread_state state, int priority)
{
  disable_interrupts();

  thread *t = kcalloc(1, sizeof(struct thread));
  t->tid = next_tid++;
  t->kernel_stack = (uint32_t)(kcalloc(STACK_SIZE, sizeof(char)) + STACK_SIZE);
  t->parent = parent;
  t->state = state;
  t->esp = t->kernel_stack - sizeof(struct trap_frame);
  plist_node_init(&t->sched_sibling, priority);

  trap_frame *frame = (trap_frame *)t->esp;
  memset(frame, 0, sizeof(struct trap_frame));

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

  process *p = kcalloc(1, sizeof(struct process));
  p->pid = next_pid++;
  p->name = strdup(name);
  if (pdir)
    p->pdir = vmm_create_address_space(pdir);
  else
    p->pdir = vmm_get_directory();
  p->parent = parent;
  p->files = clone_file_descriptor_table(parent);
  p->fs = kcalloc(1, sizeof(struct fs_struct));

  p->mm = kcalloc(1, sizeof(struct mm_struct));
  INIT_LIST_HEAD(&p->mm->mmap);

  if (parent)
  {
    memcpy(p->fs, parent->fs, sizeof(struct fs_struct));
    list_add_tail(&p->sibling, &parent->children);
  }

  INIT_LIST_HEAD(&p->children);
  INIT_LIST_HEAD(&p->threads);

  hashmap_put(&mprocess, &p->pid, p);

  enable_interrupts();

  return p;
}

void setup_swapper_process()
{
  current_process = create_process(NULL, "swapper", NULL);
  current_thread = create_kernel_thread(current_process, 0, THREAD_RUNNING, 0);

  current_process->active_thread = current_thread;
}

void task_init(void *func)
{
  hashmap_init(&mprocess, hashmap_hash_uint32, hashmap_compare_uint32, 0);
  sched_init();
  // register_interrupt_handler(IRQ0, irq_schedule_handler);
  register_interrupt_handler(14, thread_page_fault);

  setup_swapper_process();

  process *init = create_process(current_process, "init", current_process->pdir);
  thread *nt = create_kernel_thread(init, func, THREAD_WAITING, 0);

  update_thread(current_thread, THREAD_TERMINATED);
  update_thread(nt, THREAD_READY);
  schedule();
}

void user_thread_entry(thread *t)
{
  tss_set_stack(0x10, t->kernel_stack);
  return_usermode(&t->uregs);
}

void user_thread_elf_entry(thread *t, const char *path, void *setup(Elf32_Layout *))
{
  char *buf = vfs_read(path);
  Elf32_Layout *elf_layout = elf_load(buf);
  t->user_stack = elf_layout->stack;
  tss_set_stack(0x10, t->kernel_stack);
  if (setup)
    setup(&elf_layout->stack);
  enter_usermode(elf_layout->stack, elf_layout->entry, PROCESS_TRAPPED_PAGE_FAULT);
}

thread *create_user_thread(process *parent, const char *path, thread_state state, thread_policy policy, int priority, void *setup(Elf32_Layout *))
{
  disable_interrupts();

  thread *t = kcalloc(1, sizeof(struct thread));
  t->tid = next_tid++;
  t->parent = parent;
  t->state = state;
  t->policy = policy;
  t->kernel_stack = (uint32_t)(kcalloc(STACK_SIZE, sizeof(char)) + STACK_SIZE);
  t->esp = t->kernel_stack - sizeof(struct trap_frame);
  plist_node_init(&t->sched_sibling, priority);

  trap_frame *frame = (trap_frame *)t->esp;
  memset(frame, 0, sizeof(struct trap_frame));

  frame->parameter3 = (uint32_t)setup;
  frame->parameter2 = (uint32_t)strdup(path);
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

void process_load(const char *pname, const char *path, int priority, void *setup(Elf32_Layout *))
{
  process *p = create_process(current_process, pname, current_process->pdir);
  thread *t = create_user_thread(p, path, THREAD_READY, THREAD_SYSTEM_POLICY, priority, setup);
  queue_thread(t);
}

process *process_fork(process *parent)
{
  disable_interrupts();

  // fork process
  process *p = kcalloc(1, sizeof(struct process));
  p->pid = next_pid++;
  p->gid = parent->pid;
  p->name = strdup(parent->name);
  p->parent = parent;
  p->mm = clone_mm_struct(parent);

  INIT_LIST_HEAD(&p->children);
  INIT_LIST_HEAD(&p->threads);

  list_add_tail(&p->sibling, &parent->children);

  p->fs = kcalloc(1, sizeof(struct fs_struct));
  memcpy(p->fs, parent->fs, sizeof(struct fs_struct));

  p->files = clone_file_descriptor_table(parent);
  p->pdir = vmm_fork(parent->pdir);

  // copy active parent's thread
  thread *parent_thread = parent->active_thread;
  thread *t = kcalloc(1, sizeof(struct thread));
  t->tid = next_tid++;
  t->state = THREAD_READY;
  t->policy = parent_thread->policy;
  t->time_slice = 0;
  t->parent = p;
  t->kernel_stack = (uint32_t)(kcalloc(STACK_SIZE, sizeof(char)) + STACK_SIZE);
  t->user_stack = parent_thread->user_stack;
  // NOTE: MQ 2019-12-18 Setup trap frame
  t->esp = t->kernel_stack - sizeof(struct trap_frame);
  plist_node_init(&t->sched_sibling, parent_thread->sched_sibling.prio);

  memcpy(&t->uregs, &parent_thread->uregs, sizeof(struct interrupt_registers));
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

process *get_process(pid_t pid)
{
  return hashmap_get(&mprocess, &pid);
}