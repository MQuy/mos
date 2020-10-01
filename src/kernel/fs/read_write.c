#include <memory/vmm.h>
#include <proc/task.h>
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

loff_t vfs_flseek(int32_t fd, loff_t offset)
{
	struct vfs_file *file = current_process->files->fd[fd];
	return file->f_op->llseek(file, offset);
}
