#ifndef WINDOW_SERVER_LAYOUT
#define WINDOW_SERVER_LAYOUT

#include <include/msgui.h>
#include <libc/gui/layout.h>

struct window *create_window(struct msgui_window *window);
void mouse_change(struct msgui_event *event);
void init_layout(struct framebuffer *fb);
void draw_layout();

#endif