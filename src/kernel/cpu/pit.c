#include "pit.h"

#include <include/list.h>
#include <kernel/cpu/rtc.h>
#include <kernel/memory/vmm.h>
#include <kernel/system/time.h>
#include <kernel/utils/printf.h>

#include "idt.h"
#include "pic.h"

#define PIT_REG_COUNTER 0x40
#define PIT_REG_COMMAND 0x43
#define PIT_TICKS_PER_SECOND 1000

extern volatile uint64_t boot_seconds, current_seconds;

volatile uint64_t jiffies = 0;

// boot_seconds is only set on the first tick
// current_seconds are updated each tick in rtc irq handler
// each half second in pit (why? one second with latency is already in the frame)
// -> if there is a latency due to overhead in other interrupts <-> jiffies < (current_seconds - boot_seconds) * 100
// -> jiffies = (current_seconds - boot_seconds) * 1000
int32_t pit_interrupt_handler(struct interrupt_registers *regs)
{
	if (!jiffies)
	{
		struct time boot_time;
		rtc_get_datetime(&boot_time.year, &boot_time.month, &boot_time.day,
						 &boot_time.hour, &boot_time.minute, &boot_time.second);
		set_boot_seconds(get_seconds(&boot_time));
	}

	jiffies++;
	// adjust ticks due to overhead and latency
	if (jiffies % (PIT_TICKS_PER_SECOND / 2) == 0 && jiffies < (current_seconds - boot_seconds) * 1000)
		jiffies = (current_seconds - boot_seconds) * 1000;

	irq_ack(regs->int_no);

	return IRQ_HANDLER_CONTINUE;
}

// NOTE: MQ 2020-07-17
// Prefer using rtc instead of pit for scheduling stuffs which don't require much accuracy
// (timer_list for example), because we allow overhead and latency in rtc irq
// pit should only be used for keeping track of time precision
void pit_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "[pit] - Initializing");

	int divisor = 1193181 / PIT_TICKS_PER_SECOND;

	outportb(PIT_REG_COMMAND, 0x34);
	outportb(PIT_REG_COUNTER, divisor & 0xff);
	outportb(PIT_REG_COUNTER, (divisor >> 8) & 0xff);

	register_interrupt_handler(IRQ0, pit_interrupt_handler);

	DEBUG &&debug_println(DEBUG_INFO, "[pit] - Done");
}
