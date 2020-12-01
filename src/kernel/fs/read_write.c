#include <include/errno.h>
#include <include/fcntl.h>
#include <memory/vmm.h>
#include <proc/task.h>
#include <utils/string.h>

#include "vfs.h"

char *vfs_read(const char *path)
{
	int32_t fd = vfs_open(path, O_RDWR);
	if (fd < 0)
		return NULL;

	struct kstat *stat = kcalloc(1, sizeof(struct kstat));
	vfs_fstat(fd, stat);
	char *buf = kcalloc(stat->st_size, sizeof(char));
	vfs_fread(fd, buf, stat->st_size);
	return buf;
}

ssize_t vfs_fread(int32_t fd, char *buf, size_t count)
{
	struct vfs_file *file = current_process->files->fd[fd];
	if (fd < 0 || !file)
		return -EBADF;

	if (file && file->f_mode & FMODE_CAN_READ)
		return file->f_op->read(file, buf, count, file->f_pos);

	return -EINVAL;
}

ssize_t vfs_fwrite(int32_t fd, const char *buf, size_t count)
{
	struct vfs_file *file = current_process->files->fd[fd];
	if (fd < 0 || !file)
		return -EBADF;

	loff_t ppos = file->f_pos;

	if (file->f_flags & O_APPEND)
		ppos = file->f_dentry->d_inode->i_size;

	if (file->f_mode & FMODE_CAN_WRITE)
		return file->f_op->write(file, buf, count, ppos);

	return -EINVAL;
}

loff_t generic_file_llseek(struct vfs_file *file, loff_t offset, int whence)
{
	struct vfs_inode *inode = file->f_dentry->d_inode;
	loff_t foffset;

	if (whence == SEEK_SET)
		foffset = offset;
	else if (whence == SEEK_CUR)
		foffset = file->f_pos + offset;
	else if (whence == SEEK_END)
		foffset = inode->i_size + offset;
	else
		return -EINVAL;

	file->f_pos = foffset;
	return foffset;
}

loff_t vfs_flseek(int32_t fd, loff_t offset, int whence)
{
	struct vfs_file *file = current_process->files->fd[fd];
	if (fd < 0 || !file)
		return -EBADF;

	if (file && file->f_op && file->f_op->llseek)
		return file->f_op->llseek(file, offset, whence);

	return -EINVAL;
}
