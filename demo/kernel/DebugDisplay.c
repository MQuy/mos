//****************************************************************************
//**
//**    [FILE NAME]
//**    - [FILE DESCRIPTION]
//**
//****************************************************************************
//============================================================================
//    IMPLEMENTATION HEADERS
//============================================================================

#include <stdarg.h>
#include "DebugDisplay.h"

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

// Note: Some systems may require 0xB0000 Instead of 0xB8000
// We dont care for portability here, so pick either one
#define VID_MEMORY 0xB8000

// these vectors together act as a corner of a bounding rect
// This allows GotoXY() to reposition all the text that follows it
static unsigned int _xPos = 0, _yPos = 0;
static unsigned _startX = 0, _startY = 0;

// current color
static unsigned _color = 0;

//============================================================================
//    INTERFACE DATA
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTION PROTOTYPES
//============================================================================
//============================================================================
//    IMPLEMENTATION PRIVATE FUNCTIONS
//============================================================================

void DebugPutc(unsigned char c)
{

	if (c == 0)
		return;

	if (c == '\n' || c == '\r')
	{ /* start new line */
		_yPos += 2;
		_xPos = _startX;
		return;
	}

	if (_xPos > 79)
	{ /* start new line */
		_yPos += 2;
		_xPos = _startX;
		return;
	}

	/* draw the character */
	unsigned char *p = (unsigned char *)VID_MEMORY + (_xPos++) * 2 + _yPos * 80;
	*p++ = c;
	*p = _color;
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

void itoa_s(int i, unsigned base, char *buf)
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

unsigned DebugSetColor(const unsigned c)
{

	unsigned t = _color;
	_color = c;
	return t;
}

void DebugGotoXY(unsigned x, unsigned y)
{

	// reposition starting vectors for next text to follow
	// multiply by 2 do to the video modes 2byte per character layout
	_xPos = x * 2;
	_yPos = y * 2;
	_startX = _xPos;
	_startY = _yPos;
}

void DebugClrScr(const unsigned short c)
{

	unsigned char *p = (unsigned char *)VID_MEMORY;

	for (int i = 0; i < 160 * 30; i += 2)
	{

		p[i] = ' '; /* Need to watch out for MSVC++ optomization memset() call */
		p[i + 1] = c;
	}

	// go to start of previous set vector
	_xPos = _startX;
	_yPos = _startY;
}

void DebugPuts(char *str)
{

	if (!str)
		return;

	for (int i = 0; str[i] != 0; i++)
		DebugPutc(str[i]);
}

int DebugPrintf(const char *str, ...)
{

	if (!str)
		return 0;

	va_list args;
	va_start(args, str);

	for (int i = 0; str[i] != 0; i++)
	{

		switch (str[i])
		{

		case '%':

			switch (str[i + 1])
			{

			/*** characters ***/
			case 'c':
			{
				char c = va_arg(args, char);
				DebugPutc(c);
				i++; // go to next character
				break;
			}

			/*** address of ***/
			case 's':
			{
				char *c = va_arg(args, char *);
				char str[32] = {0};
				itoa_s(c, 16, str);
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
				int c = va_arg(args, int);
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
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[DebugDisplay.cpp]
//**
//****************************************************************************
