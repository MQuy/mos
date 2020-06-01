#ifndef FS_SOCKFS_H
#define FS_SOCKFS_H

#include <stdint.h>

// super.c
extern struct vfs_mount *sock_mnt;
void init_sockfs();
void exit_sockfs();
int32_t get_unused_socket_number();
char *get_next_socket_path();
struct vfs_inode *sockfs_get_inode(struct vfs_superblock *sb, uint32_t mode);

// inode.c
extern struct vfs_inode_operations sockfs_dir_inode_operations;
extern struct vfs_inode_operations sockfs_file_inode_operations;

// file.c
extern struct vfs_file_operations sockfs_file_operations;
extern struct vfs_file_operations sockfs_dir_operations;

#endif
