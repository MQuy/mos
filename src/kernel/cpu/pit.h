#ifndef CPU_PIT_H
#define CPU_PIT_H

#include <stdint.h>

#include "idt.h"

#define TICKS_PER_SECOND 1000

void pit_init();
unsigned long get_current_tick();
uint32_t get_milliseconds_from_boot();

#endif
