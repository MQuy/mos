#include "hal.h"

char *i86_cpu_get_vender()
{
	static char vender[32] = {0};
	__asm__ __volatile__(
		"mov $0, %%eax;				\n"
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
