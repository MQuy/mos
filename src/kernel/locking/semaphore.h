#ifndef LOCKING_SEMAPHORE_H
#define LOCKING_SEMAPHORE_H

#include <stdint.h>
#include <include/list.h>
#include "spinlock.h"

typedef struct semaphore
{
  spinlock_t lock;
  uint32_t count;
  uint32_t capacity;
  struct list_head wait_list;
} semaphore;

#define __SEMAPHORE_INITIALIZER(name, n)           \
  {                                                \
    .lock = 0,                                     \
    .count = n,                                    \
    .capacity = n,                                 \
    .wait_list = LIST_HEAD_INIT((name).wait_list), \
  }

#define DEFINE_SEMAPHORE(name) \
  semaphore name = __SEMAPHORE_INITIALIZER(name, 1)

static inline void sema_init(semaphore *sem, int val)
{
  *sem = (semaphore)__SEMAPHORE_INITIALIZER(*sem, val);
}

void acquire_semaphore(semaphore *sem);
void release_semaphore(semaphore *sem);

#endif