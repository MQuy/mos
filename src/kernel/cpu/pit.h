#ifndef CPU_PIT_H
#define CPU_PIT_H

#include <stdint.h>

#include "idt.h"

// NOTE: MQ 2020-07-17
// recommended value for ticks per second 60-100
// higher value might cause overload and step over next ticks
#define TICKS_PER_SECOND 100

void pit_init();
uint32_t get_milliseconds_from_boot();

#endif
