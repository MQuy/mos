#include "terminal.h"

#include <assert.h>
#include <fcntl.h>
#include <libgui/psf.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <termio.h>

int terminal_get_unit_colspan(struct terminal_unit *unit)
{
	return unit->content == '\t' ? unit->row->terminal->config.tabspan : 1;
}

int terminal_get_unit_width(struct terminal_unit *unit)
{
	struct psf_t *font = get_current_font();
	return terminal_get_unit_colspan(unit) * font->width;
}

struct terminal_row *terminal_allocate_row(struct terminal *term, struct terminal_row *at_row)
{
	struct terminal_row *new_row = calloc(1, sizeof(struct terminal_row));
	new_row->columns = 0;
	new_row->group = at_row->group;
	new_row->terminal = term;
	INIT_LIST_HEAD(&new_row->units);
	list_add(&new_row->sibling, &at_row->sibling);

	return new_row;
}

struct terminal_group *terminal_allocate_group(struct terminal *term, struct terminal_row *at_row)
{
	struct terminal_group *group = calloc(1, sizeof(struct terminal_group));
	group->number_of_rows = 1;
	group->terminal = term;
	list_add(&group->sibling, &at_row->group->sibling);

	struct terminal_row *row = calloc(1, sizeof(struct terminal_row));
	row->columns = 0;
	row->group = group;
	row->terminal = term;
	INIT_LIST_HEAD(&row->units);
	list_add(&row->sibling, &at_row->sibling);

	group->child = row;

	return group;
}

struct terminal *terminal_allocate(struct window *win)
{
#define VERTICAL_PADDING 4
#define HORIZONTAL_PADDING 8
#define MAX_LINES 1000
	struct terminal *term = calloc(1, sizeof(struct terminal));
	term->win = win;
	term->config.horizontal_padding = HORIZONTAL_PADDING;
	term->config.vertical_padding = VERTICAL_PADDING;
	term->config.max_lines = MAX_LINES;
	term->config.screen_columns = win->graphic.width / get_character_width(' ');
	term->config.screen_rows = win->graphic.height / get_character_height(' ');
	term->config.tabspan = 4;
	INIT_LIST_HEAD(&term->groups);
	INIT_LIST_HEAD(&term->rows);

	struct terminal_style *style = calloc(1, sizeof(struct terminal_style));
	style->background = 0;
	style->color = 0xffffffff;
	term->style = style;

	struct terminal_group *group = calloc(1, sizeof(struct terminal_group));
	group->terminal = term;
	list_add_tail(&group->sibling, &term->groups);

	struct terminal_row *row = calloc(1, sizeof(struct terminal_row));
	row->columns = 0;
	row->group = group;
	row->terminal = term;
	INIT_LIST_HEAD(&row->units);
	list_add_tail(&row->sibling, &term->rows);

	term->scroll_row = row;
	term->cursor_row = row;
	term->cursor_unit_index = 0;

	group->number_of_rows = 1;
	group->child = row;

	return term;
}

void terminal_free_unit(struct terminal_unit *unit)
{
	unit->row->columns -= terminal_get_unit_colspan(unit);
	unit->style->references--;
	list_del(&unit->sibling);
	free(unit);
}

void terminal_free_row(struct terminal_row *row)
{
	assert(!row->columns);
	list_del(&row->sibling);
	row->group->number_of_rows--;
	free(row);
}

void terminal_free_group(struct terminal_group *group)
{
	assert(!group->number_of_rows);
	list_del(&group->sibling);
	free(group);
}

void terminal_move_cursor(struct terminal *term, int x, int y, int whence)
{
	terminal_move_cursor_row(term, y, whence);
	terminal_move_cursor_column(term, x, whence);
}

void terminal_move_cursor_column(struct terminal *term, int n, int whence)
{
	int column;
	if (whence == CURSOR_CUR)
		column = term->cursor_unit_index + n;
	else if (whence == CURSOR_SET)
		column = n;
	else
		assert_not_reached();

	if (column < 0)
		column = 0;
	else if (column > term->cursor_row->columns)
		column = term->cursor_row->columns;
}

void terminal_move_cursor_row(struct terminal *term, int n, int whence)
{
	if (whence == CURSOR_CUR)
	{
		int i = 0;
		struct terminal_row *iter = term->cursor_row;
		if (n > 0)
			list_for_each_entry_from(iter, &term->rows, sibling)
			{
				if (i == n)
					break;
				i++;
			}
		else
			list_for_each_entry_from_reverse(iter, &term->rows, sibling)
			{
				if (i == n)
					break;
				i++;
			}

		term->cursor_row = iter;
	}
	else if (whence == CURSOR_SET)
	{
		int i = 0;
		struct terminal_row *iter;
		list_for_each_entry(iter, &term->rows, sibling)
		{
			if (i == n)
				break;
			i++;
		}
	}
	else
		assert_not_reached();

	if (term->cursor_unit_index >= term->cursor_row->columns)
		term->cursor_unit_index = term->cursor_row->columns;
}

void terminal_draw_cursor(struct terminal *term, int x, int y)
{
	gui_draw_retangle(term->win,
					  x, y,
					  get_character_width(' '), get_character_height(' '),
					  0xd0d0d0ff);
}

void terminal_draw_unit(struct terminal_unit *unit, int x, int y)
{
	struct terminal *term = unit->row->terminal;
	struct terminal_style *style = unit->style;

	unsigned char ch = unit->content;
	if (ch == '\t')
		for (int i = 0; i < term->config.tabspan; ++i)
			psf_putchar(' ',
						x + i * get_character_width(' '), y,
						style->color, style->background,
						term->win->graphic.buf, term->win->graphic.width * 4);

	else
		psf_putchar(ch,
					x, y,
					style->color, style->background,
					term->win->graphic.buf, term->win->graphic.width * 4);
}

void terminal_draw_row(struct terminal_row *row, int y)
{
	struct terminal *term = row->terminal;

	int i = 0;
	int x = term->config.horizontal_padding;
	struct terminal_unit *iter;
	list_for_each_entry(iter, &row->units, sibling)
	{
		if (i == term->config.screen_columns)
			break;

		terminal_draw_unit(iter, x, y);

		if (row == term->cursor_row && i == term->cursor_unit_index)
			terminal_draw_cursor(term, x, y);

		x += terminal_get_unit_width(iter);
		i++;
	}

	// TODO: MQ 2020-12-12 Handle cursor at the end of row
	if (row == term->cursor_row && i == term->cursor_unit_index)
		terminal_draw_cursor(term, x, y);
}

void terminal_draw(struct terminal *term)
{
	int row_height = get_character_height(' ');

	int i = 0;
	struct terminal_row *iter = term->scroll_row;
	list_for_each_entry_from(iter, &term->rows, sibling)
	{
		if (i == term->config.screen_rows)
			break;

		terminal_draw_row(iter, i * row_height + term->config.vertical_padding);
		i++;
	}
}

void terminal_input_printable(struct terminal *term, char ch)
{
	struct terminal_unit *unit = calloc(1, sizeof(struct terminal_unit));
	unit->content = ch;
	unit->style = term->style;

	if (term->cursor_row->columns == term->config.screen_columns)
	{
		struct terminal_group *group = term->cursor_row->group;
		group->number_of_rows++;

		struct terminal_row *new_row = terminal_allocate_row(term, term->cursor_row);
		new_row->columns = 1;

		unit->row = new_row;
		list_add_tail(&unit->sibling, &new_row->units);

		term->cursor_row = new_row;
		term->cursor_unit_index = new_row->columns;
	}
	else
	{
		unit->row = term->cursor_row;
		list_add_tail(&unit->sibling, &term->cursor_row->units);

		term->cursor_row->columns += terminal_get_unit_colspan(unit);
		term->cursor_unit_index = term->cursor_row->columns;
	}
}

void terminal_input_newline(struct terminal *term)
{
	struct terminal_group *group = terminal_allocate_group(term, term->cursor_row);

	term->cursor_row = group->child;
	term->cursor_unit_index = 0;
}

struct terminal_unit *terminal_get_unit_from_index(struct terminal_row *row, int index)
{
	int i = 0;
	struct terminal_unit *iter;
	list_for_each_entry(iter, &row->units, sibling)
	{
		if (i++ == index)
			return iter;
	}

	return NULL;
}

void terminal_input_erase_shift(struct terminal_group *group, struct terminal_row *row, int index)
{
	if (group->number_of_rows == index + 1)
		return;

	struct terminal_row *next_row = list_next_entry(row, sibling);
	struct terminal_unit *iter, *next;
	list_for_each_entry_safe(iter, next, &row->units, sibling)
	{
		int colspan = terminal_get_unit_colspan(iter);
		if (colspan + row->columns < group->terminal->config.screen_columns)
		{
			list_del(&iter->sibling);
			list_add_tail(&iter->sibling, &row->units);
			row->columns += colspan;
		}
		else
			break;
	}
	terminal_input_erase_shift(group, next_row, index + 1);
}

void terminal_input_erase(struct terminal *term)
{
	struct terminal_group *group = term->cursor_row->group;

	int i = 0;
	struct terminal_row *iter = group->child;
	list_for_each_entry_from(iter, &term->rows, sibling)
	{
		if (iter == term->cursor_row)
		{
			struct terminal_unit *unit = terminal_get_unit_from_index(iter, term->cursor_unit_index);
			// row is empty
			if (!unit)
			{
				assert(!term->cursor_unit_index);

				struct terminal_row *first_row = list_first_entry(&term->rows, struct terminal_row, sibling);
				if (first_row != iter)
				{
					term->cursor_row = list_prev_entry(iter, sibling);
					term->cursor_unit_index = term->cursor_row->columns - 1;

					terminal_free_row(iter);
					if (!group->number_of_rows)
						terminal_free_group(iter->group);
				}
			}
			else if (i + 1 < group->number_of_rows)
			{
				struct terminal_row *next_row = list_next_entry(iter, sibling);
				if (next_row->columns)
				{
					terminal_free_unit(unit);
					terminal_input_erase_shift(group, iter, i);
				}
				// next row is empty
				else
				{
					terminal_free_unit(unit);
					terminal_free_row(next_row);
				}
			}
			// last row in a group
			else
				terminal_free_unit(unit);

			return;
		}
		i++;
	}
}

void terminal_input_back(struct terminal *term)
{
	if (!term->cursor_unit_index)
	{
		struct terminal_group *group = term->cursor_row->group;
		if (group->child != term->cursor_row)
		{
			struct terminal_row *prev_row = list_prev_entry(term->cursor_row, sibling);
			term->cursor_row = prev_row;
			term->cursor_unit_index = min(0, prev_row->columns - 1);
		}
	}
	else
		term->cursor_unit_index--;
}

void terminal_input_sequence(struct terminal *term, char *input, int *index)
{
	// NOTE: index is increased at the end of loop below
}

void terminal_input(struct terminal *term, char *input)
{
	for (int i = 0, length = strlen(input); i < length; ++i)
	{
		char ch = input[i];

		if (ch == '\033')
			terminal_input_sequence(term, input, &i);
		else if (ch == '\b')
			terminal_input_back(term);
		else if (ch == '\177')
			terminal_input_erase(term);
		else if (ch == '\r')
			term->cursor_unit_index = 0;
		else if (ch == '\n')
			terminal_input_newline(term);
		else
			terminal_input_printable(term, ch);
	}
}
