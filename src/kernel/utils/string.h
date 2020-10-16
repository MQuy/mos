#ifndef UTILS_STRING_H
#define UTILS_STRING_H

#include <include/cdefs.h>
#include <include/types.h>
#include <stddef.h>
#include <stdint.h>

#define op_t unsigned long int
#define OPSIZ (sizeof(op_t))

void itoa(long long i, unsigned base, char *buf);
void itoa_s(long long i, unsigned base, char *buf);
int atoi(const char *buf);

int strcasecmp(const char *s1, const char *s2);
char *strcat(char *dest, const char *src);
char *strchr(const char *s, int c_in);
char *strchrnul(const char *s, int c_in);
int strcmp(const char *p1, const char *p2);
char *strcpy(char *dest, const char *src);
size_t strcspn(const char *str, const char *reject);
char *strdup(const char *s);
size_t strlen(const char *str);
int strncasecmp(const char *s1, const char *s2, int n);
char *strncat(char *s1, const char *s2, size_t n);
int strncmp(const char *s1, const char *s2, size_t n);
char *strncpy(char *s1, const char *s2, size_t n);
size_t strnlen(const char *str, size_t maxlen);
char *strpbrk(const char *s, const char *accept);
char *strrchr(const char *s, int c);
char *strsep(char **stringp, const char *delim);

// Not libc standard
char *strim(char *s);
char *strrstr(char *string, char *find);
char *strreplace(char *s, char old, char new);
int32_t striof(const char *s1, const char *s2);
int32_t strliof(const char *s1, const char *s2);
int32_t strlsplat(const char *s1, int32_t pos, char **sf, char **sl);

void *memcpy(void *dest, const void *src, size_t len);
void *memset(void *dest, char val, size_t len);
int memcmp(const void *vl, const void *vr, size_t n);

int count_array_of_pointers(void *arr);

#endif
