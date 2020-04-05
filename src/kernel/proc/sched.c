#include <include/limits.h>
#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/cpu/pit.h>
#include <kernel/cpu/tss.h>
#include <kernel/memory/vmm.h>
#include "task.h"

extern void irq_task_handler();
extern void do_switch(uint32_t *addr_current_kernel_esp, uint32_t next_kernel_esp, uint32_t cr3);

extern struct thread *current_thread;
extern struct process *current_process;

struct thread *idle_thread;
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

struct thread *pick_next_thread_from_list(struct plist_head *list)
{
  if (plist_head_empty(list))
    return NULL;

  struct thread *t = plist_first_entry(list, struct thread, sched_sibling);
  plist_del(&t->sched_sibling, list);
  return t;
}

struct thread *pick_next_thread_to_run()
{
  struct thread *nt = pick_next_thread_from_list(&kernel_ready_list);
  if (!nt)
    nt = pick_next_thread_from_list(&system_ready_list);
  if (!nt)
    nt = pick_next_thread_from_list(&app_ready_list);

  return nt;
}

struct plist_head *get_list_from_thread(enum thread_state state, enum thread_policy policy)
{
  if (state == THREAD_READY)
  {
    if (policy == THREAD_KERNEL_POLICY)
      return &kernel_ready_list;
    else if (policy == THREAD_SYSTEM_POLICY)
      return &system_ready_list;
    else
      return &app_ready_list;
  }
  else if (state == THREAD_WAITING)
    return &waiting_list;
  else if (state == THREAD_TERMINATED)
    return &terminated_list;
  return NULL;
}

int get_top_priority_from_list(enum thread_state state, enum thread_policy policy)
{
  struct plist_head *h = get_list_from_thread(state, policy);

  if (!plist_head_empty(h))
  {
    struct plist_node *node = plist_first(h);
    if (node)
      return node->prio;
  }
  return INT_MAX;
}

void queue_thread(struct thread *t)
{
  struct plist_head *h = get_list_from_thread(t->state, t->policy);

  if (h)
    plist_add(&t->sched_sibling, h);
}

void remove_thread(struct thread *t)
{
  struct plist_head *h = get_list_from_thread(t->state, t->policy);

  if (h)
    plist_del(&t->sched_sibling, h);
}

void update_thread(struct thread *thread, uint8_t state)
{
  lock_scheduler();

  remove_thread(thread);
  thread->state = state;
  queue_thread(thread);

  unlock_scheduler();
}

void switch_thread(struct thread *nt)
{
  if (current_thread == nt)
    return;

  struct thread *pt = current_thread;

  current_thread = nt;
  current_thread->time_slice = 0;
  current_thread->state = THREAD_RUNNING;
  current_process = current_thread->parent;
  current_process->active_thread = current_thread;

  uint32_t paddr_cr3 = vmm_get_physical_address((uint32_t)current_thread->parent->pdir, true);
  tss_set_stack(0x10, current_thread->kernel_stack);
  do_switch(&pt->esp, current_thread->esp, paddr_cr3);
}

void schedule()
{
  lock_scheduler();

  if (current_thread->state == THREAD_RUNNING)
    return;

  struct thread *nt = pick_next_thread_to_run();

  if (!nt)
  {
    // update_thread(current_thread, THREAD_RUNNING);
    do
    {
      enable_interrupts();
      halt();
      disable_interrupts();
      nt = pick_next_thread_to_run();
    } while (!nt);
  }

  switch_thread(nt);
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
    update_thread(current_thread, THREAD_WAITING);
  }
  schedule();

  unlock_scheduler();
}

#define SLICE_THRESHOLD 50
int32_t irq_schedule_handler(struct interrupt_registers *regs)
{
  lock_scheduler();

  bool is_schedulable = false;
  uint32_t time = get_milliseconds_from_boot();
  current_thread->time_slice++;

  struct thread *t = NULL;
  plist_for_each_entry(t, &waiting_list, sched_sibling)
  {
    if (t->expiry_when >= time)
    {
      update_thread(t, THREAD_READY);
      is_schedulable = true;
    }
  }

  if (current_thread->time_slice >= SLICE_THRESHOLD && current_thread->sched_sibling.prio == THREAD_APP_POLICY)
  {
    update_thread(current_thread, THREAD_READY);
    is_schedulable = true;
  }

  // NOTE: MQ 2019-10-15 If counter is 1, it means that there is not running scheduler
  if (is_schedulable && scheduler_lock_counter == 1)
    schedule();

  unlock_scheduler();

  return IRQ_HANDLER_CONTINUE;
}

int32_t thread_page_fault(struct interrupt_registers *regs)
{

  uint32_t faultAddr = 0;
  __asm__ __volatile__("mov %%cr2, %0"
                       : "=r"(faultAddr));

  if (faultAddr == PROCESS_TRAPPED_PAGE_FAULT && regs->cs == 0x1B)
  {
    update_thread(current_thread, THREAD_TERMINATED);
    schedule();

    return IRQ_HANDLER_STOP;
  }

  return IRQ_HANDLER_CONTINUE;
}

void sched_init()
{
  plist_head_init(&kernel_ready_list);
  plist_head_init(&system_ready_list);
  plist_head_init(&app_ready_list);
  plist_head_init(&waiting_list);
  plist_head_init(&terminated_list);
}