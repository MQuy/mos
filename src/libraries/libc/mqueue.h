#ifndef _LIBC_MQUEUE_H
#define _LIBC_MQUEUE_H 1

struct mq_attr
{
	long mq_flags;	 /* Flags (ignored for mq_open()) */
	long mq_maxmsg;	 /* Max. # of messages on queue */
	long mq_msgsize; /* Max. message size (bytes) */
	long mq_curmsgs; /* # of messages currently in queue
                                       (ignored for mq_open()) */
};

int mq_open(const char *name, int flags, struct mq_attr *attr);
int mq_close(int fd);
int mq_unlink(const char *name);
int mq_send(int fd, char *buf, unsigned int priorty, unsigned int msize);
int mq_receive(int fd, char *buf, unsigned int priorty, unsigned int msize);

#endif
