#ifndef INCLUDE_MSGUI_H
#define INCLUDE_MSGUI_H

#include <stdint.h>

// NOTE: MQ 2020-03-21 window name's length is 6, plus the null-terminated '\0'
#define WINDOW_NAME_LENGTH 7
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
  int32_t key;
};

struct msgui_window
{
  int32_t x, y;
  uint32_t width, height;
  char parent[WINDOW_NAME_LENGTH];
  char sender[WINDOW_NAME_LENGTH];
};

struct msgui_focus
{
  char sender[WINDOW_NAME_LENGTH];
};

enum msgui_type
{
  MSGUI_WINDOW,
  MSGUI_EVENT,
  MSGUI_RENDER,
  MSGUI_FOCUS,
};

struct msgui
{
  enum msgui_type type;
  char data[128];
};

#endif
