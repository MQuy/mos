#ifndef LIBC_GUI_EVENT_H
#define LIBC_GUI_EVENT_H

#include <include/list.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

enum ui_event_type
{
  KEY_PRESS,
  MOUSE_MOVE,
  MOUSE_CLICK,
};

struct ui_event
{
  enum ui_event_type event_type;
  struct list_head listeners;
};

struct ui_queue
{
  enum ui_event_type event_type;
  int32_t keychar;
  bool shift, ctrl;
  struct ui_queue *next;
};

#endif