#include "idt.h"
#include "pic.h"
#include "../graphics/DebugDisplay.h"
#include "../include/string.h"

extern void idt_flush(uint32_t);

static struct idt_descriptor _idt[I86_MAX_INTERRUPTS];
static struct idtr _idtr;

I86_IRQ_HANDLER interrupt_handlers[256];

void handle_interrupt(interrupt_registers *);
void i86_default_handler(interrupt_registers *regs);
void idt_install_ir(uint32_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER irq);

void idt_install_ir(uint32_t i, uint16_t flags, uint16_t sel, I86_IRQ_HANDLER irq)
{
  if (i > I86_MAX_INTERRUPTS)
    return;

  uint64_t base = (uint64_t) & (*irq);

  _idt[i].baseLo = base & 0xffff;
  _idt[i].baseHi = (base >> 16) & 0xffff;
  _idt[i].flags = flags;
  _idt[i].sel = sel;
  _idt[i].reserved = 0;
}

void setvect(uint8_t i, I86_IRQ_HANDLER irq)
{
  idt_install_ir(i, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32, 0x8, irq);
}

void setvect_flags(uint8_t i, I86_IRQ_HANDLER irq, uint32_t flags)
{
  idt_install_ir(i, I86_IDT_DESC_PRESENT | I86_IDT_DESC_BIT32 | flags, 0x8, irq);
}

void i86_default_handler(interrupt_registers *regs)
{
  disable_interrupts();

  uint8_t int_no = regs->int_no & 0xff;

  DebugPrintf("\n*** [i86 Hal]: unhandled exception %d", int_no);

  halt();
}

void idt_init()
{
  _idtr.limit = sizeof(struct idt_descriptor) * I86_MAX_INTERRUPTS - 1;
  _idtr.base = &_idt[0];

  memset(&_idt[0], 0, _idtr.limit);

  for (int i = 0; i < 256; ++i)
    setvect(i, i86_default_handler);

  // Install the ISRs
  setvect(0, (I86_IRQ_HANDLER)isr0);
  setvect(1, (I86_IRQ_HANDLER)isr1);
  setvect(2, (I86_IRQ_HANDLER)isr2);
  setvect(3, (I86_IRQ_HANDLER)isr3);
  setvect(4, (I86_IRQ_HANDLER)isr4);
  setvect(5, (I86_IRQ_HANDLER)isr5);
  setvect(6, (I86_IRQ_HANDLER)isr6);
  setvect(7, (I86_IRQ_HANDLER)isr7);
  setvect(8, (I86_IRQ_HANDLER)isr8);
  setvect(9, (I86_IRQ_HANDLER)isr9);
  setvect(10, (I86_IRQ_HANDLER)isr10);
  setvect(11, (I86_IRQ_HANDLER)isr11);
  setvect(12, (I86_IRQ_HANDLER)isr12);
  setvect(13, (I86_IRQ_HANDLER)isr13);
  setvect(14, (I86_IRQ_HANDLER)isr14);
  setvect(15, (I86_IRQ_HANDLER)isr15);
  setvect(16, (I86_IRQ_HANDLER)isr16);
  setvect(17, (I86_IRQ_HANDLER)isr17);
  setvect(18, (I86_IRQ_HANDLER)isr18);
  setvect(19, (I86_IRQ_HANDLER)isr19);
  setvect(20, (I86_IRQ_HANDLER)isr20);
  setvect(21, (I86_IRQ_HANDLER)isr21);
  setvect(22, (I86_IRQ_HANDLER)isr22);
  setvect(23, (I86_IRQ_HANDLER)isr23);
  setvect(24, (I86_IRQ_HANDLER)isr24);
  setvect(25, (I86_IRQ_HANDLER)isr25);
  setvect(26, (I86_IRQ_HANDLER)isr26);
  setvect(27, (I86_IRQ_HANDLER)isr27);
  setvect(28, (I86_IRQ_HANDLER)isr28);
  setvect(29, (I86_IRQ_HANDLER)isr29);
  setvect(30, (I86_IRQ_HANDLER)isr30);
  setvect(31, (I86_IRQ_HANDLER)isr31);

  // Install the IRQs
  setvect(32, (I86_IRQ_HANDLER)irq0);
  setvect(33, (I86_IRQ_HANDLER)irq1);
  setvect(34, (I86_IRQ_HANDLER)irq2);
  setvect(35, (I86_IRQ_HANDLER)irq3);
  setvect(36, (I86_IRQ_HANDLER)irq4);
  setvect(37, (I86_IRQ_HANDLER)irq5);
  setvect(38, (I86_IRQ_HANDLER)irq6);
  setvect(39, (I86_IRQ_HANDLER)irq7);
  setvect(40, (I86_IRQ_HANDLER)irq8);
  setvect(41, (I86_IRQ_HANDLER)irq9);
  setvect(42, (I86_IRQ_HANDLER)irq10);
  setvect(43, (I86_IRQ_HANDLER)irq11);
  setvect(44, (I86_IRQ_HANDLER)irq12);
  setvect(45, (I86_IRQ_HANDLER)irq13);
  setvect(46, (I86_IRQ_HANDLER)irq14);
  setvect(47, (I86_IRQ_HANDLER)irq15);

  setvect_flags(DISPATCHER_ISR, (I86_IRQ_HANDLER)isr127, I86_IDT_DESC_RING3);

  idt_flush((uint32_t)&_idtr);

  pic_remap();
}

void register_interrupt_handler(uint8_t n, I86_IRQ_HANDLER handler)
{
  interrupt_handlers[n] = handler;
}

void handle_interrupt(interrupt_registers *reg)
{
  uint8_t int_no = reg->int_no & 0xff;
  I86_IRQ_HANDLER handler = interrupt_handlers[int_no];

  if (handler != 0)
  {
    handler(reg);
  }
  else
  {
    DebugPrintf("\nunhandled interrupt %d", int_no);
  }
}

void isr_handler(interrupt_registers *reg)
{
  handle_interrupt(reg);
}

void irq_handler(interrupt_registers *reg)
{
  if (reg->int_no >= 40)
    outportb(PIC2_COMMAND, PIC_EOI);
  outportb(PIC1_COMMAND, PIC_EOI);

  handle_interrupt(reg);
}