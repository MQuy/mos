#include <kernel/utils/string.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include "vfs.h"

extern process *current_process;

char *vfs_read(const char *path)
{
  long fd = vfs_open(path);
  kstat *stat = kmalloc(sizeof(kstat));
  vfs_fstat(fd, stat);
  char *buf = kmalloc(stat->size);
  vfs_fread(fd, buf, stat->size);
  return buf;
}

ssize_t vfs_fread(uint32_t fd, char *buf, size_t count)
{
  vfs_file *file = current_process->files->fd[fd];
  return file->f_op->read(file, buf, count, file->f_pos);
}

int vfs_write(const char *path, const char *buf, size_t count)
{
  long fd = vfs_open(path);
  return vfs_fwrite(fd, buf, count);
}

ssize_t vfs_fwrite(uint32_t fd, const char *buf, size_t count)
{
  vfs_file *file = current_process->files->fd[fd];
  return file->f_op->write(file, buf, count, file->f_pos);
}

loff_t vfs_flseek(uint32_t fd, loff_t offset)
{
  vfs_file *file = current_process->files->fd[fd];
  return file->f_op->llseek(file, offset);
}