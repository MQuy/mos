#ifndef _LIBC_UTMPX_H
#define _LIBC_UTMPX_H 1

#include <paths.h>
#include <sys/time.h>

#define __UT_LINESIZE 32
#define __UT_NAMESIZE 32
#define __UT_HOSTSIZE 25

struct utmpx
{
	char ut_user[__UT_NAMESIZE];
	char ut_id[4];
	char ut_line[__UT_LINESIZE];
	pid_t ut_pid;
	short ut_type;
	struct timeval ut_tv;
};

#endif
