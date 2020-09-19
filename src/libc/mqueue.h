#ifndef LIBC_MQUEUE_H
#define LIBC_MQUEUE_H

struct mq_attr
{
	long mq_flags;	 /* Flags (ignored for mq_open()) */
	long mq_maxmsg;	 /* Max. # of messages on queue */
	long mq_msgsize; /* Max. message size (bytes) */
	long mq_curmsgs; /* # of messages currently in queue
                                       (ignored for mq_open()) */
};

#endif
