#include "message_queue.h"

#include <include/errno.h>
#include <kernel/fs/vfs.h>
#include <kernel/locking/semaphore.h>
#include <kernel/memory/vmm.h>
#include <kernel/utils/hashmap.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

static const char defaultdir[] = "/dev/mqueue/";

struct hashmap mq_map;

static char *mq_normalize_path(char *name)
{
	char *fname = kcalloc(strlen(name) + sizeof(defaultdir), sizeof(char));

	strcpy(fname, defaultdir);
	strcat(fname, name);

	return fname;
}

int32_t mq_open(const char *name, int32_t flags)
{
	char *fname = mq_normalize_path(name);
	int32_t fd = vfs_open(fname, flags);

	kfree(fname);

	return fd;
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
		update_thread(miter->sender, THREAD_READY);
	}

	struct mq_receiver *riter, *rnext;
	list_for_each_entry_safe(riter, rnext, &mq->receivers, sibling)
	{
		list_del(&riter->sibling);
		update_thread(riter->receiver, THREAD_READY);
	}

	hashmap_remove(&mq_map, name);
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
}

static void mq_add_receiver(struct message_queue *mq, struct mq_receiver *mqr)
{
	struct mq_receiver *iter;
	list_for_each_entry(iter, &mq->receivers, sibling)
	{
		if (mqr->priority > iter->priority)
			break;
	}

	if (&iter->sibling == &mq->messages)
		list_add_tail(&mqr->sibling, &mq->messages);
	else
		list_add(&mqr->sibling, iter->sibling.prev);
}

int32_t mq_send(int32_t fd, char *user_buf, uint32_t priority, uint32_t msize)
{
	struct vfs_file *file = current_thread->parent->files->fd[fd];
	struct message_queue *mq = (struct message_queue *)file->private_data;

	if (!mq)
		return -EINVAL;

	char *kernel_buf = kcalloc(msize, sizeof(char));
	memcpy(kernel_buf, user_buf, msize);

	struct mq_message *mqm = kcalloc(1, sizeof(struct mq_message));
	mqm->buf = kernel_buf;
	mqm->msize = msize;
	mqm->priority = priority;
	mqm->sender = current_thread;
	mq_add_message(mq, mqm);

	struct mq_receiver *mqr = list_first_entry_or_null(&mq->receivers, struct mq_receiver, sibling);

	if (mqr)
	{
		list_del(&mqr->sibling);
		update_thread(mqr->receiver, THREAD_READY);
	}

	wake_up(&mq->wait);
	wait_until(list_is_poison(&mqm->sibling));
	kfree(mqm);
	kfree(kernel_buf);

	return hashmap_get(&mq_map, file->f_dentry->d_name) ? 0 : -ESHUTDOWN;
}

int32_t mq_receive(int32_t fd, char *user_buf, uint32_t priority, uint32_t msize)
{
	struct vfs_file *file = current_thread->parent->files->fd[fd];
	struct message_queue *mq = (struct message_queue *)file->private_data;

	if (!mq)
		return -EINVAL;

	struct mq_message *mqm = list_first_entry_or_null(&mq->messages, struct mq_message, sibling);

	if (!mqm)
	{
		struct mq_receiver *mqr = kcalloc(1, sizeof(struct mq_receiver));
		mqr->priority = priority;
		mqr->msize = msize;
		mqr->receiver = current_thread;

		wait_until_with_prework(!list_empty(&mq->messages), ({
			if (list_is_poison(&mqr->sibling))
				mq_add_receiver(mq, mqr);
		}));

		list_del(&mqr->sibling);
		kfree(mqr);
		mqm = list_first_entry(&mq->messages, struct mq_message, sibling);
	}

	list_del(&mqm->sibling);
	memcpy(user_buf, mqm->buf, msize);
	update_thread(mqm->sender, THREAD_READY);

	return hashmap_get(&mq_map, file->f_dentry->d_name) ? 0 : -ESHUTDOWN;
}

void mq_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "[mq] - Initializing");

	hashmap_init(&mq_map, hashmap_hash_string, hashmap_compare_string, 0);

	DEBUG &&debug_println(DEBUG_INFO, "[mq] - Done");
}
