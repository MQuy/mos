#include <kernel/fs/vfs.h>
#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include <kernel/proc/task.h>
#include <kernel/system/time.h>
#include <kernel/utils/string.h>

#include "sockfs.h"

#define SOCKFS_MAGIC 0x534F434B
#define SOCKFS_ROOT "/dev/sockfs"
#define SOCK_NUMBER_LENGTH 11

static uint32_t nsock;

int32_t get_unused_socket_number()
{
	return nsock++;
}

char *get_next_socket_path()
{
	// NOTE: MQ 2020-06-01 Minus one because sizeof includes null-terminated
	uint8_t root_length = sizeof(SOCKFS_ROOT) - 1;
	char *snum_s = kcalloc(1, SOCK_NUMBER_LENGTH);
	itoa_s(get_unused_socket_number(), SOCK_NUMBER_LENGTH, snum_s);
	char *path = kcalloc(1, root_length + 1 + SOCK_NUMBER_LENGTH);
	memcpy(path, SOCKFS_ROOT, root_length);
	path[root_length] = '/';
	memcpy(path + root_length + 1, snum_s, SOCK_NUMBER_LENGTH);
	return path;
}

struct vfs_inode *sockfs_get_inode(struct vfs_superblock *sb, uint32_t mode)
{
	struct vfs_inode *i = sb->s_op->alloc_inode(sb);
	i->i_blksize = PMM_FRAME_SIZE;
	// NOTE: MQ 2020-06-01 We don't directly set mode to inode, only set permission part
	i->i_mode = S_IFSOCK | (mode & !S_IFMT);
	i->i_atime.tv_sec = get_seconds(NULL);
	i->i_ctime.tv_sec = get_seconds(NULL);
	i->i_mtime.tv_sec = get_seconds(NULL);

	// NOTE: MQ 2020-06-01
	// The only purpose of mode arugment
	// -> permission part which is set to i_mode
	// -> use to decide whether it is dir or file
	if (S_ISREG(mode))
	{
		i->i_op = &sockfs_file_inode_operations;
		i->i_fop = &sockfs_file_operations;
	}
	else if (S_ISDIR(mode))
	{
		i->i_op = &sockfs_dir_inode_operations;
		i->i_fop = &sockfs_dir_operations;
	}

	return i;
}

static struct vfs_inode *sockfs_alloc_inode(struct vfs_superblock *sb)
{
	struct socket_alloc *ei = kcalloc(1, sizeof(struct socket_alloc));

	ei->inode.i_blocks = 0;
	ei->inode.i_size = 0;
	ei->inode.i_sb = sb;
	sema_init(&ei->inode.i_sem, 1);
	atomic_set(&ei->inode.i_count, 1);

	ei->socket.flags = 0;
	ei->socket.state = SS_UNCONNECTED;

	return &ei->inode;
}

struct vfs_super_operations sockfs_super_operations = {
	.alloc_inode = sockfs_alloc_inode,
};

static struct vfs_mount *sockfs_mount(struct vfs_file_system_type *fs_type,
									  char *dev_name, char *dir_name)
{
	struct vfs_superblock *sb = kcalloc(1, sizeof(struct vfs_superblock));
	sb->s_blocksize = PMM_FRAME_SIZE;
	sb->mnt_devname = dev_name;
	sb->s_type = fs_type;
	sb->s_magic = SOCKFS_MAGIC;
	sb->s_op = &sockfs_super_operations;

	struct vfs_inode *i_root = sockfs_get_inode(sb, S_IFDIR);
	struct vfs_dentry *d_root = alloc_dentry(NULL, dir_name);
	d_root->d_inode = i_root;
	d_root->d_sb = sb;

	sb->s_root = d_root;

	struct vfs_mount *mnt = kcalloc(1, sizeof(struct vfs_mount));
	mnt->mnt_sb = sb;
	mnt->mnt_mountpoint = mnt->mnt_root = sb->s_root;
	mnt->mnt_devname = dev_name;

	return mnt;
}

struct vfs_file_system_type sockfs_fs_type = {
	.name = "sockfs",
	.mount = sockfs_mount,
};

void init_sockfs()
{
	nsock = 0;
	register_filesystem(&sockfs_fs_type);
	do_mount("sockfs", MS_NOUSER, SOCKFS_ROOT);
}

void exit_sockfs()
{
	unregister_filesystem(&sockfs_fs_type);
}
