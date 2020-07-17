#include "rtc.h"

#include <include/ctype.h>
#include <kernel/cpu/hal.h>
#include <kernel/cpu/idt.h>
#include <kernel/cpu/pic.h>
#include <kernel/system/time.h>
#include <kernel/utils/printf.h>

#define CMOS_ADDRESS 0x70
#define CMOS_DATA 0x71
#define RTC_TICKS_PER_SECOND 32

static volatile uint64_t current_ticks = 0;

uint8_t rtc_get_update_flag()
{
	outportb(CMOS_ADDRESS, 0x0A);
	return inportb(CMOS_DATA) & 0x80;
}

uint8_t rtc_get_register(uint32_t reg)
{
	outportb(CMOS_ADDRESS, reg);
	return inportb(CMOS_DATA);
}

void rtc_get_datetime(uint16_t *year, uint8_t *month, uint8_t *day,
					  uint8_t *hour, uint8_t *minute, uint8_t *second)
{
	uint8_t century;
	uint8_t last_second, last_minute, last_hour, last_day, last_month, last_year, last_century;

	while (rtc_get_update_flag())
		;
	*second = rtc_get_register(0x00);
	*minute = rtc_get_register(0x02);
	*hour = rtc_get_register(0x04);
	*day = rtc_get_register(0x07);
	*month = rtc_get_register(0x08);
	*year = rtc_get_register(0x09);
	century = rtc_get_register(0x32);

	do
	{
		last_second = *second;
		last_minute = *minute;
		last_hour = *hour;
		last_day = *day;
		last_month = *month;
		last_year = *year;
		last_century = century;

		while (rtc_get_update_flag())
			;
		*second = rtc_get_register(0x00);
		*minute = rtc_get_register(0x02);
		*hour = rtc_get_register(0x04);
		*day = rtc_get_register(0x07);
		*month = rtc_get_register(0x08);
		*year = rtc_get_register(0x09);
		century = rtc_get_register(0x32);

	} while ((last_second != *second) || (last_minute != *minute) || (last_hour != *hour) ||
			 (last_day != *day) || (last_month != *month) || (last_year != *year) ||
			 (last_century != century));

	uint8_t registerB = rtc_get_register(0x0B);
	// Convert BCD to binary values if necessary
	if (!(registerB & 0x04))
	{
		*second = (*second & 0x0F) + ((*second / 16) * 10);
		*minute = (*minute & 0x0F) + ((*minute / 16) * 10);
		*hour = ((*hour & 0x0F) + (((*hour & 0x70) / 16) * 10)) | (*hour & 0x80);
		*day = (*day & 0x0F) + ((*day / 16) * 10);
		*month = (*month & 0x0F) + ((*month / 16) * 10);
		*year = (*year & 0x0F) + ((*year / 16) * 10);
		century = (century & 0x0F) + ((century / 16) * 10);
	}
	*year = century * 100 + *year;

	// Convert 12 hour clock to 24 hour clock if necessary
	if (!(registerB & 0x02) && (*hour & 0x80))
	{
		*hour = ((*hour & 0x7F) + 12) % 24;
	}
}

int32_t rtc_irq_handler(struct interrupt_registers *regs)
{
	current_ticks++;

	if (current_ticks % (RTC_TICKS_PER_SECOND / 2) == 0)
	{
		uint16_t year;
		uint8_t second, minute, hour, day, month;

		rtc_get_datetime(&year, &month, &day, &hour, &minute, &second);
		set_current_time(year, month, day, hour, minute, second);
	}

	outportb(0x70, 0x0C);  // select register C
	inportb(0x71);		   // just throw away contents

	irq_ack(regs->int_no);
	return IRQ_HANDLER_CONTINUE;
}

void rtc_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "[rtc] - Initializing");

	uint8_t rate = log2(32768 / RTC_TICKS_PER_SECOND) + 1;
	disable_interrupts();
	outportb(0x70, 0x8A);		  // select Status Register A, and disable NMI (by setting the 0x80 bit)
	outportb(0x71, 0x20 | rate);  // write to CMOS/RTC RAM and interrupt rate

	// turn on irq8
	outportb(0x70, 0x8B);		  // select register B, and disable NMI
	char prev = inportb(0x71);	  // read the current value of register B
	outportb(0x70, 0x8B);		  // set the index again (a read will reset the index to register D)
	outportb(0x71, prev | 0x40);  // write the previous value ORed with 0x40. This turns on bit 6 of register B
	register_interrupt_handler(IRQ8, rtc_irq_handler);
	pic_clear_mask(IRQ8);

	DEBUG &&debug_println(DEBUG_INFO, "[rtc] - Initializing");
}
