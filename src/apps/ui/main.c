#include <stdint.h>

void main()
{
  uint32_t x = 1;
  const char *str = "hello from elf";
  __asm__ __volatile__("int $0x7F"
                       : "=a"(x)
                       : "0"(0), "b"(str));
}