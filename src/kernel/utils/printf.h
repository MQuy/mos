#ifndef UTILS_PRINTF_H
#define UTILS_PRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

enum debug_level
{
	DEBUG_TRACE = 0,
	DEBUG_INFO = 1,
	DEBUG_WARNING = 2,
	DEBUG_ERROR = 3,
	DEBUG_FATAL = 4,
};

size_t vsprintf(char *buffer, const char *fmt, va_list args);
void debug_printf(enum debug_level level, const char *fmt, ...);
void debug_println(enum debug_level level, const char *fmt, ...);

#endif
