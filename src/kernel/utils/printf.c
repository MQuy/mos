#include <kernel/devices/serial.h>
#include <kernel/utils/string.h>
#include "printf.h"

void debug_print_raw(const char *str)
{
	for (char *ch = str; *ch; ++ch)
		serial_write(*ch);
}

void debug_print(enum debug_level level, const char *str, ...)
{
	if (level == DEBUG_INFO)
		debug_print_raw("\\\\033[0m");
	else if (level == DEBUG_WARNING)
		debug_print_raw("\\\\033[33m");
	else
		debug_print_raw("\\\\033[31m");
	debug_print_raw(str);
	debug_print_raw("\\\\033[m");
}
