#include <fs/vfs.h>
#include <include/errno.h>
#include <ipc/message_queue.h>

#include "mqueuefs.h"

static int mqueue_file_open(struct vfs_inode *inode, struct vfs_file *file)
{
	struct mqueuefs_inode *mqi = inode->i_fs_info;
	struct message_queue *mq = hashmap_get(&mq_map, &mqi->key);

	if (!mq)
	{
		mq = kcalloc(1, sizeof(struct message_queue));
		INIT_LIST_HEAD(&mq->senders);
		INIT_LIST_HEAD(&mq->receivers);
		INIT_LIST_HEAD(&mq->messages);
		INIT_LIST_HEAD(&mq->wait.list);

		if (!hashmap_put(&mq_map, &mqi->key, mq))
			return -EINVAL;
	}

	file->private_data = mq;
	return 0;
}

static unsigned int mqueue_file_poll(struct vfs_file *file, struct poll_table *pt)
{
	struct message_queue *mq = (struct message_queue *)file->private_data;
	poll_wait(file, &mq->wait, pt);
	return (!list_empty(&mq->messages) ? POLLIN : 0) | POLLOUT;
}

struct vfs_file_operations mqueuefs_file_operations = {
	.open = mqueue_file_open,
	.poll = mqueue_file_poll};

struct vfs_file_operations mqueuefs_dir_operations = {
	.readdir = generic_memory_readdir,
};
