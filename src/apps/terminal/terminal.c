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
		dup2(fds, 2);

		setsid();
		execve("/bin/shell", NULL, NULL);
	}
	else
	{
		close(fds);
		tab->shell_pid = pid;
	}
}

void recalculate_tab(struct terminal_tab *tab)
{
	int lines = 0;
	int rows = 0;
	struct terminal_line *iter;
	list_for_each_entry(iter, &tab->lines, sibling)
	{
		iter->started_row = rows;
		rows += iter->rowspan;
		lines++;
	}
	tab->line_count = lines;
	tab->row_count = rows;
}

int pixels_from_content(const char *content)
{
	int columns = 0;
	for (char *ch = content; *ch; ch++)
	{
		columns += get_character_width(*ch);
	}
	return columns;
}

int count_rows_in_line_range(struct terminal_line *from_line, int from_row, struct terminal_line *to_line, int to_row)
{
	return (to_line->started_row + to_row) - (from_line->started_row + from_row);
}

void draw_cursor(struct terminal_line *from_line, int from_row, struct terminal_line *to_line, int to_row)
{
	struct terminal_tab *tab = iterm->active_tab;
	struct terminal_line *cursor_line = tab->cursor_line;
	int cursor_row = 0;
	int cursor_x = 0;
	int ix = 0;
	for (int i = 0, length = strlen(cursor_line->content); i < length && i < (int)tab->cursor_column; ++i)
	{
		if (ix + get_character_width(cursor_line->content[i]) >= (cursor_row + 1) * iterm->width)
		{
			cursor_row++;
			cursor_x = 0;
		}
		ix += get_character_width(cursor_line->content[i]);
		cursor_x += get_character_width(cursor_line->content[i]);
	}

	if (from_line->started_row + from_row >= cursor_line->started_row + cursor_row &&
		cursor_line->started_row + cursor_row <= to_line->started_row + to_row)
	{
		int relative_row = cursor_line->started_row + cursor_row - (from_line->started_row + from_row);
		gui_draw_retangle(win, cursor_x, relative_row * get_character_height(0), get_character_width(0), get_character_height(0), 0xd0d0d0ff);
	}
}

void draw_terminal_line(struct terminal_line *line, int from_row, int to_row, int prow)
{
	for (int i = 0, ix = 0, length = strlen(line->content); i < length; ++i)
	{
		if (ix + get_character_width(line->content[i]) >= from_row * iterm->width &&
			ix + get_character_width(line->content[i]) <= (to_row + 1) * iterm->width)
		{
			int jx = 0;
			while (i < length)
			{
				char ch = line->content[i];
				if (jx + get_character_width(ch) >= iterm->width)
					break;
				psf_putchar(ch,
							jx, prow * get_character_width(0),
							0xffffffff, 0,
							win->graphic.buf, win->graphic.width * 4);
				jx += get_character_width(ch);
				i++;
			}
			ix = div_ceil(ix + iterm->width, iterm->width) * iterm->width;
			continue;
		}
		ix += get_character_width(line->content[i]);
	}
}

void draw_terminal()
{
	struct terminal_tab *tab = iterm->active_tab;
	struct terminal_line *last_line = list_last_entry(&tab->lines, struct terminal_line, sibling);
	struct terminal_line *from_line, *to_line;
	int from_row, to_row;
	int rows = count_rows_in_line_range(tab->scroll_line, 0, last_line, last_line->rowspan - 1);
	if (rows <= 20)
	{
		int count = 0;
		from_row = 0;
		struct terminal_line *iter;
		list_for_each_entry_reverse(iter, &tab->lines, sibling)
		{
			from_line = iter;
			count += from_line->rowspan;
			if (count >= 20)
			{
				from_row = from_line->rowspan - (count - 20);
				break;
			}
		}
		to_line = last_line;
		to_row = last_line->rowspan - 1;
	}
	else
	{
		to_line = from_line = tab->scroll_line;
		from_row = 0;
		to_row = to_line->rowspan - 1;
		int count = 0;
		struct terminal_line *iter = to_line;
		list_for_each_entry_from(iter, &tab->lines, sibling)
		{
			to_line = iter;
			count += to_line->rowspan;
			if (count >= 20)
			{
				to_row = to_line->rowspan - (count - 20);
				break;
			}
		}
	}

	struct terminal_line *iter_line = from_line;
	int irow = 0;
	list_for_each_entry_from(iter_line, to_line->sibling.next, sibling)
	{
		int ifrom = 0;
		int ito = to_line->rowspan - 1;

		if (iter_line == from_line)
			ifrom = from_row;
		if (iter_line == to_line)
			ito = to_row;
		draw_terminal_line(iter_line, ifrom, ito, irow);
		irow += ito - ifrom + 1;
	}

	draw_cursor(from_line, from_row, to_line, to_row);
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

	struct terminal_tab *tab = iterm->active_tab;
	for (int i = 0, length = strlen(input); i < length; ++i)
	{
		char ch = input[i];

		if (ch == '\033')
		{
			// TODO: MQ 2020-09-13 handle ANSI escape sequences
		}
		else if (ch == '\21')
		{
			// if ch is Device Control 1 and cursor line content is empty -> record timestamp
			if (strlen(tab->cursor_line->content) == 0)
				tab->cursor_line->seconds = time(NULL);
		}
		else if (ch == '\n')
		{
			struct terminal_line *first_line = list_first_entry(&tab->lines, struct terminal_line, sibling);
			struct terminal_line *last_line = list_last_entry(&tab->lines, struct terminal_line, sibling);

			if (last_line && last_line->seconds)
				last_line->seconds = time(NULL);

			struct terminal_line *line = alloc_terminal_line();
			list_add_tail(&tab->lines, &line->sibling);

			if (tab->row_count == iterm->max_rows)
			{
				list_del(&first_line->sibling);
				free(first_line);
				recalculate_tab(tab);
			}
			else
			{
				line->started_row = last_line ? last_line->started_row + last_line->rowspan : 0;
				tab->line_count++;
				tab->row_count++;
			}

			if (tab->scroll_line == last_line)
				tab->scroll_line == line;
		}
		// printable characters
		else
		{
			struct terminal_line *cursor_line = tab->cursor_line;

			if (tab->cursor_column > CHARACTERS_PER_LINE)
				continue;

			cursor_line->content[tab->cursor_column++] = ch;

			int pixels = pixels_from_content(cursor_line->content);
			cursor_line->rowspan = div_ceil(pixels, iterm->width);
			recalculate_tab(tab);
		}
	}

	draw_terminal();
	gui_render(win);
}

int main(void)
{
	win = init_window(50, 50, 600, 400);
	struct terminal_tab *tab = alloc_terminal_tab();

	iterm = calloc(1, sizeof(struct terminal));
	iterm->width = win->graphic.width;
	iterm->height = win->graphic.height;
	iterm->active_tab = tab;
	iterm->max_rows = MAX_ROWS;
	INIT_LIST_HEAD(&iterm->tabs);
	list_add_tail(&iterm->active_tab->sibling, &iterm->tabs);
	init_terminal_tab_dev(tab);

	active_shell_pid = iterm->active_tab->fd_ptm;
	enter_event_loop(win, handle_x11_event, &active_shell_pid, 1, handle_slave_event);
	return 0;
}
