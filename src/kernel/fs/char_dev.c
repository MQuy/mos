#include "char_dev.h"

#include <include/errno.h>

struct list_head devlist;

struct char_device *get_chrdev(dev_t dev)
{
	struct char_device *iter = NULL;
	list_for_each_entry(iter, &devlist, sibling)
	{
		if (iter->dev == dev)
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

int chrdev_open(struct vfs_inode *inode, struct vfs_file *filp)
{
	struct char_device *cdev = get_chrdev(inode->i_rdev);
	if (cdev == NULL)
		return -ENODEV;

	if (cdev->f_ops->open)
	{
		cdev->f_ops->open(inode, filp);
		return 0;
	}
	return -EINVAL;
}

struct vfs_file_operations def_chr_fops = {
	.open = chrdev_open,
};

void chrdev_init()
{
	INIT_LIST_HEAD(&devlist);
}
