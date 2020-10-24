#include "char_dev.h"

#include <include/errno.h>
#include <memory/vmm.h>
#include <utils/string.h>

struct list_head devlist;

struct char_device *alloc_chrdev(const char *name, uint32_t major, uint32_t minor, int32_t minorct, struct vfs_file_operations *ops)
{
	struct char_device *cdev = kcalloc(1, sizeof(struct char_device));
	cdev->name = strdup(name);
	cdev->major = major;
	cdev->baseminor = minor;
	cdev->minorct = minorct;
	cdev->dev = MKDEV(major, minor);
	cdev->f_ops = ops;

	return cdev;
}

static struct char_device *get_chrdev(dev_t dev)
{
	struct char_device *iter = NULL;
	list_for_each_entry(iter, &devlist, sibling)
	{
		if (iter->dev == dev || (MKDEV(iter->major, iter->baseminor) <= dev && dev < MKDEV(iter->major, iter->baseminor + iter->minorct)))
			return iter;
	};
	return NULL;
}

int register_chrdev(struct char_device *new_cdev)
{
	struct char_device *exist = get_chrdev(new_cdev->dev);
	if (exist == NULL)
	{
		list_add_tail(&new_cdev->sibling, &devlist);
		return 0;
	}
	else
		return -EEXIST;
}

int unregister_chrdev(dev_t dev)
{
	struct char_device *cdev = get_chrdev(dev);
	if (cdev)
	{
		list_del(&cdev->sibling);
		return 0;
	}
	else
		return -ENODEV;
}

static int chrdev_open(struct vfs_inode *inode, struct vfs_file *filp)
{
	struct char_device *cdev = get_chrdev(inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->open)
		return cdev->f_ops->open(inode, filp);

	return -EINVAL;
}

static ssize_t chrdev_read(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	struct char_device *cdev = get_chrdev(file->f_dentry->d_inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->read)
		return cdev->f_ops->read(file, buf, count, ppos);

	return -EINVAL;
}

static ssize_t chrdev_write(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
	struct char_device *cdev = get_chrdev(file->f_dentry->d_inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->write)
		return cdev->f_ops->write(file, buf, count, ppos);

	return -EINVAL;
}

static unsigned int chrdev_poll(struct vfs_file *file, struct poll_table *pt)
{
	struct char_device *cdev = get_chrdev(file->f_dentry->d_inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->poll)
		return cdev->f_ops->poll(file, pt);

	return -EINVAL;
}

static int chrdev_ioctl(struct vfs_inode *inode, struct vfs_file *file, unsigned int cmd, unsigned long arg)
{
	struct char_device *cdev = get_chrdev(file->f_dentry->d_inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->ioctl)
		return cdev->f_ops->ioctl(inode, file, cmd, arg);

	return -EINVAL;
}

static int chrdev_release(struct vfs_inode *inode, struct vfs_file *file)
{
	struct char_device *cdev = get_chrdev(inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->release)
		return cdev->f_ops->release(inode, file);

	return -EINVAL;
}

loff_t chrdev_llseek(struct vfs_file *file, loff_t ppos, int whence)
{
	struct char_device *cdev = get_chrdev(file->f_dentry->d_inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->llseek)
		return cdev->f_ops->llseek(file, ppos, whence);

	return -EINVAL;
}

struct vfs_file_operations def_chr_fops = {
	.open = chrdev_open,
	.read = chrdev_read,
	.write = chrdev_write,
	.llseek = chrdev_llseek,
	.poll = chrdev_poll,
	.ioctl = chrdev_ioctl,
	.release = chrdev_release,
};

void chrdev_init()
{
	INIT_LIST_HEAD(&devlist);
}
