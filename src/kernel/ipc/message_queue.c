#include "message_queue.h"

#include <include/errno.h>
#include <kernel/locking/semaphore.h>
#include <kernel/memory/vmm.h>
#include <kernel/utils/hashmap.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

struct semaphore mq_locking;
struct hashmap mq_map;

void mq_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "[mq] - Initializing");

	sema_init(&mq_locking, 1);
	hashmap_init(&mq_map, hashmap_hash_string, hashmap_compare_string, 0);

	DEBUG &&debug_println(DEBUG_INFO, "[mq] - Done");
}

int32_t mq_open(const char *name, int32_t flags)
{
	struct message_queue *mq = kcalloc(1, sizeof(struct message_queue));
	INIT_LIST_HEAD(&mq->senders);
	INIT_LIST_HEAD(&mq->receivers);
	INIT_LIST_HEAD(&mq->messages);

	if (!hashmap_put(&mq_map, strdup(name), mq))
		return -EINVAL;

	return 0;
}

int32_t mq_close(const char *name)
{
	int32_t ret = 0;
	struct message_queue *mq = hashmap_get(&mq_map, name);
	if (mq)
	{
		hashmap_remove(&mq_map, name);
		kfree(mq);
	}
	return ret;
}

// NOTE: MQ 2020-03-13 The method only be called in kernel
int32_t mq_enqueue(const char *name, char *kernel_buf, int32_t mtype, uint32_t msize)
{
	struct message_queue *mq = hashmap_get(&mq_map, name);

	if (!mq)
		return -EINVAL;

	struct mq_receiver *iter = NULL;
	struct mq_receiver *mqr = NULL;
	list_for_each_entry(iter, &mq->receivers, sibling)
	{
		if (iter->mtype == mtype)
		{
			mqr = iter;
			break;
		}
	}

	if (mqr)
	{
		mqr->sender = current_thread;
		mqr->msize = msize;
		mqr->buf = kernel_buf;
		list_del(&mqr->sibling);
		update_thread(mqr->receiver, THREAD_READY);
	}
	else
	{
		struct mq_message *msg = kcalloc(1, sizeof(struct mq_message));
		msg->buf = kernel_buf;
		msg->msize = msize;
		msg->mtype = mtype;
		msg->sender = current_thread;
		list_add_tail(&msg->sibling, &mq->messages);
	}
	return 0;
}

int32_t mq_send(const char *name, char *user_buf, int32_t mtype, uint32_t msize)
{
	struct message_queue *mq = hashmap_get(&mq_map, name);

	if (!mq)
		return -EINVAL;

	char *kernel_buf = kcalloc(msize, sizeof(char));
	memcpy(kernel_buf, user_buf, msize);

	struct mq_receiver *iter = NULL;
	struct mq_receiver *mqr = NULL;
	list_for_each_entry(iter, &mq->receivers, sibling)
	{
		if (iter->mtype == mtype)
		{
			mqr = iter;
			break;
		}
	}

	if (mqr)
	{
		mqr->sender = current_thread;
		mqr->msize = msize;
		mqr->buf = kernel_buf;
		list_del(&mqr->sibling);
		update_thread(mqr->receiver, THREAD_READY);
	}
	else
	{
		struct mq_message *msg = kcalloc(1, sizeof(struct mq_message));
		msg->buf = kernel_buf;
		msg->msize = msize;
		msg->mtype = mtype;
		msg->sender = current_thread;
		list_add_tail(&msg->sibling, &mq->messages);
	}

	if (mtype >= 0)
		// NOTE: MQ 2020-01-06 Negative thread id number is reserved for ack channel
		// waiting to receive an ack from receiver
		return mq_receive(name, NULL, -current_thread->tid, 0);
	else
		return 0;
}

int32_t mq_receive(const char *name, char *user_buf, int32_t mtype, uint32_t msize)
{
	struct message_queue *mq = hashmap_get(&mq_map, name);

	if (!mq)
		return -EINVAL;

	struct mq_message *iter = NULL;
	struct mq_message *mqm = NULL;
	list_for_each_entry(iter, &mq->messages, sibling)
	{
		if (iter->mtype == mtype)
		{
			mqm = iter;
			break;
		}
	}

	char *kernel_buf = NULL;
	void *msg = NULL;
	tid_t sender_id;
	if (mqm)
	{
		msg = mqm;
		kernel_buf = mqm->buf;
		sender_id = mqm->sender->tid;
		list_del(&mqm->sibling);
	}
	else
	{
		struct mq_receiver *mqr = kcalloc(1, sizeof(struct mq_receiver));
		mqr->mtype = mtype;
		mqr->msize = msize;
		mqr->receiver = current_thread;
		list_add_tail(&mqr->sibling, &mq->receivers);
		update_thread(current_thread, THREAD_WAITING);
		schedule();

		if (!mqr->buf && mtype >= 0)
			return -EINVAL;

		msg = mqr;
		kernel_buf = mqr->buf;
		sender_id = mqr->sender->tid;
		list_del(&mqr->sibling);
	}

	if (sender_id != current_thread->tid && mtype >= 0)
		// send ack signal to sender
		mq_send(name, NULL, -sender_id, 0);
	if (mtype >= 0)
	{
		memcpy(user_buf, kernel_buf, msize);
		kfree(kernel_buf);
	}
	kfree(msg);

	return 0;
}
