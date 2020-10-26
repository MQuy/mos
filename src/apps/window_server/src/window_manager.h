#ifndef WINDOW_MANAGER_H
#define WINDOW_MANAGER_H

#include <libgui/event.h>
#include <libgui/layout.h>
#include <libgui/msgui.h>

struct mouse_event
{
	int32_t x;
	int32_t y;
	uint8_t buttons;
	unsigned int state;
} mouse_event;

enum key_event_type
{
	KEY_PRESS = 0,
	KEY_RELEASE = 1,
};

struct key_event
{
	enum key_event_type type;
	unsigned int key;
	unsigned int state;
};

struct window *create_window(struct msgui_window *msgwin);
void init_layout(struct framebuffer *fb);
void draw_layout();
void draw_window_in_layout(char *name);
void handle_mouse_event(struct mouse_event *event);
void handle_keyboard_event(struct key_event *event);
void handle_focus_event(struct msgui_focus *focus);
void handle_window_remove(struct msgui_close *msgclose);

#endif
