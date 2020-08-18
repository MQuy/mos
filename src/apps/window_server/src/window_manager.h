#ifndef WINDOW_SERVER_LAYOUT
#define WINDOW_SERVER_LAYOUT

#include <libc/gui/layout.h>
#include <libc/gui/msgui.h>

struct mouse_event
{
	int32_t x;
	int32_t y;
	uint8_t buttons;
} mouse_event;

enum key_event_type
{
	KEY_PRESS = 0,
	KEY_RELEASE = 1,
};

struct key_event
{
	enum key_event_type type;
	int32_t key;
};

struct window *create_window(struct msgui_window *msgwin);
struct window *get_window_from_mouse_position(int32_t px, int32_t py);
void handle_mouse_event(struct mouse_event *event);
void handle_keyboard_event(struct key_event *event);
void handle_focus_event(struct msgui_focus *focus);
void init_layout(struct framebuffer *fb);
void draw_layout();

#endif
