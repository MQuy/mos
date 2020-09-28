#include "tss.h"

#include <kernel/cpu/gdt.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

extern void tss_flush();

static struct tss_entry TSS;

void tss_set_stack(uint32_t kernelSS, uint32_t kernelESP)
{
	TSS.ss0 = kernelSS;
	TSS.esp0 = kernelESP;
}

void install_tss(uint32_t idx, uint32_t kernelSS, uint32_t kernelESP)
{
	DEBUG &&debug_println(DEBUG_INFO, "TSS: Initializing");

	//! install TSS descriptor
	uint32_t base = (uint32_t)&TSS;

	//! install descriptor
	gdt_set_descriptor(idx, base, base + sizeof(struct tss_entry),
					   I86_GDT_DESC_ACCESS | I86_GDT_DESC_EXEC_CODE | I86_GDT_DESC_DPL | I86_GDT_DESC_MEMORY,
					   0);

	//! initialize TSS
	memset((void *)&TSS, 0, sizeof(struct tss_entry));

	//! set stack and segments
	TSS.ss0 = kernelSS;
	TSS.esp0 = kernelESP;
	TSS.cs = 0x0b;
	TSS.ss = 0x13;
	TSS.es = 0x13;
	TSS.ds = 0x13;
	TSS.fs = 0x13;
	TSS.gs = 0x13;
	TSS.iomap = sizeof(struct tss_entry);

	tss_flush();

	DEBUG &&debug_println(DEBUG_INFO, "TSS: Done");
}
