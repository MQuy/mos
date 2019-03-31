//****************************************************************************
//**
//**    DebugDisplay.cpp
//**    - Provides display capabilities for debugging. Because it is
//**	  specifically for debugging and not final release, we don't
//** 	  care for portability here
//**
//****************************************************************************
//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include "DebugDisplay.h"
#include "./Include/string.h"

//============================================================================
//    IMPLEMENTATION PRIVATE DEFINITIONS / ENUMERATIONS / SIMPLE TYPEDEFS
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE CLASS PROTOTYPES / EXTERNAL CLASS REFERENCES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE STRUCTURES / UTILITY CLASSES
//============================================================================
//============================================================================
//    IMPLEMENTATION REQUIRED EXTERNAL REFERENCES (AVOID)
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE DATA
//============================================================================

//! video memory
uint16_t *video_memory = (uint16_t *)0xB8000;

//! current position
uint8_t cursor_x = 0;
uint8_t cursor_y = 0;

//! current color
uint8_t _color = 0;

//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================

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

char tbuf[32];
char bchars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void itoa(unsigned i, unsigned base, char *buf)
{
	int pos = 0;
	int opos = 0;
	int top = 0;

	if (i == 0 || base > 16)
	{
		buf[0] = '0';
		buf[1] = '\0';
		return;
	}

	while (i != 0)
	{
		tbuf[pos] = bchars[i % base];
		pos++;
		i /= base;
	}
	top = pos--;
	for (opos = 0; opos < top; pos--, opos++)
	{
		buf[opos] = tbuf[pos];
	}
	buf[opos] = 0;
}

// NOTE: Using long long to prevent sign is changed due to hex memory address beyonds long's scope
void itoa_s(long long i, unsigned base, char *buf)
{
	if (base > 16)
		return;
	if (i < 0)
	{
		*buf++ = '-';
		i *= -1;
	}
	itoa(i, base, buf);
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
