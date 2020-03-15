#include <include/fcntl.h>
#include <include/mman.h>
#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/string.h>
#include <libc/bmp.h>
#include "window_manager.h"

struct desktop *desktop;
uint32_t nwin = 1;

char *get_window_name()
{
  char *wd = calloc(1, WINDOW_NAME_LENGTH + 1);
  memcpy(wd, "wd0000", WINDOW_NAME_LENGTH);
  for (int32_t i = 5, inum = nwin++; i > 1; --i, inum /= 10)
    wd[i] = (inum % 10) + '0';
  return wd;
}

struct window *find_window_in_window(struct window *win, char *name)
{
  if (!strcmp(win->name, name))
    return win;

  struct window *iter;
  list_for_each_entry(iter, &win->children, sibling)
  {
    struct window *temp = find_window_in_window(iter, name);
    if (temp)
      return temp;
  }
  return NULL;
}

struct window *find_window_in_root(char *name)
{
  struct window *iter;
  list_for_each_entry(iter, &desktop->children, sibling)
  {
    struct window *temp = find_window_in_window(iter, name);
    if (temp)
      return temp;
  }
  return NULL;
}

struct window *create_window(struct msgui_window *msgwin)
{
  char *window_name = get_window_name();
  int32_t fd = shm_open(window_name, O_RDWR | O_CREAT, 0);

  uint32_t screen_size = msgwin->height * msgwin->width * 4;
  ftruncate(fd, screen_size);

  struct window *win = malloc(sizeof(struct window));
  win->name = window_name;
  win->graphic.buf = mmap(NULL, screen_size, PROT_WRITE, MAP_SHARED, fd);
  win->graphic.x = msgwin->x;
  win->graphic.y = msgwin->y;
  win->graphic.width = msgwin->width;
  win->graphic.height = msgwin->height;
  win->graphic.bg_color = 0x000000;

  INIT_LIST_HEAD(&win->children);
  INIT_LIST_HEAD(&win->events);

  if (msgwin->parent)
  {
    struct window *parent = find_window_in_root(msgwin->parent);
    list_add_tail(&win->sibling, &parent->children);
  }
  else
    list_add_tail(&win->sibling, &desktop->children);

  return win;
}

void init_mouse()
{
  struct graphic *graphic = &desktop->mouse.graphic;
  graphic->x = 0;
  graphic->y = 0;
  graphic->width = 20;
  graphic->height = 20;
  graphic->bg_color = 0xFFFFFF;
  graphic->buf = malloc(graphic->width * graphic->height * 4);

  uint32_t fd = open("/images/cursor.bmp", NULL, NULL);
  struct stat *stat = malloc(sizeof(struct stat));
  fstat(fd, stat);
  char *buf = malloc(stat->size);
  read(fd, buf, stat->size);

  bmp_draw(graphic, buf, 0, 0);
}

void init_dekstop_graphic(struct framebuffer *fb)
{
  struct graphic *graphic = &desktop->graphic;
  graphic->x = 0;
  graphic->y = 0;
  graphic->width = fb->width;
  graphic->height = fb->height;
  graphic->bg_color = 0x000000;
  graphic->buf = malloc(fb->width * fb->height * 4);

  uint32_t fd = open("/images/background.bmp", NULL, NULL);
  struct stat *stat = malloc(sizeof(struct stat));
  fstat(fd, stat);
  char *buf = malloc(stat->size);
  read(fd, buf, stat->size);

  bmp_draw(graphic, buf, 0, 0);
}

void init_layout(struct framebuffer *fb)
{
  desktop = malloc(sizeof(struct desktop));
  desktop->fb = fb;
  INIT_LIST_HEAD(&desktop->children);

  init_dekstop_graphic(fb);
  init_mouse();
}

void draw_graphic(char *buf, char *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  for (uint32_t i = 0; i < height; ++i)
  {
    char *ibuf = buf + (y + i) * desktop->fb->pitch + x * 4;
    char *iwin = win + i * width * 4;
    for (uint32_t j = 0; j < width; ++j)
    {
      *(uint32_t *)ibuf = *(uint32_t *)iwin;
      ibuf += 4;
      iwin += 4;
    }
  }
}

void draw_alpha_graphic(char *buf, char *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  for (uint32_t i = 0; i < height; ++i)
  {
    char *ibuf = buf + (y + i) * desktop->fb->pitch + x * 4;
    char *iwin = win + i * width * 4;
    for (uint32_t j = 0; j < width; ++j)
    {
      set_pixel(ibuf, iwin[0], iwin[1], iwin[2], iwin[3]);
      ibuf += 4;
      iwin += 4;
    }
  }
}

void draw_mouse(char *buf)
{
  struct graphic *graphic = &desktop->mouse.graphic;
  draw_alpha_graphic(buf, graphic->buf, graphic->x, graphic->y, graphic->width, graphic->height);
}

void draw_window(char *buf, struct window *win, int32_t px, int32_t py)
{
  int32_t ax = px + win->graphic.x;
  int32_t ay = py + win->graphic.y;

  draw_graphic(buf, win->graphic.buf, ax, ay, win->graphic.width, win->graphic.height);

  struct window *iter_w;
  list_for_each_entry(iter_w, &win->children, sibling)
  {
    draw_window(buf, iter_w, ax, ay);
  }
}

void draw_layout()
{
  char *buf = malloc(desktop->fb->pitch * desktop->fb->height);

  draw_graphic(buf, desktop->graphic.buf, 0, 0, desktop->graphic.width, desktop->graphic.height);
  struct window *iter_w;
  list_for_each_entry(iter_w, &desktop->children, sibling)
  {
    draw_window(buf, iter_w, 0, 0);
  }
  draw_mouse(buf);

  memcpy(desktop->fb->addr, buf, desktop->fb->pitch * desktop->fb->height);
  free(buf);
}

void mouse_change(struct msgui_event *event)
{
  struct graphic *graphic = &desktop->mouse.graphic;

  graphic->x += event->mouse_x;
  graphic->y += event->mouse_y;

  if (graphic->x < 0)
    graphic->x = 0;
  else if (graphic->x > desktop->graphic.width - graphic->width)
    graphic->x = desktop->graphic.width - graphic->width;

  if (graphic->y < 0)
    graphic->y = 0;
  else if (graphic->y > desktop->graphic.height - graphic->height)
    graphic->y = desktop->graphic.height - graphic->height;

  desktop->mouse.state = event->mouse_state;
}