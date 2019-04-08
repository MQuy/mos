#include "common.h"

int div_ceil(int x, int y)
{
  return x / y + (x % y > 0);
}
