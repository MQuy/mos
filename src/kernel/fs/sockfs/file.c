#include <net/net.h>

#include "sockfs.h"

// TODO: MQ 2020-06-01 Support socket syscall read
static ssize_t sockfs_read_file(struct vfs_file *file, char *buf, size_t count, loff_t ppos)
{
	return 0;
}

// TODO: MQ 2020-06-01 Support socket syscall write
static ssize_t sockfs_write_file(struct vfs_file *file, const char *buf, size_t count, loff_t ppos)
{
	return 0;
}

// TODO: MQ 2020-06-06 Cleanup socket, sock
static int sockfs_release_file(struct vfs_inode *inode, struct vfs_file *file)
{
	return 0;
}

int sockfs_ioctl(struct vfs_inode *inode, struct vfs_file *file, unsigned int cmd, unsigned long arg)
{
	struct socket *sock = SOCKET_I(inode);

	if (sock->ops->ioctl)
		return sock->ops->ioctl(sock, cmd, arg);

	return 0;
}

struct vfs_file_operations sockfs_file_operations = {
	.read = sockfs_read_file,
	.write = sockfs_write_file,
	.release = sockfs_release_file,
	.ioctl = sockfs_ioctl,
};

struct vfs_file_operations sockfs_dir_operations = {
	.readdir = generic_memory_readdir,
};
