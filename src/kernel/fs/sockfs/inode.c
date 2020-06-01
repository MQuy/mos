#include <include/errno.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/time.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include "sockfs.h"

extern struct process *current_process;

struct vfs_inode *sockfs_create_inode(struct vfs_inode *dir, char *filename, mode_t mode)
{
  return sockfs_get_inode(dir->i_sb, mode);
}

struct vfs_inode_operations sockfs_file_inode_operations = {};

struct vfs_inode_operations sockfs_dir_inode_operations = {
    .create = sockfs_create_inode,
};
