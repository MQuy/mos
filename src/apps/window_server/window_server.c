#include <stdint.h>
#include <include/msgui.h>
#include <libc/unistd.h>
#include <libc/gui/layout.h>
#include <libc/stdlib.h>
#include "src/window_manager.h"

int main(struct framebuffer fb)
{
  init_layout(&fb);
  draw_layout();

  while (true)
  {
    struct msgui *buf = malloc(sizeof(struct msgui));
    msgrcv(WINDOW_SERVER_SHM, buf, 0, sizeof(struct msgui));
    if (buf->type == MSGUI_WINDOW)
    {
      struct msgui_window *msgwin = (struct msgui_window *)buf->data;
      struct window *win = create_window(msgwin);
      msgsnd(msgwin->sender, win->name, 0, WINDOW_NAME_LENGTH);
    }
    else if (buf->type == MSGUI_EVENT)
    {
      struct msgui_event *event = buf->data;
      if (event->type == MSGUI_MOUSE)
        handle_mouse_event(event);
      else if (event->type == MSGUI_KEYBOARD)
        handle_keyboard_event(event);
      draw_layout();
    }
    else if (buf->type == MSGUI_FOCUS)
    {
      struct msgui_focus *focus = buf->data;
      handle_focus_event(focus);
      draw_layout();
    }
  }

  return 0;
}
