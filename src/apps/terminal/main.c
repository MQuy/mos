#include <assert.h>
#include <fcntl.h>
#include <libgui/layout.h>
#include <libgui/psf.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>
#include <time.h>
#include <unistd.h>

#include "src/asci.h"
#include "src/terminal.h"

struct terminal *iterm;
struct window *app_win;
struct window *container_win;
int active_ptm;

static void init_terminal_dev(struct terminal *term)
{
	int fdm = posix_openpt(O_RDWR);
	int fds = open(ptsname(fdm), O_RDWR, 0);

	if (fdm < 0 || fds < 0)
		return;

	term->fd_ptm = fdm;
	term->fd_pts = fds;

	int pid = fork();

	if (!pid)
	{
		close(fdm);
		dup2(fds, 0);
		dup2(fds, 1);
		dup2(fds, 2);

		setsid();
		ioctl(fds, TIOCSCTTY, 0);
		// Maximum tty column and row to prevent gnu bash to columnize and rowize
		struct winsize ws;
		ioctl(fds, TIOCGWINSZ, &ws);
		ws.ws_col = MAX_COLUMNS;
		ws.ws_row = MAX_ROWS;
		ioctl(fds, TIOCSWINSZ, &ws);

		execve("/bin/bash", NULL, NULL);
	}
	else
	{
		close(fds);
		term->shell_pid = pid;
	}
}

static void handle_x11_event(struct xevent *evt)
{
	if (evt->type == XKEY_EVENT)
	{
		struct xkey_event *kevt = (struct xkey_event *)evt->data;
		if (kevt->action == XKEY_RELEASE)
		{
			int nbuf = 0;
			unsigned char buf[100] = {0};
			nbuf = convert_key_event_to_code(kevt->key, kevt->state, buf);

			if (nbuf > 0)
				write(active_ptm, (const char *)buf, nbuf);
		}
	}
}

static void handle_master_event(struct pollfd *pfds, unsigned int nfds)
{
	bool has_event = false;
	for (unsigned int i = 0; i < nfds; ++i)
	{
		if (pfds[i].revents & POLLIN && pfds[i].fd == active_ptm)
		{
			has_event = true;
			break;
		}
	}
	if (!has_event)
		return;

	char input[4096] = {0};
	int ret = read(active_ptm, input, sizeof(input));
	if (ret < 0)
		return;

	terminal_input(iterm, input);
	memset(container_win->graphic.buf, 0, container_win->graphic.width * container_win->graphic.height * 4);
	terminal_draw(iterm);
	gui_render(app_win);
}

int main()
{
	asci_init();
	app_win = init_window(50, 50, 656, 432);
	container_win = list_last_entry(&app_win->children, struct window, sibling);
	iterm = terminal_allocate(container_win);
	init_terminal_dev(iterm);

	active_ptm = iterm->fd_ptm;
	enter_event_loop(app_win, handle_x11_event, &active_ptm, 1, handle_master_event);
	return 0;
}
