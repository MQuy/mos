#ifndef TERMINAL_H
#define TERMINAL_H 1

#include <libgui/layout.h>
#include <list.h>
#include <stdint.h>
#include <sys/types.h>

#define CHARACTERS_PER_LINE 256
#define MAX_COLUMNS 4096
#define MAX_ROWS 400

#define CURSOR_SET 0 /* move cursor from beginning of terminal */
#define CURSOR_CUR 1 /* move cursor from current position */

struct terminal_style
{
	uint32_t background, color;
	char bold : 1;
	char light : 1;
	char italic : 1;
	char underline : 1;
	char slow_blink : 1;
	char rapid_blink : 1;
	char reverse_video : 1;
	char strike_throught : 1;
	unsigned int references;
};

struct terminal_unit
{
	unsigned char content;
	struct terminal_row *row;
	struct list_head sibling;
	struct terminal_style *style;
};

struct terminal_row
{
	struct list_head units;
	unsigned short columns;

	struct terminal_group *group;
	struct terminal *terminal;
	struct list_head sibling;
};

struct terminal_group
{
	struct terminal_row *child;	 // the first child
	unsigned short number_of_rows;

	struct terminal *terminal;
	struct list_head sibling;
};

struct terminal_config
{
	unsigned char vertical_padding : 4;
	unsigned char horizontal_padding : 4;
	char tabspan;
	unsigned short max_lines;
	short screen_columns;
	short screen_rows;
};

struct terminal
{
	struct list_head groups;
	struct list_head rows;

	struct terminal_row *cursor_row;
	unsigned short cursor_unit_index;
	struct terminal_row *scroll_row;
	struct terminal_style *style;

	struct window *win;
	struct terminal_config config;

	int fd_ptm, fd_pts;
	int shell_pid;
};

struct terminal *terminal_allocate(struct window *win);
void terminal_draw(struct terminal *term);
void terminal_move_cursor(struct terminal *term, int x, int y, int whence);
void terminal_move_cursor_column(struct terminal *term, int n, int whence);
void terminal_move_cursor_row(struct terminal *term, int n, int whence);
void terminal_input(struct terminal *term, char *input);

#endif
