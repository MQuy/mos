#ifndef WINDOW_SERVER_LAYOUT
#define WINDOW_SERVER_LAYOUT

#include <include/msgui.h>
#include <libc/gui/layout.h>

struct window *create_window(struct msgui_window *msgwin);
struct window *get_window_from_mouse_position(int32_t px, int32_t py);
void handle_mouse_event(struct msgui_event *event);
void handle_keyboard_event(struct msgui_event *event);
void handle_focus_event(struct msgui_focus *focus);
void init_layout(struct framebuffer *fb);
void draw_layout();

#endif
