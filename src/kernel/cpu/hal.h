#ifndef CPU_HAL_H
#define CPU_HAL_H

#include <stdint.h>

void enable_interrupts();
void disable_interrupts();
void halt();
void io_wait();

unsigned char inportb(unsigned short id);
void outportb(unsigned short id, unsigned char value);
uint32_t inportl(uint16_t _port);
void outportl(uint16_t _port, uint32_t _data);

const char *get_cpu_vender();

#endif