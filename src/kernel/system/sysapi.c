#include <kernel/cpu/hal.h>
#include <kernel/fs/vfs.h>
#include <kernel/utils/printf.h>
#include <kernel/proc/task.h>
#include <kernel/proc/elf.h>
#include <kernel/fs/vfs.h>
#include "sysapi.h"

extern thread *current_thread;
extern process *current_process;

typedef uint32_t (*SYSTEM_FUNC)(unsigned int, ...);

void sys_exit(int32_t code)
{
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

static void *syscalls[] = {0, sys_exit, sys_fork, sys_read, sys_write};

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