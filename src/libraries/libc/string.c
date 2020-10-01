#include "string.h"

#include <ctype.h>
#include <stdlib.h>

static char tbuf[32];
static char bchars[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

void itoa(long long i, unsigned base, char *buf)
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

int atoi(const char *s)
{
	int sign = 1;
	if (*s == '-')
	{
		sign = -1;
		s++;
	}
	else if (*s == '+')
		s++;

	int res = 0;
	while (*s && '0' <= *s && *s <= '9')
	{
		res = 10 * res + (*s - '0');
		s++;
	}
	return res * sign;
}

// Not libc standard
static char *skip_spaces(const char *str)
{
	while (isspace(*str))
		++str;
	return (char *)str;
}

char *strim(char *s)
{
	size_t size;
	char *end;

	size = strlen(s);
	if (!size)
		return s;

	end = s + size - 1;
	while (end >= s && isspace(*end))
		end--;
	*(end + 1) = '\0';

	return skip_spaces(s);
}

char *strrstr(char *string, char *find)
{
	size_t stringlen, findlen;
	char *cp;

	findlen = strlen(find);
	stringlen = strlen(string);
	if (findlen > stringlen)
		return NULL;

	for (cp = string + stringlen - findlen; cp >= string; cp--)
		if (strncmp(cp, find, findlen) == 0)
			return cp;

	return NULL;
}

char *strreplace(char *s, char old, char new)
{
	for (; *s; ++s)
		if (*s == old)
			*s = new;
	return s;
}

int32_t striof(const char *s1, const char *s2)
{
	const char *s = strpbrk(s1, s2);
	if (s)
		return s - s1;
	else
		return -1;
}

int32_t strliof(const char *s1, const char *s2)
{
	const char *s = strrstr(s1, s2);
	if (s)
		return s - s1;
	else
		return -1;
}

int32_t strlsplat(const char *s1, int32_t pos, char **sf, char **sl)
{
	if (pos < 0)
		return -1;

	size_t length = strlen(s1);
	if (pos)
	{
		*sf = calloc(pos + 1, sizeof(char));
		memcpy(*sf, s1, pos);
	}
	if (pos < (int32_t)length)
	{
		*sl = calloc(length - pos, sizeof(char));
		memcpy(*sl, s1 + pos + 1, length - 1 - pos);
	}
	return 0;
}
