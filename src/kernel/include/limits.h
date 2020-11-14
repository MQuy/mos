#ifndef INCLUDE_LIMITS_H
#define INCLUDE_LIMITS_H

#include <stdint.h>

#define PATH_MAX 4096
#if !defined MAXPATHLEN && defined PATH_MAX
#define MAXPATHLEN PATH_MAX
#endif

#define INT_MAX ((int)(~0U >> 1))
#define INT_MIN (-INT_MAX - 1)
#define UINT_MAX (~0U)
#define LONG_MAX ((long)(~0UL >> 1))
#define LONG_MIN (-LONG_MAX - 1)
#define ULONG_MAX (~0UL)
#define USHRT_MAX 65535
#define SHRT_MAX 32767
#define ULLONG_MAX (~0ULL)

#endif
