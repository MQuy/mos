#include <kernel/cpu/hal.h>
#include "serial.h"

#define SERIAL_PORT_A 0x3F8
#define SERIAL_PORT_B 0x2F8
#define SERIAL_PORT_C 0x3E8
#define SERIAL_PORT_D 0x2E8

int serial_transmit_empty(int port)
{
  return inportb(port + 5) & 0x20;
}

void serial_write(char a)
{
  while (serial_transmit_empty(SERIAL_PORT_A) == 0)
    ;

  outportb(SERIAL_PORT_A, a);
}

void serial_enable(int port)
{
  outportb(port + 1, 0x00); // Disable all interrupts
  outportb(port + 3, 0x80); // Enable DLAB (set baud rate divisor)
  outportb(port + 0, 0x03); // Set divisor to 3 (lo byte) 38400 baud
  outportb(port + 1, 0x00); //                  (hi byte)
  outportb(port + 3, 0x03); // 8 bits, no parity, one stop bit
  outportb(port + 2, 0xC7); // Enable FIFO, clear them, with 14-byte threshold
  outportb(port + 4, 0x0B); // IRQs enabled, RTS/DSR set
}

void serial_init()
{
  serial_enable(SERIAL_PORT_A);
}
