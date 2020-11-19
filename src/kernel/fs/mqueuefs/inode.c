#include <fs/vfs.h>
#include <include/errno.h>
#include <ipc/message_queue.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <system/time.h>

#include "mqueuefs.h"

static struct vfs_inode *mqueuefs_create_inode(struct vfs_inode *dir, struct vfs_dentry *dentry, mode_t mode)
{
	return mqueuefs_get_inode(dir->i_sb, mode);
}

struct vfs_inode_operations mqueuefs_file_inode_operations = {};

int mqueuefs_unlink(struct vfs_inode *dir, struct vfs_dentry *dentry)
{
	return mq_unlink(dentry->d_name);
}

struct vfs_inode_operations mqueuefs_dir_inode_operations = {
	.create = mqueuefs_create_inode,
	.unlink = mqueuefs_unlink,
	.rename = generic_memory_rename,
};
