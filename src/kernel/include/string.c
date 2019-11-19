#include <kernel/memory/malloc.h>
#include "string.h"

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

//! compare two strings
int strcmp(const char *str1, const char *str2)
{

	int res = 0;
	while (!(res = *(unsigned char *)str1 - *(unsigned char *)str2) && *str2)
		++str1, ++str2;

	if (res < 0)
		res = -1;
	if (res > 0)
		res = 1;

	return res;
}

//! copies string s2 to s1
char *strcpy(char *s1, const char *s2)
{
	char *s1_p = s1;
	while (*s1++ = *s2++)
		;
	return s1_p;
}

//! returns length of string
size_t strlen(const char *str)
{
	const char *s;
	for (s = str; *s; ++s)
		;
	return (s - str);
}

char *strdup(const char *src)
{
	char *dst = malloc(strlen(src) + 1); // Space for length plus nul
	if (dst == NULL)
		return NULL;		// No memory
	strcpy(dst, src); // Copy the characters
	return dst;				// Return the new string
}

//! copies count bytes from src to dest
void *memcpy(void *dest, const void *src, size_t len)
{
	char *d = dest;
	const char *s = src;
	while (len--)
		*d++ = *s++;
	return dest;
}

//! sets count bytes of dest to val
void *memset(void *dest, char val, size_t len)
{
	unsigned char *ptr = dest;
	while (len-- > 0)
		*ptr++ = val;
	return dest;
}

//! sets count bytes of dest to val
unsigned short *memsetw(unsigned short *dest, unsigned short val, size_t count)
{
	unsigned short *temp = (unsigned short *)dest;
	for (; count != 0; count--)
		*temp++ = val;
	return dest;
}

//! locates first occurance of character in string
char *strchr(char *str, int character)
{
	do
	{
		if (*str == character)
			return (char *)str;
	} while (*str++);

	return 0;
}

//============================================================================
//    INTERFACE CLASS BODIES
//============================================================================
//****************************************************************************
//**
//**    END[String.cpp]
//**
//****************************************************************************
