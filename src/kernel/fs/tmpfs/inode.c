#include <fs/vfs.h>
#include <include/errno.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <system/time.h>

#include "tmpfs.h"

int tmpfs_setsize(struct vfs_inode *inode, loff_t new_size)
{
	uint32_t aligned_new_size = PAGE_ALIGN(new_size);
	uint32_t aligned_size = PAGE_ALIGN(inode->i_size);
	if (aligned_size < aligned_new_size)
	{
		uint32_t extended_frames = (aligned_new_size - aligned_size) / PMM_FRAME_SIZE;
		for (uint32_t i = 0; i < extended_frames; ++i)
		{
			struct page *p = kcalloc(1, sizeof(struct page));
			p->frame = (uint32_t)pmm_alloc_block();
			list_add_tail(&p->sibling, &inode->i_data.pages);
		}
	}
	else if (aligned_size > aligned_new_size)
	{
		uint32_t shrink_frames = (aligned_size - aligned_new_size) / PMM_FRAME_SIZE;
		for (uint32_t i = 0; i < shrink_frames; ++i)
			list_del(inode->i_data.pages.prev);
	}
	inode->i_data.npages = aligned_new_size / PMM_FRAME_SIZE;
	inode->i_size = new_size;
	return 0;
}

static int tmpfs_mknod(struct vfs_inode *dir, struct vfs_dentry *dentry, int mode, dev_t dev)
{
	struct vfs_inode *i = tmpfs_get_inode(dir->i_sb, mode);
	uint32_t current_seconds = get_seconds(NULL);
	i->i_ctime.tv_sec = current_seconds;
	i->i_mtime.tv_sec = current_seconds;
	i->i_atime.tv_sec = current_seconds;
	i->i_rdev = dev;

	dentry->d_inode = i;
	dir->i_mtime.tv_sec = current_seconds;
	return 0;
}

static struct vfs_inode *tmpfs_create_inode(struct vfs_inode *dir, struct vfs_dentry *dentry, mode_t mode)
{
	return tmpfs_get_inode(dir->i_sb, mode | S_IFREG);
}

static int tmpfs_setattr(struct vfs_dentry *d, struct iattr *attrs)
{
	struct vfs_inode *i = d->d_inode;
	if (attrs->ia_valid & ATTR_SIZE && attrs->ia_size != i->i_size)
		tmpfs_setsize(i, attrs->ia_size);
	return 0;
}

struct vfs_inode_operations tmpfs_file_inode_operations = {
	.setattr = tmpfs_setattr};

struct vfs_inode_operations tmpfs_dir_inode_operations = {
	.create = tmpfs_create_inode,
	.mknod = tmpfs_mknod,
};

struct vfs_inode_operations tmpfs_special_inode_operations = {};
