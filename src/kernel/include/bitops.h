#ifndef INCLUDE_BITOPS_H
#define INCLUDE_BITOPS_H

#define ADDR (*(volatile int*)addr)

/**
 * ffz - find first zero in word.
 * @word: The word to search
 *
 * Undefined if no zero exists, so code should check against ~0UL first.
 */
static inline unsigned int ffz(unsigned int word)
{
	__asm__("bsfl %1,%0"
			: "=r"(word)
			: "r"(~word));
	return word;
}

/**
 * __set_bit - Set a bit in memory
 * @nr: the bit to set
 * @addr: the address to start counting from
 */
static inline void set_bit(int nr, volatile unsigned int* addr)
{
	__asm__(
		"btsl %1,%0"
		: "=m"(ADDR)
		: "Ir"(nr));
}

static inline void clear_bit(int nr, volatile unsigned int* addr)
{
	__asm__ __volatile__(
		"btrl %1,%0"
		: "=m"(ADDR)
		: "Ir"(nr));
}

static inline int test_bit(int nr, const unsigned int* addr)
{
	int mask;

	addr += nr >> 5;
	mask = 1 << (nr & 0x1f);
	return ((mask & *addr) != 0);
}

#endif
