#ifndef LIBC_GUI_LAYOUT_H
#define LIBC_GUI_LAYOUT_H

#include <stdint.h>
#include <include/list.h>
#include <include/msgui.h>
#include <libc/hashtable/hashmap.h>
#include "framebuffer.h"
#include "event.h"

struct graphic
{
  char *buf;
  int32_t x, y;
  uint32_t width, height;
};

struct ui_mouse
{
  struct graphic graphic;
  int32_t state;
};

struct icon
{
  char *label;
  char *exec_path;
  char *icon_path;
  struct graphic icon_graphic;
  struct graphic box_graphic;
  bool active;
};

struct desktop
{
  struct graphic graphic;
  struct ui_mouse mouse;
  struct framebuffer *fb;
  struct window *active_window;
  struct list_head children;
  struct hashmap icons;
};

struct window
{
  char name[WINDOW_NAME_LENGTH];
  struct graphic graphic;
  struct desktop *parent;
  struct widget *active_widget;
  struct list_head sibling;
  struct list_head children;
  struct list_head events;
};

struct ui_label
{
  struct window window;
  char *text;
  void (*set_text)(char *text);
};

struct ui_input
{
  struct window window;
  char *value;
};

void gui_create_window(struct window *parent, struct window *win, int32_t x, int32_t y, uint32_t width, uint32_t height);
void gui_create_label(struct window *parent, struct ui_label *label, int32_t x, int32_t y, uint32_t width, uint32_t height, char *text);
void gui_create_input(struct window *parent, struct ui_input *input, int32_t x, int32_t y, uint32_t width, uint32_t height, char *content);
void enter_event_loop(struct window *win);

#define _inline inline __attribute__((always_inline))

static _inline void set_pixel(char *pixel_dest, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha_raw)
{
  uint8_t red_dest = pixel_dest[0];
  uint8_t green_dest = pixel_dest[1];
  uint8_t blue_dest = pixel_dest[2];
  uint8_t alpha_raw_dest = pixel_dest[3];

  float alpha = alpha_raw / (float)255;
  float alpha_dest = alpha_raw_dest / (float)255;

  float adj = (1 - alpha) * alpha_dest;
  pixel_dest[0] = red * alpha + adj * red_dest;
  pixel_dest[1] = green * alpha + adj * green_dest;
  pixel_dest[2] = blue * alpha + adj * blue_dest;
  pixel_dest[3] = (alpha + adj) * 255;
}

#endif