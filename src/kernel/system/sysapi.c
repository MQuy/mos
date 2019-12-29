#include <kernel/cpu/hal.h>
#include <kernel/fs/vfs.h>
#include <kernel/utils/printf.h>
#include <kernel/proc/task.h>
#include <kernel/proc/elf.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/pipe.h>
#include "sysapi.h"

extern thread *current_thread;
extern process *current_process;

typedef uint32_t (*SYSTEM_FUNC)(unsigned int, ...);

void sys_exit(int32_t code)
{
  current_thread->exit_code = code;
  update_thread(current_thread, TERMINATED);
  schedule();
}

pid_t sys_fork()
{
  process *child = process_fork(current_process);
  thread *t = list_first_entry(&child->threads, struct thread, sibling);

  queue_thread(t);

  return child->pid;
}

uint32_t sys_read(uint32_t fd, char *buf, size_t count)
{
  return vfs_fread(fd, buf, count);
}

uint32_t sys_write(uint32_t fd, char *buf, size_t count)
{
  return vfs_fwrite(fd, buf, count);
}

uint32_t sys_open(const char *path, int32_t flag, int32_t mode)
{
  return vfs_open(path);
}

uint32_t sys_close(uint32_t fd)
{
  return vfs_close(fd);
}

uint32_t sys_pipe(int32_t *fd)
{
  return do_pipe(fd);
}

#define __NR_exit 1
#define __NR_fork 2
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_pipe 42

static void *syscalls[] = {
  [__NR_exit] = sys_exit,
  [__NR_fork] = sys_fork,
  [__NR_read] = sys_read,
  [__NR_write] = sys_write,
  [__NR_open] = sys_open,
  [__NR_close] = sys_close,
  [__NR_pipe] = sys_pipe
  };

int32_t syscall_dispatcher(interrupt_registers *regs)
{
  int idx = regs->eax;

  SYSTEM_FUNC func = (SYSTEM_FUNC)syscalls[idx];

  if (!func)
    return;

  memcpy(&current_thread->uregs, regs, sizeof(interrupt_registers));

  uint32_t ret = func(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
  regs->eax = ret;

  return IRQ_HANDLER_CONTINUE;
}

void syscall_init()
{
  register_interrupt_handler(DISPATCHER_ISR, syscall_dispatcher);
}