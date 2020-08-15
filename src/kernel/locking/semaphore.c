#include "semaphore.h"

#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>

struct semaphore_waiter
{
	struct list_head sibling;
	struct thread *task;
};

void acquire_semaphore(struct semaphore *sem)
{
	// TODO: MQ 2020-07-20 should we use lock/unlock_scheduler instead?
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
		struct semaphore_waiter *waiter = kcalloc(1, sizeof(struct semaphore_waiter));
		waiter->task = current_thread;

		list_add_tail(&waiter->sibling, &sem->wait_list);
		update_thread(current_thread, THREAD_WAITING);
		spin_unlock(&sem->lock);
		schedule();
	}
}

void release_semaphore(struct semaphore *sem)
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
		struct semaphore_waiter *waiter = list_first_entry(&sem->wait_list, struct semaphore_waiter, sibling);

		list_del(&waiter->sibling);
		update_thread(waiter->task, THREAD_READY);
	}

	spin_unlock(&sem->lock);
	enable_interrupts();
}
