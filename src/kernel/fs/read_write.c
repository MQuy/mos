#include <memory/vmm.h>
#include <proc/task.h>
#include <shared/errno.h>
#include <shared/fcntl.h>
#include <utils/string.h>

#include "vfs.h"

char *vfs_read(const char *path)
{
	int32_t fd = vfs_open(path, O_RDWR);
	struct kstat *stat = kcalloc(1, sizeof(struct kstat));
	vfs_fstat(fd, stat);
	char *buf = kcalloc(stat->size, sizeof(char));
	vfs_fread(fd, buf, stat->size);
	return buf;
}

ssize_t vfs_fread(int32_t fd, char *buf, size_t count)
{
	struct vfs_file *file = current_process->files->fd[fd];
	return file->f_op->read(file, buf, count, file->f_pos);
}

int vfs_write(const char *path, const char *buf, size_t count)
{
	int32_t fd = vfs_open(path, O_RDWR);
	return vfs_fwrite(fd, buf, count);
}

ssize_t vfs_fwrite(int32_t fd, const char *buf, size_t count)
{
	struct vfs_file *file = current_process->files->fd[fd];
	return file->f_op->write(file, buf, count, file->f_pos);
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

	if (file->f_op->llseek)
		return file->f_op->llseek(file, offset, whence);

	return -1;
}
