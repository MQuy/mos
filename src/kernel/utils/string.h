#ifndef LIBC_STRING_H
#define LIBC_STRING_H

#include <stddef.h>

void itoa(unsigned i, unsigned base, char *buf);
void itoa_s(long long i, unsigned base, char *buf);

char *strcpy(char *s1, const char *s2);
size_t strlen(const char *str);
int strcmp(const char *str1, const char *str2);
char *strdup(const char *src);

void *memcpy(void *dest, const void *src, size_t count);
void *memset(void *dest, char val, size_t count);
unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count);

char *strchr(char *str, int character);

#endif
