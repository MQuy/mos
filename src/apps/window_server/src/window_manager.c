#include <include/fcntl.h>
#include <include/mman.h>
#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/string.h>
#include <libc/hashtable/hashmap.h>
#include <libc/bmp.h>
#include <libc/ini/ini.h>
#include <libc/gui/psf.h>
#include "window_manager.h"

struct desktop *desktop;
uint32_t nwin = 1;

char *get_window_name()
{
  char *wd = calloc(1, WINDOW_NAME_LENGTH);
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
  memcpy(win->name, window_name, WINDOW_NAME_LENGTH);
  win->graphic.buf = mmap(NULL, screen_size, PROT_WRITE, MAP_SHARED, fd);
  win->graphic.x = msgwin->x;
  win->graphic.y = msgwin->y;
  win->graphic.width = msgwin->width;
  win->graphic.height = msgwin->height;
  for (uint32_t i = 0; i < msgwin->height; ++i)
  {
    char *ibuf = win->graphic.buf + i * msgwin->height * 4;
    for (uint32_t j = 0; j < msgwin->width; ++j)
    {
      ibuf[0] = 0xFF;
      ibuf[1] = 0xFF;
      ibuf[2] = 0xFF;
      ibuf[3] = 0xFF;
      ibuf += 4;
    }
  }

  INIT_LIST_HEAD(&win->children);
  INIT_LIST_HEAD(&win->events);

  if (msgwin->parent && strlen(msgwin->parent))
  {
    struct window *parent = find_window_in_root(msgwin->parent);
    list_add_tail(&win->sibling, &parent->children);
  }
  else
    list_add_tail(&win->sibling, &desktop->children);

  return win;
}

static int icon_ini_handler(void *_desktop, const char *section, const char *name,
                            const char *value)
{
  struct icon *icon = hashmap_get(&desktop->icons, section);
  if (!icon)
  {
    icon = malloc(sizeof(struct icon));

    icon->icon_graphic.x = 20;
    icon->icon_graphic.y = 4;
    icon->icon_graphic.width = icon->icon_graphic.height = 48;
    icon->icon_graphic.buf = malloc(icon->icon_graphic.width * icon->icon_graphic.height * 4);

    icon->box_graphic.width = 88;
    icon->box_graphic.height = 82;
    icon->box_graphic.buf = malloc(icon->box_graphic.width * icon->box_graphic.height * 4);

    hashmap_put(&desktop->icons, section, icon);
  }
  if (!strcmp(name, "label"))
    icon->label = strdup(value);
  else if (!strcmp(name, "icon"))
    icon->icon_path = strdup(value);
  else if (!strcmp(name, "path"))
    icon->exec_path = strdup(value);
  else if (!strcmp(name, "px"))
    icon->box_graphic.x = atoi(value);
  else if (!strcmp(name, "py"))
    icon->box_graphic.y = atoi(value);
}

void init_icons()
{
  hashmap_init(&desktop->icons, hashmap_hash_string, hashmap_compare_string, 0);
  ini_parse("/etc/desktop.ini", icon_ini_handler, &desktop);

  struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
  while (iter)
  {
    struct icon *icon = hashmap_iter_get_data(iter);
    struct graphic *graphic = &icon->box_graphic;

    uint32_t fd = open(icon->icon_path, NULL, NULL);
    struct stat *stat = malloc(sizeof(struct stat));
    fstat(fd, stat);
    char *buf = malloc(stat->size);
    read(fd, buf, stat->size);
    bmp_draw(&icon->icon_graphic, buf, 0, 0);

    iter = hashmap_iter_next(&desktop->icons, iter);
  }
}

void init_mouse()
{
  struct graphic *graphic = &desktop->mouse.graphic;
  graphic->x = 0;
  graphic->y = 0;
  graphic->width = 20;
  graphic->height = 20;
  graphic->buf = malloc(graphic->width * graphic->height * 4);

  uint32_t fd = open("/usr/share/images/cursor.bmp", NULL, NULL);
  struct stat *stat = malloc(sizeof(struct stat));
  fstat(fd, stat);
  char *buf = malloc(stat->size);
  read(fd, buf, stat->size);

  bmp_draw(graphic, buf, 0, 0);
}

void init_dekstop_graphic()
{
  struct graphic *graphic = &desktop->graphic;
  graphic->x = 0;
  graphic->y = 0;
  graphic->width = desktop->fb->width;
  graphic->height = desktop->fb->height;
  graphic->buf = malloc(graphic->width * graphic->height * 4);

  uint32_t fd = open("/usr/share/images/background.bmp", NULL, NULL);
  struct stat *stat = malloc(sizeof(struct stat));
  fstat(fd, stat);
  char *buf = malloc(stat->size);
  read(fd, buf, stat->size);

  bmp_draw(graphic, buf, 0, 0);
}

void init_fonts()
{
  uint32_t fd = open("/usr/share/fonts/ter-powerline-v16n.psf", NULL, NULL);
  struct stat *stat = malloc(sizeof(struct stat));
  fstat(fd, stat);
  char *buf = malloc(stat->size);
  read(fd, buf, stat->size);
  psf_init(buf, stat->size);
}

void init_layout(struct framebuffer *fb)
{
  desktop = malloc(sizeof(struct desktop));
  desktop->fb = fb;
  INIT_LIST_HEAD(&desktop->children);

  init_dekstop_graphic();
  init_fonts();
  init_icons();
  init_mouse();
}

void draw_graphic(char *buf, uint32_t scanline, char *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  for (uint32_t i = 0; i < height; ++i)
  {
    char *ibuf = buf + (y + i) * scanline + x * 4;
    char *iwin = win + i * width * 4;
    for (uint32_t j = 0; j < width; ++j)
    {
      *(uint32_t *)ibuf = *(uint32_t *)iwin;
      ibuf += 4;
      iwin += 4;
    }
  }
}

void draw_alpha_graphic(char *buf, uint32_t scanline, char *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  for (uint32_t i = 0; i < height; ++i)
  {
    char *ibuf = buf + (y + i) * scanline + x * 4;
    char *iwin = win + i * width * 4;
    for (uint32_t j = 0; j < width; ++j)
    {
      set_pixel(ibuf, iwin[0], iwin[1], iwin[2], iwin[3]);
      ibuf += 4;
      iwin += 4;
    }
  }
}

void draw_desktop_icons(char *buf)
{
  struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
  while (iter)
  {
    struct icon *icon = hashmap_iter_get_data(iter);
    struct graphic *box_graphic = &icon->box_graphic;
    struct graphic *icon_graphic = &icon->icon_graphic;

    // 88x82
    // --------- 4 --------
    // 16 - 4 - 48 - 4 - 16
    // --------- 4 --------
    // --------- 2 --------
    // 4 ------- 24 ----- 4
    if (icon->active)
    {
      for (uint32_t j = 0; j < 56; ++j)
      {
        char *iblock = box_graphic->buf + j * box_graphic->width * 4 + 16 * 4;
        for (uint32_t i = 0; i < 56; ++i)
        {
          iblock[0] = 0xAA;
          iblock[1] = 0xAA;
          iblock[2] = 0xAA;
          iblock[3] = 0x33;
          iblock += 4;
        }
      }
    }
    else
      memset(box_graphic->buf, 0, box_graphic->width * box_graphic->height * 4);

    uint8_t label_length = strlen(icon->label);
    // TODO Implement multi lines label
    if (label_length <= 20)
    {
      uint8_t padding = ((20 - label_length) / 2) * 16;
      psf_puts(icon->label, 4 + padding, 58, 0xffffffff, 0x00000000, box_graphic->buf, box_graphic->width * 4);
    }
    draw_alpha_graphic(
        box_graphic->buf, box_graphic->width * 4,
        icon_graphic->buf, icon_graphic->x, icon_graphic->y, icon_graphic->width, icon_graphic->height);
    draw_alpha_graphic(
        buf, desktop->fb->pitch,
        box_graphic->buf, box_graphic->x, box_graphic->y, box_graphic->width, box_graphic->height);

    iter = hashmap_iter_next(&desktop->icons, iter);
  }
}

void draw_mouse(char *buf)
{
  struct graphic *graphic = &desktop->mouse.graphic;
  draw_alpha_graphic(buf, desktop->fb->pitch, graphic->buf, graphic->x, graphic->y, graphic->width, graphic->height);
}

void draw_window(char *buf, struct window *win, int32_t px, int32_t py)
{
  int32_t ax = px + win->graphic.x;
  int32_t ay = py + win->graphic.y;

  draw_graphic(buf, desktop->fb->pitch, win->graphic.buf, ax, ay, win->graphic.width, win->graphic.height);

  struct window *iter_w;
  list_for_each_entry(iter_w, &win->children, sibling)
  {
    draw_window(buf, iter_w, ax, ay);
  }
}

// TODO: MQ 2020-03-21 Improve render speed via dirty rects
void draw_layout()
{
  char *buf = malloc(desktop->fb->pitch * desktop->fb->height);

  draw_graphic(buf, desktop->fb->pitch, desktop->graphic.buf, 0, 0, desktop->graphic.width, desktop->graphic.height);
  draw_desktop_icons(buf);

  struct window *iter_win;
  list_for_each_entry(iter_win, &desktop->children, sibling)
  {
    draw_window(buf, iter_win, 0, 0);
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

struct window *get_window_from_mouse_position(int32_t px, int32_t py)
{
  struct window *iter_win;
  list_for_each_entry(iter_win, &desktop->children, sibling)
  {
    if (iter_win->graphic.x < px && px < iter_win->graphic.x + iter_win->graphic.width &&
        iter_win->graphic.y < py && py < iter_win->graphic.y + iter_win->graphic.height)
      return iter_win;
  }
  return NULL;
}

struct window *get_active_window()
{
  return desktop->active_window;
}

struct icon *find_icon_from_mouse_position(int32_t px, int32_t py)
{
  struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
  while (iter)
  {
    struct icon *icon = hashmap_iter_get_data(iter);
    struct graphic *graphic = &icon->box_graphic;

    if (graphic->x <= px && px <= graphic->x + graphic->width &&
        graphic->y <= py && py <= graphic->y + graphic->height)
      return icon;

    iter = hashmap_iter_next(&desktop->icons, iter);
  }
  return NULL;
}

void handle_mouse_event(struct msgui_event *event)
{
  mouse_change(event);

  if (event->mouse_state == MOUSE_LEFT_CLICK)
  {
    struct ui_mouse *mouse = &desktop->mouse;
    struct window *active_win = get_window_from_mouse_position(mouse->graphic.x, mouse->graphic.y);
    if (active_win && active_win == desktop->active_window)
    {
      struct ui_event *ui_event = malloc(sizeof(ui_event));
      ui_event->event_type = MOUSE_CLICK;
      ui_event->mouse_x = desktop->mouse.graphic.x;
      ui_event->mouse_y = desktop->mouse.graphic.y;
      msgsnd(active_win->name, ui_event, 0, sizeof(struct ui_event));
    }
    else if (active_win)
      desktop->active_window = active_win;
    else
    {
      struct icon *icon = find_icon_from_mouse_position(mouse->graphic.x, mouse->graphic.y);
      if (icon)
      {
        if (icon->active)
          posix_spawn(icon->exec_path);
        else
          icon->active = true;
      }
      struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
      while (iter)
      {
        struct icon *i = hashmap_iter_get_data(iter);
        if (i != icon)
          i->active = false;
        iter = hashmap_iter_next(&desktop->icons, iter);
      }
    }
  }
}

void handle_keyboard_event(struct msgui_event *event)
{
}