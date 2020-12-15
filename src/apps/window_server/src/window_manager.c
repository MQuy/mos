#include "window_manager.h"

#include <assert.h>
#include <fcntl.h>
#include <libcore/hashtable/hashmap.h>
#include <libcore/ini/ini.h>
#include <libgui/bmp.h>
#include <libgui/psf.h>
#include <mqueue.h>
#include <stdlib.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/mman.h>
#include <unistd.h>

#define ICON_IMAGE_WIDTH 48
#define ICON_IMAGE_HEIGHT 48
#define ICON_IMAGE_PADDING 4
#define ICON_BOX_WIDTH 88
#define ICON_BOX_HEIGHT 82
#define ICON_LABEL_MARGIN_TOP 2
#define ICON_LABEL_MARGIN_LEFT 4
#define CURSOR_WIDTH 20
#define CURSOR_HEIGHT 20

static struct desktop *desktop;
static char *desktop_buf;
static uint32_t nwin = 1;

static char *get_window_name()
{
	char *wd = calloc(1, WINDOW_NAME_LENGTH);
	memcpy(wd, "wd0000", WINDOW_NAME_LENGTH);
	for (int32_t i = 5, inum = nwin++; i > 1; --i, inum /= 10)
		wd[i] = (inum % 10) + '0';
	return wd;
}

static struct window *find_window_in_window(struct window *win, char *name)
{
	if (!strcmp(win->name, name))
		return win;

	struct window *iter;
	list_for_each_entry(iter, &win->children, sibling)
	{
		struct window *temp = find_window_in_window(iter, name);
		if (temp)
			return temp;
	}
	return NULL;
}

static struct window *find_window_in_root(char *name)
{
	struct window *iter;
	list_for_each_entry(iter, &desktop->children, sibling)
	{
		struct window *temp = find_window_in_window(iter, name);
		if (temp)
			return temp;
	}
	return NULL;
}

struct window *create_window(struct msgui_window *msgwin)
{
	char *window_name = get_window_name();
	uint32_t screen_size = msgwin->height * msgwin->width * 4;
	int32_t fd = shm_open(window_name, O_RDWR | O_CREAT, 0);
	ftruncate(fd, screen_size);

	struct window *win = calloc(1, sizeof(struct window));
	memcpy(win->name, window_name, WINDOW_NAME_LENGTH);
	win->graphic.buf = (char *)mmap(NULL, screen_size, PROT_WRITE, MAP_SHARED, fd, 0);
	win->graphic.x = msgwin->x;
	win->graphic.y = msgwin->y;
	win->graphic.width = msgwin->width;
	win->graphic.height = msgwin->height;
	win->graphic.transparent = msgwin->transparent;

	INIT_LIST_HEAD(&win->children);
	hashmap_init(&win->events, hashmap_hash_string, hashmap_compare_string, 0);

	if (msgwin->parent && strlen(msgwin->parent))
	{
		struct window *parent = find_window_in_root(msgwin->parent);
		win->parent = parent;
		list_add_tail(&win->sibling, &parent->children);
	}
	else
		list_add_tail(&win->sibling, &desktop->children);

	return win;
}

void remove_window(struct window *win)
{
	if (!win)
		return;

	list_del(&win->sibling);
	struct window *iter, *next;
	list_for_each_entry_safe(iter, next, &win->children, sibling)
	{
		remove_window(iter);
	}

	int32_t screen_size = win->graphic.height * win->graphic.width * 4;
	munmap(win->graphic.buf, screen_size);
	// TODO: MQ 2020-09-24 Close file descriptor and memory which is openned via `shm_open`

	hashmap_destroy(&win->events);
	free(win);
}

void handle_window_remove(struct msgui_close *msgclose)
{
	struct window *win = find_window_in_root(msgclose->sender);
	remove_window(win);
	desktop->active_window = NULL;
}

static int icon_ini_handler(__unused void *_desktop, const char *section, const char *name,
							const char *value)
{
	struct icon *icon = hashmap_get(&desktop->icons, section);
	if (!icon)
	{
		icon = calloc(1, sizeof(struct icon));

		icon->icon_graphic.x = (ICON_BOX_WIDTH - ICON_IMAGE_WIDTH) / 2;
		icon->icon_graphic.y = ICON_IMAGE_PADDING;
		icon->icon_graphic.width = ICON_IMAGE_WIDTH;
		icon->icon_graphic.height = ICON_IMAGE_HEIGHT;
		icon->icon_graphic.buf = calloc(icon->icon_graphic.width * icon->icon_graphic.height * 4, sizeof(char));

		icon->box_graphic.width = ICON_BOX_WIDTH;
		icon->box_graphic.height = ICON_BOX_HEIGHT;
		icon->box_graphic.buf = calloc(icon->box_graphic.width * icon->box_graphic.height * 4, sizeof(char));

		hashmap_put(&desktop->icons, section, icon);
	}
	if (!strcmp(name, "label"))
		icon->label = strdup(value);
	else if (!strcmp(name, "icon"))
		icon->icon_path = strdup(value);
	else if (!strcmp(name, "path"))
		icon->exec_path = strdup(value);
	else if (!strcmp(name, "px"))
		icon->box_graphic.x = atoi(value);
	else if (!strcmp(name, "py"))
		icon->box_graphic.y = atoi(value);

	return 1;
}

static void init_icons()
{
	hashmap_init(&desktop->icons, hashmap_hash_string, hashmap_compare_string, 0);
	ini_parse("/etc/desktop.ini", icon_ini_handler, &desktop);

	struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
	while (iter)
	{
		struct icon *icon = hashmap_iter_get_data(iter);
		char *buf = load_bmp(icon->icon_path);
		bmp_draw(&icon->icon_graphic, buf, 0, 0);
		free(buf);

		iter = hashmap_iter_next(&desktop->icons, iter);
	}
}

static void init_mouse()
{
	struct graphic *graphic = &desktop->mouse.graphic;
	graphic->x = 0;
	graphic->y = 0;
	graphic->width = CURSOR_WIDTH;
	graphic->height = CURSOR_HEIGHT;
	graphic->buf = calloc(graphic->width * graphic->height * 4, sizeof(char));

	char *buf = load_bmp("/usr/share/images/cursor.bmp");
	bmp_draw(graphic, buf, 0, 0);
	free(buf);
}

static void init_dekstop_graphic()
{
	struct graphic *graphic = &desktop->graphic;
	graphic->x = 0;
	graphic->y = 0;
	graphic->width = desktop->fb->width;
	graphic->height = desktop->fb->height;
	graphic->buf = calloc(graphic->width * graphic->height * 4, sizeof(char));

	char *buf = load_bmp("/usr/share/images/background.bmp");
	bmp_draw(graphic, buf, 0, 0);
	free(buf);
	desktop_buf = calloc(desktop->fb->pitch * desktop->fb->height, sizeof(char));
}

void init_layout(struct framebuffer *fb)
{
	desktop = calloc(1, sizeof(struct desktop));
	desktop->fb = fb;
	INIT_LIST_HEAD(&desktop->children);

	init_dekstop_graphic();
	init_fonts();
	init_icons();
	init_mouse();
}

// TODO: MQ 2020-03-24 Handle when win is not in buf's area
static void draw_graphic(char *buf, uint32_t scanline, char *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	for (uint32_t i = 0; i < height; ++i)
	{
		char *ibuf = buf + (y + i) * scanline + x * 4;
		char *iwin = win + i * width * 4;
		for (uint32_t j = 0; j < width; ++j)
		{
			*(uint32_t *)ibuf = *(uint32_t *)iwin;
			ibuf += 4;
			iwin += 4;
		}
	}
}

// TODO: MQ 2020-03-24 Handle when win is not in buf's area
static void draw_alpha_graphic(char *buf, uint32_t scanline, char *win, int32_t x, int32_t y, uint32_t width, uint32_t height)
{
	for (uint32_t i = 0; i < height; ++i)
	{
		char *ibuf = buf + (y + i) * scanline + x * 4;
		char *iwin = win + i * width * 4;
		for (uint32_t j = 0; j < width; ++j)
		{
			set_pixel(ibuf, iwin[0], iwin[1], iwin[2], iwin[3]);
			ibuf += 4;
			iwin += 4;
		}
	}
}

static void draw_desktop_icons(char *buf)
{
	struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
	while (iter)
	{
		struct icon *icon = hashmap_iter_get_data(iter);
		struct graphic *box_graphic = &icon->box_graphic;
		struct graphic *icon_graphic = &icon->icon_graphic;

		// 88x82
		// --------- 4 --------
		// 16 - 4 - 48 - 4 - 16
		// --------- 4 --------
		// --------- 2 --------
		// 4 ------- 24 ----- 4
		if (icon->active)
		{
			int size = ICON_IMAGE_WIDTH + ICON_IMAGE_PADDING * 2;
			int margin_left = (ICON_BOX_WIDTH - ICON_IMAGE_WIDTH - ICON_IMAGE_PADDING * 2) / 2;
			for (int j = 0; j < size; ++j)
			{
				char *iblock = box_graphic->buf + (j * box_graphic->width + margin_left) * 4;
				for (int i = 0; i < size; ++i)
				{
					iblock[0] = 0xAA;
					iblock[1] = 0xAA;
					iblock[2] = 0xAA;
					iblock[3] = 0x33;
					iblock += 4;
				}
			}
		}
		else
			memset(box_graphic->buf, 0, box_graphic->width * box_graphic->height * 4);

		uint8_t label_length = strlen(icon->label);
		// TODO Implement multi lines label
		if (label_length <= 10)
		{
			uint8_t padding = ((10 - label_length) / 2) * get_character_width(' ');
			psf_puts(icon->label, ICON_LABEL_MARGIN_LEFT + padding, ICON_IMAGE_WIDTH + ICON_IMAGE_PADDING * 2 + ICON_LABEL_MARGIN_TOP, 0xffffffff, 0x00000000, box_graphic->buf, box_graphic->width * 4);
		}
		else
			assert_not_implemented();
		draw_alpha_graphic(
			box_graphic->buf, box_graphic->width * 4,
			icon_graphic->buf, icon_graphic->x, icon_graphic->y, icon_graphic->width, icon_graphic->height);
		draw_alpha_graphic(
			buf, desktop->fb->pitch,
			box_graphic->buf, box_graphic->x, box_graphic->y, box_graphic->width, box_graphic->height);

		iter = hashmap_iter_next(&desktop->icons, iter);
	}
}

static void draw_mouse(char *buf)
{
	struct graphic *graphic = &desktop->mouse.graphic;
	draw_alpha_graphic(buf, desktop->fb->pitch, graphic->buf, graphic->x, graphic->y, graphic->width, graphic->height);
}

static void draw_window(char *buf, struct window *win, int32_t px, int32_t py)
{
	int32_t ax = px + win->graphic.x;
	int32_t ay = py + win->graphic.y;

	if (win->graphic.transparent)
		draw_alpha_graphic(buf, desktop->fb->pitch, win->graphic.buf, ax, ay, win->graphic.width, win->graphic.height);
	else
		draw_graphic(buf, desktop->fb->pitch, win->graphic.buf, ax, ay, win->graphic.width, win->graphic.height);

	struct window *iter_w;
	list_for_each_entry(iter_w, &win->children, sibling)
	{
		draw_window(buf, iter_w, ax, ay);
	}
}

void draw_window_in_layout(char *name)
{
	struct window *win = find_window_in_root(name);
	draw_window(desktop_buf, win, 0, 0);

	memcpy((char *)desktop->fb->addr, desktop_buf, desktop->fb->pitch * desktop->fb->height);
}

// TODO: MQ 2020-03-21 Improve render speed via dirty rects
void draw_layout()
{
	draw_graphic(desktop_buf, desktop->fb->pitch, desktop->graphic.buf, 0, 0, desktop->graphic.width, desktop->graphic.height);
	draw_desktop_icons(desktop_buf);

	struct window *iter_win;
	list_for_each_entry(iter_win, &desktop->children, sibling)
	{
		draw_window(desktop_buf, iter_win, 0, 0);
	}
	draw_mouse(desktop_buf);

	memcpy((char *)desktop->fb->addr, desktop_buf, desktop->fb->pitch * desktop->fb->height);
}

static void mouse_change(struct mouse_event *event)
{
	struct graphic *graphic = &desktop->mouse.graphic;

	graphic->x += event->x;
	graphic->y += event->y;

	if (graphic->x < 0)
		graphic->x = 0;
	else if (graphic->x + (int32_t)graphic->width > (int32_t)desktop->graphic.width)
		graphic->x = desktop->graphic.width - graphic->width;

	if (graphic->y < 0)
		graphic->y = 0;
	else if (graphic->y + (int32_t)graphic->height > (int32_t)desktop->graphic.height)
		graphic->y = desktop->graphic.height - graphic->height;
}

struct window *get_window_from_mouse_position(int32_t px, int32_t py)
{
	struct window *iter_win;
	list_for_each_entry(iter_win, &desktop->children, sibling)
	{
		if (iter_win->graphic.x < px && px < iter_win->graphic.x + (int32_t)iter_win->graphic.width &&
			iter_win->graphic.y < py && py < iter_win->graphic.y + (int32_t)iter_win->graphic.height)
			return iter_win;
	}
	return NULL;
}

static struct icon *find_icon_from_mouse_position(int32_t px, int32_t py)
{
	struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
	while (iter)
	{
		struct icon *icon = hashmap_iter_get_data(iter);
		struct graphic *graphic = &icon->box_graphic;

		if (graphic->x <= px && px <= graphic->x + (int32_t)graphic->width &&
			graphic->y <= py && py <= graphic->y + (int32_t)graphic->height)
			return icon;

		iter = hashmap_iter_next(&desktop->icons, iter);
	}
	return NULL;
}

void handle_mouse_event(struct mouse_event *mevent)
{
	desktop->event_state = (desktop->event_state & ~0b1110000) | mevent->state;
	mouse_change(mevent);

	if ((mevent->buttons & BUTTON_LEFT) && !(desktop->event_state & BUTTON_LEFT_MASK))
	{
		struct ui_mouse *mouse = &desktop->mouse;
		struct window *active_win = get_window_from_mouse_position(mouse->graphic.x, mouse->graphic.y);
		if (active_win && active_win == desktop->active_window)
		{
			struct xevent *event = create_xbutton_event(BUTTON_LEFT, XBUTTON_PRESS, desktop->mouse.graphic.x, desktop->mouse.graphic.y, desktop->event_state);
			int32_t fd = mq_open(active_win->name, O_WRONLY, &(struct mq_attr){
																 .mq_msgsize = sizeof(struct xevent),
																 .mq_maxmsg = 32,
															 });
			mq_send(fd, (char *)event, 0, sizeof(struct xevent));
			mq_close(fd);
			free(event);
		}
		else if (active_win)
			desktop->active_window = active_win;
		else
		{
			struct icon *icon = find_icon_from_mouse_position(mouse->graphic.x, mouse->graphic.y);
			struct hashmap_iter *iter = hashmap_iter(&desktop->icons);
			while (iter)
			{
				struct icon *i = hashmap_iter_get_data(iter);
				if (i != icon)
					i->active = false;
				iter = hashmap_iter_next(&desktop->icons, iter);
			}
			if (icon)
			{
				if (icon->active)
				{
					icon->active = false;
					posix_spawn(icon->exec_path);
				}
				else
					icon->active = true;
			}
		}
	}
}

void handle_keyboard_event(struct key_event *kevent)
{
	desktop->event_state = (desktop->event_state & ~0b1111) | kevent->state;
	if (desktop->active_window)
	{
		struct xevent *event = create_xkey_event(kevent->key, kevent->type, desktop->event_state);
		int32_t fd = mq_open(desktop->active_window->name, O_WRONLY, &(struct mq_attr){
																		 .mq_msgsize = sizeof(struct xevent),
																		 .mq_maxmsg = 32,
																	 });
		mq_send(fd, (char *)event, 0, sizeof(struct xevent));
		mq_close(fd);
		free(event);
	}
}

void handle_focus_event(struct msgui_focus *focus)
{
	struct window *win = find_window_in_root(focus->sender);
	desktop->active_window = win;
}
