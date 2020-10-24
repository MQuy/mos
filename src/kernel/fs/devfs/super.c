#include <fs/vfs.h>
#include <memory/pmm.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <system/time.h>
#include <utils/string.h>

#include "devfs.h"

#define DEVFS_MAGIC 0xA302B109

struct vfs_inode *devfs_get_inode(struct vfs_superblock *sb, uint32_t mode)
{
	struct vfs_inode *i = sb->s_op->alloc_inode(sb);
	i->i_blksize = PMM_FRAME_SIZE;
	i->i_mode = mode;
	i->i_atime.tv_sec = get_seconds(NULL);
	i->i_ctime.tv_sec = get_seconds(NULL);
	i->i_mtime.tv_sec = get_seconds(NULL);

	if (S_ISDIR(i->i_mode))
	{
		i->i_op = &devfs_dir_inode_operations;
		i->i_fop = &devfs_dir_operations;
	}
	else
	{
		i->i_op = &devfs_special_inode_operations;
		init_special_inode(i, i->i_mode, i->i_rdev);
	}

	return i;
}

static struct vfs_inode *devfs_alloc_inode(struct vfs_superblock *sb)
{
	struct vfs_inode *inode = init_inode();
	inode->i_sb = sb;
	atomic_set(&inode->i_count, 1);

	return inode;
}

struct vfs_super_operations devfs_super_operations = {
	.alloc_inode = devfs_alloc_inode,
};

static int devfs_fill_super(struct vfs_superblock *sb)
{
	sb->s_magic = DEVFS_MAGIC;
	sb->s_blocksize = PMM_FRAME_SIZE;
	sb->s_op = &devfs_super_operations;
	return 0;
}

static struct vfs_mount *devfs_mount(struct vfs_file_system_type *fs_type,
									 char *dev_name, char *dir_name)
{
	struct vfs_superblock *sb = kcalloc(1, sizeof(struct vfs_superblock));
	sb->s_blocksize = PMM_FRAME_SIZE;
	sb->mnt_devname = strdup(dev_name);
	sb->s_type = fs_type;
	devfs_fill_super(sb);

	struct vfs_inode *i_root = devfs_get_inode(sb, S_IFDIR);
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

struct vfs_file_system_type devfs_fs_type = {
	.name = "devfs",
	.mount = devfs_mount,
};

void init_devfs()
{
	register_filesystem(&devfs_fs_type);
	do_mount("devfs", MS_NOUSER, "/dev");
	vfs_mknod("/dev/input", S_IFDIR, 0);
	vfs_mknod("/dev/pts", S_IFDIR, 0);
}

void exit_devfs()
{
	unregister_filesystem(&devfs_fs_type);
}
