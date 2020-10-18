#ifndef LIBC_ALLOCA_H
#define LIBC_ALLOCA_H

#ifdef __GNUC__
#define alloca(size) __builtin_alloca(size)
#endif

#endif
