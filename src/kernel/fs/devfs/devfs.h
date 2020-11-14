#ifndef FS_DEVFS_H
#define FS_DEVFS_H

#include <stdint.h>

// super.c
struct vfs_inode *devfs_get_inode(struct vfs_superblock *sb, uint32_t mode);
void init_devfs();
void exit_devfs();

// inode.c
extern struct vfs_inode_operations devfs_dir_inode_operations;
extern struct vfs_inode_operations devfs_file_inode_operations;
extern struct vfs_inode_operations devfs_special_inode_operations;

// file.c
extern struct vfs_file_operations devfs_dir_operations;
extern struct vfs_file_operations devfs_file_operations;

#endif
