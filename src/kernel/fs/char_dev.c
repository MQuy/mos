#include <include/errno.h>
#include "dev.h"

struct list_head devlist;

char_device *get_chrdev(uint32_t major)
{
  char_device *iter = NULL;
  list_for_each_entry(iter, &devlist, sibling)
  {
    if (iter->major == major)
      return iter;
  };
  return NULL;
}

int register_chrdev(char_device *new_dev)
{
  char_device *exist = get_chrdev(new_dev->major);
  if (exist == NULL)
  {
    list_add_tail(&new_dev->sibling, &devlist);
    return 0;
  }
  else
    return -EEXIST;
}

int unregister_chrdev(uint32_t major)
{
  char_device *dev = get_chrdev(major);
  if (dev)
  {
    list_del(&dev->sibling);
    return 0;
  }
  else
    return -ENODEV;
}

int chrdev_open(vfs_inode *inode, vfs_file *filp)
{
  char_device *dev = get_chrdev(MAJOR(inode->i_rdev));
  if (dev == NULL)
    return -ENODEV;

  if (dev->f_ops->open)
  {
    dev->f_ops->open(inode, filp);
    return 0;
  }
  return -EINVAL;
}

vfs_file_operations def_chr_fops = {
    .open = chrdev_open,
};

void chrdev_init()
{
  INIT_LIST_HEAD(&devlist);
}