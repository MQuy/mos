#ifndef _LIBC_DPRINT_H
#define _LIBC_DPRINT_H

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

#define assert(statement) ((statement) ? (void)0 : (void)({ debug_println(DEBUG_ERROR, "%s:%d %s", __FILE__, __LINE__, __FUNCTION__); __asm__ __volatile("int $0x01"); }))

#endif
