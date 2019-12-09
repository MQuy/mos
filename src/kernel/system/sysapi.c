#include <kernel/cpu/hal.h>
#include <kernel/fs/vfs.h>
#include <kernel/utils/printf.h>
#include <kernel/proc/task.h>
#include <kernel/proc/elf.h>
#include "sysapi.h"

typedef uint32_t (*SYSTEM_FUNC)(unsigned int, ...);

#define MAX_SYSCALL 3

static void *syscalls[] = {
    DebugPrintf,
    DebugGotoXY,
    DebugPutc};

void syscall_dispatcher(interrupt_registers *regs)
{
  int idx = regs->eax;

  if (idx >= MAX_SYSCALL)
    return;

  SYSTEM_FUNC func = (SYSTEM_FUNC)syscalls[idx];

  if (!func)
    return;

  func(regs->ebx, regs->ecx, regs->edx, regs->esi, regs->edi);
}

void syscall_init()
{
  register_interrupt_handler(DISPATCHER_ISR, syscall_dispatcher);
}

int sys_printf(char *str)
{
  int a;
  __asm__ __volatile__("int $0x7F"
                       : "=a"(a)
                       : "0"(0), "b"(str));

  return a;
}

int sys_printg(uint32_t x, uint32_t y)
{
  int a;
  __asm__ __volatile__("int $0x7F"
                       : "=a"(a)
                       : "0"(1), "b"(x), "c"(y));
  return a;
}

int sys_printc(char c)
{
  int a;
  __asm__ __volatile__("int $0x7F"
                       : "=a"(a)
                       : "0"(2), "b"(c));
  return a;
}

pid_t sys_fork()
{
}

int sys_execve(const char *filename, char *const argv[], char *const envp[])
{
  kstat *stat = malloc(sizeof(kstat));
  vfs_stat(filename, stat);
  long fd = vfs_open(filename);
  char *buf = malloc(stat->size);
  vfs_fread(fd, buf, stat->size);

  process *np = create_process(NULL, "", 0);
  elf_load(buf, np->pdir);
}