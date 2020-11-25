#include "terminal.h"

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

#define VERTICAL_PADDING 4
#define HORIZONTAL_PADDING 8

struct terminal *iterm;
struct window *app_win;
struct window *container_win;
int active_ptm;

static struct terminal_line *alloc_terminal_line()
{
	struct terminal_line *line = calloc(1, sizeof(struct terminal_line));
	line->rowspan = 1;
	return line;
}

static struct terminal_tab *alloc_terminal_tab()
{
	struct terminal_tab *tab = calloc(1, sizeof(struct terminal_tab));
	struct terminal_line *line = alloc_terminal_line();
	tab->scroll_line = line;
	tab->cursor_line = line;
	tab->line_count = tab->row_count = 1;
	INIT_LIST_HEAD(&tab->lines);
	list_add_tail(&line->sibling, &tab->lines);

	return tab;
}

static void init_terminal_tab_dev(struct terminal_tab *tab)
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
		ioctl(fds, TIOCSCTTY, 0);
		execve("/bin/bash", NULL, NULL);
	}
	else
	{
		close(fds);
		tab->shell_pid = pid;
	}
}

static void recalculate_tab(struct terminal_tab *tab)
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

static int pixels_from_content(const char *content)
{
	int pixels = 0;
	for (char *ch = content; *ch; ch++)
	{
		pixels += get_character_width(*ch);
	}
	return pixels;
}

static __inline int count_rows_in_line_range(struct terminal_line *from_line, int from_row, struct terminal_line *to_line, int to_row)
{
	return (to_line->started_row + to_row) - (from_line->started_row + from_row);
}

static void draw_cursor(struct terminal_line *from_line, int from_row, struct terminal_line *to_line, int to_row)
{
	struct terminal_tab *tab = iterm->active_tab;
	struct terminal_line *cursor_line = tab->cursor_line;
	int cursor_row = 0;
	int cursor_x = 0;
	int ix = 0;
	int width = iterm->width - 2 * HORIZONTAL_PADDING;
	for (int i = 0, length = strlen(cursor_line->content); i < length && i < (int)tab->cursor_column; ++i)
	{
		if (ix + get_character_width(cursor_line->content[i]) >= (cursor_row + 1) * width)
		{
			cursor_row++;
			cursor_x = 0;
		}
		else
			cursor_x += get_character_width(cursor_line->content[i]);
		ix += get_character_width(cursor_line->content[i]);
	}

	int abs_from_row = from_line->started_row + from_row;
	int abs_to_row = to_line->started_row + to_row;
	int abs_row = cursor_line->started_row + cursor_row;
	int relative_row = abs_row - abs_from_row;
	int max_rows_in_ui = (container_win->graphic.height - 2 * VERTICAL_PADDING) / get_character_height(0);
	if (abs_from_row <= abs_row &&
		(abs_row <= abs_to_row ||
		 // when cursor in the new row and first column of multiple span rows last line
		 (abs_row == abs_to_row + 1 && cursor_x == 0 && relative_row <= max_rows_in_ui)))
		gui_draw_retangle(container_win, cursor_x + HORIZONTAL_PADDING, relative_row * get_character_height(0) + VERTICAL_PADDING, get_character_width(0), get_character_height(0), 0xd0d0d0ff);
}

static void draw_terminal_line(struct terminal_line *line, int from_row, int to_row, int prow)
{
	int width = iterm->width - 2 * HORIZONTAL_PADDING;
	int py = prow * get_character_height(0);
	for (int i = 0, ix = 0, length = strlen(line->content); i < length;)
	{
		if (ix + get_character_width(line->content[i]) >= from_row * width &&
			ix + get_character_width(line->content[i]) <= (to_row + 1) * width)
		{
			int jx = 0;
			while (i < length)
			{
				char ch = line->content[i];
				if (jx + get_character_width(ch) > width)
					break;
				psf_putchar(ch,
							jx + HORIZONTAL_PADDING, py + VERTICAL_PADDING,
							0xffffffff, 0,
							container_win->graphic.buf, container_win->graphic.width * 4);
				jx += get_character_width(ch);
				i++;
			}
			ix = div_ceil(ix + width, width) * width;
			py += get_character_height(0);
			continue;
		}
		else
		{
			ix += get_character_width(line->content[i]);
			i++;
		}
	}
}

static void draw_terminal()
{
	struct terminal_tab *tab = iterm->active_tab;
	struct terminal_line *last_line = list_last_entry(&tab->lines, struct terminal_line, sibling);
	struct terminal_line *from_line, *to_line;
	int from_row, to_row;
	int rows = count_rows_in_line_range(tab->scroll_line, 0, last_line, last_line->rowspan - 1);
	int max_rows_in_ui = (container_win->graphic.height - 2 * VERTICAL_PADDING) / get_character_height(0);
	if (rows <= max_rows_in_ui)
	{
		int count = 0;
		from_row = 0;
		struct terminal_line *iter;
		list_for_each_entry_reverse(iter, &tab->lines, sibling)
		{
			from_line = iter;
			count += from_line->rowspan;
			if (count >= max_rows_in_ui)
			{
				from_row = from_line->rowspan - (count - max_rows_in_ui);
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
			if (count >= max_rows_in_ui)
			{
				to_row = to_line->rowspan - (count - max_rows_in_ui);
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

static void handle_x11_event(struct xevent *evt)
{
	if (evt->type == XKEY_EVENT)
	{
		struct xkey_event *kevt = (struct xkey_event *)evt->data;
		if (kevt->action == XKEY_RELEASE)
		{
			int nbuf = 0;
			unsigned char buf[100] = {0};
			convert_keycode_to_ascii(kevt->key, kevt->state, buf, &nbuf);

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
			// if ch is Device Control 1 and previous character is new line -> record timestamp
			if (i >= 0 && input[i - 1] == '\n')
				tab->cursor_line->seconds = time(NULL);
		}
		else if (ch == '\177')
		{
			struct terminal_line *cursor_line = tab->cursor_line;
			struct terminal_line *last_line = list_last_entry(&tab->lines, struct terminal_line, sibling);

			if (cursor_line != last_line)
				continue;

			int length = strlen(cursor_line->content);
			if (length == 0 || (int)tab->cursor_column > length)
				continue;

			if (length < (int)tab->cursor_column)
			{
				for (int i = tab->cursor_column; i < length - 1; ++i)
				{
					cursor_line->content[i] = cursor_line->content[i + 1];
				}
				cursor_line->content[length - 1] = 0;
			}
			else
				cursor_line->content[tab->cursor_column--] = 0;
		}
		else if (ch == '\r')
		{
			tab->cursor_column = 0;
		}
		else if (ch == '\n')
		{
			struct terminal_line *first_line = list_first_entry(&tab->lines, struct terminal_line, sibling);
			struct terminal_line *last_line = list_last_entry(&tab->lines, struct terminal_line, sibling);

			if (last_line && last_line->seconds)
			{
				last_line->seconds = time(NULL);
				struct tm *now = localtime(&last_line->seconds);
				log("Terminal: Line is recorded at %d:%d:%d", now->tm_hour, now->tm_min, now->tm_sec);
				free(now);
			}

			struct terminal_line *line = alloc_terminal_line();
			list_add_tail(&line->sibling, &tab->lines);

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
				tab->scroll_line = line;
			if (tab->cursor_line == last_line)
				tab->cursor_line = line;
		}
		// printable characters
		else
		{
			struct terminal_line *cursor_line = tab->cursor_line;
			int width = iterm->width - 2 * HORIZONTAL_PADDING;

			if (tab->cursor_column > CHARACTERS_PER_LINE)
				continue;

			cursor_line->content[tab->cursor_column++] = ch;

			int pixels = pixels_from_content(cursor_line->content);
			cursor_line->rowspan = div_ceil(pixels, width);
			recalculate_tab(tab);
		}
	}

	memset(container_win->graphic.buf, 0, container_win->graphic.width * container_win->graphic.height * 4);
	draw_terminal();
	gui_render(app_win);
}

int main()
{
	app_win = init_window(50, 50, 600, 400);
	container_win = list_last_entry(&app_win->children, struct window, sibling);

	struct terminal_tab *tab = alloc_terminal_tab();

	iterm = calloc(1, sizeof(struct terminal));
	iterm->width = app_win->graphic.width;
	iterm->height = app_win->graphic.height;
	iterm->active_tab = tab;
	iterm->max_rows = MAX_ROWS;
	INIT_LIST_HEAD(&iterm->tabs);
	list_add_tail(&iterm->active_tab->sibling, &iterm->tabs);
	init_terminal_tab_dev(tab);

	active_ptm = iterm->active_tab->fd_ptm;
	enter_event_loop(app_win, handle_x11_event, &active_ptm, 1, handle_master_event);
	return 0;
}
