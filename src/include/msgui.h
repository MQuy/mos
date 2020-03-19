#ifndef INCLUDE_MSGUI_H
#define INCLUDE_MSGUI_H

#include <stdint.h>

#define WINDOW_NAME_LENGTH 6
#define WINDOW_SERVER_SHM "/dev/shm/window_server"

#define MOUSE_LEFT_CLICK 0x01
#define MOUSE_RIGHT_CLICK 0x02
#define MOUSE_MIDDLE_CLICK 0x04

enum msgui_event_type
{
  MSGUI_KEYBOARD,
  MSGUI_MOUSE,
};

struct msgui_event
{
  enum msgui_event_type type;
  int32_t mouse_x, mouse_y;
  int32_t mouse_state;
  int32_t keycode;
};

struct msgui_window
{
  int32_t x, y;
  uint32_t width, height;
  char *parent;
  char *sender;
};

enum msgui_type
{
  MSGUI_WINDOW,
  MSGUI_EVENT,
};

struct msgui
{
  enum msgui_type type;
  char data[128];
};

#endif