#ifndef FS_MQUEUEFS_H
#define FS_MQUEUEFS_H

#include <stdint.h>

// super.c
void init_mqueuefs();
void exit_mqueuefs();
struct vfs_inode *mqueuefs_get_inode(struct vfs_superblock *sb, uint32_t mode);

// inode.c
extern struct vfs_inode_operations mqueuefs_dir_inode_operations;
extern struct vfs_inode_operations mqueuefs_file_inode_operations;

// file.c
extern struct vfs_file_operations mqueuefs_file_operations;
extern struct vfs_file_operations mqueuefs_dir_operations;

#endif
