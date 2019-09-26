#include "hal.h"

unsigned char inportb(unsigned short portid)
{

  uint8_t data;
  __asm__ __volatile__("in %%dx, %%al"
                       : "=a"(data)
                       : "d"(portid));
  return data;
}

void outportb(unsigned short portid, unsigned char value)
{
  __asm__ __volatile__("out %%al, %%dx"
                       :
                       : "a"(value), "d"(portid));
}

void outportw(uint16_t portid, uint16_t value)
{
  __asm__ __volatile__("outw %%ax, %%dx"
                       :
                       : "d"(portid), "a"(value));
}

uint16_t inportw(uint16_t portid)
{
  uint16_t ret;
  __asm__ __volatile__("inw %%dx, %%ax"
                       : "=a"(ret)
                       : "d"(portid));
  return ret;
}

uint32_t inportl(uint16_t portid)
{
  uint32_t rv;
  __asm__ __volatile__("inl %%dx, %%eax"
                       : "=a"(rv)
                       : "dN"(portid));
  return rv;
}

void outportl(uint16_t portid, uint32_t value)
{
  __asm__ __volatile__("outl %%eax, %%dx"
                       :
                       : "dN"(portid), "a"(value));
}

void inportsw(uint16_t portid, void *addr, size_t count)
{
  __asm__ __volatile__("rep insw"
                       : "+D"(addr), "+c"(count)
                       : "d"(portid)
                       : "memory");
}

void outportsw(uint16_t portid, const void *addr, size_t count)
{
  __asm__ __volatile__("rep outsw"
                       : "+S"(addr), "+c"(count)
                       : "d"(portid));
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

void cpuid(int code, uint32_t *a, uint32_t *d)
{
  asm volatile("cpuid"
               : "=a"(*a), "=d"(*d)
               : "a"(code)
               : "ecx", "ebx");
}