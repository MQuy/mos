#include <fs/char_dev.h>
#include <fs/vfs.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <system/time.h>
#include <utils/string.h>

#include "mqueuefs.h"

#define MQUEUEFS_MAGIC 0xC01D03F2
#define MQUEUEFS_ROOT "/dev/mqueue"
#define MQUEUE_MINOR 19

struct vfs_inode *mqueuefs_get_inode(struct vfs_superblock *sb, uint32_t mode)
{
	struct vfs_inode *i = sb->s_op->alloc_inode(sb);
	i->i_blksize = PMM_FRAME_SIZE;
	i->i_mode = mode;
	i->i_atime.tv_sec = get_seconds(NULL);
	i->i_ctime.tv_sec = get_seconds(NULL);
	i->i_mtime.tv_sec = get_seconds(NULL);

	if (S_ISDIR(i->i_mode))
	{
		i->i_op = &mqueuefs_dir_inode_operations;
		i->i_fop = &mqueuefs_dir_operations;
	}
	else if (S_ISREG(i->i_mode))
	{
		i->i_op = &mqueuefs_file_inode_operations;
		i->i_fop = &mqueuefs_file_operations;
	}

	return i;
}

static struct vfs_inode *mqueuefs_alloc_inode(struct vfs_superblock *sb)
{
	struct vfs_inode *inode = init_inode();
	inode->i_sb = sb;
	atomic_set(&inode->i_count, 1);

	return inode;
}

struct vfs_super_operations mqueuefs_super_operations = {
	.alloc_inode = mqueuefs_alloc_inode,
};

static int mqueuefs_fill_super(struct vfs_superblock *sb)
{
	sb->s_magic = MQUEUEFS_MAGIC;
	sb->s_blocksize = PMM_FRAME_SIZE;
	sb->s_op = &mqueuefs_super_operations;
	return 0;
}

static struct vfs_mount *mqueuefs_mount(struct vfs_file_system_type *fs_type,
										char *dev_name, char *dir_name)
{
	struct vfs_superblock *sb = kcalloc(1, sizeof(struct vfs_superblock));
	sb->s_blocksize = PMM_FRAME_SIZE;
	sb->mnt_devname = strdup(dev_name);
	sb->s_type = fs_type;
	mqueuefs_fill_super(sb);

	struct vfs_inode *i_root = mqueuefs_get_inode(sb, S_IFDIR);
	struct vfs_dentry *d_root = alloc_dentry(NULL, dir_name);
	d_root->d_inode = i_root;
	d_root->d_sb = sb;

	sb->s_root = d_root;

	struct vfs_mount *mnt = kcalloc(1, sizeof(struct vfs_mount));
	mnt->mnt_sb = sb;
	mnt->mnt_mountpoint = mnt->mnt_root = sb->s_root;
	mnt->mnt_devname = sb->mnt_devname;

	return mnt;
}

struct vfs_file_system_type mqueuefs_fs_type = {
	.name = "mqueuefs",
	.mount = mqueuefs_mount,
};

void init_mqueuefs()
{
	register_filesystem(&mqueuefs_fs_type);
	vfs_mknod(MQUEUEFS_ROOT, O_RDWR, MKDEV(0, MQUEUE_MINOR));
	do_mount("mqueuefs", MS_NOUSER, MQUEUEFS_ROOT);
}

void exit_mqueuefs()
{
	unregister_filesystem(&mqueuefs_fs_type);
}
