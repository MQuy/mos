#include "idt.h"
#include "pic.h"
#include "pit.h"

#define PIT_REG_COUNTER 0x40
#define PIT_REG_COMMAND 0x43

static volatile uint32_t _pit_ticks = 0;

void pit_interrupt_handler(interrupt_registers *regs)
{
  _pit_ticks++;
}

void pit_init()
{
  int divisor = 1193180 / 1000;

  outportb(PIT_REG_COMMAND, 0x34);
  outportb(PIT_REG_COUNTER, divisor & 0xff);
  outportb(PIT_REG_COUNTER, (divisor >> 8) & 0xff);

  register_interrupt_handler(IRQ0, pit_interrupt_handler);
}