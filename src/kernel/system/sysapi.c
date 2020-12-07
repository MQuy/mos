#include "sysapi.h"

#include <cpu/hal.h>
#include <devices/char/tty.h>
#include <fs/pipefs/pipe.h>
#include <fs/sockfs/sockfs.h>
#include <fs/vfs.h>
#include <include/errno.h>
#include <include/fcntl.h>
#include <include/limits.h>
#include <include/mman.h>
#include <include/utsname.h>
#include <ipc/message_queue.h>
#include <ipc/signal.h>
#include <net/net.h>
#include <proc/elf.h>
#include <proc/task.h>
#include <system/time.h>
#include <utils/debug.h>
#include <utils/string.h>

extern volatile uint64_t jiffies;

static int interpret_path_from_fd(int dirfd, const char *path, char **interpreted_path)
{
	if (path[0] == '/' || (path[0] != '/' && dirfd == AT_FDCWD))
		*interpreted_path = path;
	else
	{
		struct vfs_file *df = current_process->files->fd[dirfd];
		if (!df)
			return -EBADF;
		if (!(df->f_dentry->d_inode->i_mode & S_IFDIR))
			return -ENOTDIR;

		*interpreted_path = kcalloc(MAXPATHLEN, sizeof(char));
		vfs_build_path_backward(df->f_dentry, *interpreted_path);
		strcpy(*interpreted_path, "/");
		strcpy(*interpreted_path, path);
	}
	return 0;
}

static void sys_exit(int32_t code)
{
	do_exit(code & 0xff);
}

pid_t sys_fork()
{
	struct process *child = process_fork(current_process);
	queue_thread(child->thread);

	return child->pid;
}

static int32_t sys_waitid(idtype_t idtype, id_t id, struct infop *infop, int options)
{
	int ret = do_wait(idtype, id, infop, options);
	return ret >= 0 ? 0 : ret;
}

static int32_t sys_read(uint32_t fd, char *buf, size_t count)
{
	return vfs_fread(fd, buf, count);
}

static int32_t sys_write(uint32_t fd, char *buf, size_t count)
{
	return vfs_fwrite(fd, buf, count);
}

static int32_t sys_open(const char *path, int32_t flags, mode_t mode)
{
	return vfs_open(path, flags, mode);
}

static int32_t sys_fstat(int32_t fd, struct kstat *stat)
{
	return vfs_fstat(fd, stat);
}

static int32_t sys_stat(const char *path, struct kstat *stat)
{
	return vfs_stat(path, stat);
}

static int32_t sys_close(uint32_t fd)
{
	return vfs_close(fd);
}

static int32_t sys_lseek(int fd, off_t offset, int whence)
{
	return vfs_flseek(fd, offset, whence);
}

static int32_t sys_waitpid(pid_t pid, int *wstatus, int options)
{
	idtype_t idtype;
	if (pid < -1)
	{
		idtype = P_PGID;
		pid = -pid;
	}
	else if (pid == -1)
		idtype = P_ALL;
	else if (pid == 0)
	{
		idtype = P_PGID;
		pid = current_process->gid;
	}
	else
		idtype = P_PID;

	if (options & WUNTRACED)
	{
		options &= ~WUNTRACED;
		options |= WEXITED | WSTOPPED;
	}

	struct infop ifp;
	int ret = do_wait(idtype, pid, &ifp, options);
	if (ret <= 0)
		return ret;

	if (wstatus)
	{
		if (ifp.si_code & CLD_EXITED)
			*wstatus = WSEXITED | (ifp.si_status << 8);
		else if (ifp.si_code & CLD_KILLED || ifp.si_code & CLD_DUMPED)
		{
			*wstatus = WSSIGNALED;
			if (ifp.si_code & CLD_KILLED)
				*wstatus |= (ifp.si_status << 8);
			if (ifp.si_code & CLD_DUMPED)
				*wstatus |= WSCOREDUMP;
		}
		else if (ifp.si_code & CLD_STOPPED)
			*wstatus = WSSTOPPED | (ifp.si_status << 8);
		else if (ifp.si_code & CLD_CONTINUED)
			*wstatus = WSCONTINUED;
	}

	return ifp.si_pid;
}

static int32_t sys_rename(const char *oldpath, const char *newpath)
{
	return vfs_rename(oldpath, newpath);
}

static int32_t sys_renameat(int olddirfd, const char *oldpath,
							int newdirfd, const char *newpath)
{
	int ret;
	char *interpreted_oldpath, *interpreted_newpath;
	if ((ret = interpret_path_from_fd(olddirfd, oldpath, &interpreted_oldpath)) >= 0 &&
		(ret = interpret_path_from_fd(newdirfd, newpath, &interpreted_newpath)) >= 0)
		ret = vfs_rename(interpreted_oldpath, interpreted_newpath);

	if (interpreted_oldpath != oldpath)
		kfree(interpreted_oldpath);
	if (interpreted_newpath != newpath)
		kfree(interpreted_newpath);
	return ret;
}

static int32_t sys_fchmod(int fildes, mode_t mode)
{
	struct vfs_file *filp = current_process->files->fd[fildes];
	if (!filp)
		return -EBADF;

	struct iattr newattrs;
	newattrs.ia_mode = (mode & S_IALLUGO) || (filp->f_dentry->d_inode->i_mode & ~S_IALLUGO);
	newattrs.ia_valid = ATTR_MODE;

	return vfs_setattr(filp->f_dentry, &newattrs);
}

static int32_t sys_chmod(const char *path, mode_t mode)
{
	int ret = vfs_open(path, O_RDONLY);
	if (ret < 0)
		return ret;

	return sys_fchmod(ret, mode);
}

static int32_t sys_mknodat(int fd, const char *path, mode_t mode, dev_t dev)
{
	char *interpreted_path;
	int ret = interpret_path_from_fd(fd, path, &interpreted_path);

	if (ret >= 0)
		ret = vfs_mknod(interpreted_path, mode, dev);

	if (interpreted_path != path)
		kfree(interpreted_path);
	return ret;
}

static int32_t sys_mknod(const char *path, mode_t mode, dev_t dev)
{
	return vfs_mknod(path, mode, dev);
}

static int32_t sys_getcwd(char *buf, size_t size)
{
	if (!buf || !size)
		return -EINVAL;

	char *abs_path = kcalloc(MAXPATHLEN, sizeof(char));
	vfs_build_path_backward(current_process->fs->d_root, abs_path);

	int32_t ret = (int32_t)buf;
	int plen = strlen(abs_path);
	if (plen < size)
		memcpy(buf, abs_path, plen + 1);
	else
		ret = -ERANGE;

	kfree(abs_path);
	return ret;
}

static int32_t sys_getdents(unsigned int fd, struct dirent *dirent, unsigned int count)
{
	struct vfs_file *file = current_process->files->fd[fd];

	if (!file)
		return -EBADF;

	if (file->f_op->readdir)
		return file->f_op->readdir(file, dirent, count);

	return -ENOTDIR;
}

static int32_t sys_time(time_t *tloc)
{
	time_t t = get_seconds(NULL);
	if (tloc)
		*tloc = t;
	return t;
}

static int32_t sys_times(struct tms *buffer)
{
	// TODO: MQ 2020-10-29 How do we calculate these values
	buffer->tms_stime = buffer->tms_utime = 0;
	buffer->tms_cstime = buffer->tms_cutime = 0;

	return jiffies;
}

static int32_t sys_gettimeofday(struct timeval *restrict tp, void *restrict tzp)
{
	uint64_t ms = get_milliseconds_since_epoch();
	tp->tv_sec = ms / 1000;
	tp->tv_usec = ms * 1000;

	return 0;
}

static int32_t sys_execve(const char *pathname, char *const argv[], char *const envp[])
{
	return process_execve(pathname, argv, envp);
}

int32_t sys_dup2(int oldfd, int newfd)
{
	current_process->files->fd[newfd] = current_process->files->fd[oldfd];
	return newfd;
}

static int32_t sys_pipe(int32_t *fd)
{
	return do_pipe(fd);
}

static int32_t sys_mmap(struct kmmap_args *args)
{
	return do_mmap((uint32_t)args->addr, args->len, args->prot, args->flags, args->fildes, args->off);
}

static int32_t sys_munmap(void *addr, size_t len)
{
	return do_munmap(current_process->mm, (uint32_t)addr, len);
}

static int32_t sys_truncate(const char *path, int32_t length)
{
	return vfs_truncate(path, length);
}

static int32_t sys_ftruncate(uint32_t fd, int32_t length)
{
	return vfs_ftruncate(fd, length);
}

static int32_t sys_access(const char *path, int amode)
{
	return vfs_access(path, amode);
}

static int32_t sys_faccessat(int fd, const char *path, int amode, int flag)
{
	if (flag && flag & ~(AT_SYMLINK_NOFOLLOW || AT_EACCESS))
		return -EINVAL;

	char *interpreted_path;
	int ret = interpret_path_from_fd(fd, path, &interpreted_path);

	if (ret >= 0)
		ret = sys_access(interpreted_path, amode);

	if (interpreted_path != path)
		kfree(interpreted_path);

	return ret;
}

static int32_t sys_unlink(const char *path)
{
	return vfs_unlink(path, 0);
}

static int32_t sys_unlinkat(int fd, const char *path, int flag)
{
	if (flag && flag & ~AT_REMOVEDIR)
		return -EINVAL;

	char *interpreted_path;
	int ret = interpret_path_from_fd(fd, path, &interpreted_path);

	if (ret >= 0)
		ret = vfs_unlink(interpreted_path, flag);

	if (interpreted_path != path)
		kfree(interpreted_path);
	return ret;
}

static int32_t sys_mkdir(const char *path, mode_t mode)
{
	return vfs_mkdir(path, mode);
}

static int32_t sys_mkdirat(int fd, const char *path, mode_t mode)
{
	char *interpreted_path;
	int ret = interpret_path_from_fd(fd, path, &interpreted_path);

	if (ret >= 0)
		ret = vfs_mkdir(interpreted_path, mode);

	if (interpreted_path != path)
		kfree(interpreted_path);
	return ret;
}

static int32_t sys_fchdir(int fildes)
{
	struct vfs_file *filp = current_process->files->fd[fildes];
	if (!filp)
		return -EBADF;

	current_process->fs->d_root = filp->f_dentry;
	return 0;
}

static int32_t sys_chdir(const char *path)
{
	int ret = vfs_open(path, O_RDONLY);
	if (ret < 0)
		return ret;

	return sys_fchdir(ret);
}

static int32_t sys_brk(uint32_t brk)
{
	struct mm_struct *current_mm = current_process->mm;
	if (brk < current_mm->start_brk)
		return -EINVAL;

	do_brk(current_mm->start_brk, brk - current_mm->start_brk);
	return 0;
}

int32_t sys_sbrk(intptr_t increment)
{
	uint32_t brk = current_process->mm->brk;
	sys_brk(current_process->mm->brk + increment);
	return brk;
}

static int32_t sys_getpgrp()
{
	return current_process->gid;
}

static int32_t sys_getpid()
{
	return current_process->pid;
}

static int32_t sys_getpgid(pid_t pid)
{
	if (!pid)
		return current_process->gid;

	struct process *p = find_process_by_pid(pid);
	if (!p)
		return -ESRCH;

	return current_process->gid;
}

static int32_t sys_getppid()
{
	return current_process->parent->pid;
}

static int32_t sys_setpgid(pid_t pid, pid_t pgid)
{
	struct process *p = !pid ? current_process : find_process_by_pid(pid);
	struct process *l = !pgid ? p : find_process_by_pid(pgid);

	if (l->sid != p->sid)
		return -1;

	p->gid = l->pid;
	return 0;
}

static int32_t sys_getsid()
{
	return current_process->sid;
}

static int32_t sys_setsid()
{
	if (current_process->pid == current_process->gid)
		return -1;

	current_process->sid = current_process->gid = current_process->pid;
	current_process->tty = NULL;
	return 0;
}

static int32_t sys_getuid()
{
	return 0;
}

static int32_t sys_setuid(uid_t uid)
{
	return 0;
}

static int32_t sys_getegid()
{
	return 0;
}

static int32_t sys_geteuid()
{
	return 0;
}

static int32_t sys_getgid()
{
	return 0;
}

static int32_t sys_setgid(gid_t gid)
{
	return 0;
}

static int32_t sys_alarm(unsigned int seconds)
{
	uint64_t current_time = get_milliseconds(NULL);
	int remain_time = current_process->sig_alarm_timer.expires - current_time;
	if (seconds)
		mod_timer(&current_process->sig_alarm_timer, get_milliseconds(NULL) + seconds * 1000);
	else
		del_timer(&current_process->sig_alarm_timer);
	return remain_time;
}

static int32_t sys_signal(int signum, __sighandler_t handler)
{
	struct sigaction act;
	sigset_t mask;

	sigfillset(&mask);
	act.sa_handler = handler;
	act.sa_flags = SA_RESETHAND | SA_NODEFER;
	act.sa_mask = mask;

	return do_sigaction(signum, &act, NULL);
}

static int32_t sys_sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)
{
	return do_sigaction(signum, act, oldact);
}

static int32_t sys_sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	return do_sigprocmask(how, set, oldset);
}

static int32_t sys_sigsuspend(const sigset_t *set)
{
	return do_sigsuspend(set);
}

static int32_t sys_kill(pid_t pid, int sig)
{
	return do_kill(pid, sig);
}

static void posix_spawn_setup_stack(struct Elf32_Layout *layout)
{
	setup_user_thread_stack(layout, 0, NULL, NULL);
};

static int32_t sys_posix_spawn(char *path)
{
	int top = get_top_priority_from_list(THREAD_READY, THREAD_SYSTEM_POLICY);
	process_load(path, path, THREAD_APP_POLICY, top - 1, posix_spawn_setup_stack);
	return 0;
}

int32_t sys_socket(int32_t family, enum socket_type type, int32_t protocal)
{
	char *path = get_next_socket_path();
	int32_t fd = vfs_open(path, O_RDWR | O_CREAT, S_IFSOCK);
	socket_setup(family, type, protocal, current_process->files->fd[fd]);
	return fd;
}

static int32_t sys_bind(int32_t sockfd, struct sockaddr *addr, uint32_t addrlen)
{
	struct socket *sock = sockfd_lookup(sockfd);
	return sock->ops->bind(sock, addr, addrlen);
}

static int32_t sys_connect(int32_t sockfd, struct sockaddr *addr, uint32_t addrlen)
{
	struct socket *sock = sockfd_lookup(sockfd);
	return sock->ops->connect(sock, addr, addrlen);
}

static int32_t sys_send(int32_t sockfd, void *msg, size_t len)
{
	struct socket *sock = sockfd_lookup(sockfd);
	return sock->ops->sendmsg(sock, msg, len);
}

static int32_t sys_recv(int32_t sockfd, void *msg, size_t len)
{
	struct socket *sock = sockfd_lookup(sockfd);
	return sock->ops->recvmsg(sock, msg, len);
}

// NOTE: MQ 2020-08-26 we only support millisecond precision
static int32_t sys_nanosleep(const struct timespec *req, struct timespec *rem)
{
	thread_sleep(req->tv_nsec / 1000);
	return 0;
}

static int32_t sys_poll(struct pollfd *fds, uint32_t nfds)
{
	return do_poll(fds, nfds);
}

static int32_t sys_ioctl(int fd, unsigned int cmd, unsigned long arg)
{
	struct vfs_file *file = current_process->files->fd[fd];

	if (file && file->f_op->ioctl)
		return file->f_op->ioctl(file->f_dentry->d_inode, file, cmd, arg);

	return -EINVAL;
}

static int32_t sys_fcntl(int fd, int cmd, unsigned long arg)
{
	return do_fcntl(fd, cmd, arg);
}

static int32_t sys_umask(mode_t cmask)
{
	// TODO: MQ 2020-11-10 Implement umask
	return 0;
}

static int32_t sys_mq_open(const char *name, int32_t flags, struct mq_attr *attr)
{
	return mq_open(name, flags, attr);
}

static int32_t sys_mq_close(int32_t fd)
{
	return mq_close(fd);
}

static int32_t sys_mq_unlink(const char *name)
{
	return mq_unlink(name);
}

static int32_t sys_mq_send(int32_t fd, char *buf, uint32_t priority, uint32_t msize)
{
	return mq_send(fd, buf, priority, msize);
}

static int32_t sys_mq_receive(int32_t fd, char *buf, uint32_t priority, uint32_t msize)
{
	return mq_receive(fd, buf, priority, msize);
}

static int32_t sys_getptsname(int32_t fdm, char *buf)
{
	struct tty_struct *ttym = current_process->files->fd[fdm]->private_data;
	if (!ttym || ttym->magic != TTY_MAGIC)
		return -ENOTTY;

	struct tty_struct *ttys = ttym->link;
	sprintf(buf, "/dev/%s", ttys->name);
	return 0;
}

static int32_t sys_clock_gettime(clockid_t clk_id, struct timespec *tp)
{
	if (clk_id != CLOCK_PROCESS_CPUTIME_ID)
		return -EINVAL;
	if (!tp)
		return -EFAULT;

	uint64_t msec = get_milliseconds_since_epoch();
	tp->tv_sec = msec / 1000;
	tp->tv_nsec = (msec % 1000) * 1000;

	return 0;
}

static int32_t sys_uname(struct utsname *info)
{
	strcpy(info->sysname, "mOS");
	strcpy(info->nodename, "root");
	strcpy(info->release, "0.1.0");
	strcpy(info->version, "2020-11-30");
	strcpy(info->machine, "x86");

	return 0;
}

static int32_t sys_debug_printf(enum debug_level level, const char *out)
{
	return debug_printf(level, out);
}

static int32_t sys_debug_println(enum debug_level level, const char *out)
{
	return debug_println(level, out);
}

// FIXME MQ 2020-05-12 copy define constants from https://github.com/torvalds/linux/blob/master/arch/x86/entry/syscalls/syscall_32.tbl
#define __NR_exit 1
#define __NR_fork 2
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_waitpid 7
#define __NR_unlink 10
#define __NR_execve 11
#define __NR_chdir 12
#define __NR_time 13
#define __NR_mknod 14
#define __NR_chmod 15
#define __NR_brk 17
#define __NR_sbrk 18
#define __NR_lseek 19
#define __NR_getpid 20
#define __NR_setuid 23
#define __NR_getuid 24
#define __NR_alarm 27
#define __NR_access 33
#define __NR_kill 37
#define __NR_rename 38
#define __NR_mkdir 39
#define __NR_dup 41
#define __NR_pipe 42
#define __NR_times 43
#define __NR_setgid 46
#define __NR_getgid 47
#define __NR_signal 48
#define __NR_geteuid 49
#define __NR_getegid 50
#define __NR_ioctl 54
#define __NR_fcntl 55
#define __NR_setpgid 57
#define __NR_umask 60
#define __NR_dup2 63
#define __NR_getppid 64
#define __NR_getpgrp 65
#define __NR_setsid 66
#define __NR_sigaction 67
#define __NR_sigsuspend 72
#define __NR_gettimeofday 78
#define __NR_mmap 90
#define __NR_munmap 91
#define __NR_truncate 92
#define __NR_ftruncate 93
#define __NR_fchmod 94
#define __NR_socket 97
#define __NR_connect 98
#define __NR_accept 99
#define __NR_getpriority 100
#define __NR_send 101
#define __NR_recv 102
#define __NR_sigreturn 103
#define __NR_bind 104
#define __NR_listen 105
#define __NR_stat 106
#define __NR_fstat 108
#define __NR_uname 122
#define __NR_sigprocmask 126
#define __NR_fchdir 133
#define __NR_getpgid 132
#define __NR_getdents 141
#define __NR_getsid 147
#define __NR_nanosleep 162
#define __NR_poll 168
#define __NR_getcwd 183
#define __NR_clock_gettime 265
#define __NR_mq_open 277
#define __NR_mq_close (__NR_mq_open + 1)
#define __NR_mq_unlink (__NR_mq_open + 2)
#define __NR_mq_send (__NR_mq_open + 3)
#define __NR_mq_receive (__NR_mq_open + 4)
#define __NR_waitid 284
#define __NR_mkdirat 296
#define __NR_mknodat 297
#define __NR_unlinkat 301
#define __NR_renameat 302
#define __NR_faccessat 307
#define __NR_sendto 369
// TODO: MQ 2020-09-05 Use ioctl-FIODGNAME to get pts name
#define __NR_getptsname 370
// TODO: MQ 2020-09-16 Replace by writting to /dev/ttyS0
#define __NR_dprintf 512
#define __NR_dprintln 513
#define __NR_posix_spawn 514

static void *syscalls[] = {
	[__NR_exit] = sys_exit,
	[__NR_fork] = sys_fork,
	[__NR_read] = sys_read,
	[__NR_write] = sys_write,
	[__NR_open] = sys_open,
	[__NR_stat] = sys_stat,
	[__NR_fstat] = sys_fstat,
	[__NR_close] = sys_close,
	[__NR_lseek] = sys_lseek,
	[__NR_rename] = sys_rename,
	[__NR_renameat] = sys_renameat,
	[__NR_chmod] = sys_chmod,
	[__NR_fchmod] = sys_fchmod,
	[__NR_mknod] = sys_mknod,
	[__NR_mknodat] = sys_mknodat,
	[__NR_mkdir] = sys_mkdir,
	[__NR_mkdirat] = sys_mkdirat,
	[__NR_getcwd] = sys_getcwd,
	[__NR_waitpid] = sys_waitpid,
	[__NR_getdents] = sys_getdents,
	[__NR_execve] = sys_execve,
	[__NR_dup2] = sys_dup2,
	[__NR_time] = sys_time,
	[__NR_times] = sys_times,
	[__NR_gettimeofday] = sys_gettimeofday,
	[__NR_alarm] = sys_alarm,
	[__NR_brk] = sys_brk,
	[__NR_sbrk] = sys_sbrk,
	[__NR_kill] = sys_kill,
	[__NR_ioctl] = sys_ioctl,
	[__NR_fcntl] = sys_fcntl,
	[__NR_umask] = sys_umask,
	[__NR_access] = sys_access,
	[__NR_faccessat] = sys_faccessat,
	[__NR_unlink] = sys_unlink,
	[__NR_unlinkat] = sys_unlinkat,
	[__NR_chdir] = sys_chdir,
	[__NR_fchdir] = sys_fchdir,
	[__NR_getpgrp] = sys_getpgrp,
	[__NR_getuid] = sys_getuid,
	[__NR_setuid] = sys_setuid,
	[__NR_getegid] = sys_getegid,
	[__NR_geteuid] = sys_geteuid,
	[__NR_getgid] = sys_getgid,
	[__NR_setgid] = sys_setgid,
	[__NR_getpid] = sys_getpid,
	[__NR_getppid] = sys_getppid,
	[__NR_getpgid] = sys_getpgid,
	[__NR_setpgid] = sys_setpgid,
	[__NR_getsid] = sys_getsid,
	[__NR_setsid] = sys_setsid,
	[__NR_signal] = sys_signal,
	[__NR_sigaction] = sys_sigaction,
	[__NR_sigprocmask] = sys_sigprocmask,
	[__NR_sigsuspend] = sys_sigsuspend,
	[__NR_pipe] = sys_pipe,
	[__NR_posix_spawn] = sys_posix_spawn,
	[__NR_mmap] = sys_mmap,
	[__NR_munmap] = sys_munmap,
	[__NR_truncate] = sys_truncate,
	[__NR_ftruncate] = sys_ftruncate,
	[__NR_socket] = sys_socket,
	[__NR_connect] = sys_connect,
	[__NR_bind] = sys_bind,
	[__NR_send] = sys_send,
	[__NR_recv] = sys_recv,
	[__NR_nanosleep] = sys_nanosleep,
	[__NR_poll] = sys_poll,
	[__NR_mq_open] = sys_mq_open,
	[__NR_mq_close] = sys_mq_close,
	[__NR_mq_unlink] = sys_mq_unlink,
	[__NR_mq_send] = sys_mq_send,
	[__NR_mq_receive] = sys_mq_receive,
	[__NR_waitid] = sys_waitid,
	[__NR_uname] = sys_uname,
	[__NR_getptsname] = sys_getptsname,
	[__NR_clock_gettime] = sys_clock_gettime,
	[__NR_dprintf] = sys_debug_printf,
	[__NR_dprintln] = sys_debug_println,
};

static int32_t syscall_dispatcher(struct interrupt_registers *regs)
{
	int idx = regs->eax;

	uint32_t (*func)(unsigned int, ...) = syscalls[idx];

	if (!func)
		return IRQ_HANDLER_STOP;

	memcpy(&current_thread->uregs, regs, sizeof(struct interrupt_registers));

	uint32_t ret = func(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
	regs->eax = ret;

	return IRQ_HANDLER_CONTINUE;
}

void syscall_init()
{
	register_interrupt_handler(DISPATCHER_ISR, syscall_dispatcher);
}
