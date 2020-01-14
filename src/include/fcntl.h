#ifndef INCLUDE_FCNTL_H
#define INCLUDE_FCNTL_H

#define O_ACCMODE 0003
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 01000   /* not fcntl */
#define O_TRUNC 02000   /* not fcntl */
#define O_EXCL 04000    /* not fcntl */
#define O_NOCTTY 010000 /* not fcntl */

#define O_NONBLOCK 00004
#define O_APPEND 00010
#define O_NDELAY O_NONBLOCK
#define O_SYNC 040000
#define FASYNC 020000       /* fcntl, for BSD compatibility */
#define O_DIRECTORY 0100000 /* must be a directory */
#define O_NOFOLLOW 0200000  /* don't follow links */
#define O_LARGEFILE 0400000 /* will be set by the kernel on every open */
#define O_DIRECT 02000000   /* direct disk access - should check with OSF/1 */
#define O_NOATIME 04000000

#endif