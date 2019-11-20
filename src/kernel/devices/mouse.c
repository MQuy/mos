#include <kernel/cpu/idt.h>
#include <kernel/cpu/hal.h>
#include <kernel/memory/malloc.h>
#include "mouse.h"

#define MOUSE_LEFT_CLICK 0x01
#define MOUSE_RIGHT_CLICK 0x02
#define MOUSE_MIDDLE_CLICK 0x04

#define MOUSE_PORT 0x60
#define MOUSE_STATUS 0x64

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[4];
static mouse_device mouse_device_info;

void mouse_calculate_position()
{
  mouse_cycle = 0;
  uint8_t state = mouse_byte[0];
  int move_x = mouse_byte[1];
  int move_y = mouse_byte[2];
  mouse_device_info.x += move_x - ((state << 4) & 0x100);
  mouse_device_info.y += move_y - ((state << 3) & 0x100);

  mouse_device_info.state = 0;
  if (state & MOUSE_LEFT_CLICK)
  {
    mouse_device_info.state |= MOUSE_LEFT_CLICK;
  }
  if (state & MOUSE_RIGHT_CLICK)
  {
    mouse_device_info.state |= MOUSE_RIGHT_CLICK;
  }
  if (state & MOUSE_MIDDLE_CLICK)
  {
    mouse_device_info.state |= MOUSE_MIDDLE_CLICK;
  }
}

void mouse_handler(interrupt_registers *regs)
{
  uint8_t status = inportb(MOUSE_STATUS);
  if ((status & 0x01) && (status & 0x20))
  {

    int8_t mouse_in = inportb(MOUSE_PORT);
    switch (mouse_cycle)
    {
    case 0:
      mouse_byte[0] = mouse_in;
      if (!(mouse_in & 0x8) || (mouse_in & 0x80) || (mouse_in & 0x40))
        break;
      mouse_cycle++;
      break;
    case 1:
      mouse_byte[1] = mouse_in;
      mouse_cycle++;
      break;
    case 2:
      mouse_byte[2] = mouse_in;
      mouse_calculate_position();
      break;
    }
  }
}

void mouse_wait(uint32_t type)
{
  if (type == 0)
  {
    for (uint32_t i = 0; i < 100000; i++)
    {
      if ((inportb(MOUSE_STATUS) & 1) == 1)
      {
        return;
      }
    }
    return;
  }
  else
  {
    for (uint32_t i = 0; i < 100000; i++)
    {
      if ((inportb(MOUSE_STATUS) & 2) == 0)
      {
        return;
      }
    }
    return;
  }
}

void mouse_write(uint8_t value)
{
  mouse_wait(1);
  outportb(MOUSE_STATUS, 0xD4);
  mouse_wait(1);
  outportb(MOUSE_PORT, value);
}

uint8_t mouse_read(void)
{
  mouse_wait(0);
  return inportb(MOUSE_PORT);
}

void mouse_init()
{
  uint8_t status = 0;

  mouse_device_info.x = mouse_device_info.y = 0;
  mouse_device_info.state = 0;

  mouse_wait(1);
  outportb(MOUSE_STATUS, 0xA8);

  mouse_wait(1);
  outportb(MOUSE_STATUS, 0x20);

  mouse_wait(0);
  status = (inportb(MOUSE_PORT) | 2);

  mouse_wait(1);
  outportb(MOUSE_STATUS, 0x60);

  mouse_wait(1);
  outportb(MOUSE_PORT, status);

  //set sample rate
  mouse_write(0xF6);
  mouse_read();

  //start sending packets
  mouse_write(0xF4);
  mouse_read();

  register_interrupt_handler(IRQ12, mouse_handler);
}