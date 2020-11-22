#include <fs/vfs.h>
#include <include/errno.h>
#include <include/limits.h>
#include <net/net.h>
#include <proc/task.h>
#include <utils/debug.h>

void vfs_build_path_backward(struct vfs_dentry *dentry, char *path)
{
	if (dentry->d_parent)
	{
		vfs_build_path_backward(dentry->d_parent, path);
		int len = strlen(path);
		int dlen = strlen(dentry->d_name);

		if (path[len - 1] != '/')
		{
			memcpy(path + len, "/", 1);
			memcpy(path + len + 1, dentry->d_name, dlen);
			path[len + 1 + dlen] = 0;
		}
		else
		{
			memcpy(path + len, dentry->d_name, dlen);
			path[len + dlen] = 0;
		}
	}
	else
		strcpy(path, "/");
}

static void absolutize_path_from_process(const char *path, char **abs_path)
{
	if (path[0] != '/')
	{
		*abs_path = kcalloc(MAXPATHLEN, sizeof(char));
		vfs_build_path_backward(current_process->fs->d_root, *abs_path);
		strcpy(*abs_path, "/");
		strcpy(*abs_path, path);
	}
	else
		*abs_path = path;
}

int vfs_unlink(const char *path, int flag)
{
	log("File system: Unlink %s with flag=%d", path, flag);
	char *abs_path;
	absolutize_path_from_process(path, &abs_path);

	int ret = vfs_open(abs_path, O_RDONLY);
	if (ret >= 0)
	{
		struct vfs_file *file = current_process->files->fd[ret];
		if (!file)
			ret = -EBADF;
		else if (flag & AT_REMOVEDIR && file->f_dentry->d_inode->i_mode & S_IFREG)
			ret = -ENOTDIR;
		else
		{
			struct vfs_inode *dir = file->f_dentry->d_parent->d_inode;
			if (dir->i_op && dir->i_op->unlink)
				ret = dir->i_op->unlink(dir, file->f_dentry);
			list_del(&file->f_dentry->d_sibling);
		}
		vfs_close(ret);
	}

	if (abs_path != path)
		kfree(abs_path);

	return ret;
}

int vfs_rename(const char *oldpath, const char *newpath)
{
	log("File system: Rename from %s to %s", oldpath, newpath);
	char *abs_oldpath, *abs_newpath;
	absolutize_path_from_process(oldpath, &abs_oldpath);
	absolutize_path_from_process(newpath, &abs_newpath);

	int oldfd, newfd;
	if ((oldfd = vfs_open(oldpath, O_RDONLY)) < 0)
		return oldfd;

	struct vfs_file *oldfilp = current_process->files->fd[oldfd];
	struct vfs_dentry *old_dentry = oldfilp->f_dentry;
	struct vfs_inode *old_dir = old_dentry->d_parent->d_inode;

	mode_t old_mode = old_dentry->d_inode->i_mode;
	newfd = vfs_open(newpath, O_RDONLY);
	if (newfd >= 0)
	{
		struct kstat old_stat;
		vfs_fstat(oldfd, &old_stat);
		struct kstat new_stat;
		vfs_fstat(newfd, &new_stat);
		struct vfs_file *newfilp = current_process->files->fd[newfd];
		struct vfs_dentry *new_dentry = newfilp->f_dentry;
		mode_t new_mode = newfilp->f_dentry->d_inode->i_mode;

		if (!S_ISDIR(old_mode) && S_ISDIR(new_mode))
			return -EISDIR;
		else if (S_ISDIR(old_mode) && !S_ISDIR(new_mode))
			return -ENOTDIR;
		else if ((S_ISREG(old_mode) == S_ISREG(new_mode) && old_stat.st_ino == new_stat.st_ino) ||
				 (S_ISCHR(old_mode) == S_ISCHR(new_mode) && old_stat.st_rdev == new_stat.st_rdev) ||
				 (S_ISSOCK(old_mode) == S_ISSOCK(new_mode) && SOCKET_I(old_dentry->d_inode) == SOCKET_I(new_dentry->d_inode)))
			return 0;
		else if (S_ISDIR(old_mode) && S_ISDIR(new_mode) && new_stat.st_size > 0)
			return -ENOTEMPTY;

		vfs_unlink(newpath, 0);
	}

	char *new_dirpath = NULL;
	char *new_filename = NULL;
	strlsplat(newpath, strliof(newpath, "/"), &new_dirpath, &new_filename);
	if (!new_dirpath)
		new_dirpath = "/";

	int ret = 0;
	struct nameidata nd;
	if ((ret = path_walk(&nd, new_dirpath, O_RDONLY, S_IFDIR)) >= 0)
	{
		struct vfs_inode *new_dir = nd.dentry->d_inode;
		struct vfs_dentry *new_dentry = alloc_dentry(nd.dentry, new_filename);

		if (oldfilp->f_vfsmnt != nd.mnt)
			ret = -EXDEV;
		else if (old_dir->i_op && old_dir->i_op->rename)
			ret = old_dir->i_op->rename(old_dir, old_dentry, new_dir, new_dentry);
	}
	else
		ret = -ENOENT;

	if (ret >= 0)
	{
		vfs_unlink(oldpath, 0);
		list_del(&old_dentry->d_sibling);
	}

	kfree(new_dirpath);
	kfree(new_filename);

	return ret;
}

int generic_memory_rename(struct vfs_inode *old_dir, struct vfs_dentry *old_dentry,
						  struct vfs_inode *new_dir, struct vfs_dentry *new_dentry)
{
	new_dentry->d_inode = old_dentry->d_inode;
	list_add_tail(&new_dentry->d_sibling, &new_dentry->d_parent->d_subdirs);

	return 0;
}
