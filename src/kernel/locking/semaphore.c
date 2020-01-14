#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include "semaphore.h"

extern thread *current_thread;

typedef struct semaphore_waiter
{
  struct list_head sibling;
  struct thread *task;
} semaphore_waiter;

void acquire_semaphore(semaphore *sem)
{
  disable_interrupts();
  spin_lock(&sem->lock);
  if (sem->count > 0)
  {
    sem->count--;
    spin_unlock(&sem->lock);
    enable_interrupts();
  }
  else
  {
    semaphore_waiter *waiter = kmalloc(sizeof(semaphore_waiter));
    waiter->task = current_thread;

    list_add_tail(&waiter->sibling, &sem->wait_list);
    update_thread(current_thread, THREAD_WAITING);
    spin_unlock(&sem->lock);
    schedule();
  }
}

void release_semaphore(semaphore *sem)
{
  disable_interrupts();
  spin_lock(&sem->lock);
  if (list_empty(&sem->wait_list))
  {
    if (sem->count < sem->capacity)
      sem->count++;
  }
  else
  {
    semaphore_waiter *waiter = list_first_entry(&sem->wait_list, struct semaphore_waiter, sibling);

    list_del(&waiter->sibling);
    update_thread(waiter->task, THREAD_READY);
  }

  spin_unlock(&sem->lock);
  enable_interrupts();
}