#ifndef SYSTEM_SYSAPI_H
#define SYSTEM_SYSAPI_H

#include "../cpu/idt.h"

#define MAX_SYSCALL 1

void syscall_dispatcher(interrupt_registers *registers);
void syscall_init();

int syscall_printf(char *);

#endif