#include <stdint.h>
#include <include/ctype.h>
#include <libc/unistd.h>

int main()
{
  uint32_t x = 1;
  int32_t ret = fork();
  switch (ret)
  {
  case -1: /* Handle error */
    break;

  case 0: /* Child - reads from pipe */
    x += 1;
    break;

  default: /* Parent - writes to pipe */
    x += 2;
    break;
  }
  return x;
}