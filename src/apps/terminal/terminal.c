#include <stdint.h>
#include <libc/unistd.h>
#include <libc/gui/layout.h>
#include <libc/stdlib.h>

int main()
{
  struct window *win = init_window(100, 100, 100, 100);
  enter_event_loop(win);

  return 0;
}
