#ifndef FS_DEVICES_H
#define FS_DEVICES_H

#include <stdint.h>

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
	uint32_t baseminor;
	int32_t minorct;

	dev_t dev;
	struct list_head sibling;
	struct vfs_file_operations *f_ops;
};

#define DECLARE_CHRDEV(_name, _major, _baseminor, _minorct, _ops) \
	{                                                             \
		.name = _name,                                            \
		.major = _major,                                          \
		.baseminor = _baseminor,                                  \
		.minorct = _minorct,                                      \
		.dev = MKDEV(_major, _baseminor),                         \
		.f_ops = _ops,                                            \
	}

struct char_device *alloc_chrdev(const char *name, uint32_t major, uint32_t minor, int32_t minorct, struct vfs_file_operations *ops);
int register_chrdev(struct char_device *cdev);
int unregister_chrdev(dev_t dev);
void chrdev_init();

extern struct vfs_file_operations def_chr_fops;

#endif
