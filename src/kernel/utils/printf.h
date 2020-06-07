#ifndef UTILS_PRINTF_H
#define UTILS_PRINTF_H

#include <stdarg.h>
#include <stdint.h>

enum debug_level
{
  DEBUG_INFO = 0,
  DEBUG_WARNING = 1,
  DEBUG_ERROR = 2,
};

void debug_print(enum debug_level level, const char *str, ...);

#endif
