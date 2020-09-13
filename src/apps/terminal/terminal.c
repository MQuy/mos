#include "terminal.h"

#include <libc/gui/layout.h>
#include <libc/gui/psf.h>
#include <libc/math.h>
#include <libc/poll.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>
#include <stdint.h>

struct terminal *iterm;
struct window *win;
int active_shell_pid;

struct terminal_line *alloc_terminal_line()
{
	struct terminal_line *line = calloc(1, sizeof(struct terminal_line));
	line->rowspan = 1;
	return line;
}

struct terminal_tab *alloc_terminal_tab()
{
	struct terminal_tab *tab = calloc(1, sizeof(struct terminal_tab));
	struct terminal_line *line = alloc_terminal_line();
	tab->scroll_line = line;
	tab->cursor_line = line;
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

	tab->fd_ptm = fdm;
	tab->fd_pts = fds;

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

int pixels_per_character(unsigned char ch)
{
	return (ch == '\t' ? 4 : 1) * PIXELS_PER_CHARACTER;
}

void recalculate_tab_line_and_row(struct terminal_tab *tab)
{
	int lines = 0;
	int rows = 0;

	struct terminal_line *iter;
	list_for_each_entry(iter, &tab->lines, sibling)
	{
		lines++;
		rows += iter->rowspan;
	}
	tab->line_count = lines;
	tab->row_count = rows;
}

int pixels_from_content(const char *content)
{
	int columns = 0;
	for (char *ch = content; *ch; ch++)
	{
		columns += pixels_per_character(*ch);
	}
	return columns;
}

int count_rows_in_line_range(struct terminal_line *from, struct terminal_line *to)
{
	int rows = to->rowspan;
	struct terminal_line *iter = from;
	list_for_each_entry_from(iter, &to->sibling, sibling)
	{
		rows += iter->rowspan;
	}
	return rows;
}

void draw_cursor()
{
}

void draw_terminal_line(struct terminal_line *line, int from_rowspan, int to_rowspan, int prow)
{
	for (int i = 0, ix = 0, length = strlen(line->content); i < length; ++i)
	{
		if (ix + pixels_per_character(line->content[i]) >= from_rowspan * iterm->width &&
			ix + pixels_per_character(line->content[i]) <= (to_rowspan + 1) * iterm->width)
		{
			int jx = 0;
			while (i < length)
			{
				char ch = line->content[i];
				if (jx + pixels_per_character(ch) >= iterm->width)
					break;
				psf_putchar(ch,
							jx, prow * PIXELS_PER_CHARACTER,
							0xffffff, 0,
							win->graphic.buf, win->graphic.width * 4);
				jx += pixels_per_character(ch);
				i++;
			}
			ix = div_ceil(ix + iterm->width, iterm->width) * iterm->width;
			continue;
		}
		ix += pixels_per_character(line->content[i]);
	}
}

void draw_terminal()
{
	struct terminal_tab *tab = iterm->active_tab;
	struct terminal_line *from_line, *to_line;
	struct terminal_line *last_line = list_last_entry(&tab->lines, struct terminal_line, sibling);
	int from_rowspan, to_rowspan;
	int rows = count_rows_in_line_range(tab->scroll_line, last_line);
	if (rows <= 20)
	{
		int count = 0;
		from_rowspan = 0;
		list_for_each_entry_reverse(from_line, &tab->lines, sibling)
		{
			count += from_line->rowspan;
			if (count >= 20)
			{
				from_rowspan = from_line->rowspan - (count - 20);
				break;
			}
		}
		to_line = last_line;
		to_rowspan = last_line->rowspan;
	}
	else
	{
		to_line = from_line = tab->scroll_line;
		from_rowspan = 0;
		to_rowspan = to_line->rowspan;
		int count = 0;
		list_for_each_entry_from(to_line, &tab->lines, sibling)
		{
			count += to_line->rowspan;
			if (count >= 20)
			{
				to_rowspan = to_line->rowspan - (count - 20);
				break;
			}
		}
	}

	struct terminal_line *iter_line;
	int irow = 0;
	list_for_each_entry_from(iter_line, to_line->sibling.next, sibling)
	{
		if (iter_line == from_line)
		{
			draw_terminal_line(iter_line, from_rowspan, iter_line->rowspan - 1, irow);
			irow += iter_line->rowspan - from_rowspan;
		}
		else if (iter_line == to_line)
		{
			draw_terminal_line(iter_line, 0, to_rowspan, irow);
			irow += to_rowspan + 1;
		}
		else
		{
			draw_terminal_line(iter_line, 0, iter_line->rowspan - 1, irow);
			irow += iter_line->rowspan;
		}
	}
	draw_cursor();
}

void handle_x11_event(struct xevent *evt)
{
	if (evt->type == XKEY_EVENT)
	{
		int nbuf;
		unsigned char buf[100] = {0};
		struct xkey_event *kevt = (struct xkey_event *)evt->data;
		convert_keycode_to_ascii(kevt->key, kevt->state, buf, &nbuf);

		write(active_shell_pid, (const char *)buf, nbuf);
	}
}

void handle_slave_event(struct pollfd *pfds, unsigned int nfds)
{
	bool has_event = false;
	for (unsigned int i = 0; i < nfds; ++i)
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

		if (ch == '\033')
		{
			// TODO: MQ 2020-09-13 handle ANSI escape sequences
		}
		else if (ch == '\n')
		{
			struct terminal_tab *tab = iterm->active_tab;
			struct terminal_line *line = alloc_terminal_line();
			list_add_tail(&tab->lines, &line->sibling);

			if (tab->row_count == iterm->max_rows)
			{
				struct terminal_line *first_line = list_first_entry(&tab->lines, struct terminal_line, sibling);
				list_del(&first_line->sibling);
				free(first_line);
				recalculate_tab_line_and_row(tab);
			}
			else
			{
				tab->line_count++;
				tab->row_count++;
			}

			int nrows = count_rows_in_line_range(tab->scroll_line, line);
			if (nrows <= iterm->height / PIXELS_PER_CHARACTER)
				tab->scroll_line = line;
		}
		// printable characters
		else
		{
			struct terminal_tab *tab = iterm->active_tab;
			struct terminal_line *line = tab->cursor_line;
			line->content[tab->cursor_column++] = ch;

			int pixels = pixels_from_content(line->content);
			line->rowspan = div_ceil(pixels, iterm->width);
			recalculate_tab_line_and_row(tab);
		}

		draw_terminal();
	}
}

int main(void)
{
	win = init_window(50, 50, 600, 400);
	struct terminal_tab *tab = alloc_terminal_tab();

	iterm = calloc(1, sizeof(struct terminal));
	iterm->width = win->graphic.width;
	iterm->height = win->graphic.height;
	iterm->active_tab = tab;
	iterm->max_rows = iterm->height / PIXELS_PER_CHARACTER;
	INIT_LIST_HEAD(&iterm->tabs);
	list_add_tail(&iterm->active_tab->sibling, &iterm->tabs);
	init_terminal_tab_dev(tab);

	active_shell_pid = iterm->active_tab->fd_pts;
	enter_event_loop(win, handle_x11_event, &active_shell_pid, 1, handle_slave_event);
	return 0;
}
