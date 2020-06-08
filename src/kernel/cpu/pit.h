#ifndef CPU_PIT_H
#define CPU_PIT_H

#include <stdint.h>

#include "idt.h"

void pit_init();
uint32_t get_milliseconds_from_boot();

#endif
