#include "string.h"

#include <include/ctype.h>
#include <libc/stdlib.h>

int memcmp(const void *vl, const void *vr, size_t n)
{
	const unsigned char *l = vl;
	const unsigned char *r = vr;
	for (; n && *l == *r; n--, l++, r++)
		;
	return n ? *l - *r : 0;
}

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

int strcmp(const char *cs, const char *ct)
{
	unsigned char c1, c2;

	while (1)
	{
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

int strncmp(const char *cs, const char *ct, size_t count)
{
	unsigned char c1, c2;

	while (count)
	{
		c1 = *cs++;
		c2 = *ct++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}
	return 0;
}

char *strcpy(char *dest, const char *src)
{
	char *out = dest;
	for (; (*dest = *src); src++, dest++)
		;
	return out;
}

char *strncpy(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	while (count)
	{
		if ((*tmp = *src) != 0)
			src++;
		tmp++;
		count--;
	}
	return dest;
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
	char *dst = calloc(strlen(src) + 1, sizeof(char));	// Space for length plus nul
	if (dst == NULL)
		return NULL;   // No memory
	strcpy(dst, src);  // Copy the characters
	return dst;		   // Return the new string
}

char *strchr(const char *s, int c)
{
	for (; *s != (char)c; ++s)
		if (*s == '\0')
			return NULL;
	return (char *)s;
}

char *strrchr(const char *s, int c)
{
	const char *last = NULL;
	do
	{
		if (*s == (char)c)
			last = s;
	} while (*s++);
	return (char *)last;
}

int strcasecmp(const char *s1, const char *s2)
{
	int c1, c2;

	do
	{
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while (c1 == c2 && c1 != 0);
	return c1 - c2;
}

int strncasecmp(const char *s1, const char *s2, int n)
{
	int c1, c2;

	do
	{
		c1 = tolower(*s1++);
		c2 = tolower(*s2++);
	} while ((--n > 0) && c1 == c2 && c1 != 0);
	return c1 - c2;
}

char *strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest)
		dest++;
	while ((*dest++ = *src++) != '\0')
		;
	return tmp;
}

char *strncat(char *dest, const char *src, size_t count)
{
	char *tmp = dest;

	if (count)
	{
		while (*dest)
			dest++;
		while ((*dest++ = *src++) != 0)
		{
			if (--count == 0)
			{
				*dest = '\0';
				break;
			}
		}
	}
	return tmp;
}

char *skip_spaces(const char *str)
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

char *strpbrk(const char *cs, const char *ct)
{
	const char *sc1, *sc2;

	for (sc1 = cs; *sc1 != '\0'; ++sc1)
	{
		for (sc2 = ct; *sc2 != '\0'; ++sc2)
		{
			if (*sc1 == *sc2)
				return (char *)sc1;
		}
	}
	return NULL;
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

char *strsep(char **s, const char *ct)
{
	char *sbegin = *s;
	char *end;

	if (sbegin == NULL)
		return NULL;

	end = strpbrk(sbegin, ct);
	if (end)
		*end++ = '\0';
	*s = end;
	return sbegin;
}

char *strreplace(char *s, char old, char new)
{
	for (; *s; ++s)
		if (*s == old)
			*s = new;
	return s;
}

// Not libc standard
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

int32_t strlsplat(const char *s1, uint32_t pos, char **sf, char **sl)
{
	uint32_t length = strlen(s1);
	*sf = calloc(pos, sizeof(char));
	memcpy(*sf, s1, pos);
	*sl = calloc(length - 1 - pos, sizeof(char));
	memcpy(*sl, s1 + pos + 1, length - 1 - pos);
	return 0;
}
