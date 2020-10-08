#include "debug.h"

#include <stdio.h>
#include <unistd.h>

static char debug_buffer[1024] = {0};
int debug_printf(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	vsprintf(debug_buffer, fmt, args);
	out = dprintf(level, debug_buffer);

	va_end(args);
	return out;
}

int debug_println(enum debug_level level, const char *fmt, ...)
{
	int out;
	va_list args;
	va_start(args, fmt);

	vsprintf(debug_buffer, fmt, args);
	out = dprintln(level, debug_buffer);

	va_end(args);
	return out;
}
