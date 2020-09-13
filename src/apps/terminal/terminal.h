#ifndef TAB_H
#define TAB_H

#include <include/ctype.h>
#include <include/list.h>

#define PIXELS_PER_CHARACTER 20

struct terminal_line
{
	char content[256];	// better to use linked list for buf
	unsigned long msec;
	unsigned int rowspan;
	struct list_head sibling;
};

struct terminal_tab
{
	struct list_head lines;
	struct terminal_line *scroll_line;
	pid_t shell_pid;
	unsigned int line_count, row_count;
	struct terminal_line *cursor_line;
	unsigned int cursor_column;
	unsigned int fd_ptm, fd_pts;
	struct list_head sibling;
};

struct terminal
{
	struct list_head tabs;
	struct terminal_tab *active_tab;
	int width, height;
	unsigned int max_rows;
};

#endif
