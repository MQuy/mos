#ifndef INCLUDE_CDEFS_H
#define INCLUDE_CDEFS_H

#define __unused __attribute__((__unused__))
#define __inline inline __attribute__((always_inline))

// NOTE: MQ 2020-06-15 Branch prediction and instruction rearrangement
#define likely(x) __builtin_expect((x), 1)
#define unlikely(x) __builtin_expect((x), 0)

#define DEBUG 1

#define __BEGIN_DECLS
#define __END_DECLS

#define container_of(ptr, type, member) ({ \
    const typeof( ((type *)0)->member ) \
    *__mptr = (ptr); \
    (type *)( (char *)__mptr - offsetof(type,member) ); })

#endif
