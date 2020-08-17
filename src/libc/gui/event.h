#ifndef LIBC_GUI_EVENT_H
#define LIBC_GUI_EVENT_H

#include <include/list.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MOUSE_LEFT_CLICK 0x01
#define MOUSE_RIGHT_CLICK 0x02
#define MOUSE_MIDDLE_CLICK 0x04

enum ui_event_type
{
	KEY_PRESS,
	MOUSE_MOVE,
	MOUSE_CLICK,
};

struct ui_event
{
	enum ui_event_type event_type;
	int32_t key;
	bool shift, ctrl;
	int32_t mouse_x, mouse_y;
};

#endif
