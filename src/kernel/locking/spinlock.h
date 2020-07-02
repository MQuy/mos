#ifndef LOCKING_SPINLOCK_H
#define LOCKING_SPINLOCK_H

#define barrier() asm volatile("" \
							   :  \
							   :  \
							   : "memory")

/* Pause instruction to prevent excess processor bus usage */
#define cpu_relax() asm volatile("pause\n" \
								 :         \
								 :         \
								 : "memory")

static inline unsigned short xchg_8(void *ptr, unsigned char x)
{
	__asm__ __volatile__("xchgb %0,%1"
						 : "=r"(x)
						 : "m"(*(volatile unsigned char *)ptr), "0"(x)
						 : "memory");

	return x;
}

#define SPINLOCK_UNLOCKED 0
#define SPINLOCK_LOCK 1

typedef unsigned char spinlock_t;

static inline void spin_lock(spinlock_t *lock)
{
	while (1)
	{
		if (!xchg_8(lock, SPINLOCK_LOCK))
			return;

		while (*lock)
			cpu_relax();
	}
}

static inline void spin_unlock(spinlock_t *lock)
{
	barrier();
	*lock = 0;
}

static inline int spin_trylock(spinlock_t *lock)
{
	return xchg_8(lock, SPINLOCK_LOCK);
}

#endif
