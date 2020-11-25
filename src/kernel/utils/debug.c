#include "debug.h"

#include <cpu/rtc.h>
#include <devices/char/tty.h>
#include <include/ctype.h>
#include <proc/task.h>
#include <system/time.h>
#include <utils/string.h>

#define SERIAL_PORT_A 0x3f8

extern struct time current_time;

// LOG
static char log_buffer[1024];
static char log_body[1024];
static char *DEBUG_NAME[] = {
	[DEBUG_TRACE] = "TRACE",
	[DEBUG_INFO] = "INFO",
	[DEBUG_WARNING] = "WARNING",
	[DEBUG_ERROR] = "ERROR",
	[DEBUG_FATAL] = "FATAL",
};

static void debug_write(const char *str)
{
	for (char *ch = str; *ch; ++ch)
		serial_output(SERIAL_PORT_A, *ch);
}

static int debug_vsprintf(enum debug_level level, const char *fmt, va_list args)
{
	vsprintf(log_body, fmt, args);

	pid_t pid = current_process ? current_process->pid : 0;
	char *process_name = current_process ? current_process->name : "swapper";

	int out;
	// NOTE: MQ 2020-11-25
	// when executing kernel_main, rtc_irq_handler is not running due to disable interrupt
	// -> current_time is not setup, we have to manually get time from rtc
	if (current_time.year)
		out = sprintf(log_buffer, "[%s] %04d-%02d-%02d %02d:%02d:%02d %d %s -- %s",
					  DEBUG_NAME[level],
					  current_time.year, current_time.month, current_time.day,
					  current_time.hour, current_time.minute, current_time.second,
					  pid, process_name,
					  log_body);
	else
	{
		uint16_t year;
		uint8_t month, day, hour, minute, second;
		rtc_get_datetime(&year, &month, &day, &hour, &minute, &second);

		out = sprintf(log_buffer, "[%s] %04d-%02d-%02d %02d:%02d:%02d %d %s -- %s",
					  DEBUG_NAME[level],
					  year, month, day,
					  hour, minute, second,
					  pid, process_name,
					  log_body);
	}

	debug_write(log_buffer);
	return out;
}

int debug_printf(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	out = debug_vsprintf(level, fmt, args);

	va_end(args);
	return out;
}

int debug_println(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	out = debug_vsprintf(level, fmt, args);
	debug_write("\n");

	va_end(args);
	return out + 1;
}

void debug_init()
{
	serial_enable(SERIAL_PORT_A);
}

void __dbg(enum debug_level level, bool prefix, const char *file, int line, const char *func, ...)
{
	va_list args;
	va_start(args, func);
	const char *fmt = va_arg(args, const char *);

	char msg[1024];
	if (prefix)
		sprintf(msg, "%s on %s:%d %s\n", func, file, line, fmt);
	else
		sprintf(msg, "%s\n", fmt);
	debug_vsprintf(level, msg, args);

	va_end(args);
}
