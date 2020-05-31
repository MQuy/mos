#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/time.h>
#include <kernel/proc/task.h>
#include "tmpfs.h"

#define TMPFS_MAGIC 0x01021994

extern struct process *current_process;

struct vfs_inode *tmpfs_get_inode(struct vfs_superblock *sb, uint32_t mode)
{
  struct vfs_inode *i = sb->s_op->alloc_inode(sb);
  i->i_blksize = PMM_FRAME_SIZE;
  i->i_mode = mode;
  i->i_atime.tv_sec = get_seconds(NULL);
  i->i_ctime.tv_sec = get_seconds(NULL);
  i->i_mtime.tv_sec = get_seconds(NULL);

  if (S_ISREG(i->i_mode))
  {
    i->i_op = &tmpfs_file_inode_operations;
    i->i_fop = &tmpfs_file_operations;
  }
  else if (S_ISDIR(i->i_mode))
  {
    i->i_op = &tmpfs_dir_inode_operations;
    i->i_fop = &tmpfs_dir_operations;
  }

  return i;
}

struct vfs_inode *tmpfs_alloc_inode(struct vfs_superblock *sb)
{
  struct vfs_inode *inode = init_inode();
  INIT_LIST_HEAD(&inode->i_data.pages);
  inode->i_sb = sb;

  return inode;
}

void tmpfs_read_inode(struct vfs_inode *i)
{
}

void tmpfs_write_inode(struct vfs_inode *i)
{
}

struct vfs_super_operations tmpfs_super_operations = {
    .alloc_inode = tmpfs_alloc_inode,
    .read_inode = tmpfs_read_inode,
    .write_inode = tmpfs_write_inode,
};

int tmpfs_fill_super(struct vfs_superblock *sb)
{
  struct tmpfs_sb_info *sbinfo = kcalloc(1, sizeof(struct tmpfs_sb_info));
  sbinfo->max_blocks = get_total_frames() / 2;
  sbinfo->free_blocks = sbinfo->max_blocks;
  sbinfo->max_inodes = sbinfo->max_blocks;
  sbinfo->free_inodes = sbinfo->max_inodes;

  sb->s_fs_info = sbinfo;
  sb->s_magic = TMPFS_MAGIC;
  sb->s_blocksize = PMM_FRAME_SIZE;
  sb->s_op = &tmpfs_super_operations;
  return 0;
}

struct vfs_mount *tmpfs_mount(struct vfs_file_system_type *fs_type,
                              char *dev_name, char *dir_name)
{
  struct vfs_mount *mnt = kcalloc(1, sizeof(struct vfs_mount));
  struct vfs_superblock *sb = kcalloc(1, sizeof(struct vfs_superblock));
  sb->s_blocksize = PMM_FRAME_SIZE;
  sb->mnt_devname = dev_name;
  sb->s_type = fs_type;
  tmpfs_fill_super(sb);

  struct vfs_inode *i_root = tmpfs_get_inode(sb, S_IFDIR);
  struct vfs_dentry *d_root = alloc_dentry(NULL, dir_name);
  d_root->d_inode = i_root;
  d_root->d_sb = sb;

  sb->s_root = d_root;

  mnt->mnt_sb = sb;
  mnt->mnt_mountpoint = sb->s_root;
  mnt->mnt_devname = dev_name;

  return mnt;
}

struct vfs_file_system_type tmpfs_fs_type = {
    .name = "tmpfs",
    .mount = tmpfs_mount,
};

void init_tmpfs()
{
  register_filesystem(&tmpfs_fs_type);
  do_mount("tmpfs", MS_NOUSER, "/dev/shm");
}

void exit_tmpfs()
{
  unregister_filesystem(&tmpfs_fs_type);
}
