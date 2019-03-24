/************************************************************************
*                                                                       *
*	limits.h - type limits												*
*                                                                       *
*   Copyright (c) BrokenThorn Entertainment Co. All Rights Reserved.	*
*                                                                       *
************************************************************************/

#ifndef LIMITS_H
#define LIMITS_H

#include "float.h"

/*
 * Sizes of integral types
 */
#define CHAR_BIT 8			 /* max # of bits in a "char" */
#define SCHAR_MIN (-128) /* min value of a "signed char" */
#define SCHAR_MAX 127		 /* max value of a "signed char" */
#define UCHAR_MAX 255		 /* max value of an "unsigned char" */

#define MB_LEN_MAX 5

#define CHAR_MIN SCHAR_MIN /* min value of a "char" */
#define CHAR_MAX SCHAR_MAX /* max value of a "char" */

#define SHRT_MIN (-32768)						 /* min value of a "short int" */
#define SHRT_MAX 32767							 /* max value of a "short int" */
#define USHRT_MAX 65535							 /* max value of "unsigned short int" */
#define INT_MIN (-2147483647 - 1)		 /* min value of an "int" */
#define INT_MAX 2147483647					 /* max value of an "int" */
#define UINT_MAX 4294967295U				 /* max value of an "unsigned int" */
#define LONG_MIN (-2147483647L - 1L) /* min value of a "long int" */
#define LONG_MAX 2147483647L				 /* max value of a "long int" */
#define ULONG_MAX 4294967295UL			 /* max value of "unsigned long int" */

#if defined(__EXTENSIONS__) || __STDC__ - 0 == 0 || \
		defined(_POSIX_C_SOURCE) || defined(_XOPEN_SOURCE)

#define SSIZE_MAX INT_MAX /* max value of an "ssize_t" */

/*
 * ARG_MAX is calculated as follows:
 * NCARGS - ((sizeof(struct arg_hunk *))*(0x10000/(sizeof)(struct arg_hunk)))
 */
#define ARG_MAX 1048320 /* max length of arguments to exec */

#define LINK_MAX 32767 /* max # of links to a single file */

#ifndef MAX_CANON
#define MAX_CANON 256 /* max bytes in line for canonical processing */
#endif

#ifndef MAX_INPUT
#define MAX_INPUT 512 /* max size of a char input buffer */
#endif

#define NGROUPS_MAX 16 /* max number of groups for a user */

#ifndef PATH_MAX
#define PATH_MAX 1024 /* max # of characters in a path name */
#endif

#define PIPE_BUF 5120 /* max # bytes atomic in write to a pipe */

#ifndef TMP_MAX
#define TMP_MAX 17576 /* 26 * 26 * 26 */
#endif

/*
 * POSIX conformant definitions - An implementation may define
 * other symbols which reflect the actual implementation. Alternate
 * definitions may not be as restrictive as the POSIX definitions.
 */
#define _POSIX_AIO_LISTIO_MAX 2
#define _POSIX_AIO_MAX 1
#define _POSIX_ARG_MAX 4096
#define _POSIX_CHILD_MAX 6
#define _POSIX_DELAYTIMER_MAX 32
#define _POSIX_LINK_MAX 8
#define _POSIX_MAX_CANON 255
#define _POSIX_MAX_INPUT 255
#define _POSIX_MQ_OPEN_MAX 8
#define _POSIX_MQ_PRIO_MAX 32
#define _POSIX_NAME_MAX 14
#define _POSIX_NGROUPS_MAX 0
#define _POSIX_OPEN_MAX 16
#define _POSIX_PATH_MAX 255
#define _POSIX_PIPE_BUF 512
#define _POSIX_RTSIG_MAX 8
#define _POSIX_SEM_NSEMS_MAX 256
#define _POSIX_SEM_VALUE_MAX 32767
#define _POSIX_SIGQUEUE_MAX 32
#define _POSIX_SSIZE_MAX 32767
#define _POSIX_STREAM_MAX 8
#define _POSIX_TIMER_MAX 32
#define _POSIX_TZNAME_MAX 3
/* POSIX.1c conformant */
#define _POSIX_LOGIN_NAME_MAX 9
#define _POSIX_THREAD_DESTRUCTOR_INTERATION 4
#define _POSIX_THREAD_KEYS_MAX 128
#define _POSIX_THREAD_THREADS_MAX 64
#define _POSIX_TTY_NAME_MAX 9

/*
 * POSIX.2 and XPG4-XSH4 conformant definitions
 */

#define _POSIX2_BC_BASE_MAX 99
#define _POSIX2_BC_DIM_MAX 2048
#define _POSIX2_BC_SCALE_MAX 99
#define _POSIX2_BC_STRING_MAX 1000
#define _POSIX2_COLL_WEIGHTS_MAX 2
#define _POSIX2_EXPR_NEST_MAX 32
#define _POSIX2_LINE_MAX 2048
#define _POSIX2_RE_DUP_MAX 255

#define BC_BASE_MAX _POSIX2_BC_BASE_MAX
#define BC_DIM_MAX _POSIX2_BC_DIM_MAX
#define BC_SCALE_MAX _POSIX2_BC_SCALE_MAX
#define BC_STRING_MAX _POSIX2_BC_STRING_MAX
#define COLL_WEIGHTS_MAX _POSIX2_COLL_WEIGHTS_MAX
#define EXPR_NEST_MAX _POSIX2_EXPR_NEST_MAX
#define LINE_MAX _POSIX2_LINE_MAX
#define RE_DUP_MAX _POSIX2_RE_DUP_MAX

#endif /* defined(__EXTENSIONS__) ||  __STDC__ - 0 == 0 || ... */

#if defined(__EXTENSIONS__) ||                          \
		(__STDC__ - 0 == 0 && !defined(_POSIX_C_SOURCE)) || \
		defined(_XOPEN_SOURCE)

#define PASS_MAX 8 /* max # of characters in a password */

#define CHARCLASS_NAME_MAX 14 /* max # bytes in a char class name */
#define NL_ARGMAX 9						/* max value of "digit" in calls to the	*/
															/* NLS printf() and scanf() */
#define NL_LANGMAX 14					/* max # of bytes in a LANG name */
#define NL_MSGMAX 32767				/* max message number */
#define NL_NMAX 1							/* max # bytes in N-to-1 mapping characters */
#define NL_SETMAX 255					/* max set number */
#define NL_TEXTMAX 2048				/* max set number */
#define NZERO 20							/* default process priority */

#define WORD_BIT 32 /* # of bits in a "word" or "int" */
#define LONG_BIT 32 /* # of bits in a "long" */

#define DBL_DIG 15 /* digits of precision of a "double" */
//#define	DBL_MAX		1.7976931348623157E+308	/* max decimal value of a */
/* "double" */
//#define	DBL_MIN		2.2250738585072014E-308	/* min decimal value of a */
/* "double" */
#define FLT_DIG 6 /* digits of precision of a "float" */
//#define	FLT_MAX		3.402823466E+38F  /* max decimal value of a "float" */
//#define	FLT_MIN		1.175494351E-38F  /* min decimal value of a "float" */

#endif /* defined(__EXTENSIONS__) || (__STDC__ - 0 == 0 && ... */

#if defined(__EXTENSIONS__) || (__STDC__ - 0 == 0 && \
																!defined(_POSIX_C_SOURCE) && !defined(_XOPEN_SOURCE))

#define FCHR_MAX 1048576 /* max size of a file in bytes */
#define PID_MAX 30000		 /* max value for a process ID */

/*
 * POSIX 1003.1a, section 2.9.5, table 2-5 contains [NAME_MAX] and the
 * related text states:
 *
 * A definition of one of the values from Table 2-5 shall be omitted from the
 * <limits.h> on specific implementations where the corresponding value is
 * equal to or greater than the stated minimum, but where the value can vary
 * depending on the file to which it is applied. The actual value supported for
 * a specific pathname shall be provided by the pathconf() (5.7.1) function.
 *
 * This is clear that any machine supporting multiple file system types
 * and/or a network can not include this define, regardless of protection
 * by the _POSIX_SOURCE and _POSIX_C_SOURCE flags.
 *
 * #define	NAME_MAX	14
 */

#define CHILD_MAX 25 /* max # of processes per user id */
#ifndef OPEN_MAX
#define OPEN_MAX 64 /* max # of files a process can have open */
#endif

#define PIPE_MAX 5120 /* max # bytes written to a pipe in a write */

#define STD_BLK 1024			 /* # bytes in a physical I/O block */
#define UID_MAX 60002			 /* max value for a user or group ID */
#define USI_MAX 4294967295 /* max decimal value of an "unsigned" */
#define SYSPID_MAX 1			 /* max pid of system processes */

#ifndef SYS_NMLN		 /* also defined in sys/utsname.h */
#define SYS_NMLN 257 /* 4.0 size of utsname elements */
#endif

#ifndef CLK_TCK
extern long _sysconf(int);	/* System Private interface to sysconf() */
#define CLK_TCK _sysconf(3) /* 3 is _SC_CLK_TCK */
#endif

#define LOGNAME_MAX 8		/* max # of characters in a login name */
#define TTYNAME_MAX 128 /* max # of characters in a tty name */

/*
 * Size constants for long long
 */
#define LLONG_MIN (-9223372036854775807LL - 1LL)
#define LLONG_MAX 9223372036854775807LL
#define ULLONG_MAX 18446744073709551615ULL

#endif /* defined(__EXTENSIONS__) || (__STDC__ - 0 == 0 && ... */

/*
 * POSIX.1c Note:
 * PTHREAD_STACK_MIN is also defined in <pthread.h>.
 */
#if defined(__EXTENSIONS__) || (_POSIX_C_SOURCE >= 199506L)
#include <sys/unistd.h>
extern long _sysconf(int); /* System Private interface to sysconf() */
#define PTHREAD_STACK_MIN _sysconf(_SC_THREAD_STACK_MIN)
#endif /* defined(__EXTENSIONS__) || (_POSIX_C_SOURCE >= 199506L) */

#endif
