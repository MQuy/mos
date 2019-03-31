#ifndef SYSAPI_H
#define SYSAPI_H

#include "Hal/Hal.h"
#include "Hal/idt.h"
#include "DebugDisplay.h"

#define MAX_SYSCALL 3

void syscall_dispatcher(interrupt_registers *registers);
void syscall_init();

int syscall_printf(char *);

#endif