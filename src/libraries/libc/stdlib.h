#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H 1

#include <alloca.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define _PATH_DEV "/dev/"
#define _PATH_TMP "/tmp/"
#define SPECNAMELEN 255 /* max length of devicename */

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 32767

#define MB_CUR_MAX 1

#ifndef NULL
#define __need_NULL
#include <stddef.h>
#endif

#ifndef __size_t_defined
#define __size_t_defined
#define __need_size_t
#include <stddef.h>
#endif

#ifndef __wchar_t_defined
#define __wchar_t_defined
#define __need_wchar_t
#include <stddef.h>
#endif

typedef struct
{
	int quot;
	int rem;
} div_t;

typedef struct
{
	long quot;
	long rem;
} ldiv_t;

typedef struct
{
	long long quot;
	long long rem;
} lldiv_t;

void *malloc(size_t size);
void *calloc(size_t n, size_t size);
void free(void *ptr);
void *realloc(void *ptr, size_t size);
void *reallocarray(void *ptr, size_t nmemb, size_t size);
int posix_openpt(int flags);
char *ptsname(int fd);
void abort();
void exit(int status);
int atexit(void (*)(void));
int atoi(const char *);
char *getenv(const char *);
double strtod(const char *restrict nptr, char **restrict endptr);

int rand();
int rand_r(unsigned *);
void srand(unsigned seed);

#endif
