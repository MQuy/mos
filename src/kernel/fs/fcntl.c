#include <fs/vfs.h>
#include <proc/task.h>

int do_fcntl(int fd, int cmd, unsigned long arg)
{
	struct vfs_file *filp = current_process->files->fd[fd];
	int ret = 0;

	switch (cmd)
	{
	case F_GETFL:
		ret = filp->f_flags;
		break;

	default:
		break;
	}

	return ret;
}
