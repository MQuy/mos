#include "pic.h"
#include "hal.h"

void pic_remap()
{
  unsigned char a1, a2;

  a1 = inportb(PIC1_DATA); // save masks
  a2 = inportb(PIC2_DATA);

  outportb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4); // starts the initialization sequence (in cascade mode)
  io_wait();

  outportb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
  io_wait();

  outportb(PIC1_DATA, 0x20); // ICW2: Master PIC vector offset
  io_wait();
  outportb(PIC2_DATA, 0x28); // ICW2: Slave PIC vector offset
  io_wait();

  outportb(PIC1_DATA, 4); // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
  io_wait();
  outportb(PIC2_DATA, 2); // ICW3: tell Slave PIC its cascade identity (0000 0010)
  io_wait();

  outportb(PIC1_DATA, ICW4_8086);
  io_wait();
  outportb(PIC2_DATA, ICW4_8086);
  io_wait();

  outportb(PIC1_DATA, a1); // restore saved masks.
  outportb(PIC2_DATA, a2);
}

// Send End-Of-Interrupt command
void pic_eoi(uint8_t irq)
{
  if (irq >= 8)
    outportb(PIC2_COMMAND, PIC_EOI);

  outportb(PIC1_COMMAND, PIC_EOI);
  io_wait();
}

void pic_disable()
{
  outportb(PIC1_DATA, 0xff);
  io_wait();

  outportb(PIC2_DATA, 0xff);
  io_wait();

  pic_eoi(8);
}
