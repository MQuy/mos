#include <include/limits.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/utils/string.h>

#include "vfs.h"

struct vfs_dentry *alloc_dentry(struct vfs_dentry *parent, char *name)
{
	struct vfs_dentry *d = kcalloc(1, sizeof(struct vfs_dentry));
	d->d_name = name;
	d->d_parent = parent;
	INIT_LIST_HEAD(&d->d_subdirs);

	if (parent)
		d->d_sb = parent->d_sb;

	return d;
}

struct nameidata *path_walk(const char *path, mode_t mode)
{
	struct nameidata *nd = kcalloc(1, sizeof(struct nameidata));
	nd->dentry = current_process->fs->d_root;
	nd->mnt = current_process->fs->mnt_root;

	for (int i = 1, length = strlen(path); i < length; ++i)
	{
		char *part_name = kcalloc(length, sizeof(char));

		for (int j = 0; path[i] != '/' && i < length; ++i, ++j)
			part_name[j] = path[i];

		struct vfs_dentry *iter = NULL;
		struct vfs_dentry *d_child = NULL;
		list_for_each_entry(iter, &nd->dentry->d_subdirs, d_sibling)
		{
			if (strcmp(part_name, iter->d_name) == 0)
			{
				d_child = iter;
				break;
			}
		}

		if (d_child)
			nd->dentry = d_child;
		else
		{
			d_child = alloc_dentry(nd->dentry, part_name);
			struct vfs_inode *inode = NULL;
			if (nd->dentry->d_inode->i_op->lookup)
				inode = nd->dentry->d_inode->i_op->lookup(nd->dentry->d_inode, d_child->d_name);
			if (inode == NULL)
			{
				uint32_t part_mode = S_IFDIR;
				if (i == length)
					part_mode = mode;
				inode = nd->dentry->d_inode->i_op->create(nd->dentry->d_inode, d_child->d_name, part_mode);
			}
			d_child->d_inode = inode;
			list_add_tail(&d_child->d_sibling, &nd->dentry->d_subdirs);
			nd->dentry = d_child;
		}

		struct vfs_mount *mnt = lookup_mnt(nd->dentry);
		if (mnt)
			nd->mnt = mnt;
	};
	return nd;
}

struct vfs_file *get_empty_filp()
{
	struct vfs_file *file = kcalloc(1, sizeof(struct vfs_file));
	file->f_maxcount = INT_MAX;
	atomic_set(&file->f_count, 1);
	return file;
}

int32_t vfs_open(const char *path, int32_t flags)
{
	int fd = find_unused_fd_slot();
	struct nameidata *nd = path_walk(path, S_IFREG);

	struct vfs_file *file = get_empty_filp();
	file->f_dentry = nd->dentry;
	file->f_vfsmnt = nd->mnt;
	file->f_flags = flags;
	file->f_op = nd->dentry->d_inode->i_fop;

	if (file->f_op && file->f_op->open)
		file->f_op->open(nd->dentry->d_inode, file);

	current_process->files->fd[fd] = file;
	return fd;
}

int32_t vfs_close(int32_t fd)
{
	struct files_struct *files = current_process->files;

	acquire_semaphore(&files->lock);

	struct vfs_file *f = files->fd[fd];
	atomic_dec(&f->f_count);
	if (!atomic_read(&f->f_count) && f->f_op->release)
		f->f_op->release(f->f_dentry->d_inode, f);
	files->fd[fd] = NULL;

	release_semaphore(&files->lock);
	return 0;
}

static void generic_fillattr(struct vfs_inode *inode, struct kstat *stat)
{
	stat->dev = inode->i_sb->s_dev;
	stat->ino = inode->i_ino;
	stat->mode = inode->i_mode;
	stat->nlink = inode->i_nlink;
	stat->uid = inode->i_uid;
	stat->gid = inode->i_gid;
	stat->rdev = inode->i_rdev;
	stat->atime = inode->i_atime;
	stat->mtime = inode->i_mtime;
	stat->ctime = inode->i_ctime;
	stat->size = inode->i_size;
	stat->blocks = inode->i_blocks;
	stat->blksize = inode->i_blksize;
}

static int do_getattr(struct vfs_mount *mnt, struct vfs_dentry *dentry, struct kstat *stat)
{
	struct vfs_inode *inode = dentry->d_inode;
	if (inode->i_op->getattr)
		return inode->i_op->getattr(mnt, dentry, stat);

	generic_fillattr(inode, stat);
	if (!stat->blksize)
	{
		struct vfs_superblock *s = inode->i_sb;
		unsigned blocks;
		blocks = (stat->size + s->s_blocksize - 1) >> s->s_blocksize_bits;
		stat->blocks = (s->s_blocksize / 512) * blocks;
		stat->blksize = s->s_blocksize;
	}
	return 0;
}

int vfs_stat(const char *path, struct kstat *stat)
{
	struct nameidata *nd = path_walk(path, S_IFREG);
	return do_getattr(nd->mnt, nd->dentry, stat);
}

int vfs_fstat(int32_t fd, struct kstat *stat)
{
	struct vfs_file *f = current_process->files->fd[fd];
	return do_getattr(f->f_vfsmnt, f->f_dentry, stat);
}

int vfs_mknod(const char *path, int mode, dev_t dev)
{
	char *dir, *name;
	strlsplat(path, strliof(path, "/"), &dir, &name);

	struct nameidata *nd = path_walk(dir, S_IFDIR);
	struct vfs_dentry *d_child = alloc_dentry(nd->dentry, name);
	int ret = nd->dentry->d_inode->i_op->mknod(nd->dentry->d_inode, d_child, mode, dev);
	list_add_tail(&d_child->d_sibling, &nd->dentry->d_subdirs);

	return ret;
}

static int simple_setattr(struct vfs_dentry *d, struct iattr *attrs)
{
	struct vfs_inode *inode = d->d_inode;

	if (attrs->ia_valid & ATTR_SIZE)
		inode->i_size = attrs->ia_size;
	return 0;
}

static int do_truncate(struct vfs_dentry *dentry, int32_t length)
{
	struct vfs_inode *inode = dentry->d_inode;
	struct iattr *attrs = kcalloc(1, sizeof(struct iattr));
	attrs->ia_valid = ATTR_SIZE;
	attrs->ia_size = length;

	if (inode->i_op->setattr)
		inode->i_op->setattr(dentry, attrs);
	else
		simple_setattr(dentry, attrs);
	return 0;
}

int vfs_truncate(const char *path, int32_t length)
{
	struct nameidata *nd = path_walk(path, S_IFREG);
	return do_truncate(nd->dentry, length);
}

int vfs_ftruncate(int32_t fd, int32_t length)
{
	struct vfs_file *f = current_process->files->fd[fd];
	return do_truncate(f->f_dentry, length);
}
