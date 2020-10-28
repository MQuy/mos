#ifndef _LIBC_SYS_CDEFS_H
#define _LIBC_SYS_CDEFS_H 1

#define __unused __attribute__((__unused__))
#define __inline inline __attribute__((always_inline))

// NOTE: MQ 2020-06-15 Branch prediction and instruction rearrangement
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define DEBUG 1

#define __BEGIN_DECLS
#define __END_DECLS

#endif
