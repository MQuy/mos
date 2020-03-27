#include <stdint.h>
#include <libc/unistd.h>
#include <libc/string.h>
#include <libc/stdlib.h>
#include <libc/gui/layout.h>

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
  struct ui_label *label = malloc(sizeof(struct ui_label));
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

void on_click_label_ac(struct window *win)
{
  struct ui_label *lresult = labels[LABEL_RESULT];
  char *text = calloc(1, sizeof(char));
  lresult->set_text(lresult, text);
}

// TODO: MQ 2020-03-27 Handle equal click
void on_click_label_equal(struct window *win)
{
}

int main(struct framebuffer fb)
{
  struct window *win = init_window(336, 196, 128, 208);

  for (uint32_t i = 0; i < win->graphic.height; ++i)
  {
    char *ibuf = win->graphic.buf + i * win->graphic.width * 4;
    for (uint32_t j = 0; j < win->graphic.width; ++j)
    {
      ibuf[0] = 0x32;
      ibuf[1] = 0x2C;
      ibuf[2] = 0x28;
      ibuf[3] = 0xFF;
      ibuf += 4;
    }
  }

  struct ui_style *style1 = malloc(sizeof(struct ui_style));
  style1->padding_top = 8;
  style1->padding_left = 10;
  struct ui_style *style2 = malloc(sizeof(struct ui_style));
  style2->padding_top = 8;
  style2->padding_left = 26;

  labels[LABEL_RESULT] = create_label(win, 0, 16, 128, 32, "", style1, on_click_label_equal);

  labels[LABEL_AC] = create_label(win, 0, 48, 32, 32, "C", style1, on_click_label_ac);
  labels[LABEL_SIGN] = create_label(win, 32, 48, 32, 32, "~", style1, on_click_label);
  labels[LABEL_PERCENT] = create_label(win, 64, 48, 32, 32, "%", style1, on_click_label);
  labels[LABEL_DIV] = create_label(win, 96, 48, 32, 32, "/", style1, on_click_label);

  labels[LABEL_SEVEN] = create_label(win, 0, 80, 32, 32, "7", style1, on_click_label);
  labels[LABEL_EIGHT] = create_label(win, 32, 80, 32, 32, "8", style1, on_click_label);
  labels[LABEL_SEVEN] = create_label(win, 64, 80, 32, 32, "9", style1, on_click_label);
  labels[LABEL_MUL] = create_label(win, 96, 80, 32, 32, "*", style1, on_click_label);

  labels[LABEL_FOUR] = create_label(win, 0, 112, 32, 32, "4", style1, on_click_label);
  labels[LABEL_FIVE] = create_label(win, 32, 112, 32, 32, "5", style1, on_click_label);
  labels[LABEL_SIX] = create_label(win, 64, 112, 32, 32, "6", style1, on_click_label);
  labels[LABEL_PLUS] = create_label(win, 96, 112, 32, 32, "+", style1, on_click_label);

  labels[LABEL_ONE] = create_label(win, 0, 144, 32, 32, "1", style1, on_click_label);
  labels[LABEL_TWO] = create_label(win, 32, 144, 32, 32, "2", style1, on_click_label);
  labels[LABEL_THREE] = create_label(win, 64, 144, 32, 32, "3", style1, on_click_label);
  labels[LABEL_MINUS] = create_label(win, 96, 144, 32, 32, "-", style1, on_click_label);

  labels[LABEL_ZERO] = create_label(win, 0, 176, 64, 32, "0", style2, on_click_label);
  labels[LABEL_DOT] = create_label(win, 64, 176, 32, 32, ".", style1, on_click_label);
  labels[LABEL_EQUAL] = create_label(win, 96, 176, 32, 32, "=", style1, on_click_label_equal);

  enter_event_loop(win);

  return 0;
}
