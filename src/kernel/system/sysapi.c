#include <include/ctype.h>
#include <include/errno.h>
#include <kernel/cpu/hal.h>
#include <kernel/fs/vfs.h>
#include <kernel/utils/printf.h>
#include <kernel/proc/task.h>
#include <kernel/proc/elf.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/sockfs/sockfs.h>
#include <kernel/fs/pipefs/pipe.h>
#include <kernel/ipc/message_queue.h>
#include <kernel/net/net.h>
#include <kernel/utils/string.h>
#include "sysapi.h"

extern struct thread *current_thread;
extern struct process *current_process;

typedef uint32_t (*SYSTEM_FUNC)(unsigned int, ...);

void sys_exit(int32_t code)
{
  current_thread->exit_code = code;
  update_thread(current_thread, THREAD_TERMINATED);
  schedule();
}

pid_t sys_fork()
{
  struct process *child = process_fork(current_process);
  struct thread *t = list_first_entry(&child->threads, struct thread, sibling);

  queue_thread(t);

  return child->pid;
}

int32_t sys_read(uint32_t fd, char *buf, size_t count)
{
  return vfs_fread(fd, buf, count);
}

int32_t sys_write(uint32_t fd, char *buf, size_t count)
{
  return vfs_fwrite(fd, buf, count);
}

int32_t sys_open(const char *path, int32_t flag, int32_t mode)
{
  return vfs_open(path);
}

int32_t sys_fstat(int32_t fd, struct kstat *stat)
{
  return vfs_fstat(fd, stat);
}

int32_t sys_stat(const char *path, struct kstat *stat)
{
  return vfs_stat(path, stat);
}

int32_t sys_close(uint32_t fd)
{
  return vfs_close(fd);
}

int32_t sys_pipe(int32_t *fd)
{
  return do_pipe(fd);
}

int32_t sys_mmap(uint32_t addr, size_t length, uint32_t prot, uint32_t flags,
                 int32_t fd)
{
  return do_mmap(addr, length, prot, flags, fd);
}

int32_t sys_truncate(const char *path, int32_t length)
{
  return vfs_truncate(path, length);
}

int32_t sys_ftruncate(uint32_t fd, int32_t length)
{
  return vfs_ftruncate(fd, length);
}

int32_t sys_msgopen(const char *name, int32_t flags)
{
  return mq_open(name, flags);
}

int32_t sys_msgclose(const char *name)
{
  return mq_close(name);
}

int32_t sys_brk(uint32_t brk)
{
  struct mm_struct *current_mm = current_process->mm;
  if (brk < current_mm->start_brk)
    return -EINVAL;

  do_brk(current_mm->start_brk, brk - current_mm->start_brk);
  return brk;
}

int32_t sys_sbrk(intptr_t increment)
{
  return sys_brk(current_process->mm->brk + increment);
}

int32_t sys_msgsnd(const char *name, char *buf, int32_t mtype, uint32_t msize)
{
  return mq_send(name, buf, mtype, msize);
}

int32_t sys_msgrcv(const char *name, char *buf, int32_t mtype, uint32_t msize)
{
  return mq_receive(name, buf, mtype, msize);
}

int32_t sys_getpid()
{
  return current_process->pid;
}

int32_t sys_posix_spawn(char *path)
{
  int top = get_top_priority_from_list(THREAD_READY, THREAD_SYSTEM_POLICY);
  process_load(path, path, top - 1, NULL);
  return 0;
}

int32_t sys_socket(int32_t family, enum socket_type type, int32_t protocal)
{
  char *path = get_next_socket_path();
  int32_t fd = vfs_open(path);
  socket_setup(family, type, protocal, current_process->files->fd[fd]);
  return fd;
}

int32_t sys_bind(int32_t sockfd, struct sockaddr *addr, uint32_t addrlen)
{
  struct socket *sock = sockfd_lookup(sockfd);
  return sock->ops->bind(sock, addr, addrlen);
}

int32_t sys_connect(int32_t sockfd, struct sockaddr *addr, uint32_t addrlen)
{
  struct socket *sock = sockfd_lookup(sockfd);
  return sock->ops->connect(sock, addr, addrlen);
}

int32_t sys_send(int32_t sockfd, void *msg, size_t len)
{
  struct socket *sock = sockfd_lookup(sockfd);
  return sock->ops->sendmsg(sock, msg, len);
}

int32_t sys_recv(int32_t sockfd, void *msg, size_t len)
{
  struct socket *sock = sockfd_lookup(sockfd);
  return sock->ops->recvmsg(sock, msg, len);
}

#define __NR_exit 1
#define __NR_fork 2
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_brk 17
#define __NR_sbrk 18
#define __NR_getpid 20
#define __NR_pipe 42
#define __NR_posix_spawn 49
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
#define __NR_sendto 133
#define __NR_msgopen 200
#define __NR_msgclose 201
#define __NR_msgrcv 202
#define __NR_msgsnd 203

static void *syscalls[] = {
    [__NR_exit] = sys_exit,
    [__NR_fork] = sys_fork,
    [__NR_read] = sys_read,
    [__NR_write] = sys_write,
    [__NR_open] = sys_open,
    [__NR_stat] = sys_stat,
    [__NR_fstat] = sys_fstat,
    [__NR_close] = sys_close,
    [__NR_brk] = sys_brk,
    [__NR_sbrk] = sys_sbrk,
    [__NR_getpid] = sys_getpid,
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
    [__NR_msgopen] = sys_msgopen,
    [__NR_msgclose] = sys_msgclose,
    [__NR_msgsnd] = sys_msgsnd,
    [__NR_msgrcv] = sys_msgrcv,
};

int32_t syscall_dispatcher(struct interrupt_registers *regs)
{
  int idx = regs->eax;

  SYSTEM_FUNC func = (SYSTEM_FUNC)syscalls[idx];

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
