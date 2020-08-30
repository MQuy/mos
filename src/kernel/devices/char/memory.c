#include <include/errno.h>
#include <kernel/fs/char_dev.h>
#include <kernel/utils/math.h>
#include <kernel/utils/printf.h>

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

struct char_device cdev_null = (struct char_device)DECLARE_CHRDEV("null", MEMORY_MAJOR, NULL_DEVICE, 1, &null_fops);

struct char_device cdev_random = (struct char_device)DECLARE_CHRDEV("random", MEMORY_MAJOR, RANDOM_DEVICE, 1, &random_fops);

void chrdev_memory_init()
{
	DEBUG &&debug_println(DEBUG_INFO, "[dev] - Mount null");
	register_chrdev(&cdev_null);
	vfs_mknod("/dev/null", S_IFCHR, cdev_null.dev);

	DEBUG &&debug_println(DEBUG_INFO, "[dev] - Mount random");
	register_chrdev(&cdev_random);
	vfs_mknod("/dev/random", S_IFCHR, cdev_random.dev);
}
