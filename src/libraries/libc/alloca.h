#ifndef _LIBC_ALLOCA_H
#define _LIBC_ALLOCA_H 1

void* alloca(size_t);
#define alloca(size) __builtin_alloca(size)

#endif
