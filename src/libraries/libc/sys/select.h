#ifndef _LIBC_SYS_SELECT_H
#define _LIBC_SYS_SELECT_H 1

#include <string.h>

#define FD_SETSIZE 1024
#define __FD_ELEM_SIZE ((int)sizeof(__fd_mask))
#define __FD_ELEM_BITS (8 * __FD_ELEM_SIZE)

typedef unsigned long __fd_mask;

typedef struct
{
	__fd_mask __fds_bits[FD_SETSIZE / (8 * (int)sizeof(__fd_mask))];
} fd_set;

#define __FD_INDEX(fd) ((fd) / __FD_ELEM_BITS)
#define __FD_EXPONENT(fd) ((__fd_mask)(fd) % __FD_ELEM_BITS)
#define __FD_MASK(fd) (1UL << __FD_EXPONENT(fd))
#define __FD_ACCESS(fd, fdsetp) ((fdsetp)->__fds_bits[__FD_INDEX(fd)])

#define FD_CLR(fd, fdsetp) (__FD_ACCESS(fd, fdsetp) &= ~__FD_MASK(fd))
#define FD_ISSET(fd, fdsetp) (__FD_ACCESS(fd, fdsetp) & __FD_MASK(fd))
#define FD_SET(fd, fdsetp) (__FD_ACCESS(fd, fdsetp) |= __FD_MASK(fd))
#define FD_ZERO(fdsetp) memset(fdsetp, 0, sizeof(fd_set))

struct timeval;

int select(int nfds, fd_set *readfds, fd_set *writefds,
		   fd_set *exceptfds, struct timeval *timeout);

#endif
