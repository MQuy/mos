#ifndef FS_TMPFS_H
#define FS_TMPFS_H

#include <stdint.h>

struct tmpfs_sb_info
{
  unsigned long max_blocks;  /* How many blocks are allowed */
  unsigned long free_blocks; /* How many are left for allocation */
  unsigned long max_inodes;  /* How many inodes are allowed */
  unsigned long free_inodes; /* How many are left for allocation */
};

// super.c
void init_tmpfs();
void exit_tmpfs();
struct vfs_inode *tmpfs_get_inode(struct vfs_superblock *sb, uint32_t mode);

// inode.c
int tmpfs_setsize(struct vfs_inode *i, loff_t new_size);
extern struct vfs_inode_operations tmpfs_dir_inode_operations;
extern struct vfs_inode_operations tmpfs_file_inode_operations;
extern struct vfs_inode_operations tmpfs_special_inode_operations;

// file.c
extern struct vfs_file_operations tmpfs_file_operations;
extern struct vfs_file_operations tmpfs_dir_operations;

#endif