#ifndef CPU_HAL_H
#define CPU_HAL_H

#include <stdint.h>

void enable_interrupts();
void disable_interrupts();
void halt();

unsigned char inportb(unsigned short id);
void outportb(unsigned short id, unsigned char value);

const char *get_cpu_vender();

#endif