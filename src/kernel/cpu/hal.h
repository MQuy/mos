#ifndef CPU_HAL_H
#define CPU_HAL_H

#include <stdint.h>
#include <stddef.h>

void enable_interrupts();
void disable_interrupts();
void halt();
void io_wait();

unsigned char inportb(unsigned short portid);
void outportb(unsigned short portid, unsigned char value);
uint16_t inportw(uint16_t portid);
void outportw(uint16_t portid, uint16_t value);
uint32_t inportl(uint16_t portid);
void outportl(uint16_t portid, uint32_t value);
void inportsw(uint16_t portid, void *addr, size_t count);
void outportsw(uint16_t portd, const void *addr, size_t count);
void cpuid(int code, uint32_t *a, uint32_t *d);

const char *get_cpu_vender();

#endif