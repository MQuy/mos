#ifndef TAB_H
#define TAB_H

#include <include/ctype.h>
#include <include/list.h>

#define PIXELS_PER_ROW 20

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
	unsigned int nth_rowline;
	unsigned int line_count;
	unsigned int row_count;
	unsigned int cursor_row, cursor_column;
	struct list_head sibling;
	unsigned int fd_ptm;
	unsigned int fd_pts;
	pid_t shell_pid;
};

struct terminal
{
	struct list_head tabs;
	struct terminal_tab *active_tab;
	unsigned int width, height;
	unsigned int max_rows;
};

#endif
