#include <include/errno.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/time.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include "sockfs.h"

extern struct process *current_process;

int sockfs_mknod(struct vfs_inode *dir, char *name, int mode, dev_t dev)
{
    struct vfs_inode *i = sockfs_get_inode(dir->i_sb, mode);
    uint32_t current_seconds = get_seconds(NULL);
    dir->i_ctime.tv_sec = current_seconds;
    dir->i_mtime.tv_sec = current_seconds;
    return 0;
}

struct vfs_inode *sockfs_create_inode(struct vfs_inode *dir, char *filename, mode_t mode)
{
    return sockfs_get_inode(dir->i_sb, mode | S_IFSOCK);
}

int sockfs_mkdir(struct vfs_inode *dir, char *name, int mode)
{
    return sockfs_mknod(dir, name, mode | S_IFDIR, 0);
}

struct vfs_inode_operations sockfs_file_inode_operations = {};

struct vfs_inode_operations sockfs_dir_inode_operations = {
    .create = sockfs_create_inode,
    .mknod = sockfs_mknod,
    .mkdir = sockfs_mkdir,
};
