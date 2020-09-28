#ifndef INCLUDE_CTYPE_H
#define INCLUDE_CTYPE_H

#define USHRT_MAX 65535
#define log2(X) ((unsigned)(8 * sizeof(unsigned long long) - __builtin_clzll((X)) - 1))

typedef unsigned int __kernel_dev_t;
typedef __kernel_dev_t dev_t;

typedef unsigned short umode_t;
typedef long long loff_t;
typedef long off_t;
typedef unsigned long long ino_t;
typedef unsigned short mode_t;
typedef long ssize_t;
typedef unsigned long sector_t;
typedef int pid_t;
typedef int tid_t;
typedef int uid_t;
typedef int gid_t;
typedef int sid_t;
typedef unsigned long sigset_t;
typedef int idtype_t;
typedef int id_t;
typedef long long time_t;
typedef int clockid_t;
typedef long long clock_t;

#define isspace(c) ((c) == ' ' || ((c) >= '\t' && (c) <= '\r'))
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define islower(c) ((c) >= 'a' && (c) <= 'z')
#define isalpha(c) (isupper(c) || islower(c))
#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isxdigit(c) (isdigit(c) || ((c) >= 'A' && (c) <= 'F') || ((c) >= 'a' && (c) <= 'f'))
#define isprint(c) ((c) >= ' ' && (c) <= '~')
#define toupper(c) ((c)-0x20 * (((c) >= 'a') && ((c) <= 'z')))
#define tolower(c) ((c) + 0x20 * (((c) >= 'A') && ((c) <= 'Z')))
#define isascii(c) ((unsigned)(c) <= 0x7F)
#define toascii(c) ((unsigned)(c)&0x7F)

/*
 * min()/max() macros that also do
 * strict type-checking.. See the
 * "unnecessary" pointer comparison.
 */
#define min(x, y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x < _y ? _x : _y; })

#define max(x, y) ({ \
	typeof(x) _x = (x);	\
	typeof(y) _y = (y);	\
	(void) (&_x == &_y);		\
	_x > _y ? _x : _y; })

/*
 * ..and if you can't take the strict
 * types, you can specify one yourself.
 *
 * Or not use min/max at all, of course.
 */
#define min_t(type, x, y) \
	({ type __x = (x); type __y = (y); __x < __y ? __x: __y; })
#define max_t(type, x, y) \
	({ type __x = (x); type __y = (y); __x > __y ? __x: __y; })

struct timespec
{
	time_t tv_sec;
	long tv_nsec;
};
#define CLOCK_REALTIME 0
#define CLOCK_MONOTONIC 1
#define CLOCK_PROCESS_CPUTIME_ID 2
#define CLOCK_THREAD_CPUTIME_ID 3
#define CLOCK_MONOTONIC_RAW 4
#define CLOCK_REALTIME_COARSE 5
#define CLOCK_MONOTONIC_COARSE 6
#define CLOCK_BOOTTIME 7
#define CLOCK_REALTIME_ALARM 8
#define CLOCK_BOOTTIME_ALARM 9

#endif
