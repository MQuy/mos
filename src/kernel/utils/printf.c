#include <kernel/cpu/hal.h>
#include <kernel/utils/string.h>
#include "printf.h"

//! video memory
uint16_t *video_memory = (uint16_t *)0xC00B8000;

//! current position
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

//! current color
uint8_t _color = 0;

//! Updates hardware cursor
void DebugUpdateCur(int x, int y)
{

	// get location
	uint16_t cursorLocation = y * 80 + x;

	// send location to vga controller to set cursor
	outportb(0x3D4, 14);
	outportb(0x3D5, cursorLocation >> 8); // Send the high byte.
	outportb(0x3D4, 15);
	outportb(0x3D5, cursorLocation); // Send the low byte.
}

void scroll()
{

	if (cursor_y >= 25)
	{

		uint16_t attribute = _color << 8;

		//! move current display up one line
		for (int i = 0 * 80; i < 24 * 80; i++)
			video_memory[i] = video_memory[i + 80];

		//! clear the bottom line
		for (int i = 24 * 80; i < 25 * 80; i++)
			video_memory[i] = attribute | ' ';

		cursor_y = 24;
	}
}

//! Displays a character
void DebugPutc(unsigned char c)
{

	uint16_t attribute = _color << 8;

	//! backspace character
	if (c == 0x08 && cursor_x)
		cursor_x--;

	//! tab character
	else if (c == 0x09)
		cursor_x = (cursor_x + 8) & ~(8 - 1);

	//! carriage return
	else if (c == '\r')
		cursor_x = 0;

	//! new line
	else if (c == '\n')
	{
		cursor_x = 0;
		cursor_y++;
	}

	//! printable characters
	else if (c >= ' ')
	{

		//! display character on screen
		uint16_t *location = video_memory + (cursor_y * 80 + cursor_x);
		*location = c | attribute;
		cursor_x++;
	}

	//! if we are at edge of row, go to new line
	if (cursor_x >= 80)
	{

		cursor_x = 0;
		cursor_y++;
	}

	//! if we are at the last line, scroll up
	if (cursor_y >= 25)
		scroll();

	//! update hardware cursor
	DebugUpdateCur(cursor_x, cursor_y);
}

//============================================================================
//    INTERFACE FUNCTIONS
//============================================================================

//! Sets new font color
unsigned DebugSetColor(const unsigned c)
{

	unsigned t = _color;
	_color = c;
	return t;
}

//! Sets new position
void DebugGotoXY(unsigned x, unsigned y)
{

	if (cursor_x <= 80)
		cursor_x = x;

	if (cursor_y <= 25)
		cursor_y = y;

	//! update hardware cursor to new position
	DebugUpdateCur(cursor_x, cursor_y);
}

//! returns position
void DebugGetXY(unsigned *x, unsigned *y)
{

	if (x == 0 || y == 0)
		return;

	*x = cursor_x;
	*y = cursor_y;
}

//! returns horzontal width
int DebugGetHorz()
{

	return 80;
}

//! returns vertical height
int DebugGetVert()
{

	return 24;
}

//! Clear screen
void DebugClrScr(const uint8_t c)
{

	//! clear video memory by writing space characters to it
	for (int i = 0; i < 80 * 25; i++)
		video_memory[i] = ' ' | (c << 8);

	//! move position back to start
	DebugGotoXY(0, 0);
}

//! Displays a string
void DebugPuts(char *str)
{

	if (!str)
		return;

	//! err... displays a string
	for (unsigned int i = 0; i < strlen(str); i++)
		DebugPutc(str[i]);
}

//! Displays a formatted string
int DebugPrintf(const char *str, ...)
{

	if (!str)
		return 0;

	va_list args;
	va_start(args, str);
	size_t i;
	for (i = 0; i < strlen(str); i++)
	{

		switch (str[i])
		{

		case '%':

			switch (str[i + 1])
			{

			/*** characters ***/
			case 'c':
			{
				char c = va_arg(args, int);
				DebugPutc(c);
				i++; // go to next character
				break;
			}

			/*** address of ***/
			case 's':
			{
				char *c = va_arg(args, char *);
				char str[64];
				strcpy(str, (const char *)c);
				DebugPuts(str);
				i++; // go to next character
				break;
			}

			/*** integers ***/
			case 'd':
			case 'i':
			{
				int c = va_arg(args, int);
				char str[32] = {0};
				itoa_s(c, 10, str);
				DebugPuts(str);
				i++; // go to next character
				break;
			}

			/*** display in hex ***/
			case 'X':
			case 'x':
			{
				unsigned int c = va_arg(args, unsigned int);
				char str[32] = {0};
				itoa_s(c, 16, str);
				DebugPuts(str);
				i++; // go to next character
				break;
			}

			default:
				va_end(args);
				return 1;
			}

			break;

		default:
			DebugPutc(str[i]);
			break;
		}
	}

	va_end(args);
	return i;
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[DebugDisplay.cpp]
//**
//****************************************************************************
