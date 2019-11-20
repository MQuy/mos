#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/pit.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/memory/malloc.h>
#include "time.h"
#include "task.h"
#include "tss.h"
#include <kernel/fs/vfs.h>

#define KERNEL_THREAD_SIZE 0x2000

#define get_next_thread(p) list_entry((p)->threads.next, struct thread, sibling)
#define get_prev_thread(p) list_entry((p)->threads.prev, struct thread, sibling)

extern void irq_task_handler();
extern void do_switch(uint32_t addr_current_kernel_esp, uint32_t next_kernel_esp, uint32_t cr3);

volatile thread *current_thread;
volatile process *current_process;
static uint32_t next_pid = 0;
static uint32_t next_tid = 0;

typedef struct thread_queue
{
  thread *thread;
  struct thread_queue *next;
} thread_queue;
thread_queue *terminated_list, *waiting_list;
thread_queue *kernel_ready_list, *system_ready_list, *app_ready_list;

static uint32_t scheduler_lock_counter = 0;
void lock_scheduler()
{
  disable_interrupts();
  scheduler_lock_counter++;
}

void unlock_scheduler()
{
  scheduler_lock_counter--;
  if (scheduler_lock_counter == 0)
    enable_interrupts();
}

void block_thread(thread *thread, uint8_t state)
{
  thread->state = state;
}

void unblock_thread(thread *thread)
{
  thread->state = READY_TO_RUN;
}

thread *pick_next_thread_from_queue(thread_queue *queue)
{
  thread *nt;
  for (thread_queue **q = &queue; *q; q = &(*q)->next)
    if ((*q)->thread)
    {
      nt = (*q)->thread;
      *q = (*q)->next;
    }
  return nt;
}

thread *pick_next_thread_to_run()
{
  thread *nt = pick_next_thread_from_queue(kernel_ready_list);
  if (!nt)
    nt = pick_next_thread_from_queue(system_ready_list);
  if (!nt)
    nt = pick_next_thread_from_queue(app_ready_list);
  return nt;
}

void add_thread_to_queue(thread_queue *queue, thread *t)
{
  if (!queue->thread)
  {
    queue->thread = t;
  }
  else
  {
    thread_queue *nq = malloc(sizeof(thread_queue));
    nq->thread = t;
    thread_queue **q;
    for (q = &queue; *q; q = &(*q)->next)
      if ((*q)->thread->priority >= t)
        break;
    if (*q)
      nq->next = (*q)->next;
    (*q) = &nq;
  }
}

void switch_thread(thread *nt)
{
  nt->time_used = 0;
  do_switch(&current_thread->esp, nt->esp, nt->parent->pdir);
  current_thread = nt;
  tss_set_stack(0x10, current_thread->kernel_stack);
}

void schedule()
{
  lock_scheduler();

  if (current_thread->state == RUNNING)
    return;

  thread *next_thread = pick_next_thread_to_run();

  if (!next_thread)
    do
    {
      enable_interrupts();
      halt();
      disable_interrupts();
      next_thread = pick_next_thread_to_run();
    } while (!next_thread);

  switch_thread(next_thread);

  unlock_scheduler();
}

void sleep(uint32_t delay)
{
  lock_scheduler();

  uint32_t time = get_milliseconds_from_boot();
  uint32_t when = time + delay;
  if (when > time)
  {
    current_thread->expiry_when = when;
    block_thread(current_thread, WAITING);
    add_thread_to_queue(waiting_list, current_thread);
  }
  schedule();

  unlock_scheduler();
}

#define SLICE_THRESHOLD 10
#define TICK_TIME 1000
void irq_schedule_handler(interrupt_registers *regs)
{
  lock_scheduler();

  bool is_schedulable;
  uint32_t time = get_milliseconds_from_boot();
  current_thread->time_used += TICK_TIME;

  for (thread_queue *q = waiting_list; q; q = q->next)
    if (q->thread->expiry_when >= time)
    {
      unblock_thread(q);
      is_schedulable = true;
    }

  if (current_thread->time_used >= SLICE_THRESHOLD * TICK_TIME && current_thread->policy == APP_POLICY)
  {
    block_thread(current_thread, READY_TO_RUN);
    is_schedulable = true;
  }

  // NOTE: MQ 2019-10-15 If counter is 1, it means that there is not running scheduler
  if (is_schedulable && scheduler_lock_counter == 1)
    schedule();

  unlock_scheduler();
}

void thread_main_entry(thread *t, void *flow())
{
  flow();
  schedule();
}

thread *create_kernel_thread(process *parent, uint32_t eip)
{
  thread *t = malloc(sizeof(thread));
  t->tid = next_tid++;
  t->kernel_stack = malloc(KERNEL_THREAD_SIZE) + KERNEL_THREAD_SIZE;
  t->parent = parent;
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

  return t;
}

process *create_process(process *parent, const char *name, pdirectory *pdir, uint32_t eip, bool is_kernel)
{
  process *p = malloc(sizeof(process));
  p->pid = next_pid++;
  p->name = strdup(name);
  p->files = malloc(sizeof(files_struct));
  p->fs = malloc(sizeof(fs_struct));
  if (pdir)
    p->pdir = create_address_space(pdir);
  else
    p->pdir = vmm_get_directory();
  p->parent = parent;

  INIT_LIST_HEAD(&p->children);
  if (current_process)
    list_add_tail(&p->sibling, &current_process->children);

  thread *t = create_kernel_thread(p, eip);
  INIT_LIST_HEAD(&p->threads);
  list_add_tail(&t->sibling, &p->threads);

  return p;
}

void setup_swapper_process()
{
  current_process = create_process(NULL, "swapper", NULL, 0, true);
  current_thread = get_next_thread(current_process);
  current_thread->state = RUNNING;
}

void task_init(void *func)
{
  setup_swapper_process();
  register_pit_handler(irq_schedule_handler);

  process *init = create_process(current_process, "init", current_process->pdir, func, true);
  thread *nt = get_next_thread(init);
  nt->state = READY_TO_RUN;

  switch_thread(nt);
}