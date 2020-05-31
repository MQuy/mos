#include <kernel/memory/pmm.h>
#include <kernel/memory/vmm.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/time.h>
#include <kernel/proc/task.h>
#include <kernel/net/net.h>
#include "sockfs.h"

#define SOCKFS_MAGIC 0x534F434B
#define SOCKFS_ROOT "/dev/sockfs"
#define SOCK_NUMBER_LENGTH 11

extern struct process *current_process;

uint32_t nsock;

int32_t get_unused_socket_number()
{
  return nsock++;
}

char *get_next_socket_path()
{
  char *snum_s = kcalloc(1, SOCK_NUMBER_LENGTH);
  itoa_s(get_unused_socket_number(), SOCK_NUMBER_LENGTH, snum_s);
  char *path = kcalloc(1, sizeof(SOCKFS_ROOT) + SOCK_NUMBER_LENGTH);
  memcpy(path, SOCKFS_ROOT, sizeof(SOCKFS_ROOT));
  memcpy(path + sizeof(SOCKFS_ROOT) + 1, snum_s, SOCK_NUMBER_LENGTH);
  return path;
}

struct vfs_inode *sockfs_get_inode(struct vfs_superblock *sb, uint32_t mode)
{
  struct vfs_inode *i = sb->s_op->alloc_inode(sb);
  i->i_blksize = PMM_FRAME_SIZE;
  i->i_mode = mode;
  i->i_atime.tv_sec = get_seconds(NULL);
  i->i_ctime.tv_sec = get_seconds(NULL);
  i->i_mtime.tv_sec = get_seconds(NULL);

  if (S_ISREG(i->i_mode))
  {
    i->i_op = &sockfs_file_inode_operations;
    i->i_fop = &sockfs_file_operations;
  }
  else if (S_ISDIR(i->i_mode))
  {
    i->i_op = &sockfs_dir_inode_operations;
    i->i_fop = &sockfs_dir_operations;
  }

  return i;
}

struct vfs_inode *sockfs_alloc_inode(struct vfs_superblock *sb)
{
  struct socket_alloc *ei = kcalloc(1, sizeof(struct socket_alloc));

  ei->inode.i_blocks = 0;
  ei->inode.i_size = 0;
  sema_init(&ei->inode.i_sem, 1);

  ei->socket.flags = 0;
  ei->socket.state = SS_UNCONNECTED;

  return &ei->inode;
}

void sockfs_read_inode(struct vfs_inode *i)
{
}

void sockfs_write_inode(struct vfs_inode *i)
{
}

struct vfs_super_operations sockfs_super_operations = {
    .alloc_inode = sockfs_alloc_inode,
    .read_inode = sockfs_read_inode,
    .write_inode = sockfs_write_inode,
};

int sockfs_fill_super(struct vfs_superblock *sb)
{
  sb->s_magic = SOCKFS_MAGIC;
  sb->s_blocksize = PMM_FRAME_SIZE;
  sb->s_op = &sockfs_super_operations;
  return 0;
}

struct vfs_mount *sockfs_mount(struct vfs_file_system_type *fs_type,
                               char *dev_name, char *dir_name)
{
  sock_mnt = kcalloc(1, sizeof(struct vfs_mount));
  struct vfs_superblock *sb = kcalloc(1, sizeof(struct vfs_superblock));
  sb->s_blocksize = PMM_FRAME_SIZE;
  sb->mnt_devname = dev_name;
  sb->s_type = fs_type;
  sockfs_fill_super(sb);

  struct vfs_inode *i_root = sockfs_get_inode(sb, S_IFDIR);
  struct vfs_dentry *d_root = alloc_dentry(NULL, dir_name);
  d_root->d_inode = i_root;
  d_root->d_sb = sb;

  sb->s_root = d_root;

  sock_mnt->mnt_sb = sb;
  sock_mnt->mnt_mountpoint = sb->s_root;
  sock_mnt->mnt_devname = dev_name;

  return sock_mnt;
}

struct vfs_mount *sock_mnt;

struct vfs_file_system_type sockfs_fs_type = {
    .name = "sockfs",
    .mount = sockfs_mount,
};

void init_sockfs()
{
  nsock = 0;
  register_filesystem(&sockfs_fs_type);
  do_mount("sockfs", MS_NOUSER, SOCKFS_ROOT);
}

void exit_sockfs()
{
  unregister_filesystem(&sockfs_fs_type);
}
