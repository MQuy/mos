#ifndef _LIBC_ALLOCA_H
#define _LIBC_ALLOCA_H 1

#include <stddef.h>

void* alloca(size_t size);
#define alloca(size) __builtin_alloca(size)

#endif
