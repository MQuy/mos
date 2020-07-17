#include "pit.h"

#include <include/list.h>
#include <kernel/memory/vmm.h>
#include <kernel/utils/printf.h>

#include "idt.h"
#include "pic.h"

#define PIT_REG_COUNTER 0x40
#define PIT_REG_COMMAND 0x43

volatile unsigned long jiffies = 0;

int32_t pit_interrupt_handler(struct interrupt_registers *regs)
{
	jiffies++;
	irq_ack(regs->int_no);

	// adjust ticks due to overhead and latency

	return IRQ_HANDLER_CONTINUE;
}

uint32_t get_milliseconds_from_boot()
{
	return jiffies * (1000 / TICKS_PER_SECOND);
}

void pit_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "[pit] - Initializing");

	int divisor = 1193181 / TICKS_PER_SECOND;

	outportb(PIT_REG_COMMAND, 0x34);
	outportb(PIT_REG_COUNTER, divisor & 0xff);
	outportb(PIT_REG_COUNTER, (divisor >> 8) & 0xff);

	register_interrupt_handler(IRQ0, pit_interrupt_handler);

	DEBUG &&debug_println(DEBUG_INFO, "[pit] - Done");
}
