#include <errno.h>
#include <mqueue.h>
#include <unistd.h>

_syscall3(mq_open, const char *, int, struct mq_attr *);
int mq_open(const char *name, int flags, struct mq_attr *attr)
{
	SYSCALL_RETURN_ORIGINAL(syscall_mq_open(name, flags, attr));
}

_syscall1(mq_close, int);
int mq_close(int fd)
{
	SYSCALL_RETURN(syscall_mq_close(fd));
}

_syscall1(mq_unlink, const char *);
int mq_unlink(const char *name)
{
	SYSCALL_RETURN(syscall_mq_unlink(name));
}

_syscall4(mq_send, int, char *, unsigned int, unsigned int);
int mq_send(int fd, char *buf, unsigned int priorty, unsigned int msize)
{
	SYSCALL_RETURN_ORIGINAL(syscall_mq_send(fd, buf, priorty, msize));
}

_syscall4(mq_receive, int, char *, unsigned int, unsigned int);
int mq_receive(int fd, char *buf, unsigned int priorty, unsigned int msize)
{
	SYSCALL_RETURN_ORIGINAL(syscall_mq_receive(fd, buf, priorty, msize));
}
