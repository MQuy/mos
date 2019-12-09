#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/pit.h>
#include <kernel/memory/vmm.h>
#include "task.h"

extern void irq_task_handler();
extern void do_switch(uint32_t addr_current_kernel_esp, uint32_t next_kernel_esp, uint32_t cr3);

extern thread *current_thread;
extern process *current_process;

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

void queue_thread(thread *t)
{
  if (t->state == READY_TO_RUN)
  {
    if (t->policy == KERNEL_POLICY)
      add_thread_to_queue(kernel_ready_list, t);
    else if (t->policy == SYSTEM_POLICY)
      add_thread_to_queue(system_ready_list, t);
    else
      add_thread_to_queue(app_ready_list, t);
  }
  else if (t->state == WAITING)
    add_thread_to_queue(waiting_list, t);
  else if (t->state == TERMINATED)
    add_thread_to_queue(terminated_list, t);
}

void remove_thread_from_queue(thread *thread)
{
}

void update_thread(thread *thread, uint8_t state)
{
  lock_scheduler();

  remove_thread_from_queue(thread);
  thread->state = state;
  queue_thread(thread);

  unlock_scheduler();
}

void switch_thread(thread *nt)
{
  disable_interrupts();

  thread *pt = current_thread;

  current_thread = nt;
  current_thread->time_used = 0;
  current_process = current_thread->parent;

  tss_set_stack(0x10, current_thread->kernel_stack);
  do_switch(&pt->esp, current_thread->esp, vmm_get_physical_address(current_thread->parent->pdir));
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

  unlock_scheduler();
  switch_thread(next_thread);
}

void sleep(uint32_t delay)
{
  lock_scheduler();

  uint32_t time = get_milliseconds_from_boot();
  uint32_t when = time + delay;
  if (when > time)
  {
    current_thread->expiry_when = when;
    update_thread(current_thread, WAITING);
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
      update_thread(q, READY_TO_RUN);
      is_schedulable = true;
    }

  if (current_thread->time_used >= SLICE_THRESHOLD * TICK_TIME && current_thread->policy == APP_POLICY)
  {
    update_thread(current_thread, READY_TO_RUN);
    is_schedulable = true;
  }

  // NOTE: MQ 2019-10-15 If counter is 1, it means that there is not running scheduler
  if (is_schedulable && scheduler_lock_counter == 1)
    schedule();

  unlock_scheduler();
}