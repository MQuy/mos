#include <stdint.h>
#include <libc/unistd.h>
#include <libc/gui/layout.h>
#include <libc/stdlib.h>

int main(struct framebuffer fb)
{
  struct window *win = malloc(sizeof(struct window));
  gui_create_window(NULL, win, 100, 100, 100, 100);
  enter_event_loop(win);

  return 0;
}
