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

enum kybrd_type
{
	KEY_PRRESS,
	KEY_RELEASE,
};

struct kybrd_event
{
	enum kybrd_type type;
	int32_t key;
};

struct window *create_window(struct msgui_window *msgwin);
struct window *get_window_from_mouse_position(int32_t px, int32_t py);
void handle_mouse_event(struct mouse_event *event);
void handle_keyboard_event(struct kybrd_event *event);
void handle_focus_event(struct msgui_focus *focus);
void init_layout(struct framebuffer *fb);
void draw_layout();

#endif
