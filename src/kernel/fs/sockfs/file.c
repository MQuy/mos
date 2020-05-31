#include <include/errno.h>
#include <include/ctype.h>
#include <kernel/fs/vfs.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include "sockfs.h"

extern struct process *current_process;

loff_t sockfs_llseek_file(struct vfs_file *file, loff_t ppos)
{
}

ssize_t sockfs_read_file(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
}

ssize_t sockfs_write_file(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
}

int sockfs_mmap_file(struct vfs_file *file, struct vm_area_struct *new_vma)
{
}

struct vfs_file_operations sockfs_file_operations = {
    .llseek = sockfs_llseek_file,
    .read = sockfs_read_file,
    .write = sockfs_write_file,
    .mmap = sockfs_mmap_file,
};

struct vfs_file_operations sockfs_dir_operations = {};