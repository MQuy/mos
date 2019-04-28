#include "../cpu/hal.h"
#include "../graphics/DebugDisplay.h"
#include "sysapi.h"

typedef uint32_t (*SYSTEM_FUNC)(unsigned int, ...);

void *syscalls[] = {
    DebugPrintf};

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

int syscall_printf(char *str)
{
  int a;
  __asm__ __volatile__("int $0x7F"
                       : "=a"(a)
                       : "0"(0), "b"(str));

  return a;
}

void syscall_init()
{
  register_interrupt_handler(DISPATCHER_ISR, syscall_dispatcher);
}
