#include <fs/vfs.h>
#include <include/errno.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <system/time.h>

#include "sockfs.h"

static struct vfs_inode *sockfs_create_inode(struct vfs_inode *dir, struct vfs_dentry *dentry, mode_t mode)
{
	return sockfs_get_inode(dir->i_sb, mode);
}

struct vfs_inode_operations sockfs_file_inode_operations = {};

struct vfs_inode_operations sockfs_dir_inode_operations = {
	.create = sockfs_create_inode,
	.rename = generic_memory_rename,
};
