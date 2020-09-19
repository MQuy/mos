#ifndef IPC_MESSAGE_QUEUE_H
#define IPC_MESSAGE_QUEUE_H

#include <include/list.h>
#include <kernel/fs/poll.h>
#include <kernel/proc/task.h>
#include <stddef.h>
#include <stdint.h>

#define MAX_NUMBER_OF_MQ_MESSAGES 32
#define MAX_MQ_MESSAGE_SIZE 512

struct mq_attr
{
	long mq_flags;	 /* Flags (ignored for mq_open()) */
	long mq_maxmsg;	 /* Max. # of messages on queue */
	long mq_msgsize; /* Max. message size (bytes) */
	long mq_curmsgs; /* # of messages currently in queue
                                       (ignored for mq_open()) */
};

struct mq_sender
{
	struct thread *sender;
	struct list_head sibling;
	uint32_t priority;
};

struct mq_receiver
{
	struct thread *receiver;
	uint32_t priority;
	struct list_head sibling;
};

struct mq_message
{
	char *buf;
	uint32_t priority;
	uint32_t msize;
	struct list_head sibling;
};

struct message_queue
{
	struct wait_queue_head wait;
	struct list_head messages;
	struct list_head senders;
	struct list_head receivers;
	struct mq_attr *attr;
};

extern struct hashmap mq_map;
void mq_init();
int32_t mq_open(const char *name, int32_t flags, struct mq_attr *attr);
int32_t mq_close(int32_t fd);
int32_t mq_unlink(const char *name);
int32_t mq_send(int32_t fd, char *buf, uint32_t priorty, uint32_t msize);
int32_t mq_receive(int32_t fd, char *buf, uint32_t priority, uint32_t msize);

#endif
