#ifndef LIBC_STDIO_H
#define LIBC_STDIO_H

#include <stdarg.h>
#include <stddef.h>
#include <sys/reent.h>

#define EOF (-1)

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

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
int debug_printf(enum debug_level level, const char *fmt, ...);
int debug_println(enum debug_level level, const char *fmt, ...);

#endif
