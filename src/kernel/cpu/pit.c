#include "pit.h"

#include <include/list.h>
#include <kernel/memory/vmm.h>
#include <kernel/utils/printf.h>

#include "idt.h"
#include "pic.h"

#define PIT_REG_COUNTER 0x40
#define PIT_REG_COMMAND 0x43

volatile uint32_t pit_ticks = 0;

int32_t pit_interrupt_handler(struct interrupt_registers *regs)
{
	pit_ticks++;

	return IRQ_HANDLER_CONTINUE;
}

uint32_t get_milliseconds_from_boot()
{
	return pit_ticks;
}

void pit_init()
{
	debug_println(DEBUG_INFO, "[pit] - Initializing");

	int divisor = 1193181 / 19886;

	outportb(PIT_REG_COMMAND, 0x34);
	outportb(PIT_REG_COUNTER, divisor & 0xff);
	outportb(PIT_REG_COUNTER, (divisor >> 8) & 0xff);

	register_interrupt_handler(IRQ0, pit_interrupt_handler);

	debug_println(DEBUG_INFO, "[pit] - Done");
}
