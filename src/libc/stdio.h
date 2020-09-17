#ifndef LIBC_STDIO_H
#define LIBC_STDIO_h

#include <include/ctype.h>
#include <stdarg.h>
#include <stddef.h>

enum debug_level
{
	DEBUG_TRACE = 0,
	DEBUG_INFO = 1,
	DEBUG_WARNING = 2,
	DEBUG_ERROR = 3,
	DEBUG_FATAL = 4,
};

size_t vsprintf(char *buffer, const char *fmt, va_list args);
size_t sprintf(char *buffer, const char *fmt, ...);

#endif
