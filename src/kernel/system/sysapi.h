#ifndef SYSTEM_SYSAPI_H
#define SYSTEM_SYSAPI_H

#include <include/ctype.h>
#include <kernel/cpu/idt.h>

enum socket_type;

void syscall_init();
pid_t sys_fork();
int32_t sys_socket(int32_t family, enum socket_type type, int32_t protocal);

#endif
