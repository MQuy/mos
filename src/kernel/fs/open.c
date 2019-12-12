#include <kernel/utils/string.h>
#include <kernel/memory/malloc.h>
#include <kernel/proc/task.h>
#include "vfs.h"

extern process *current_process;

vfs_dentry *alloc_dentry(vfs_dentry *parent, char *name)
{
  vfs_dentry *d = malloc(sizeof(vfs_dentry));
  d->d_name = name;
  d->d_parent = parent;
  INIT_LIST_HEAD(&d->d_subdirs);
  if (parent)
  {
    d->d_sb = parent->d_sb;
    list_add_tail(&d->d_sibling, &parent->d_subdirs);
  }

  return d;
}

nameidata *path_walk(char *path)
{
  nameidata *nd = malloc(sizeof(nameidata));
  nd->dentry = current_process->fs->d_root;
  nd->mnt = current_process->fs->mnt_root;

  for (int i = 1, length = strlen(path); i < length; ++i)
  {
    char *part_name = malloc(length);

    for (int j = 0; path[i] != '/' && i < length; ++i, ++j)
      part_name[j] = path[i];

    vfs_dentry *iter = NULL;
    vfs_dentry *d_child = NULL;
    list_for_each_entry(iter, &nd->dentry->d_subdirs, d_sibling)
    {
      if (strcmp(part_name, iter->d_name) == 0)
      {
        d_child = iter;
        break;
      }
    }

    if (d_child)
    {
      nd->dentry = d_child;
    }
    else
    {
      d_child = alloc_dentry(nd->dentry, part_name);
      vfs_inode *inode = nd->dentry->d_inode->i_op->lookup(nd->dentry->d_inode, d_child->d_name);
      if (inode == NULL)
      {
        uint32_t mode = S_IFDIR;
        if (i == length)
          mode = S_IFREG;
        inode = nd->dentry->d_inode->i_op->create(nd->dentry->d_inode, d_child->d_name, mode);
      }
      d_child->d_inode = inode;
      nd->dentry = d_child;
    }

    vfs_mount *mnt = lookup_mnt(nd->dentry);
    if (mnt)
      nd->mnt = mnt;
  };
  return nd;
}

long vfs_open(char *path)
{
  int fd = find_unused_fd_slot();
  nameidata *nd = path_walk(path);

  vfs_file *file = malloc(sizeof(vfs_file));
  file->f_dentry = nd->dentry;
  file->f_vfsmnt = nd->mnt;
  file->f_pos = 0;
  file->f_op = nd->dentry->d_inode->i_fop;

  if (file->f_op && file->f_op->open)
  {
    file->f_op->open(nd->dentry->d_inode, file);
  }

  current_process->files->fd_array[fd] = file;
  return fd;
}

void generic_fillattr(vfs_inode *inode, struct kstat *stat)
{
  stat->dev = inode->i_sb->s_dev;
  stat->ino = inode->i_ino;
  stat->mode = inode->i_mode;
  stat->nlink = inode->i_nlink;
  stat->uid = inode->i_uid;
  stat->gid = inode->i_gid;
  stat->rdev = inode->i_rdev;
  stat->atime = inode->i_atime;
  stat->mtime = inode->i_mtime;
  stat->ctime = inode->i_ctime;
  stat->size = inode->i_size;
  stat->blocks = inode->i_blocks;
  stat->blksize = inode->i_blksize;
}

int vfs_getattr(vfs_mount *mnt, vfs_dentry *dentry, kstat *stat)
{
  vfs_inode *inode = dentry->d_inode;
  if (inode->i_op->getattr)
    return inode->i_op->getattr(mnt, dentry, stat);

  generic_fillattr(inode, stat);
  if (!stat->blksize)
  {
    vfs_superblock *s = inode->i_sb;
    unsigned blocks;
    blocks = (stat->size + s->s_blocksize - 1) >> s->s_blocksize_bits;
    stat->blocks = (s->s_blocksize / 512) * blocks;
    stat->blksize = s->s_blocksize;
  }
}

void vfs_stat(char *path, kstat *stat)
{
  nameidata *nd = path_walk(path);
  vfs_getattr(nd->mnt, nd->dentry, stat);
}

void vfs_fstat(uint32_t fd, kstat *stat)
{
  vfs_file *f = current_process->files->fd_array[fd];
  vfs_getattr(f->f_vfsmnt, f->f_dentry, stat);
}

int vfs_mknod(char *path, int mode, dev_t dev)
{
  uint32_t length = strlen(path);
  uint32_t last_index = length - 1;
  for (; last_index >= 0 && path[last_index] != '/'; last_index--)
    ;
  char *dir_path = calloc(last_index + 1, sizeof(char));
  char *dev_name = calloc(length - last_index, sizeof(char));

  memcpy(dir_path, path, last_index);
  memcpy(dev_name, path + last_index + 1, length - 1 - last_index);

  nameidata *nd = path_walk(dir_path);
  return nd->dentry->d_inode->i_op->mknod(nd->dentry->d_inode, dev_name, mode, dev);
}