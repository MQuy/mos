#ifndef LIBC_DEBUG_H
#define LIBC_DEBUG_H

#include <stdarg.h>

enum debug_level
{
	DEBUG_TRACE = 0,
	DEBUG_INFO = 1,
	DEBUG_WARNING = 2,
	DEBUG_ERROR = 3,
	DEBUG_FATAL = 4,
};

int debug_printf(enum debug_level level, const char *fmt, ...);
int debug_println(enum debug_level level, const char *fmt, ...);

#endif
