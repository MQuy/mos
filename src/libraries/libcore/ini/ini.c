/* inih -- simple .INI file parser

SPDX-License-Identifier: BSD-3-Clause

Copyright (C) 2009-2020, Ben Hoyt

inih is released under the New BSD license (see LICENSE.txt). Go to the project
home page for more info:

https://github.com/benhoyt/inih

*/

#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <ctype.h>
#include <fcntl.h>
#include <libcore/ini/ini.h>
#include <stdbool.h>
#include <string.h>
#include <sys/cdefs.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#if !INI_USE_STACK
#include <stdlib.h>
#endif

#define MAX_SECTION 50
#define MAX_NAME 50

/* Used by ini_parse_string() to keep track of string parsing state. */
struct ini_parse_string_ctx
{
	const char *ptr;
	size_t num_left;
};

/* Strip whitespace chars off end of given string, in place. Return s. */
static char *rstrip(char *s)
{
	char *p = s + strlen(s);
	while (p > s && isspace((unsigned char)(*--p)))
		*p = '\0';
	return s;
}

/* Return pointer to first non-whitespace char in given string. */
static char *lskip(const char *s)
{
	while (*s && isspace((unsigned char)(*s)))
		s++;
	return (char *)s;
}

/* Return pointer to first char (of chars) or inline comment in given string,
   or pointer to null at end of string if neither found. Inline comment must
   be prefixed by a whitespace character to register as a comment. */
static char *find_chars_or_comment(const char *s, const char *chars)
{
#if INI_ALLOW_INLINE_COMMENTS
	int was_space = 0;
	while (*s && (!chars || !strchr(chars, *s)) &&
		   !(was_space && strchr(INI_INLINE_COMMENT_PREFIXES, *s)))
	{
		was_space = isspace((unsigned char)(*s));
		s++;
	}
#else
	while (*s && (!chars || !strchr(chars, *s)))
	{
		s++;
	}
#endif
	return (char *)s;
}

/* Version of strncpy that ensures dest (size bytes) is null-terminated. */
static char *strncpy0(char *dest, const char *src, size_t size)
{
	strncpy(dest, src, size - 1);
	dest[size - 1] = '\0';
	return dest;
}

static char *streol(char *buf, char *line)
{
	memset(line, 0, INI_MAX_LINE);
	for (uint8_t i = 0; i < INI_MAX_LINE; ++i)
	{
		uint8_t ch = *(buf++);
		if (ch == 0x0D)
			continue;
		if (ch == 0x0A)
			break;
		*(line++) = ch;
	}
	return buf;
}

/* See documentation in header file. */
int ini_parse_stream(char *buf, ini_handler handler,
					 void *user)
{
	/* Uses a fair bit of stack (use heap instead if you need to) */
#if INI_USE_STACK
	char line[INI_MAX_LINE];
	int max_line = INI_MAX_LINE;
#else
	char *line;
	size_t __unused max_line = INI_INITIAL_ALLOC;
#endif
#if INI_ALLOW_REALLOC && !INI_USE_STACK
	char *new_line;
	size_t offset;
#endif
	char section[MAX_SECTION] = "";
	char prev_name[MAX_NAME] = "";

	char *start;
	char *end;
	char *name;
	char *value;
	int lineno = 0;
	int error = 0;

#if !INI_USE_STACK
	line = (char *)calloc(INI_INITIAL_ALLOC, sizeof(char));
	if (!line)
	{
		return -2;
	}
#endif

#if INI_HANDLER_LINENO
#define HANDLER(u, s, n, v) handler(u, s, n, v, lineno)
#else
#define HANDLER(u, s, n, v) handler(u, s, n, v)
#endif

	char *ibuf = buf;
	/* Scan through stream line by line */
	while (true)
	{
		ibuf = streol(ibuf, line);
		if (!*line)
			break;

#if INI_ALLOW_REALLOC && !INI_USE_STACK
		offset = strlen(line);
		while (offset == max_line - 1 && line[offset - 1] != '\n')
		{
			max_line *= 2;
			if (max_line > INI_MAX_LINE)
				max_line = INI_MAX_LINE;
			new_line = realloc(line, max_line);
			if (!new_line)
			{
				free(line);
				return -2;
			}
			line = new_line;
			if (reader(line + offset, (int)(max_line - offset), stream) == NULL)
				break;
			if (max_line >= INI_MAX_LINE)
				break;
			offset += strlen(line + offset);
		}
#endif

		lineno++;

		start = line;
#if INI_ALLOW_BOM
		if (lineno == 1 && (unsigned char)start[0] == 0xEF &&
			(unsigned char)start[1] == 0xBB &&
			(unsigned char)start[2] == 0xBF)
		{
			start += 3;
		}
#endif
		start = lskip(rstrip(start));

		if (strchr(INI_START_COMMENT_PREFIXES, *start))
		{
			/* Start-of-line comment */
		}
#if INI_ALLOW_MULTILINE
		else if (*prev_name && *start && start > line)
		{
			/* Non-blank line with leading whitespace, treat as continuation
               of previous name's value (as per Python configparser). */
			if (!HANDLER(user, section, prev_name, start) && !error)
				error = lineno;
		}
#endif
		else if (*start == '[')
		{
			/* A "[section]" line */
			end = find_chars_or_comment(start + 1, "]");
			if (*end == ']')
			{
				*end = '\0';
				strncpy0(section, start + 1, sizeof(section));
				*prev_name = '\0';
#if INI_CALL_HANDLER_ON_NEW_SECTION
				if (!HANDLER(user, section, NULL, NULL) && !error)
					error = lineno;
#endif
			}
			else if (!error)
			{
				/* No ']' found on section line */
				error = lineno;
			}
		}
		else if (*start)
		{
			/* Not a comment, must be a name[=:]value pair */
			end = find_chars_or_comment(start, "=:");
			if (*end == '=' || *end == ':')
			{
				*end = '\0';
				name = rstrip(start);
				value = end + 1;
#if INI_ALLOW_INLINE_COMMENTS
				end = find_chars_or_comment(value, NULL);
				if (*end)
					*end = '\0';
#endif
				value = lskip(value);
				rstrip(value);

				/* Valid name[=:]value pair found, call handler */
				strncpy0(prev_name, name, sizeof(prev_name));
				if (!HANDLER(user, section, name, value) && !error)
					error = lineno;
			}
			else if (!error)
			{
				/* No '=' or ':' found on name[=:]value line */
#if INI_ALLOW_NO_VALUE
				*end = '\0';
				name = rstrip(start);
				if (!HANDLER(user, section, name, NULL) && !error)
					error = lineno;
#else
				error = lineno;
#endif
			}
		}

#if INI_STOP_ON_FIRST_ERROR
		if (error)
			break;
#endif
	}

#if !INI_USE_STACK
	free(line);
#endif

	return error;
}

/* See documentation in header file. */
int ini_parse_file(int32_t fd, ini_handler handler, void *user)
{
	struct stat *stat = calloc(1, sizeof(struct stat));
	fstat(fd, stat);
	char *buf = calloc(stat->st_size, sizeof(char));
	read(fd, buf, stat->st_size);
	return ini_parse_stream(buf, handler, user);
}

/* See documentation in header file. */
int ini_parse(const char *filename, ini_handler handler, void *user)
{
	int error;

	int32_t fd = open(filename, O_RDONLY);
	if (!fd)
		return -1;
	error = ini_parse_file(fd, handler, user);
	close(fd);
	return error;
}

/* An ini_reader function to read the next line from a string buffer. This
   is the fgets() equivalent used by ini_parse_string(). */
static char *ini_reader_string(char *str, int num, void *stream)
{
	struct ini_parse_string_ctx *ctx = (struct ini_parse_string_ctx *)stream;
	const char *ctx_ptr = ctx->ptr;
	size_t ctx_num_left = ctx->num_left;
	char *strp = str;
	char c;

	if (ctx_num_left == 0 || num < 2)
		return NULL;

	while (num > 1 && ctx_num_left != 0)
	{
		c = *ctx_ptr++;
		ctx_num_left--;
		*strp++ = c;
		if (c == '\n')
			break;
		num--;
	}

	*strp = '\0';
	ctx->ptr = ctx_ptr;
	ctx->num_left = ctx_num_left;
	return str;
}

/* See documentation in header file. */
int ini_parse_string(char *string, ini_handler handler, void *user)
{
	return ini_parse_stream(string, handler, user);
}
