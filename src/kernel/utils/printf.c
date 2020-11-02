#include "printf.h"

#include <devices/char/tty.h>
#include <include/ctype.h>
#include <utils/string.h>

#define SERIAL_PORT_A 0x3f8

// LOG
static char log_buffer[1024];
static char *tag_opening[] = {
	[DEBUG_TRACE] = "\\\\033[38;5;14m",
	[DEBUG_WARNING] = "\\\\033[38;5;11m",
	[DEBUG_ERROR] = "\\\\033[38;5;9m",
	[DEBUG_FATAL] = "\\\\033[48;5;9m",
};
static char tag_closing[] = "\\\\033[m";

static void debug_write(const char *str)
{
	for (char *ch = str; *ch; ++ch)
		serial_output(SERIAL_PORT_A, *ch);
}

static int debug_vsprintf(enum debug_level level, const char *fmt, va_list args)
{
	int out = vsprintf(log_buffer, fmt, args);
	if (level != DEBUG_INFO)
	{
		debug_write(tag_opening[level]);
		debug_write(log_buffer);
		debug_write(tag_closing);
	}
	else
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

void __assert_debug(char *file, int line, char *func, ...)
{
	va_list args;
	va_start(args, func);
	char *message = va_arg(args, char *);
	va_end(args);

	if (message)
		debug_println(DEBUG_INFO, "%s", message);
	debug_println(DEBUG_ERROR, "%s:%d %s", file, line, func);
}
