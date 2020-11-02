#ifndef _LIBC_STRING_H
#define _LIBC_STRING_H 1

#include <sys/types.h>

#ifndef __size_t_defined
#define __size_t_defined
#define __need_size_t
#include <stddef.h>
#endif

#ifndef NULL
#define __need_NULL
#include <stddef.h>
#endif

#ifndef __locale_t_defined
#define __locale_t_defined
typedef __locale_t locale_t;
#endif

#define op_t unsigned long int
#define OPSIZ (sizeof(op_t))

void itoa(long long i, unsigned base, char *buf);
void itoa_s(long long i, unsigned base, char *buf);

void *memccpy(void *dest_ptr, const void *src_ptr, int c, size_t n);
void *memchr(const void *s, int c, size_t size);
int memcmp(const void *vl, const void *vr, size_t n);
void *memcpy(void *dest, const void *src, size_t len);
void *memmove(void *dest_ptr, const void *src_ptr, size_t n);
void *memset(void *dest, char val, size_t len);

char *stpcpy(char *restrict s1, const char *restrict s2);
char *stpncpy(char *restrict s1, const char *restrict s2, size_t n);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c_in);
int strcmp(const char *p1, const char *p2);
int strcoll(const char *s1, const char *s2);
int strcoll_l(const char *s1, const char *s2, locale_t locale);
char *strcpy(char *dest, const char *src);
size_t strcspn(const char *str, const char *reject);
char *strdup(const char *s);
char *strerror(int errnum);
char *strerror_l(int errnum, locale_t locale);
int strerror_r(int errnum, char *strerrbuf, size_t buflen);
size_t strlen(const char *str);
char *strncat(char *s1, const char *s2, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *s1, const char *s2, size_t n);
char *strndup(const char *s, size_t size);
size_t strnlen(const char *str, size_t maxlen);
char *strpbrk(const char *s, const char *accept);
char *strrchr(const char *s, int c);
char *strsignal(int signum);
size_t strspn(const char *s1, const char *s2);
char *strstr(const char *s1, const char *s2);
char *strtok(char *restrict s, const char *restrict sep);
char *strtok_r(char *restrict s, const char *restrict sep, char **restrict state);
size_t strxfrm(char *restrict s1, const char *restrict s2, size_t n);
size_t strxfrm_l(char *restrict s1, const char *restrict s2, size_t n, locale_t locale);

// Not libc standard
char *strchrnul(const char *s, int c_in);
char *strsep(char **stringp, const char *delim);
void *mempcpy(void *__restrict __dest, const void *__restrict __src, size_t __n);

char *strim(char *s);
char *strrstr(char *string, char *find);
char *strreplace(char *s, char old, char new);
int striof(const char *s1, const char *s2);
int strliof(const char *s1, const char *s2);
int strlsplat(const char *s1, int pos, char **sf, char **sl);
char *skip_spaces(const char *str);

#endif
