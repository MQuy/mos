#ifndef SYSTEM_SYSAPI_H
#define SYSTEM_SYSAPI_H

#include <include/ctype.h>
#include <kernel/cpu/idt.h>

enum socket_type;

int32_t syscall_dispatcher(struct interrupt_registers *registers);
void syscall_init();

int sys_printf(char *);
int sys_printg(uint32_t x, uint32_t y);
int sys_printc(char c);
pid_t sys_fork();
int sys_execve(const char *filename, char *const argv[], char *const envp[]);
int32_t sys_socket(int32_t family, enum socket_type type, int32_t protocal);

#endif
