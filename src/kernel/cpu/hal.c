#include "hal.h"

unsigned char inportb(unsigned short portid)
{

  uint8_t data;
  __asm__("in %%dx, %%al"
          : "=a"(data)
          : "d"(portid));
  return data;
}

//! write byte to device through port mapped io
void outportb(unsigned short portid, unsigned char value)
{
  __asm__("out %%al, %%dx"
          :
          : "a"(value), "d"(portid));
}

void io_wait()
{
  /* Port 0x80 is used for 'checkpoints' during POST. */
  /* The Linux kernel seems to think it is free for use :-/ */
  __asm__ __volatile__("outb %%al, $0x80"
                       :
                       : "a"(0));
}

//! enable all hardware interrupts
void enable_interrupts()
{
  __asm__ __volatile__("sti");
}

//! disable all hardware interrupts
void disable_interrupts()
{
  __asm__ __volatile__("cli");
}

void halt()
{
  __asm__ __volatile__("hlt");
}

char *i86_cpu_get_vender()
{
  static char vender[32] = {0};
  __asm__ __volatile__("mov $0, %%eax;				\n"
                       "cpuid;								\n"
                       "lea (%0), %%eax;			\n"
                       "mov %%ebx, (%%eax);		\n"
                       "mov %%edx, 0x4(%%eax);\n"
                       "mov %%ecx, 0x8(%%eax)	\n"
                       : "=m"(vender));
  return vender;
}