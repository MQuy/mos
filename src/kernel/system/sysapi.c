#include "sysapi.h"

#include <include/ctype.h>
#include <include/errno.h>
#include <include/fcntl.h>
#include <kernel/cpu/hal.h>
#include <kernel/devices/char/tty.h>
#include <kernel/fs/pipefs/pipe.h>
#include <kernel/fs/sockfs/sockfs.h>
#include <kernel/fs/vfs.h>
#include <kernel/ipc/message_queue.h>
#include <kernel/ipc/signal.h>
#include <kernel/net/net.h>
#include <kernel/proc/elf.h>
#include <kernel/proc/task.h>
#include <kernel/system/time.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

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
	return do_wait(idtype, id, infop, options);
}

static int32_t sys_read(uint32_t fd, char *buf, size_t count)
{
	return vfs_fread(fd, buf, count);
}

static int32_t sys_write(uint32_t fd, char *buf, size_t count)
{
	return vfs_fwrite(fd, buf, count);
}

static int32_t sys_open(const char *path, int32_t flags, int32_t mode)
{
	return vfs_open(path, flags);
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

static int32_t sys_time(time_t *tloc)
{
	time_t t = get_seconds(NULL);
	if (tloc)
		*tloc = t;
	return t;
}

static int32_t sys_execve(const char *pathname, char *const argv[], char *const envp[])
{
	return process_execve(pathname, argv, envp);
}

static int32_t sys_dup2(int oldfd, int newfd)
{
	current_process->files->fd[newfd] = current_process->files->fd[oldfd];
	return newfd;
}

static int32_t sys_pipe(int32_t *fd)
{
	return do_pipe(fd);
}

static int32_t sys_mmap(uint32_t addr, size_t length, uint32_t prot, uint32_t flags,
						int32_t fd)
{
	return do_mmap(addr, length, prot, flags, fd);
}

static int32_t sys_truncate(const char *path, int32_t length)
{
	return vfs_truncate(path, length);
}

static int32_t sys_ftruncate(uint32_t fd, int32_t length)
{
	return vfs_ftruncate(fd, length);
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

static int32_t sys_getpid()
{
	return current_process->pid;
}

static int32_t sys_getpgid()
{
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

static int32_t sys_kill(pid_t pid, int sig)
{
	return do_kill(pid, sig);
}

static int32_t sys_posix_spawn(char *path)
{
	int top = get_top_priority_from_list(THREAD_READY, THREAD_SYSTEM_POLICY);
	process_load(path, path, THREAD_APP_POLICY, top - 1, NULL);
	return 0;
}

int32_t sys_socket(int32_t family, enum socket_type type, int32_t protocal)
{
	char *path = get_next_socket_path();
	int32_t fd = vfs_open(path, O_RDWR);
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

static int32_t sys_debug_printf(enum debug_level level, const char *out)
{
	return debug_printf(level, out);
}

static int32_t sys_debug_println(enum debug_level level, const char *out)
{
	return debug_println(level, out);
}

#define __NR_exit 1
#define __NR_fork 2
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_execve 11
#define __NR_time 13
#define __NR_brk 17
#define __NR_sbrk 18
#define __NR_getpid 20
#define __NR_kill 37
#define __NR_pipe 42
#define __NR_getgid 47
#define __NR_signal 48
#define __NR_posix_spawn 49
#define __NR_ioctl 54
#define __NR_setpgid 57
#define __NR_dup2 63
#define __NR_getppid 64
#define __NR_setsid 66
#define __NR_sigaction 67
#define __NR_mmap 90
#define __NR_munmap 91
#define __NR_truncate 92
#define __NR_ftruncate 93
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
#define __NR_sigprocmask 126
#define __NR_getpgid 132
#define __NR_getsid 147
#define __NR_nanosleep 162
#define __NR_poll 168
#define __NR_mq_open 277
#define __NR_mq_close (__NR_mq_open + 1)
#define __NR_mq_unlink (__NR_mq_open + 2)
#define __NR_mq_send (__NR_mq_open + 3)
#define __NR_mq_receive (__NR_mq_open + 4)
#define __NR_waitid 284
#define __NR_sendto 369
#define __NR_getptsname 370
#define __NR_debug_printf 512
#define __NR_debug_println 513

static void *syscalls[] = {
	[__NR_exit] = sys_exit,
	[__NR_fork] = sys_fork,
	[__NR_read] = sys_read,
	[__NR_write] = sys_write,
	[__NR_open] = sys_open,
	[__NR_stat] = sys_stat,
	[__NR_fstat] = sys_fstat,
	[__NR_close] = sys_close,
	[__NR_execve] = sys_execve,
	[__NR_dup2] = sys_dup2,
	[__NR_time] = sys_time,
	[__NR_brk] = sys_brk,
	[__NR_sbrk] = sys_sbrk,
	[__NR_kill] = sys_kill,
	[__NR_ioctl] = sys_ioctl,
	[__NR_getpid] = sys_getpid,
	[__NR_getppid] = sys_getppid,
	[__NR_getpgid] = sys_getpgid,
	[__NR_setpgid] = sys_setpgid,
	[__NR_getsid] = sys_getsid,
	[__NR_setsid] = sys_setsid,
	[__NR_signal] = sys_signal,
	[__NR_sigaction] = sys_sigaction,
	[__NR_sigprocmask] = sys_sigprocmask,
	[__NR_pipe] = sys_pipe,
	[__NR_posix_spawn] = sys_posix_spawn,
	[__NR_mmap] = sys_mmap,
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
	[__NR_getptsname] = sys_getptsname,
	[__NR_debug_printf] = sys_debug_printf,
	[__NR_debug_println] = sys_debug_println,
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
