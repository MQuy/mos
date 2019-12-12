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

struct plist_head terminated_list, waiting_list;
struct plist_head kernel_ready_list, system_ready_list, app_ready_list;

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

thread *pick_next_thread_from_list(struct plist_head *list)
{
  if (plist_head_empty(list))
    return NULL;

  thread *t = plist_first_entry(list, thread, sched_sibling);
  plist_del(&t->sched_sibling.node_list, list);
  return t;
}

thread *pick_next_thread_to_run()
{
  thread *nt = pick_next_thread_from_list(&kernel_ready_list);
  if (!nt)
    nt = pick_next_thread_from_list(&system_ready_list);
  if (!nt)
    nt = pick_next_thread_from_list(&app_ready_list);
  return nt;
}

struct plist_head *get_list_from_thread(thread *t)
{
  if (t->state == READY_TO_RUN)
  {
    if (t->policy == KERNEL_POLICY)
      return &kernel_ready_list;
    else if (t->policy == SYSTEM_POLICY)
      return &system_ready_list;
    else
      return &app_ready_list;
  }
  else if (t->state == WAITING)
    return &waiting_list;
  else if (t->state == TERMINATED)
    return &terminated_list;
  return NULL;
}

void queue_thread(thread *t)
{
  struct plist_head *h = get_list_from_thread(t);

  if (h)
    plist_add(&t->sched_sibling.node_list, h);
}

void remove_thread(thread *t)
{
  struct plist_head *h = get_list_from_thread(t);

  if (h)
    plist_del(&t->sched_sibling.node_list, h);
}

void update_thread(thread *thread, uint8_t state)
{
  lock_scheduler();

  remove_thread(thread);
  thread->state = state;
  queue_thread(thread);

  unlock_scheduler();
}

void terminate_thread()
{
  update_thread(current_thread, TERMINATED);
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

  thread *nt = pick_next_thread_to_run();

  if (!nt)
    do
    {
      enable_interrupts();
      halt();
      disable_interrupts();
      nt = pick_next_thread_to_run();
    } while (!nt);

  unlock_scheduler();
  switch_thread(nt);
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
#define TICKS_PER_SECOND 1000
void irq_schedule_handler(interrupt_registers *regs)
{
  lock_scheduler();

  bool is_schedulable = false;
  uint32_t time = get_milliseconds_from_boot();
  current_thread->time_used += TICKS_PER_SECOND;

  thread *t = NULL;
  plist_for_each_entry(t, &waiting_list, sched_sibling)
  {
    if (t->expiry_when >= time)
    {
      update_thread(t, READY_TO_RUN);
      is_schedulable = true;
    }
  }

  if (current_thread->time_used >= SLICE_THRESHOLD * TICKS_PER_SECOND && current_thread->policy == APP_POLICY)
  {
    update_thread(current_thread, READY_TO_RUN);
    is_schedulable = true;
  }

  // NOTE: MQ 2019-10-15 If counter is 1, it means that there is not running scheduler
  if (is_schedulable && scheduler_lock_counter == 1)
    schedule();

  unlock_scheduler();
}

void sched_init()
{
  plist_head_init(&kernel_ready_list);
  plist_head_init(&system_ready_list);
  plist_head_init(&app_ready_list);
  plist_head_init(&waiting_list);
  plist_head_init(&terminated_list);
}