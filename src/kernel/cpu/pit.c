#include <include/list.h>
#include <kernel/memory/vmm.h>
#include "idt.h"
#include "pic.h"
#include "pit.h"

#define PIT_REG_COUNTER 0x40
#define PIT_REG_COMMAND 0x43
#define TICKS_PER_SECOND 1000

volatile uint32_t pit_ticks = 0;

int32_t pit_interrupt_handler(interrupt_registers *regs)
{
  pit_ticks++;

  return IRQ_HANDLER_CONTINUE;
}

uint32_t get_milliseconds_from_boot()
{
  return pit_ticks * TICKS_PER_SECOND;
}

void pit_init()
{
  int divisor = 1193181 / TICKS_PER_SECOND;

  outportb(PIT_REG_COMMAND, 0x34);
  outportb(PIT_REG_COUNTER, divisor & 0xff);
  outportb(PIT_REG_COUNTER, (divisor >> 8) & 0xff);

  register_interrupt_handler(IRQ0, pit_interrupt_handler);
}