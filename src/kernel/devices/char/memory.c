#include <include/errno.h>
#include <kernel/utils/math.h>
#include <kernel/fs/dev.h>

#define MEMORY_MAJOR 1
#define NULL_DEVICE 3
#define RANDOM_DEVICE 8

extern struct vfs_file_operations def_chr_fops;

int null_open(struct vfs_inode *inode, struct vfs_file *filp)
{
  return 0;
}

int null_release(struct vfs_inode *inode, struct vfs_file *filp)
{
  return 0;
}

loff_t null_llseek(struct vfs_file *file, loff_t ppos)
{
  return 0;
}

ssize_t null_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
  return 0;
}
ssize_t null_write(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
  return 0;
}

struct vfs_file_operations null_fops = {
    .llseek = null_llseek,
    .read = null_read,
    .write = null_write,
    .open = null_open,
    .release = null_release,
};

int random_open(struct vfs_inode *inode, struct vfs_file *filp)
{
  return 0;
}

int random_release(struct vfs_inode *inode, struct vfs_file *filp)
{
  return 0;
}

loff_t random_llseek(struct vfs_file *file, loff_t ppos)
{
  return ppos;
}

ssize_t random_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{

  *(uint32_t *)buf = rand();
  return count;
}
ssize_t random_write(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
  return count;
}

struct vfs_file_operations random_fops = {
    .llseek = random_llseek,
    .read = random_read,
    .write = random_write,
    .open = random_open,
    .release = random_release,
};

int memory_open(struct vfs_inode *inode, struct vfs_file *filp)
{
  switch (MINOR(inode->i_rdev))
  {
  case NULL_DEVICE:
    filp->f_op = &null_fops;
    return 0;
  case RANDOM_DEVICE:
    filp->f_op = &random_fops;
    return 0;
  }
  return -EINVAL;
}

struct vfs_file_operations memory_fops = {
    .open = memory_open,
};

struct char_device chrdev_memory = {
    .name = "memory",
    .major = MEMORY_MAJOR,
    .f_ops = &memory_fops,
};

void chrdev_memory_init()
{
  register_chrdev(&chrdev_memory);

  vfs_mknod("/dev/null", S_IFCHR, MKDEV(MEMORY_MAJOR, NULL_DEVICE));
  vfs_mknod("/dev/random", S_IFCHR, MKDEV(MEMORY_MAJOR, RANDOM_DEVICE));
}
