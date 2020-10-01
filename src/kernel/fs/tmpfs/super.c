#include <fs/vfs.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <system/time.h>

#include "tmpfs.h"

#define TMPFS_MAGIC 0x01021994

struct vfs_inode *tmpfs_get_inode(struct vfs_superblock *sb, uint32_t mode)
{
	struct vfs_inode *i = sb->s_op->alloc_inode(sb);
	i->i_blksize = PMM_FRAME_SIZE;
	i->i_mode = mode;
	i->i_atime.tv_sec = get_seconds(NULL);
	i->i_ctime.tv_sec = get_seconds(NULL);
	i->i_mtime.tv_sec = get_seconds(NULL);

	if (S_ISREG(i->i_mode))
	{
		i->i_op = &tmpfs_file_inode_operations;
		i->i_fop = &tmpfs_file_operations;
	}
	else if (S_ISDIR(i->i_mode))
	{
		i->i_op = &tmpfs_dir_inode_operations;
		i->i_fop = &tmpfs_dir_operations;
	}
	else
	{
		i->i_op = &tmpfs_special_inode_operations;
		init_special_inode(i, i->i_mode, i->i_rdev);
	}

	return i;
}

static struct vfs_inode *tmpfs_alloc_inode(struct vfs_superblock *sb)
{
	struct vfs_inode *inode = init_inode();
	inode->i_sb = sb;
	atomic_set(&inode->i_count, 1);
	INIT_LIST_HEAD(&inode->i_data.pages);

	return inode;
}

struct vfs_super_operations tmpfs_super_operations = {
	.alloc_inode = tmpfs_alloc_inode,
};

static int tmpfs_fill_super(struct vfs_superblock *sb)
{
	struct tmpfs_sb_info *sbinfo = kcalloc(1, sizeof(struct tmpfs_sb_info));
	sbinfo->max_blocks = get_total_frames() / 2;
	sbinfo->free_blocks = sbinfo->max_blocks;
	sbinfo->max_inodes = sbinfo->max_blocks;
	sbinfo->free_inodes = sbinfo->max_inodes;

	sb->s_fs_info = sbinfo;
	sb->s_magic = TMPFS_MAGIC;
	sb->s_blocksize = PMM_FRAME_SIZE;
	sb->s_op = &tmpfs_super_operations;
	return 0;
}

static struct vfs_mount *tmpfs_mount(struct vfs_file_system_type *fs_type,
									 char *dev_name, char *dir_name)
{
	struct vfs_superblock *sb = kcalloc(1, sizeof(struct vfs_superblock));
	sb->s_blocksize = PMM_FRAME_SIZE;
	sb->mnt_devname = dev_name;
	sb->s_type = fs_type;
	tmpfs_fill_super(sb);

	struct vfs_inode *i_root = tmpfs_get_inode(sb, S_IFDIR);
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

struct vfs_file_system_type tmpfs_fs_type = {
	.name = "tmpfs",
	.mount = tmpfs_mount,
};

void init_tmpfs()
{
	register_filesystem(&tmpfs_fs_type);
	do_mount("tmpfs", MS_NOUSER, "/dev/shm");
}

void exit_tmpfs()
{
	unregister_filesystem(&tmpfs_fs_type);
}
