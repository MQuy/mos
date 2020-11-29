#ifndef _LIBC_SYS_RESOURCE_H
#define _LIBC_SYS_RESOURCE_H 1

#include <sys/time.h>

/* Transmute defines to enumerations.  The macro re-definitions are
   necessary because some programs want to test for operating system
   features with #ifdef RUSAGE_SELF.  In ISO C the reflexive
   definition is a no-op.  */

/* Kinds of resource limit.  */
enum __rlimit_resource
{
	/* Per-process CPU limit, in seconds.  */
	RLIMIT_CPU = 0,
#define RLIMIT_CPU RLIMIT_CPU

	/* Largest file that can be created, in bytes.  */
	RLIMIT_FSIZE = 1,
#define RLIMIT_FSIZE RLIMIT_FSIZE

	/* Maximum size of data segment, in bytes.  */
	RLIMIT_DATA = 2,
#define RLIMIT_DATA RLIMIT_DATA

	/* Maximum size of stack segment, in bytes.  */
	RLIMIT_STACK = 3,
#define RLIMIT_STACK RLIMIT_STACK

	/* Largest core file that can be created, in bytes.  */
	RLIMIT_CORE = 4,
#define RLIMIT_CORE RLIMIT_CORE

	/* Largest resident set size, in bytes.
     This affects swapping; processes that are exceeding their
     resident set size will be more likely to have physical memory
     taken from them.  */
	__RLIMIT_RSS = 5,
#define RLIMIT_RSS __RLIMIT_RSS

	/* Number of open files.  */
	RLIMIT_NOFILE = 7,
	__RLIMIT_OFILE = RLIMIT_NOFILE, /* BSD name for same.  */
#define RLIMIT_NOFILE RLIMIT_NOFILE
#define RLIMIT_OFILE __RLIMIT_OFILE

	/* Address space limit.  */
	RLIMIT_AS = 9,
#define RLIMIT_AS RLIMIT_AS

	/* Number of processes.  */
	__RLIMIT_NPROC = 6,
#define RLIMIT_NPROC __RLIMIT_NPROC

	/* Locked-in-memory address space.  */
	__RLIMIT_MEMLOCK = 8,
#define RLIMIT_MEMLOCK __RLIMIT_MEMLOCK

	/* Maximum number of file locks.  */
	__RLIMIT_LOCKS = 10,
#define RLIMIT_LOCKS __RLIMIT_LOCKS

	/* Maximum number of pending signals.  */
	__RLIMIT_SIGPENDING = 11,
#define RLIMIT_SIGPENDING __RLIMIT_SIGPENDING

	/* Maximum bytes in POSIX message queues.  */
	__RLIMIT_MSGQUEUE = 12,
#define RLIMIT_MSGQUEUE __RLIMIT_MSGQUEUE

	/* Maximum nice priority allowed to raise to.
     Nice levels 19 .. -20 correspond to 0 .. 39
     values of this resource limit.  */
	__RLIMIT_NICE = 13,
#define RLIMIT_NICE __RLIMIT_NICE

	/* Maximum realtime priority allowed for non-priviledged
     processes.  */
	__RLIMIT_RTPRIO = 14,
#define RLIMIT_RTPRIO __RLIMIT_RTPRIO

	/* Maximum CPU time in µs that a process scheduled under a real-time
     scheduling policy may consume without making a blocking system
     call before being forcibly descheduled.  */
	__RLIMIT_RTTIME = 15,
#define RLIMIT_RTTIME __RLIMIT_RTTIME

	__RLIMIT_NLIMITS = 16,
	__RLIM_NLIMITS = __RLIMIT_NLIMITS
#define RLIMIT_NLIMITS __RLIMIT_NLIMITS
#define RLIM_NLIMITS __RLIM_NLIMITS
};

/* Whose usage statistics do you want?  */
enum __rusage_who
{
	/* The calling process.  */
	RUSAGE_SELF = 0,
#define RUSAGE_SELF RUSAGE_SELF

	/* All of its terminated child processes.  */
	RUSAGE_CHILDREN = -1
#define RUSAGE_CHILDREN RUSAGE_CHILDREN

#ifdef __USE_GNU
	,
	/* The calling thread.  */
	RUSAGE_THREAD = 1
#define RUSAGE_THREAD RUSAGE_THREAD
/* Name for the same functionality on Solaris.  */
#define RUSAGE_LWP RUSAGE_THREAD
#endif
};

enum __priority_which
{
	PRIO_PROCESS = 0, /* WHO is a process ID.  */
#define PRIO_PROCESS PRIO_PROCESS
	PRIO_PGRP = 1, /* WHO is a process group ID.  */
#define PRIO_PGRP PRIO_PGRP
	PRIO_USER = 2 /* WHO is a user ID.  */
#define PRIO_USER PRIO_USER
};

/* We can represent all limits.  */
#define RLIM_SAVED_MAX RLIM_INFINITY
#define RLIM_SAVED_CUR RLIM_INFINITY

#define RLIM_INFINITY ((rlim_t)-1)

struct rlimit
{
	rlim_t rlim_cur;
	rlim_t rlim_max;
};

struct rusage
{
	struct timeval ru_utime;
	struct timeval ru_stime;
};

int getrlimit(int resource, struct rlimit *rlp);
int setrlimit(int resource, const struct rlimit *rlp);

#endif
