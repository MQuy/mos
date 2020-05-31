#ifndef DEVICES_MOUSE_H
#define DEVICES_MOUSE_H

#include <stdint.h>

struct mouse_device
{
  uint32_t x;
  uint32_t y;
  uint32_t state;
} mouse_device;

void mouse_init();

#endif
