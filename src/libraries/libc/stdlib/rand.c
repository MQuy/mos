#include <stdlib.h>

static uint32_t rseed = 1;

int rand_r(unsigned *seed)
{
	unsigned int next = *seed;
	int result;

	next *= 1103515245;
	next += 12345;
	result = (unsigned int)(next / 65536) % 2048;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int)(next / 65536) % 1024;

	next *= 1103515245;
	next += 12345;
	result <<= 10;
	result ^= (unsigned int)(next / 65536) % 1024;

	*seed = next;

	return result;
}

int rand()
{
	return rand_r(&rseed);
}

void srand(unsigned seed)
{
	rseed = seed;
}
