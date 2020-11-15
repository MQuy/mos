#include "vfs.h"

#include <include/errno.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <utils/printf.h>
#include <utils/string.h>

#include "char_dev.h"
#include "devfs/devfs.h"
#include "ext2/ext2.h"
#include "mqueuefs/mqueuefs.h"
#include "sockfs/sockfs.h"
#include "tmpfs/tmpfs.h"

static struct vfs_file_system_type *file_systems;
struct list_head vfsmntlist;

static struct vfs_file_system_type **find_filesystem(const char *name)
{
	struct vfs_file_system_type **p;
	for (p = &file_systems; *p; p = &(*p)->next)
	{
		if (strcmp((*p)->name, name) == 0)
			break;
	}
	return p;
}

int register_filesystem(struct vfs_file_system_type *fs)
{
	struct vfs_file_system_type **p = find_filesystem(fs->name);

	if (*p)
		return -EBUSY;
	else
		*p = fs;

	return 0;
}

int unregister_filesystem(struct vfs_file_system_type *fs)
{
	struct vfs_file_system_type **p;
	for (p = &file_systems; *p; p = &(*p)->next)
		if (strcmp((*p)->name, fs->name) == 0)
		{
			*p = (*p)->next;
			return 0;
		}
	return -EINVAL;
}

int find_unused_fd_slot(int lowerlimit)
{
	for (int i = lowerlimit; i < 256; ++i)
		if (!current_process->files->fd[i])
			return i;

	return -EINVAL;
}

struct vfs_inode *init_inode()
{
	struct vfs_inode *i = kcalloc(1, sizeof(struct vfs_inode));
	i->i_blocks = 0;
	i->i_size = 0;
	sema_init(&i->i_sem, 1);

	return i;
}

void init_special_inode(struct vfs_inode *inode, umode_t mode, dev_t dev)
{
	inode->i_mode = mode;
	if (S_ISCHR(mode))
	{
		inode->i_fop = &def_chr_fops;
		inode->i_rdev = dev;
	}
}

struct vfs_mount *lookup_mnt(struct vfs_dentry *d)
{
	struct vfs_mount *iter;
	list_for_each_entry(iter, &vfsmntlist, sibling)
	{
		if (iter->mnt_mountpoint == d)
			return iter;
	}

	return NULL;
}

struct vfs_mount *do_mount(const char *fstype, int flags, const char *path)
{
	char *dir = NULL;
	char *name = NULL;
	strlsplat(path, strliof(path, "/"), &dir, &name);
	if (!dir)
		dir = "/";

	struct vfs_file_system_type *fs = *find_filesystem(fstype);
	struct vfs_mount *mnt = fs->mount(fs, fstype, name);

	struct nameidata nd;
	path_walk(&nd, dir, O_RDONLY, S_IFDIR);

	struct vfs_dentry *iter, *next;
	list_for_each_entry_safe(iter, next, &nd.dentry->d_subdirs, d_sibling)
	{
		if (!strcmp(iter->d_name, name))
		{
			// TODO: MQ 2020-10-24 Make sure path is empty folder
			list_del(&iter->d_sibling);
			kfree(iter);
		}
	}

	mnt->mnt_mountpoint->d_parent = nd.dentry;
	list_add_tail(&mnt->mnt_mountpoint->d_sibling, &nd.dentry->d_subdirs);
	list_add_tail(&mnt->sibling, &vfsmntlist);

	return mnt;
}

static void init_rootfs(struct vfs_file_system_type *fs_type, char *dev_name)
{
	init_ext2_fs();

	struct vfs_mount *mnt = fs_type->mount(fs_type, dev_name, "/");
	list_add_tail(&mnt->sibling, &vfsmntlist);

	current_process->fs->d_root = mnt->mnt_root;
	current_process->fs->mnt_root = mnt;
}

// NOTE: MQ 2019-07-24
// we use device mounted name as identifier https://en.wikibooks.org/wiki/Guide_to_Unix/Explanations/Filesystems_and_Swap#Disk_Partitioning
void vfs_init(struct vfs_file_system_type *fs, char *dev_name)
{
	DEBUG &&debug_println(DEBUG_INFO, "VFS: Initializing");

	INIT_LIST_HEAD(&vfsmntlist);

	DEBUG &&debug_println(DEBUG_INFO, "VFS: Mount ext2");
	init_rootfs(fs, dev_name);

	DEBUG &&debug_println(DEBUG_INFO, "VFS: Mount devfs");
	init_devfs();

	DEBUG &&debug_println(DEBUG_INFO, "VFS: Mount mqueuefs");
	init_mqueuefs();

	DEBUG &&debug_println(DEBUG_INFO, "VFS: Mount tmpfs");
	init_tmpfs();

	DEBUG &&debug_println(DEBUG_INFO, "VFS: Mount sockfs");
	init_sockfs();

	DEBUG &&debug_println(DEBUG_INFO, "VFS: Mount chrdev");
	chrdev_init();

	DEBUG &&debug_println(DEBUG_INFO, "VFS: Done");
}
