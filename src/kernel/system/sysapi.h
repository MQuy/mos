#ifndef SYSTEM_SYSAPI_H
#define SYSTEM_SYSAPI_H

#include <kernel/cpu/idt.h>

void syscall_dispatcher(interrupt_registers *registers);
void syscall_init();

int sys_printf(char *);
int sys_printg(uint32_t x, uint32_t y);
int sys_printc(char c);

#endif