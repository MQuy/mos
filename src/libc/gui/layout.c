#include <include/msgui.h>
#include <include/fcntl.h>
#include <include/mman.h>
#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/string.h>
#include "layout.h"

void gui_create_window(struct window *parent, struct window *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  char *pid = calloc(1, 10);
  itoa(getpid(), 10, pid);

  struct msgui *msgui_sender = malloc(sizeof(struct msgui));
  msgui_sender->type = MSGUI_WINDOW;
  struct msgui_window *msgwin = msgui_sender->data;
  msgwin->x = x;
  msgwin->y = y;
  msgwin->width = width;
  msgwin->height = height;
  msgwin->parent = NULL;
  msgwin->sender = pid;
  msgsnd(WINDOW_SERVER_SHM, msgui_sender, 0, sizeof(struct msgui));

  win->graphic.x = x;
  win->graphic.y = y;
  win->graphic.width = width;
  win->graphic.height = height;
  win->graphic.bg_color = 0;

  INIT_LIST_HEAD(&win->children);
  INIT_LIST_HEAD(&win->events);

  if (parent)
    list_add_tail(&win->sibling, &parent->children);

  win->name = malloc(WINDOW_NAME_LENGTH);
  msgrcv(pid, win->name, 0, WINDOW_NAME_LENGTH);

  uint32_t buf_size = width * height * 4;
  int32_t fd = shm_open(win->name, O_RDWR | O_CREAT, 0);
  win->graphic.buf = mmap(NULL, buf_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd);
}

void gui_create_label(struct window *parent, struct ui_label *label, int32_t x, int32_t y, uint32_t width, uint32_t height, char *text)
{
  gui_create_window(parent, &label->window, x, y, width, height);
}

void gui_create_input(struct window *parent, struct ui_input *input, int32_t x, int32_t y, uint32_t width, uint32_t height, char *content)
{
  gui_create_window(parent, &input->window, x, y, width, height);
}

__attribute__((always_inline)) void set_pixel(char *pixel_dest, uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha_raw)
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