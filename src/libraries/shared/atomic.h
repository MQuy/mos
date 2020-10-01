#ifndef SHARED_ATOMIC_H
#define SHARED_ATOMIC_H

typedef struct
{
	volatile int counter;
} atomic_t;

#define ATOMIC_INIT(i) \
	{                  \
		(i)            \
	}

#define atomic_read(v) ((v)->counter)

#define atomic_set(v, i) (((v)->counter) = (i))

static inline void atomic_add(int i, atomic_t *v)
{
	__asm__ __volatile__(
		"addl %1,%0"
		: "=m"(v->counter)
		: "ir"(i), "m"(v->counter));
}

static void atomic_sub(int i, atomic_t *v)
{
	__asm__ __volatile__(
		"subl %1,%0"
		: "=m"(v->counter)
		: "ir"(i), "m"(v->counter));
}

static inline void atomic_inc(atomic_t *v)
{
	__asm__ __volatile__(
		"incl %0"
		: "=m"(v->counter)
		: "m"(v->counter));
}

static inline void atomic_dec(atomic_t *v)
{
	__asm__ __volatile__(
		"decl %0"
		: "=m"(v->counter)
		: "m"(v->counter));
}

#endif
