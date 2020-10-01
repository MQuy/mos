#ifndef LOCKING_SEMAPHORE_H
#define LOCKING_SEMAPHORE_H

#include <shared/list.h>
#include <stdint.h>

#include "spinlock.h"

struct semaphore
{
	spinlock_t lock;
	uint32_t count;
	uint32_t capacity;
	struct list_head wait_list;
};

#define __SEMAPHORE_INITIALIZER(name, n)               \
	{                                                  \
		.lock = 0,                                     \
		.count = n,                                    \
		.capacity = n,                                 \
		.wait_list = LIST_HEAD_INIT((name).wait_list), \
	}

#define DEFINE_SEMAPHORE(name) \
	struct semaphore name = __SEMAPHORE_INITIALIZER(name, 1)

static inline void sema_init(struct semaphore *sem, int val)
{
	*sem = (struct semaphore)__SEMAPHORE_INITIALIZER(*sem, val);
}

void acquire_semaphore(struct semaphore *sem);
void release_semaphore(struct semaphore *sem);

#endif
