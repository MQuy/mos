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
    else
    {
      // TODO: delegate events to active window
      struct msgui_event *event = buf->data;
      if (event->type == MSGUI_MOUSE)
        mouse_change(event);
      draw_layout();
    }
  }

  return 0;
}
