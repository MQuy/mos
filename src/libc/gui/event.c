#include "event.h"

#include <libc/stdlib.h>
#include <libc/string.h>

struct xevent *create_xkey_event(uint32_t key, enum xevent_action action, unsigned int state)
{
	struct xevent *event = calloc(1, sizeof(struct xevent));
	event->type = XKEY_EVENT;

	struct xkey_event *key_event = (struct xkey_event *)event->data;
	key_event->action = action;
	key_event->key = key;
	key_event->state = state;

	return event;
}

struct xevent *create_xmotion_event(int32_t x, int32_t y, unsigned int state)
{
	struct xevent *event = calloc(1, sizeof(struct xevent));
	event->type = XMOTION_EVENT;

	struct xmotion_event *motion_event = (struct xmotion_event *)event->data;
	motion_event->x = x;
	motion_event->y = y;
	motion_event->state = state;

	return event;
}

struct xevent *create_xbutton_event(uint8_t button, enum xevent_action action, int32_t x, int32_t y, unsigned int state)
{
	struct xevent *event = calloc(1, sizeof(struct xevent));
	event->type = XBUTTON_EVENT;

	struct xbutton_event *button_event = (struct xbutton_event *)event->data;
	button_event->action = action;
	button_event->button = button;
	button_event->x = x;
	button_event->y = y;
	button_event->state = state;

	return event;
}

struct keycode_to_ascii
{
	unsigned int keycode;
	unsigned char ascii;
};

static struct keycode_to_ascii map0[] = {
	{KEY_ESC, 0x1B},
	{KEY_1, '1'},
	{KEY_2, '2'},
	{KEY_3, '3'},
	{KEY_4, '4'},
	{KEY_5, '5'},
	{KEY_6, '6'},
	{KEY_7, '7'},
	{KEY_8, '8'},
	{KEY_9, '9'},
	{KEY_0, '0'},
	{KEY_MINUS, '-'},
	{KEY_EQUAL, '='},
	{KEY_BACKSPACE, 0x08},
	{KEY_TAB, 0x09},
	{KEY_Q, 'q'},
	{KEY_W, 'w'},
	{KEY_E, 'e'},
	{KEY_R, 'r'},
	{KEY_T, 't'},
	{KEY_Y, 'y'},
	{KEY_U, 'u'},
	{KEY_I, 'i'},
	{KEY_O, 'o'},
	{KEY_P, 'p'},
	{KEY_LEFTBRACE, '['},
	{KEY_RIGHTBRACE, ']'},
	{KEY_ENTER, '\n'},
	{KEY_A, 'a'},
	{KEY_S, 's'},
	{KEY_D, 'd'},
	{KEY_F, 'f'},
	{KEY_G, 'g'},
	{KEY_H, 'h'},
	{KEY_J, 'j'},
	{KEY_K, 'k'},
	{KEY_L, 'l'},
	{KEY_SEMICOLON, ';'},
	{KEY_APOSTROPHE, '\''},
	{KEY_GRAVE, '`'},
	{KEY_BACKSLASH, '/'},
	{KEY_Z, 'z'},
	{KEY_X, 'x'},
	{KEY_C, 'c'},
	{KEY_V, 'v'},
	{KEY_B, 'b'},
	{KEY_N, 'n'},
	{KEY_M, 'm'},
	{KEY_COMMA, ','},
	{KEY_DOT, '.'},
	{KEY_SLASH, '\\'},
	{KEY_SPACE, ' '},
};

static struct keycode_to_ascii map1[] = {
	{KEY_1, '!'},
	{KEY_2, '@'},
	{KEY_3, '#'},
	{KEY_4, '$'},
	{KEY_5, '%'},
	{KEY_6, '^'},
	{KEY_7, '&'},
	{KEY_8, '*'},
	{KEY_9, '('},
	{KEY_0, ')'},
	{KEY_MINUS, '_'},
	{KEY_EQUAL, '+'},
	{KEY_Q, 'Q'},
	{KEY_W, 'W'},
	{KEY_E, 'E'},
	{KEY_R, 'R'},
	{KEY_T, 'T'},
	{KEY_Y, 'Y'},
	{KEY_U, 'U'},
	{KEY_I, 'I'},
	{KEY_O, 'O'},
	{KEY_P, 'P'},
	{KEY_LEFTBRACE, '{'},
	{KEY_RIGHTBRACE, '}'},
	{KEY_A, 'A'},
	{KEY_S, 'S'},
	{KEY_D, 'D'},
	{KEY_F, 'F'},
	{KEY_G, 'G'},
	{KEY_H, 'H'},
	{KEY_J, 'J'},
	{KEY_K, 'K'},
	{KEY_L, 'L'},
	{KEY_SEMICOLON, ':'},
	{KEY_APOSTROPHE, '"'},
	{KEY_GRAVE, '~'},
	{KEY_BACKSLASH, '?'},
	{KEY_Z, 'Z'},
	{KEY_X, 'X'},
	{KEY_C, 'C'},
	{KEY_V, 'V'},
	{KEY_B, 'B'},
	{KEY_N, 'N'},
	{KEY_M, 'M'},
	{KEY_COMMA, '<'},
	{KEY_DOT, '>'},
	{KEY_SLASH, '|'},
};

static struct keycode_to_ascii map2[] = {
	{KEY_1, '¡'},
	{KEY_2, '™'},
	{KEY_3, '£'},
	{KEY_4, '¢'},
	{KEY_6, '§'},
	{KEY_7, '¶'},
	{KEY_8, '•'},
	{KEY_9, 'ª'},
	{KEY_0, 'º'},
	{KEY_MINUS, '–'},
	{KEY_Q, 'œ'},
	{KEY_R, '®'},
	{KEY_T, '†'},
	{KEY_Y, '¥'},
	{KEY_U, '¨'},
	{KEY_O, 'ø'},
	{KEY_LEFTBRACE, '“'},
	{KEY_RIGHTBRACE, '‘'},
	{KEY_A, 'å'},
	{KEY_S, 'ß'},
	{KEY_F, 'ƒ'},
	{KEY_G, '©'},
	{KEY_L, '¬'},
	{KEY_SEMICOLON, '…'},
	{KEY_APOSTROPHE, 'æ'},
	{KEY_GRAVE, '`'},
	{KEY_BACKSLASH, '÷'},
	{KEY_C, 'ç'},
	{KEY_N, '˜'},
	{KEY_M, 'µ'},
	{KEY_SLASH, '«'},
};

static struct keycode_to_ascii map3[] = {
	{KEY_2, '€'},
	{KEY_3, '‹'},
	{KEY_4, '›'},
	{KEY_7, '‡'},
	{KEY_8, '°'},
	{KEY_9, '·'},
	{KEY_0, '‚'},
	{KEY_MINUS, '—'},
	{KEY_EQUAL, '±'},
	{KEY_Q, 'Œ'},
	{KEY_W, '„'},
	{KEY_E, '´'},
	{KEY_R, '‰'},
	{KEY_Y, 'Á'},
	{KEY_U, '¨'},
	{KEY_I, 'ˆ'},
	{KEY_O, 'Ø'},
	{KEY_LEFTBRACE, '”'},
	{KEY_RIGHTBRACE, '’'},
	{KEY_A, 'Å'},
	{KEY_S, 'Í'},
	{KEY_D, 'Î'},
	{KEY_F, 'Ï'},
	{KEY_H, 'Ó'},
	{KEY_J, 'Ô'},
	{KEY_L, 'Ò'},
	{KEY_SEMICOLON, 'Ú'},
	{KEY_APOSTROPHE, 'Æ'},
	{KEY_BACKSLASH, '¿'},
	{KEY_Z, '¸'},
	{KEY_C, 'Ç'},
	{KEY_N, '˜'},
	{KEY_M, 'Â'},
	{KEY_COMMA, '¯'},
};

int find_ascii_from_table(struct keycode_to_ascii *map, int length, unsigned int keycode, unsigned char *buf, int *nbuf)
{
	for (int i = 0; i < length; ++i)
		if (map[i].keycode == keycode)
		{
			*buf = map[i].ascii;
			*nbuf = 1;
			return 0;
		}

	return -1;
}

int convert_keycode_to_ascii(unsigned int keycode, unsigned int state, unsigned char *buf, int *nbuf)
{
	int ret = 0;
	if (state & CONTROL_MASK && keycode >= 64 && keycode <= 96)
	{
		*buf = ((char)keycode - 64) & 0x7F;
		*nbuf = 1;
	}
	else if (state & SHIFT_MASK && state & ALT_MASK)
		ret = find_ascii_from_table(map3, sizeof(map3) / sizeof(struct keycode_to_ascii), keycode, buf, nbuf);
	else if (state & ALT_MASK)
		find_ascii_from_table(map2, sizeof(map2) / sizeof(struct keycode_to_ascii), keycode, buf, nbuf);
	else if (state & SHIFT_MASK)
		ret = find_ascii_from_table(map1, sizeof(map1) / sizeof(struct keycode_to_ascii), keycode, buf, nbuf);
	else
	{
		ret = find_ascii_from_table(map0, sizeof(map0) / sizeof(struct keycode_to_ascii), keycode, buf, nbuf);
		if (ret == -1)
		{
			if (keycode == KEY_KP4)	 // left arrow
			{
				*nbuf = 4;
				memcpy(buf, "\033[1D", *nbuf);
			}
			else if (keycode == KEY_KP6)  // right arrow
			{
				*nbuf = 4;
				memcpy(buf, "\033[1C", *nbuf);
			}
			else if (keycode == KEY_KP8)  // up arrow
			{
				*nbuf = 4;
				memcpy(buf, "\033[1A", *nbuf);
			}
			else if (keycode == KEY_KP2)  // down arrow
			{
				*nbuf = 4;
				memcpy(buf, "\033[1B", *nbuf);
			}
		}
	}

	return ret;
}
