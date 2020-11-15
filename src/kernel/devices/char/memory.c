#include <fs/char_dev.h>
#include <include/errno.h>
#include <utils/debug.h>
#include <utils/math.h>

#define MEMORY_MAJOR 1
#define NULL_DEVICE 3
#define RANDOM_DEVICE 8

extern struct vfs_file_operations def_chr_fops;

static int null_open(struct vfs_inode *inode, struct vfs_file *filp)
{
	return 0;
}

static int null_release(struct vfs_inode *inode, struct vfs_file *filp)
{
	return 0;
}

static loff_t null_llseek(struct vfs_file *file, loff_t ppos, int whence)
{
	return 0;
}

static ssize_t null_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	return 0;
}

static ssize_t null_write(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
	return 0;
}

static struct vfs_file_operations null_fops = {
	.llseek = null_llseek,
	.read = null_read,
	.write = null_write,
	.open = null_open,
	.release = null_release,
};

static int random_open(struct vfs_inode *inode, struct vfs_file *filp)
{
	return 0;
}

static int random_release(struct vfs_inode *inode, struct vfs_file *filp)
{
	return 0;
}

static loff_t random_llseek(struct vfs_file *file, loff_t ppos, int whence)
{
	return ppos;
}

static ssize_t random_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	*(uint32_t *)buf = rand();
	return count;
}

static ssize_t random_write(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
	return count;
}

static struct vfs_file_operations random_fops = {
	.llseek = random_llseek,
	.read = random_read,
	.write = random_write,
	.open = random_open,
	.release = random_release,
};

static struct char_device cdev_null = (struct char_device)DECLARE_CHRDEV("null", MEMORY_MAJOR, NULL_DEVICE, 1, &null_fops);

static struct char_device cdev_random = (struct char_device)DECLARE_CHRDEV("random", MEMORY_MAJOR, RANDOM_DEVICE, 1, &random_fops);

void chrdev_memory_init()
{
	log("Devfs: Mount null");
	register_chrdev(&cdev_null);
	vfs_mknod("/dev/null", S_IFCHR, cdev_null.dev);

	log("Devfs: Mount random");
	register_chrdev(&cdev_random);
	vfs_mknod("/dev/random", S_IFCHR, cdev_random.dev);
}
