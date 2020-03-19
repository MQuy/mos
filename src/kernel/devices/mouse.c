#include <kernel/cpu/idt.h>
#include <kernel/cpu/hal.h>
#include <kernel/memory/vmm.h>
#include <kernel/fs/vfs.h>
#include <kernel/system/uiserver.h>
#include <include/msgui.h>
#include "mouse.h"

#define MOUSE_PORT 0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT 0x02
#define MOUSE_BBIT 0x01
#define MOUSE_WRITE 0xD4
#define MOUSE_F_BIT 0x20
#define MOUSE_V_BIT 0x08

static uint8_t mouse_cycle = 0;
static uint8_t mouse_byte[4];
static struct mouse_device mouse_device_info;

void mouse_calculate_position()
{
  mouse_cycle = 0;
  uint8_t state = mouse_byte[0];
  int move_x = mouse_byte[1];
  int move_y = mouse_byte[2];

  if (move_x && state & (1 << 4))
  {
    /* Sign bit */
    move_x = move_x - 0x100;
  }
  if (move_y && state & (1 << 5))
  {
    /* Sign bit */
    move_y = move_y - 0x100;
  }
  if (state & (1 << 6) || state & (1 << 7))
  {
    /* Overflow */
    move_x = 0;
    move_y = 0;
  }

  mouse_device_info.x = move_x;
  mouse_device_info.y = move_y;

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

int32_t mouse_handler(interrupt_registers *regs)
{
  uint8_t status = inportb(MOUSE_STATUS);
  if ((status & MOUSE_BBIT) && (status & MOUSE_F_BIT))
  {

    uint8_t mouse_in = inportb(MOUSE_PORT);
    switch (mouse_cycle)
    {
    case 0:
      mouse_byte[0] = mouse_in;
      if (!(mouse_in & MOUSE_V_BIT))
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
      if (mouse_device_info.x != 0 || mouse_device_info.y != 0 || mouse_device_info.state != 0)
        enqueue_mouse_event(&mouse_device_info);
      break;
    }
  }

  return IRQ_HANDLER_CONTINUE;
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