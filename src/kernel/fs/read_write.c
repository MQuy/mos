#include <kernel/include/string.h>
#include <kernel/memory/pmm.h>
#include <kernel/system/task.h>
#include "vfs.h"

extern process *current_process;

ssize_t sys_read(uint32_t fd, char *buf, size_t count)
{
  vfs_file *file = current_process->files->fd_array[fd];
  return file->f_op->read(file, buf, count, file->f_pos);
}

ssize_t sys_write(uint32_t fd, char *buf, size_t count)
{
  vfs_file *file = current_process->files->fd_array[fd];
  return file->f_op->write(file, buf, count, file->f_pos);
}

loff_t sys_lseek(uint32_t fd, loff_t offset)
{
  vfs_file *file = current_process->files->fd_array[fd];
  return file->f_op->llseek(file, offset);
}