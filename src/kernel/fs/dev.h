#ifndef FS_DEVICES_H
#define FS_DEVICES_H

#include "vfs.h"

// cdev
#define MINORBITS 20
#define MINORMASK ((1U << MINORBITS) - 1)

#define MAJOR(dev) ((unsigned int)((dev) >> MINORBITS))
#define MINOR(dev) ((unsigned int)((dev)&MINORMASK))
#define MKDEV(ma, mi) (((ma) << MINORBITS) | (mi))

struct char_device
{
	const char *name;
	uint32_t major;
	dev_t dev;
	struct list_head sibling;
	struct vfs_file_operations *f_ops;
};

int register_chrdev(struct char_device *dev);
int unregister_chrdev(uint32_t major);
void chrdev_init();

extern struct vfs_file_operations def_chr_fops;

#endif
