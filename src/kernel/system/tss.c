#include <kernel/include/string.h>
#include <kernel/cpu/gdt.h>
#include "tss.h"

extern void tss_flush();

static tss_entry TSS;

void install_tsr(uint16_t sel);

void tss_set_stack(uint16_t kernelSS, uint16_t kernelESP)
{

	TSS.ss0 = kernelSS;
	TSS.esp0 = kernelESP;
}

void install_tss(uint32_t idx, uint16_t kernelSS, uint16_t kernelESP)
{

	//! install TSS descriptor
	uint32_t base = (uint32_t)&TSS;

	//! install descriptor
	gdt_set_descriptor(idx, base, base + sizeof(tss_entry),
										 I86_GDT_DESC_ACCESS | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_DPL | I86_GDT_DESC_MEMORY,
										 0);

	//! initialize TSS
	memset((void *)&TSS, 0, sizeof(tss_entry));

	//! set stack and segments
	TSS.ss0 = kernelSS;
	TSS.esp0 = kernelESP;
	TSS.cs = 0x0b;
	TSS.ss = 0x13;
	TSS.es = 0x13;
	TSS.ds = 0x13;
	TSS.fs = 0x13;
	TSS.gs = 0x13;

	tss_flush();
}
