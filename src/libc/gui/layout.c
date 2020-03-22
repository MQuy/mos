#include <include/msgui.h>
#include <include/fcntl.h>
#include <include/mman.h>
#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/string.h>
#include "layout.h"

void gui_create_window(struct window *parent, struct window *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
  char *pid = calloc(1, WINDOW_NAME_LENGTH);
  itoa(getpid(), 10, pid);

  struct msgui *msgui_sender = malloc(sizeof(struct msgui));
  msgui_sender->type = MSGUI_WINDOW;
  struct msgui_window *msgwin = msgui_sender->data;
  msgwin->x = x;
  msgwin->y = y;
  msgwin->width = width;
  msgwin->height = height;
  if (parent)
    memcpy(msgwin->parent, parent->name, WINDOW_NAME_LENGTH);
  memcpy(msgwin->sender, pid, WINDOW_NAME_LENGTH);
  msgopen(msgwin->sender, 0);
  msgsnd(WINDOW_SERVER_SHM, msgui_sender, 0, sizeof(struct msgui));

  win->graphic.x = x;
  win->graphic.y = y;
  win->graphic.width = width;
  win->graphic.height = height;

  INIT_LIST_HEAD(&win->children);
  INIT_LIST_HEAD(&win->events);

  if (parent)
    list_add_tail(&win->sibling, &parent->children);

  msgrcv(msgwin->sender, win->name, 0, WINDOW_NAME_LENGTH);

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

void enter_event_loop(struct window *win)
{
  struct msgui *msgui = malloc(sizeof(struct msgui));
  msgui->type = MSGUI_RENDER;
  msgsnd(WINDOW_SERVER_SHM, msgui, 0, sizeof(struct msgui));

  while (true)
  {
    // TODO MQ 2020-03-21 Add message listerns for ui event
  };
}