#include "asci.h"

#include <assert.h>
#include <libcore/hashtable/hashmap.h>
#include <libgui/event.h>
#include <stdlib.h>
#include <string.h>

#include "terminal.h"

// NOTE: MQ 2020-12-15
// (ALT | SHIFT | CTRL | CAPSLOCK)
// Use bits order from kernel/devices/kybrd.c#kybrd_set_event_state
#define ALT_PART 0x80000000
#define SHIFT_PART 0x40000000
#define CTRL_PART 0x20000000
#define CAPSLOCK_PART 0x10000000

static struct hashmap mprintable, msequence;

enum
{
	ASCI_CURSOR_UP = 0,
	ASCI_CURSOR_DOWN = 1,
	ASCI_CURSOR_FORWARD = 2,
	ASCI_CURSOR_BACK = 3,
	ASCI_CURSOR_NEXTLINE = 4,
	ASCI_CURSOR_PREVLINE = 5,
	ASCI_CURSOR_HA = 6,
	ASCI_CURSOR_POSITION = 7,
	ASCI_ERASE = 8,
	ASCI_ERASE_LINE = 9,
	ASCI_SCROLL_UP = 10,
	ASCI_SCROLL_DOWN = 11,
	ASCI_HV_POSITION = 12,
	ASCI_SELECT_GRAPHIC_POSITION = 13,
};

struct keycode_ascii
{
	unsigned int keycode;
	unsigned char ascii;
};

struct keycode_sequence
{
	unsigned int keycode;
	char *sequence;
};

static struct keycode_ascii asciis[] = {
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
	{KEY_BACKSPACE, '\177'},

	// with control
	{CTRL_PART | KEY_A, 1},
	{CTRL_PART | KEY_B, 2},
	{CTRL_PART | KEY_C, 3},
	{CTRL_PART | KEY_D, 4},
	{CTRL_PART | KEY_E, 5},
	{CTRL_PART | KEY_F, 6},
	{CTRL_PART | KEY_G, 7},
	{CTRL_PART | KEY_H, 8},
	{CTRL_PART | KEY_I, 9},
	{CTRL_PART | KEY_J, 10},
	{CTRL_PART | KEY_K, 11},
	{CTRL_PART | KEY_L, 12},
	{CTRL_PART | KEY_M, 13},
	{CTRL_PART | KEY_N, 14},
	{CTRL_PART | KEY_O, 15},
	{CTRL_PART | KEY_P, 16},
	{CTRL_PART | KEY_Q, 17},
	{CTRL_PART | KEY_R, 18},
	{CTRL_PART | KEY_S, 19},
	{CTRL_PART | KEY_T, 20},
	{CTRL_PART | KEY_U, 21},
	{CTRL_PART | KEY_V, 22},
	{CTRL_PART | KEY_W, 23},
	{CTRL_PART | KEY_X, 24},
	{CTRL_PART | KEY_Y, 25},
	{CTRL_PART | KEY_Z, 26},
	{CTRL_PART | KEY_LEFTBRACE, 27},
	{CTRL_PART | KEY_BACKSLASH, 28},
	{CTRL_PART | KEY_RIGHTBRACE, 29},

	// with shift
	{SHIFT_PART | KEY_1, '!'},
	{SHIFT_PART | KEY_2, '@'},
	{SHIFT_PART | KEY_3, '#'},
	{SHIFT_PART | KEY_4, '$'},
	{SHIFT_PART | KEY_5, '%'},
	{SHIFT_PART | KEY_6, '^'},
	{SHIFT_PART | KEY_7, '&'},
	{SHIFT_PART | KEY_8, '*'},
	{SHIFT_PART | KEY_9, '('},
	{SHIFT_PART | KEY_0, ')'},
	{SHIFT_PART | KEY_MINUS, '_'},
	{SHIFT_PART | KEY_EQUAL, '+'},
	{SHIFT_PART | KEY_Q, 'Q'},
	{SHIFT_PART | KEY_W, 'W'},
	{SHIFT_PART | KEY_E, 'E'},
	{SHIFT_PART | KEY_R, 'R'},
	{SHIFT_PART | KEY_T, 'T'},
	{SHIFT_PART | KEY_Y, 'Y'},
	{SHIFT_PART | KEY_U, 'U'},
	{SHIFT_PART | KEY_I, 'I'},
	{SHIFT_PART | KEY_O, 'O'},
	{SHIFT_PART | KEY_P, 'P'},
	{SHIFT_PART | KEY_LEFTBRACE, '{'},
	{SHIFT_PART | KEY_RIGHTBRACE, '}'},
	{SHIFT_PART | KEY_A, 'A'},
	{SHIFT_PART | KEY_S, 'S'},
	{SHIFT_PART | KEY_D, 'D'},
	{SHIFT_PART | KEY_F, 'F'},
	{SHIFT_PART | KEY_G, 'G'},
	{SHIFT_PART | KEY_H, 'H'},
	{SHIFT_PART | KEY_J, 'J'},
	{SHIFT_PART | KEY_K, 'K'},
	{SHIFT_PART | KEY_L, 'L'},
	{SHIFT_PART | KEY_SEMICOLON, ':'},
	{SHIFT_PART | KEY_APOSTROPHE, '"'},
	{SHIFT_PART | KEY_GRAVE, '~'},
	{SHIFT_PART | KEY_BACKSLASH, '?'},
	{SHIFT_PART | KEY_Z, 'Z'},
	{SHIFT_PART | KEY_X, 'X'},
	{SHIFT_PART | KEY_C, 'C'},
	{SHIFT_PART | KEY_V, 'V'},
	{SHIFT_PART | KEY_B, 'B'},
	{SHIFT_PART | KEY_N, 'N'},
	{SHIFT_PART | KEY_M, 'M'},
	{SHIFT_PART | KEY_COMMA, '<'},
	{SHIFT_PART | KEY_DOT, '>'},
	{SHIFT_PART | KEY_SLASH, '|'},

	// with alt
	{ALT_PART | KEY_1, 0xA1},			// ¡
	{ALT_PART | KEY_2, 0x99},			// ™
	{ALT_PART | KEY_3, 0xA3},			// £
	{ALT_PART | KEY_4, 0xA2},			// ¢
	{ALT_PART | KEY_6, 0xA7},			// §
	{ALT_PART | KEY_7, 0xB6},			// ¶
	{ALT_PART | KEY_8, 0x95},			// •
	{ALT_PART | KEY_9, 0xAA},			// ª
	{ALT_PART | KEY_0, 0xBA},			// º
	{ALT_PART | KEY_MINUS, 0x96},		// –
	{ALT_PART | KEY_Q, 0x9C},			// œ
	{ALT_PART | KEY_R, 0xAE},			// ®
	{ALT_PART | KEY_T, 0x86},			// †
	{ALT_PART | KEY_Y, 0xA5},			// ¥
	{ALT_PART | KEY_U, 0xA8},			// ¨
	{ALT_PART | KEY_O, 0xF8},			// ø
	{ALT_PART | KEY_LEFTBRACE, 0x93},	// “
	{ALT_PART | KEY_LEFTBRACE, 0x94},	// ”
	{ALT_PART | KEY_A, 0xE5},			// å
	{ALT_PART | KEY_S, 0xDF},			// ß
	{ALT_PART | KEY_F, 0x83},			// ƒ
	{ALT_PART | KEY_G, 0xA9},			// ©
	{ALT_PART | KEY_L, 0xAC},			// ¬
	{ALT_PART | KEY_SEMICOLON, 0x85},	// …
	{ALT_PART | KEY_APOSTROPHE, 0xE6},	// æ
	{ALT_PART | KEY_GRAVE, 0x60},		// `
	{ALT_PART | KEY_BACKSLASH, 0xF7},	// ÷
	{ALT_PART | KEY_C, 0xE7},			// ç
	{ALT_PART | KEY_N, 0x98},			// ˜
	{ALT_PART | KEY_M, 0xB5},			// µ
	{ALT_PART | KEY_SLASH, 0xAB},		// «

	// with shift and alt
	{SHIFT_PART | ALT_PART | KEY_2, 0x80},			 // €
	{SHIFT_PART | ALT_PART | KEY_3, 0x8B},			 // ‹
	{SHIFT_PART | ALT_PART | KEY_4, 0x9B},			 // ›
	{SHIFT_PART | ALT_PART | KEY_7, 0x87},			 // ‡
	{SHIFT_PART | ALT_PART | KEY_8, 0xB0},			 // °
	{SHIFT_PART | ALT_PART | KEY_9, 0xB7},			 // ·
	{SHIFT_PART | ALT_PART | KEY_0, 0x82},			 // ‚
	{SHIFT_PART | ALT_PART | KEY_MINUS, 0x97},		 // —
	{SHIFT_PART | ALT_PART | KEY_EQUAL, 0xB1},		 // ±
	{SHIFT_PART | ALT_PART | KEY_Q, 0x8C},			 // Œ
	{SHIFT_PART | ALT_PART | KEY_W, 0x84},			 // „
	{SHIFT_PART | ALT_PART | KEY_E, 0xB4},			 // ´
	{SHIFT_PART | ALT_PART | KEY_R, 0x89},			 // ‰
	{SHIFT_PART | ALT_PART | KEY_Y, 0xC1},			 // Á
	{SHIFT_PART | ALT_PART | KEY_U, 0xA8},			 // ¨
	{SHIFT_PART | ALT_PART | KEY_I, 0x88},			 // ˆ
	{SHIFT_PART | ALT_PART | KEY_O, 0xD8},			 // Ø
	{SHIFT_PART | ALT_PART | KEY_RIGHTBRACE, 0x94},	 // ”
	{SHIFT_PART | ALT_PART | KEY_RIGHTBRACE, 0x92},	 // ’
	{SHIFT_PART | ALT_PART | KEY_A, 0xC5},			 // Å
	{SHIFT_PART | ALT_PART | KEY_S, 0xCD},			 // Í
	{SHIFT_PART | ALT_PART | KEY_D, 0xCE},			 // Î
	{SHIFT_PART | ALT_PART | KEY_F, 0xCF},			 // Ï
	{SHIFT_PART | ALT_PART | KEY_H, 0xD3},			 // Ó
	{SHIFT_PART | ALT_PART | KEY_J, 0xD4},			 // Ô
	{SHIFT_PART | ALT_PART | KEY_L, 0xD2},			 // Ò
	{SHIFT_PART | ALT_PART | KEY_SEMICOLON, 0xDA},	 // Ú
	{SHIFT_PART | ALT_PART | KEY_APOSTROPHE, 0xC6},	 // Æ
	{SHIFT_PART | ALT_PART | KEY_BACKSLASH, 0xBF},	 // ¿
	{SHIFT_PART | ALT_PART | KEY_Z, 0xB8},			 // ¸
	{SHIFT_PART | ALT_PART | KEY_C, 0xC7},			 // Ç
	{SHIFT_PART | ALT_PART | KEY_N, 0x98},			 // ˜
	{SHIFT_PART | ALT_PART | KEY_M, 0xC2},			 // Â
	{SHIFT_PART | ALT_PART | KEY_COMMA, 0xAF},		 // ¯
};

// NOTE: MQ 2020-12-14
// in cursor row, gnu bash only supports subset of CSI_sequences
// bash-5.0/lib/readline/readline.c#bind_arrow_keys_internal
// we should use standard keymap instead
// bash-5.0/lib/readline/emacs_keymap.c
struct keycode_sequence sequences[] = {
	{KEY_KP4, "\x02"},	// keypad left arrow
	{KEY_KP6, "\x06"},	// keypad right arrow
	{KEY_KP8, "\x10"},	// keypad up arrow
	{KEY_KP2, "\x0E"},	// keypad down arrow
};

void asci_init()
{
	int nitems = sizeof(asciis) / sizeof(struct keycode_ascii);
	hashmap_init(&mprintable, hashmap_hash_uint32, hashmap_compare_uint32, nitems);
	for (int i = 0; i < nitems; ++i)
	{
		struct keycode_ascii *ka = &asciis[i];
		hashmap_put(&mprintable, &ka->keycode, &ka->ascii);
	}

	nitems = sizeof(sequences) / sizeof(struct keycode_sequence);
	hashmap_init(&msequence, hashmap_hash_uint32, hashmap_compare_uint32, nitems);
	for (int i = 0; i < nitems; ++i)
	{
		struct keycode_sequence *ks = &sequences[i];
		hashmap_put(&msequence, &ks->keycode, ks->sequence);
	}
}

int convert_key_event_to_code(unsigned int keycode, unsigned int state, unsigned char *buf)
{
	int count = 0;
	char *value = NULL;
	unsigned int keystate = keycode | (state << 28);

	// ascii code
	if ((value = hashmap_get(&mprintable, &keystate)))
	{
		count = 1;
		*buf = *value;
	}
	// asci escape sequence
	else if ((value = hashmap_get(&msequence, &keystate)))
	{
		count = strlen(value);
		memcpy(buf, value, count);
	}

	return count;
}

int parse_control_sequence(char *input, struct asci_control *control)
{
	int i;
	for (i = 2;; ++i)
	{
		unsigned char ch = input[i];
		if (ch == 'A')
		{
			control->type = ASCI_CURSOR_UP;
			break;
		}
		else if (ch == 'B')
		{
			control->type = ASCI_CURSOR_DOWN;
			break;
		}
		else if (ch == 'C')
		{
			control->type = ASCI_CURSOR_FORWARD;
			break;
		}
		else if (ch == 'D')
		{
			control->type = ASCI_CURSOR_FORWARD;
			break;
		}
		else if (ch == 'E')
		{
			control->type = ASCI_CURSOR_NEXTLINE;
			break;
		}
		else if (ch == 'F')
		{
			control->type = ASCI_CURSOR_PREVLINE;
			break;
		}
		else if (ch == 'G')
		{
			control->type = ASCI_CURSOR_HA;
			break;
		}
		else if (ch == 'H')
		{
			control->type = ASCI_CURSOR_POSITION;
			break;
		}
		else if (ch == 'J')
		{
			control->type = ASCI_ERASE;
			break;
		}
		else if (ch == 'K')
		{
			control->type = ASCI_ERASE_LINE;
			break;
		}
		else if (ch == 'S')
		{
			control->type = ASCI_SCROLL_UP;
			break;
		}
		else if (ch == 'T')
		{
			control->type = ASCI_SCROLL_DOWN;
			break;
		}
		else if (ch == 'm')
		{
			control->type = ASCI_SELECT_GRAPHIC_POSITION;
			break;
		}
	}

	int count = i - 2;
	if (count > 0)
	{
		control->data = calloc(count + 1, sizeof(char));
		memcpy(input + 2, control->data, count);
	}

	return count;
}

int parse_asci_sequence(char *input, struct asci_sequence *seq)
{
	if (input[0] != '\33')
		return -1;

	switch (input[1])
	{
	case '[':
		seq->type = ASCI_CONTROL_SEQUENCE_INTRODUCER;
		seq->data = calloc(1, sizeof(struct asci_control));
		return parse_control_sequence(input, seq->data);

	default:
		assert_not_reached();
		__builtin_unreachable();
	}
}

void handle_asci_control(struct terminal *term, struct asci_control *control)
{
	if (control->type == ASCI_CURSOR_BACK)
	{
		int n = control->data ? atoi(control->data) : 1;
		terminal_move_cursor_column(term, -n, CURSOR_CUR);
	}
	else if (control->type == ASCI_CURSOR_FORWARD)
	{
		int n = control->data ? atoi(control->data) : 1;
		terminal_move_cursor_column(term, n, CURSOR_CUR);
	}
	else if (control->type == ASCI_CURSOR_UP)
	{
		int n = control->data ? atoi(control->data) : 1;
		terminal_move_cursor_row(term, -n, CURSOR_CUR);
	}

	else if (control->type == ASCI_CURSOR_DOWN)
	{
		int n = control->data ? atoi(control->data) : 1;
		terminal_move_cursor_row(term, n, CURSOR_CUR);
	}

	else if (control->type == ASCI_CURSOR_NEXTLINE)
	{
		int n = control->data ? atoi(control->data) : 1;
		terminal_move_cursor_column(term, 0, CURSOR_SET);
		terminal_move_cursor_row(term, n, CURSOR_CUR);
	}

	else if (control->type == ASCI_CURSOR_PREVLINE)
	{
		int n = control->data ? atoi(control->data) : 1;
		terminal_move_cursor_column(term, 0, CURSOR_SET);
		terminal_move_cursor_row(term, -n, CURSOR_CUR);
	}

	else if (control->type == ASCI_CURSOR_HA)
	{
		int n = control->data ? atoi(control->data) : 1;
		terminal_move_cursor_column(term, n, CURSOR_SET);
	}

	else if (control->type == ASCI_CURSOR_POSITION)
	{
		int row = -1;
		int column = -1;

		if (control->data)
			for (int i = 0, flip = 0, length = strlen(control->data); i < length && flip <= 1; ++i)
			{
				unsigned char ch = ((char *)control->data)[i];

				if (ch == ';')
				{
					flip++;
					continue;
				}

				if (flip == 0)
				{
					if (row == -1)
						row = 0;
					row = row * 10 + (ch - '0');
				}
				else
				{
					if (column == -1)
						column = 0;
					column = column * 10 + (ch - '0');
				}
			}

		terminal_move_cursor(term, abs(row), abs(column), CURSOR_SET);
	}
	else
		assert_not_reached();
}

void handle_asci_sequence(struct terminal *term, struct asci_sequence *seq)
{
	if (seq->type == ASCI_CONTROL_SEQUENCE_INTRODUCER)
		handle_asci_control(term, seq->data);
	else
		assert_not_reached();
}
