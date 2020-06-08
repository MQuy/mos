#include <include/errno.h>
#include <include/ctype.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include "sockfs.h"

extern struct process *current_process;

// TODO: MQ 2020-06-01 Support socket syscall read
ssize_t sockfs_read_file(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
  return 0;
}

// TODO: MQ 2020-06-01 Support socket syscall write
ssize_t sockfs_write_file(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
  return 0;
}

// TODO: MQ 2020-06-06 Cleanup socket, sock
int sockfs_release_file(struct vfs_inode *inode, struct vfs_file *file)
{
  return 0;
}

struct vfs_file_operations sockfs_file_operations = {
    .read = sockfs_read_file,
    .write = sockfs_write_file,
    .release = sockfs_release_file,
};

struct vfs_file_operations sockfs_dir_operations = {};
