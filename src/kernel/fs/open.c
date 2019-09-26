#include <kernel/include/string.h>
#include <kernel/memory/malloc.h>
#include <kernel/system/task.h>
#include "vfs.h"

extern process *current_process;

vfs_dentry *alloc_dentry(vfs_dentry *parent, char *name)
{
  vfs_dentry *d = malloc(sizeof(vfs_dentry));
  d->d_name = name;
  d->d_parent = parent;
  d->d_sb = parent->d_sb;

  int length = 0;
  for (; length < MAX_SUB_DENTRIES && parent->d_subdirs[length]; ++length)
    ;
  parent->d_subdirs[length] = d;

  return d;
}

nameidata *path_walk(char *filename)
{
  nameidata *nd = malloc(sizeof(nameidata));
  nd->dentry = current_process->fs->d_root;
  nd->mnt = current_process->fs->mnt_root;

  for (int i = 1, length = strlen(filename); i < length; ++i)
  {
    char *path = malloc(length);

    for (int j = 0; filename[i] != '/' && i < length; ++i, ++j)
      path[j] = filename[i];

    vfs_dentry *dentry = NULL;
    for (int i = 0; i < MAX_SUB_DENTRIES && dentry == NULL; ++i)
      if (nd->dentry->d_subdirs[i] && strcmp(path, nd->dentry->d_subdirs[i]->d_name) == 0)
        dentry = nd->dentry->d_subdirs[i];

    if (dentry)
    {
      nd->dentry = dentry;
    }
    else
    {
      dentry = alloc_dentry(nd->dentry, path);
      vfs_inode *inode = nd->dentry->d_inode->i_op->lookup(nd->dentry->d_inode, dentry->d_name);
      if (inode == NULL)
      {
        uint32_t mode = S_IFDIR;
        if (i == length)
          mode = S_IFREG;
        inode = nd->dentry->d_inode->i_op->create(nd->dentry->d_inode, dentry->d_name, mode);
      }
      dentry->d_inode = inode;
      nd->dentry = dentry;
    }

    vfs_mount *mnt = lookup_mnt(nd->dentry);
    if (mnt)
      nd->mnt = mnt;
  };
  return nd;
}

long sys_open(char *filename)
{
  int fd = find_unused_fd_slot();
  nameidata *nd = path_walk(filename);

  vfs_file *file = malloc(sizeof(vfs_file));
  file->f_dentry = nd->dentry;
  file->f_vfsmnt = nd->mnt;
  file->f_pos = 0;
  file->f_op = nd->dentry->d_inode->i_fop;

  current_process->files->fd_array[fd] = file;
  return fd;
}