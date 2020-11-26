#include <fs/vfs.h>
#include <include/errno.h>
#include <proc/task.h>
#include <utils/debug.h>

int do_fcntl(int fd, int cmd, unsigned long arg)
{
	struct vfs_file *filp = current_process->files->fd[fd];
	if (!filp)
		return -EBADF;

	int ret = 0;
	switch (cmd)
	{
	case F_DUPFD:
		if ((ret = find_unused_fd_slot(arg)) < 0)
			return -EMFILE;
		current_process->files->fd[ret] = filp;
		break;
	case F_GETFD:
		ret = filp->f_flags;
		break;
	case F_SETFD:
		filp->f_flags = arg;
		break;
	case F_GETFL:
		ret = filp->f_mode;
		break;
	case F_SETFL:
		filp->f_mode = arg;
		break;

	default:
		assert_not_implemented("cmd %d is not supported", cmd);
		break;
	}

	return ret;
}
