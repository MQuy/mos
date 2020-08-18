#ifndef LIBC_GUI_EVENT_H
#define LIBC_GUI_EVENT_H

#include <include/list.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define BUTTON_LEFT 0x01
#define BUTTON_RIGHT 0x02
#define BUTTON_MIDDLE 0x04

enum xevent_action
{
	XKEY_PRESS = 0,
	XKEY_RELEASE = 1,
	XKEY_REPEAT = 2,
	XBUTTON_PRESS = 3,
	XBUTTON_RELEASE = 4,
};

struct xbutton_event
{
	enum xevent_action action;
	int32_t x;
	int32_t y;
	uint8_t button;
};

struct xmotion_event
{
	int32_t x;
	int32_t y;
};

struct xkey_event
{
	enum xevent_action action;
	uint32_t key;
};

enum xevent_type
{
	XMOTION_EVENT,
	XBUTTON_EVENT,
	XKEY_EVENT,
};

struct xevent
{
	enum xevent_type type;
	char data[32];
};

struct xevent *create_xkey_event(uint32_t key, enum xevent_action action);
struct xevent *create_xmotion_event(int32_t x, int32_t y);
struct xevent *create_xbutton_event(uint8_t button, enum xevent_action action, int32_t x, int32_t y);

#endif
