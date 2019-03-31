#include "sysapi.h"

void *_syscalls[] = {

    DebugPrintf,
    0};

void syscall_dispatcher(interrupt_registers *regs)
{
  int idx = regs->eax;

  // //! bounds check
  if (idx >= MAX_SYSCALL)
    __asm__ __volatile__("iret");

  //! get service
  void *fnct = _syscalls[idx];

  //! and call service
  __asm__ __volatile__(
      "push %%edi			\n"
      "push %%esi			\n"
      "push %%edx			\n"
      "push %%ecx			\n"
      "push %%ebx			\n"
      "call %0			\n"
      "add $20, %%esp	\n" ::"r"(fnct));
}

int syscall_printf(char *str)
{
  int a;
  __asm__ __volatile__("int $0x80"
                       : "=a"(a)
                       : "0"(0), "b"(str));

  return a;
}

void syscall_init()
{

  //! install interrupt handler!
  register_interrupt_handler(DISPATCHER_ISR, syscall_dispatcher);
}
