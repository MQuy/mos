#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H 1

#include <alloca.h>

#define _PATH_DEV "/dev/"
#define _PATH_TMP "/tmp/"
#define SPECNAMELEN 255 /* max length of devicename */

#define EXIT_FAILURE 1
#define EXIT_SUCCESS 0
#define RAND_MAX 32767

#define MB_CUR_MAX 1

#define FILENAME_CHARSET "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789._-"

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

void _Exit(int status);
void abort();
int abs(int num);
int atexit(void (*func)(void));
double atof(const char *str);
int atoi(const char *str);
long atol(const char *str);
long long atoll(const char *str);
void *bsearch(const void *, const void *, size_t, size_t,
			  int (*)(const void *, const void *));
void *calloc(size_t n, size_t size);
div_t div(int, int);
void exit(int status);
void free(void *ptr);
char *getenv(const char *);
int grantpt(int fd);
long labs(long);
ldiv_t ldiv(long numer, long denom);
long long int llabs(long long int val);
lldiv_t lldiv(long long numer, long long denom);
void *malloc(size_t size);
int mblen(const char *s, size_t n);
int posix_openpt(int flags);
char *ptsname(int fd);
int putenv(char *string);
void qsort(void *base_ptr,
		   size_t num_elements,
		   size_t element_size,
		   int (*compare)(const void *, const void *));
void qsort_r(void *base_ptr,
			 size_t num_elements,
			 size_t element_size,
			 int (*compare)(const void *, const void *, void *),
			 void *arg);
int rand();
int rand_r(unsigned *);
void srand(unsigned seed);
void *realloc(void *ptr, size_t size);
double strtod(const char *restrict, char **restrict);
int setenv(const char *envname, const char *envval, int overwrite);
float strtof(const char *restrict, char **restrict);
long strtol(const char *restrict, char **restrict, int);
long double strtold(const char *restrict, char **restrict);
long long strtoll(const char *restrict, char **restrict, int);
unsigned long strtoul(const char *restrict, char **restrict, int);
unsigned long long strtoull(const char *restrict, char **restrict, int);
int unlockpt(int);
int unsetenv(const char *name);
char *mktemp(char *template);

void *reallocarray(void *ptr, size_t nmemb, size_t size);
void _clear_on_exit();

#endif
