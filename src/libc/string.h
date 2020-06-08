#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <include/cdefs.h>
#include <stddef.h>
#include <stdint.h>

void itoa(unsigned i, unsigned base, char *buf);
void itoa_s(long long i, unsigned base, char *buf);
int atoi(const char *buf);
char *strcpy(char *s1, const char *s2);
char *strncpy(char *dest, const char *src, size_t count);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
char *strdup(const char *src);
char *strchr(const char *s, int c);
char *strrchr(const char *s, int c);
int strcasecmp(const char *s1, const char *s2);
int strncasecmp(const char *s1, const char *s2, int n);
char *strcat(char *dest, const char *src);
char *strncat(char *dest, const char *src, size_t count);
char *strim(char *s);
char *strpbrk(const char *cs, const char *ct);
char *strpbrk(const char *cs, const char *ct);
char *strsep(char **s, const char *ct);
char *strreplace(char *s, char old, char new);

int32_t striof(const char *s1, const char *s2);
int32_t strliof(const char *s1, const char *s2);
int32_t strlsplat(const char *s1, uint32_t pos, char **sf, char **sl);

static __inline void *memcpy(void *restrict dest, const void *restrict src, size_t n)
{
	asm volatile("cld; rep movsb"
				 : "=c"((int){0})
				 : "D"(dest), "S"(src), "c"(n)
				 : "flags", "memory");
	return dest;
}

static __inline void *memset(void *dest, int c, size_t n)
{
	asm volatile("cld; rep stosb"
				 : "=c"((int){0})
				 : "D"(dest), "a"(c), "c"(n)
				 : "flags", "memory");
	return dest;
}

int memcmp(const void *vl, const void *vr, size_t n);

#endif
