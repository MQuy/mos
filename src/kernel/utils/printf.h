#ifndef UTILS_PRINTF_H
#define UTILS_PRINTF_H

#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>

enum debug_level
{
  DEBUG_TRACE = 0,
  DEBUG_INFO = 1,
  DEBUG_WARNING = 2,
  DEBUG_ERROR = 3,
  DEBUG_FATAL = 4,
};

size_t vasprintf(char *buffer, const char *fmt, va_list args);
void debug_print(enum debug_level level, const char *str, ...);

#endif
