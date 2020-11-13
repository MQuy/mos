#ifndef UTILS_PRINTF_H
#define UTILS_PRINTF_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <utils/vsprintf.h>

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
void debug_init();
void __assert_debug(char *file, int line, char *func, ...);

#define assert(statement, ...) ((statement)   \
									? (void)0 \
									: (void)({ __assert_debug(__FILE__, __LINE__, __func__, ##__VA_ARGS__); __asm__ __volatile("int $0x01"); }))
#define assert_not_reached(...) ({ __assert_debug(__FILE__, __LINE__, __func__, ##__VA_ARGS__); __asm__ __volatile__("ud2"); })
#define assert_not_implemented() debug_println(DEBUG_INFO, "%s:%d %s is not implemented", __FILE__, __LINE__, __func__)

#endif
