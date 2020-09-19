#include "layout.h"

#include <include/fcntl.h>
#include <include/mman.h>
#include <libc/gui/msgui.h>
#include <libc/gui/psf.h>
#include <libc/poll.h>
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

int get_character_width(char ch)
{
	struct psf_t *font = get_current_font();
	return (ch == '\t' ? 4 : 1) * font->width;
}

int get_character_height(char ch)
{
	struct psf_t *font = get_current_font();
	return font->height;
}

static struct window *find_child_element_from_position(struct window *win, int32_t px, int32_t py, int32_t mx, int32_t my)
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

static void add_event_handler(struct window *win, char *event_name, EVENT_HANDLER handler)
{
	hashmap_put(&win->events, event_name, handler);
}

void gui_draw_retangle(struct window *win, int x, int y, unsigned int width, unsigned int height, uint32_t bg)
{
	int py = min_t(int, y + height, win->graphic.height);
	int px = min_t(int, x + width, win->graphic.width);
	for (int i = y; i < py; i += 1)
		for (int j = x; j < px; j += 1)
			*(uint32_t *)(win->graphic.buf + (i * win->graphic.width + j) * 4) = bg;
}

static void gui_create_window(struct window *parent, struct window *win, int32_t x, int32_t y, uint32_t width, uint32_t height, struct ui_style *style)
{
	char *pid = calloc(sizeof(char), WINDOW_NAME_LENGTH);
	itoa(getpid(), 10, pid);

	struct msgui *msgui_sender = calloc(1, sizeof(struct msgui));
	msgui_sender->type = MSGUI_WINDOW;
	struct msgui_window *msgwin = (struct msgui_window *)msgui_sender->data;
	msgwin->x = x;
	msgwin->y = y;
	msgwin->width = width;
	msgwin->height = height;
	if (parent)
		memcpy(msgwin->parent, parent->name, WINDOW_NAME_LENGTH);
	memcpy(msgwin->sender, pid, WINDOW_NAME_LENGTH);
	int32_t sfd = mq_open(WINDOW_SERVER_QUEUE, O_WRONLY, &(struct mq_attr){
															 .mq_msgsize = sizeof(struct msgui),
															 .mq_maxmsg = 32,
														 });
	mq_send(sfd, (char *)msgui_sender, 0, sizeof(struct msgui));
	mq_close(sfd);
	free(msgui_sender);

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

	int32_t wfd = mq_open(msgwin->sender, O_RDONLY, &(struct mq_attr){
														.mq_msgsize = WINDOW_NAME_LENGTH,
														.mq_maxmsg = 32,
													});
	mq_receive(wfd, win->name, 0, WINDOW_NAME_LENGTH);
	mq_close(wfd);

	uint32_t buf_size = width * height * 4;
	int32_t fd = shm_open(win->name, O_RDWR | O_CREAT, 0);
	win->graphic.buf = (char *)mmap(NULL, buf_size, PROT_WRITE | PROT_READ, MAP_SHARED, fd);
}

static void gui_label_set_text(struct ui_label *label, char *text)
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

void gui_render(struct window *win)
{
	struct msgui *msgui = calloc(1, sizeof(struct msgui));
	msgui->type = MSGUI_FOCUS;
	struct msgui_focus *msgfocus = (struct msgui_focus *)msgui->data;
	memcpy(msgfocus->sender, win->name, WINDOW_NAME_LENGTH);

	int32_t sfd = mq_open(WINDOW_SERVER_QUEUE, O_WRONLY, &(struct mq_attr){
															 .mq_msgsize = sizeof(struct msgui),
															 .mq_maxmsg = 32,
														 });
	mq_send(sfd, (char *)msgui, 0, sizeof(struct msgui));
	mq_close(sfd);
	free(msgui);
}

struct window *init_window(int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	init_fonts();
	struct window *win = calloc(1, sizeof(struct window));
	gui_create_window(NULL, win, x, y, width, height, NULL);

	return win;
}

#define MAX_FD 10
void enter_event_loop(struct window *win, void (*event_callback)(struct xevent *evt), int *fds, unsigned int nfds, void (*fds_callback)(struct pollfd *, unsigned int))
{
	gui_render(win);

	struct xevent *event = calloc(1, sizeof(struct xevent));
	int32_t wfd = mq_open(win->name, O_RDONLY, &(struct mq_attr){
												   .mq_msgsize = sizeof(struct xevent),
												   .mq_maxmsg = 32,
											   });
	memset(event, 0, sizeof(struct xevent));

	struct pollfd pfds[MAX_FD] = {
		{.fd = wfd, .events = POLLIN},
	};
	for (int i = 1; i < MAX_FD; ++i)
	{
		pfds[i].fd = -1;
		pfds[i].events = POLLIN;
	}

	while (true)
	{
		for (unsigned int i = 0; i < nfds; ++i)
			pfds[i + 1].fd = fds[i];

		int nr = poll(pfds, MAX_FD);
		if (nr <= 0)
			continue;

		for (int32_t i = 0; i < MAX_FD; ++i)
		{
			if (!(pfds[i].revents & POLLIN))
				continue;

			if (pfds[i].fd == wfd)
			{
				mq_receive(wfd, (char *)event, 0, sizeof(struct xevent));

				if (event->type == XBUTTON_EVENT)
				{
					struct xbutton_event *bevent = (struct xbutton_event *)event->data;

					if (bevent->action == XBUTTON_PRESS)
					{
						struct window *active_win = find_child_element_from_position(win, win->graphic.x, win->graphic.y, bevent->x, bevent->y);
						if (active_win)
						{
							EVENT_HANDLER handler = hashmap_get(&active_win->events, "click");
							if (handler)
								handler(active_win);
						}
					}
				}
				if (event_callback)
					event_callback(event);
				memset(event, 0, sizeof(struct xevent));
			}
			else if (fds_callback)
				fds_callback(pfds, MAX_FD);
		};
	}

	mq_close(wfd);
	free(event);
}
