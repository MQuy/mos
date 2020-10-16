#include "message_queue.h"

#include <fs/vfs.h>
#include <include/errno.h>
#include <include/fcntl.h>
#include <locking/semaphore.h>
#include <memory/vmm.h>
#include <utils/hashmap.h>
#include <utils/printf.h>
#include <utils/string.h>

static const char defaultdir[] = "/dev/mqueue/";
struct hashmap mq_map;

static char *mq_normalize_path(char *name)
{
	char *fname = kcalloc(strlen(name) + sizeof(defaultdir), sizeof(char));

	strcpy(fname, defaultdir);
	strcat(fname, name);

	return fname;
}

int32_t mq_open(const char *name, int32_t flags, struct mq_attr *attr)
{
	char *fname = mq_normalize_path(name);
	int32_t fd = vfs_open(fname, flags);
	int32_t ret = fd;

	struct message_queue *mq = hashmap_get(&mq_map, name);
	// TODO: MQ 2020-09-20 Implement oflag to get rid of attr checking
	if (!mq->attr)
	{
		struct mq_attr *mqattr = kcalloc(1, sizeof(struct mq_attr));
		mqattr->mq_flags = flags;
		mqattr->mq_maxmsg = attr ? attr->mq_maxmsg : MAX_NUMBER_OF_MQ_MESSAGES;
		mqattr->mq_msgsize = attr ? attr->mq_msgsize : MAX_MQ_MESSAGE_SIZE;

		mq->attr = mqattr;
	}
	else if (attr && (mq->attr->mq_maxmsg != attr->mq_maxmsg || mq->attr->mq_msgsize != attr->mq_msgsize))
		ret = -EINVAL;

	kfree(fname);

	return ret;
}

int32_t mq_close(int32_t fd)
{
	return vfs_close(fd);
}

int32_t mq_unlink(const char *name)
{
	struct message_queue *mq = hashmap_get(&mq_map, name);

	if (!mq)
		return -EINVAL;

	struct mq_message *miter, *mnext;
	list_for_each_entry_safe(miter, mnext, &mq->messages, sibling)
	{
		list_del(&miter->sibling);
		kfree(miter);
	}

	struct mq_sender *siter, *snext;
	list_for_each_entry_safe(siter, snext, &mq->senders, sibling)
	{
		list_del(&siter->sibling);
		kfree(siter);
		update_thread(siter->sender, THREAD_READY);
	}

	struct mq_receiver *riter, *rnext;
	list_for_each_entry_safe(riter, rnext, &mq->receivers, sibling)
	{
		list_del(&riter->sibling);
		kfree(riter);
		update_thread(riter->receiver, THREAD_READY);
	}

	hashmap_remove(&mq_map, name);
	kfree(mq->attr);
	kfree(mq);
	return 0;
}

static void mq_add_message(struct message_queue *mq, struct mq_message *msg)
{
	struct mq_message *iter;
	list_for_each_entry(iter, &mq->messages, sibling)
	{
		if (msg->priority > iter->priority)
			break;
	}

	if (&iter->sibling == &mq->messages)
		list_add_tail(&msg->sibling, &mq->messages);
	else
		list_add(&msg->sibling, iter->sibling.prev);

	mq->attr->mq_curmsgs += 1;
}

static void mq_add_sender(struct message_queue *mq, struct mq_sender *mqs)
{
	struct mq_sender *iter;
	list_for_each_entry(iter, &mq->senders, sibling)
	{
		if (mqs->priority > iter->priority)
			break;
	}

	if (&iter->sibling == &mq->senders)
		list_add_tail(&mqs->sibling, &mq->senders);
	else
		list_add(&mqs->sibling, iter->sibling.prev);
}

static void mq_add_receiver(struct message_queue *mq, struct mq_receiver *mqr)
{
	struct mq_receiver *iter;
	list_for_each_entry(iter, &mq->receivers, sibling)
	{
		if (mqr->priority > iter->priority)
			break;
	}

	if (&iter->sibling == &mq->receivers)
		list_add_tail(&mqr->sibling, &mq->receivers);
	else
		list_add(&mqr->sibling, iter->sibling.prev);
}

int32_t mq_send(int32_t fd, char *user_buf, uint32_t priority, uint32_t msize)
{
	struct vfs_file *file = current_thread->parent->files->fd[fd];
	struct message_queue *mq = (struct message_queue *)file->private_data;

	if (!mq)
		return -EBADF;
	else if (msize > mq->attr->mq_msgsize)
		return -EMSGSIZE;

	assert(mq->attr->mq_curmsgs <= mq->attr->mq_maxmsg);
	if (mq->attr->mq_curmsgs == mq->attr->mq_maxmsg)
	{
		if ((mq->attr->mq_flags & O_NONBLOCK) == 0)
		{
			struct mq_sender *mqs;
			wait_until_with_setup(list_is_poison(&mqs->sibling) && mq->attr->mq_curmsgs < mq->attr->mq_maxmsg, ({
									  mqs = kcalloc(1, sizeof(struct mq_sender));
									  mqs->sender = current_thread;
									  mqs->priority = priority;
									  mq_add_sender(mq, mqs);
								  }),
								  kfree(mqs));
		}
		else
			return -EAGAIN;
	}
	assert(mq->attr->mq_curmsgs < mq->attr->mq_maxmsg);

	char *kernel_buf = kcalloc(msize, sizeof(char));
	memcpy(kernel_buf, user_buf, msize);

	struct mq_message *mqm = kcalloc(1, sizeof(struct mq_message));
	mqm->buf = kernel_buf;
	mqm->msize = msize;
	mqm->priority = priority;
	mq_add_message(mq, mqm);

	struct mq_receiver *mqr = list_first_entry_or_null(&mq->receivers, struct mq_receiver, sibling);
	if (mqr)
	{
		assert(mq->attr->mq_curmsgs == 1);
		list_del(&mqr->sibling);
		update_thread(mqr->receiver, THREAD_READY);
	}

	wake_up(&mq->wait);

	return 0;
}

int32_t mq_receive(int32_t fd, char *user_buf, uint32_t priority, uint32_t msize)
{
	struct vfs_file *file = current_thread->parent->files->fd[fd];
	struct message_queue *mq = (struct message_queue *)file->private_data;

	if (!mq)
		return -EINVAL;
	else if (msize < mq->attr->mq_msgsize)
		return -EMSGSIZE;

	assert(0 <= mq->attr->mq_curmsgs && mq->attr->mq_curmsgs <= mq->attr->mq_maxmsg);
	assert(msize >= mq->attr->mq_msgsize);
	if (mq->attr->mq_curmsgs == mq->attr->mq_maxmsg)
	{
		struct mq_sender *mqs = list_first_entry_or_null(&mq->senders, struct mq_sender, sibling);
		if (mqs)
		{
			list_del(&mqs->sibling);
			update_thread(mqs->sender, THREAD_READY);
		}
	}
	else if (mq->attr->mq_curmsgs == 0)
	{
		if ((mq->attr->mq_flags & O_NONBLOCK) == 0)
		{
			struct mq_receiver *mqr;
			wait_until_with_setup(mq->attr->mq_curmsgs > 0 && list_is_poison(&mqr->sibling), ({
									  mqr = kcalloc(1, sizeof(struct mq_receiver));
									  mqr->priority = priority;
									  mqr->receiver = current_thread;
									  mq_add_receiver(mq, mqr);
								  }),
								  kfree(mqr));
		}
		else
			return -EAGAIN;
	}

	struct mq_message *mqm = list_first_entry_or_null(&mq->messages, struct mq_message, sibling);
	assert(mqm);
	list_del(&mqm->sibling);
	mq->attr->mq_curmsgs--;
	memcpy(user_buf, mqm->buf, msize);
	kfree(mqm);

	return 0;
}

void mq_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "Message Queue: Initializing");

	hashmap_init(&mq_map, hashmap_hash_string, hashmap_compare_string, 0);

	DEBUG &&debug_println(DEBUG_INFO, "Message Queue: Done");
}
