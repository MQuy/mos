#include "math.h"

uint32_t rseed = 1;

uint32_t rand_r(uint32_t *seed)
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

uint32_t rand()
{
  return rand_r(&rseed);
}

uint32_t srand(uint32_t seed)
{
  rseed = seed;
  return rand_r(&rseed);
}
