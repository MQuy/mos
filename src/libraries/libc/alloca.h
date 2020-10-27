#ifndef _LIBC_ALLOCA_H
#define _LIBC_ALLOCA_H

#ifdef __GNUC__
#define alloca(size) __builtin_alloca(size)
#endif

#endif
