#ifndef FS_TMPFS_H
#define FS_TMPFS_H

#include <stdint.h>

typedef struct tmpfs_sb_info
{
  unsigned long max_blocks;  /* How many blocks are allowed */
  unsigned long free_blocks; /* How many are left for allocation */
  unsigned long max_inodes;  /* How many inodes are allowed */
  unsigned long free_inodes; /* How many are left for allocation */
} tmpfs_sb_info;

// super.c
void init_tmpfs();
void exit_tmpfs();
vfs_inode *tmpfs_get_inode(vfs_superblock *sb, uint32_t mode);

// inode.c
int tmpfs_setsize(vfs_inode *i, loff_t new_size);
extern vfs_inode_operations tmpfs_dir_inode_operations;
extern vfs_inode_operations tmpfs_file_inode_operations;
extern vfs_inode_operations tmpfs_special_inode_operations;

// file.c
extern vfs_file_operations tmpfs_file_operations;
extern vfs_file_operations tmpfs_dir_operations;

#endif