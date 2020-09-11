#include "terminal.h"

#include <libc/gui/layout.h>
#include <libc/poll.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>
#include <stdint.h>

struct terminal *iterm;
int active_shell_pid;

struct temrinal_tab *alloc_terminal_tab()
{
	struct terminal_line *line = calloc(1, sizeof(struct terminal_line));
	struct terminal_tab *tab = calloc(1, sizeof(struct terminal_tab));
	INIT_LIST_HEAD(&tab->lines);
	list_add_tail(&line->sibling, &tab->lines);
	return tab;
}

void init_terminal_tab_dev(struct terminal_tab *tab)
{
	int fdm = posix_openpt(O_RDWR);
	int fds = open(ptsname(fdm), O_RDWR, 0);

	if (fdm < 0 || fds < 0)
		return;

	tab->pty[0] = fdm;
	tab->pty[1] = fds;

	int pid = fork();

	if (!pid)
	{
		close(fdm);
		dup2(fds, 0);
		dup2(fds, 1);
		dup2(fds, 3);

		setsid();
		execve("/bin/shell", NULL, NULL);
	}
	else
	{
		close(fds);
		tab->shell_pid = pid;
	}
}

void draw_terminal()
{
}

void handle_slave_event(struct pollfd *pfds, unsigned int nfds)
{
	bool has_event = false;
	for (int i = 0; i < nfds; ++i)
	{
		if (pfds[i].revents & POLLIN && pfds[i].fd == active_shell_pid)
		{
			has_event = true;
			break;
		}
	}
	if (!has_event)
		return;

	char input[4096] = {0};
	int ret = read(active_shell_pid, input, sizeof(input));
	if (ret < 0)
		return;

	for (int i = 0, length = strlen(input); i < length; ++i)
	{
		char ch = input[i];
		if (ch == '\l')
		{
		}
		draw_terminal();
	}
}

int main(void)
{
	struct window *win = init_window(50, 50, 600, 400);
	struct terminal_tab *tab = alloc_terminal_tab();

	iterm = calloc(1, sizeof(struct terminal));
	iterm->width = win->graphic.width;
	iterm->height = win->graphic.height;
	iterm->active_tab = tab;
	iterm->max_rows = iterm->height / PIXELS_PER_ROW;
	INIT_LIST_HEAD(&iterm->tabs);
	list_add_tail(&iterm->active_tab->sibling, &iterm->tabs);
	init_terminal_tab_dev(tab);

	active_shell_pid = iterm->active_tab->pty[1];
	enter_event_loop(win, &active_shell_pid, 1, handle_slave_event);
	return 0;
}
