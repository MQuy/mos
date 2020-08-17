#ifndef IPC_MESSAGE_QUEUE_H
#define IPC_MESSAGE_QUEUE_H

#include <include/list.h>
#include <kernel/fs/poll.h>
#include <kernel/proc/task.h>
#include <stddef.h>
#include <stdint.h>

struct mq_receiver
{
	struct thread *receiver;
	struct mq_message *mqm;
	uint32_t priority;
	uint32_t msize;
	struct list_head sibling;
};

struct mq_message
{
	struct thread *sender;
	char *buf;
	uint32_t priority;
	uint32_t msize;
	struct list_head sibling;
};

struct message_queue
{
	struct wait_queue_head wait;
	struct list_head messages;
	struct list_head receivers;
};

void mq_init();
int32_t mq_open(const char *name, int32_t flags);
int32_t mq_close(int32_t fd);
int32_t mq_unlink(const char *name);
int32_t mq_send(int32_t fd, char *buf, uint32_t priorty, uint32_t msize);
int32_t mq_receive(int32_t fd, char *buf, uint32_t priority, uint32_t msize);

#endif
