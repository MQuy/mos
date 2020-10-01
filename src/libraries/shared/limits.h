#ifndef SHARED_LIMITS_H
#define SHARED_LIMITS_H

#include <stdint.h>

#define INT_MAX ((int)(~0U >> 1))
#define INT_MIN (-INT_MAX - 1)
#define UINT_MAX (~0U)
#define LONG_MAX ((long)(~0UL >> 1))
#define LONG_MIN (-LONG_MAX - 1)
#define ULONG_MAX (~0UL)
#define USHRT_MAX 65535

#endif
