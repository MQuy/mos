#include <gui/layout.h>
#include <shared/cdefs.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LABEL_ZERO 0
#define LABEL_ONE 1
#define LABEL_TWO 2
#define LABEL_THREE 3
#define LABEL_FOUR 4
#define LABEL_FIVE 5
#define LABEL_SIX 6
#define LABEL_SEVEN 7
#define LABEL_EIGHT 8
#define LABEL_NINE 9
#define LABEL_DOT 10
#define LABEL_AC 11
#define LABEL_SIGN 12
#define LABEL_PERCENT 13
#define LABEL_DIV 14
#define LABEL_MUL 15
#define LABEL_MINUS 16
#define LABEL_PLUS 17
#define LABEL_EQUAL 18
#define LABEL_RESULT 19

struct ui_label *labels[20];

struct ui_label *create_label(
	struct window *parent,
	int32_t x, int32_t y,
	uint32_t width, uint32_t height,
	char *text,
	struct ui_style *style,
	EVENT_HANDLER handler)
{
	struct ui_label *label = calloc(1, sizeof(struct ui_label));
	gui_create_label(parent, label, x, y, width, height, text, style);
	label->window.add_event_listener(&label->window, "click", handler);
	return label;
}

void on_click_label(struct window *win)
{
	struct ui_label *label = (struct ui_label *)win;
	struct ui_label *lresult = labels[LABEL_RESULT];
	uint32_t length = strlen(lresult->text);

	char *text = calloc(length + 2, sizeof(char));
	memcpy(text, lresult->text, length);
	memcpy(text + length, label->text, 1);
	lresult->set_text(lresult, text);
}

void on_click_label_ac(__unused struct window *win)
{
	struct ui_label *lresult = labels[LABEL_RESULT];
	char *text = calloc(1, sizeof(char));
	lresult->set_text(lresult, text);
}

// TODO: MQ 2020-03-27 Handle equal click
void on_click_label_equal(__unused struct window *win)
{
}

int main()
{
	struct window *app_win = init_window(336, 196, 128, 208);
	struct window *container_win = list_last_entry(&app_win->children, struct window, sibling);

	struct ui_style *style1 = calloc(1, sizeof(struct ui_style));
	style1->padding_top = 8;
	style1->padding_left = 10;
	struct ui_style *style2 = calloc(1, sizeof(struct ui_style));
	style2->padding_top = 8;
	style2->padding_left = 26;

	labels[LABEL_RESULT] = create_label(container_win, 0, 0, 128, 32, "", style1, on_click_label_equal);

	labels[LABEL_AC] = create_label(container_win, 0, 32, 32, 32, "C", style1, on_click_label_ac);
	labels[LABEL_SIGN] = create_label(container_win, 32, 32, 32, 32, "~", style1, on_click_label);
	labels[LABEL_PERCENT] = create_label(container_win, 64, 32, 32, 32, "%", style1, on_click_label);
	labels[LABEL_DIV] = create_label(container_win, 96, 32, 32, 32, "/", style1, on_click_label);

	labels[LABEL_SEVEN] = create_label(container_win, 0, 64, 32, 32, "7", style1, on_click_label);
	labels[LABEL_EIGHT] = create_label(container_win, 32, 64, 32, 32, "8", style1, on_click_label);
	labels[LABEL_SEVEN] = create_label(container_win, 64, 64, 32, 32, "9", style1, on_click_label);
	labels[LABEL_MUL] = create_label(container_win, 96, 64, 32, 32, "*", style1, on_click_label);

	labels[LABEL_FOUR] = create_label(container_win, 0, 96, 32, 32, "4", style1, on_click_label);
	labels[LABEL_FIVE] = create_label(container_win, 32, 96, 32, 32, "5", style1, on_click_label);
	labels[LABEL_SIX] = create_label(container_win, 64, 96, 32, 32, "6", style1, on_click_label);
	labels[LABEL_PLUS] = create_label(container_win, 96, 96, 32, 32, "+", style1, on_click_label);

	labels[LABEL_ONE] = create_label(container_win, 0, 128, 32, 32, "1", style1, on_click_label);
	labels[LABEL_TWO] = create_label(container_win, 32, 128, 32, 32, "2", style1, on_click_label);
	labels[LABEL_THREE] = create_label(container_win, 64, 128, 32, 32, "3", style1, on_click_label);
	labels[LABEL_MINUS] = create_label(container_win, 96, 128, 32, 32, "-", style1, on_click_label);

	labels[LABEL_ZERO] = create_label(container_win, 0, 160, 64, 32, "0", style2, on_click_label);
	labels[LABEL_DOT] = create_label(container_win, 64, 160, 32, 32, ".", style1, on_click_label);
	labels[LABEL_EQUAL] = create_label(container_win, 96, 160, 32, 32, "=", style1, on_click_label_equal);

	enter_event_loop(app_win, NULL, NULL, 0, NULL);

	return 0;
}
