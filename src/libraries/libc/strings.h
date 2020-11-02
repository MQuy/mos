#ifndef _LIBC_STRINGS_H
#define _LIBC_STRINGS_H 1

#include <sys/types.h>

#ifndef __size_t_defined
#define __size_t_defined
#define __need_size_t
#include <stddef.h>
#endif

#ifndef __locale_t_defined
#define __locale_t_defined
typedef __locale_t locale_t;
#endif

int ffs(int i);
int strcasecmp(const char *s1, const char *s2);
int strcasecmp_l(const char *s1, const char *s2, locale_t locale);
int strncasecmp(const char *s1, const char *s2, int n);
int strncasecmp_l(const char *s1, const char *s2, size_t n, locale_t locale);
void bcopy(const void *src, void *dest, size_t n);

#endif
