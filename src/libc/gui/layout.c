#include "layout.h"

#include <include/fcntl.h>
#include <include/mman.h>
#include <include/msgui.h>
#include <libc/gui/psf.h>
#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>

void init_fonts()
{
	uint32_t fd = open("/usr/share/fonts/ter-powerline-v16n.psf", 0, 0);
	struct stat *stat = calloc(1, sizeof(struct stat));
	fstat(fd, stat);
	char *buf = calloc(stat->size, sizeof(char));
	read(fd, buf, stat->size);
	psf_init(buf, stat->size);
}

struct window *find_child_element_from_position(struct window *win, int32_t px, int32_t py, int32_t mx, int32_t my)
{
	struct window *iter_win;
	list_for_each_entry(iter_win, &win->children, sibling)
	{
		int32_t cx = px + iter_win->graphic.x;
		int32_t cy = py + iter_win->graphic.y;

		if (cx < mx && mx < cx + iter_win->graphic.width &&
			cy < my && my < cy + iter_win->graphic.height)
			return iter_win;

		struct window *w = find_child_element_from_position(iter_win, cx, cy, mx, my);
		if (w)
			return w;
	}
	return NULL;
}

void add_event_handler(struct window *win, char *event_name, EVENT_HANDLER handler)
{
	hashmap_put(&win->events, event_name, handler);
}

void gui_create_window(struct window *parent, struct window *win, int32_t x, int32_t y, uint32_t width, uint32_t height, struct ui_style *style)
{
	char *pid = calloc(1, WINDOW_NAME_LENGTH);
	itoa(getpid(), 10, pid);

	struct msgui *msgui_sender = calloc(1, sizeof(struct msgui));
	msgui_sender->type = MSGUI_WINDOW;
	struct msgui_window *msgwin = msgui_sender->data;
	msgwin->x = x;
	msgwin->y = y;
	msgwin->width = width;
	msgwin->height = height;
	if (parent)
		memcpy(msgwin->parent, parent->name, WINDOW_NAME_LENGTH);
	memcpy(msgwin->sender, pid, WINDOW_NAME_LENGTH);
	msgsnd(WINDOW_SERVER_SHM, msgui_sender, 0, sizeof(struct msgui));

	win->graphic.x = x;
	win->graphic.y = y;
	win->graphic.width = width;
	win->graphic.height = height;
	win->style = style;
	win->add_event_listener = add_event_handler;

	INIT_LIST_HEAD(&win->children);
	hashmap_init(&win->events, hashmap_hash_string, hashmap_compare_string, 0);

	if (parent)
		list_add_tail(&win->sibling, &parent->children);

	msgrcv(msgwin->sender, win->name, 0, WINDOW_NAME_LENGTH);

	uint32_t buf_size = width * height * 4;
	int32_t fd = shm_open(win->name, O_RDWR | O_CREAT, 0);
	win->graphic.buf = mmap(NULL, buf_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd);
}

void gui_label_set_text(struct ui_label *label, char *text)
{
	memset(label->window.graphic.buf, 0, label->window.graphic.width * label->window.graphic.height * 4);

	label->text = text;
	uint32_t scanline = label->window.graphic.width * 4;
	psf_puts(label->text, label->window.style->padding_left, label->window.style->padding_top, 0xffffffff, 0x00, label->window.graphic.buf, scanline);
}

void gui_create_label(struct window *parent, struct ui_label *label, int32_t x, int32_t y, uint32_t width, uint32_t height, char *text, struct ui_style *style)
{
	gui_create_window(parent, &label->window, x, y, width, height, style);
	label->set_text = gui_label_set_text;
	label->set_text(label, text);
}

void gui_create_input(struct window *parent, struct ui_input *input, int32_t x, int32_t y, uint32_t width, uint32_t height, char *content)
{
	gui_create_window(parent, &input->window, x, y, width, height, NULL);
}

struct window *init_window(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	char *pid = calloc(1, WINDOW_NAME_LENGTH);
	itoa(getpid(), 10, pid);
	msgopen(pid, 0);

	init_fonts();
	struct window *win = calloc(1, sizeof(struct window));
	gui_create_window(NULL, win, x, y, width, height, NULL);

	msgopen(win->name, 0);
	return win;
}

void enter_event_loop(struct window *win)
{
	struct msgui *msgui = calloc(1, sizeof(struct msgui));
	msgui->type = MSGUI_FOCUS;
	struct msgui_focus *msgfocus = msgui->data;
	memcpy(msgfocus->sender, win->name, WINDOW_NAME_LENGTH);
	msgsnd(WINDOW_SERVER_SHM, msgui, 0, sizeof(struct msgui));

	struct ui_event *ui_event = calloc(1, sizeof(struct ui_event));
	while (true)
	{
		memset(ui_event, 0, sizeof(struct ui_event));
		msgrcv(win->name, ui_event, 0, sizeof(struct ui_event));

		if (ui_event->event_type == MOUSE_CLICK)
		{
			struct window *active_win = find_child_element_from_position(win, win->graphic.x, win->graphic.y, ui_event->mouse_x, ui_event->mouse_y);
			if (active_win)
			{
				EVENT_HANDLER handler = hashmap_get(&active_win->events, "click");
				if (handler)
					handler(active_win);
			}
		}
	};
}
