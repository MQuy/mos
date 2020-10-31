/*
 * Copyright (c) 2013, 2014 Jonas 'Sortie' Termansen.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * time/strftime.c
 * Format time and date into a string.
 */

#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <time.h>
#include <wchar.h>

#ifndef STRFTIME
#define STRFTIME strftime
#define STRFTIME_CHAR char
#define STRFTIME_L(x) x
#endif

static const STRFTIME_CHAR* GetWeekdayAbbreviated(const struct tm* tm)
{
	switch (tm->tm_wday % 7)
	{
	case 0:
		return STRFTIME_L("Sun");
	case 1:
		return STRFTIME_L("Mon");
	case 2:
		return STRFTIME_L("Tue");
	case 3:
		return STRFTIME_L("Wed");
	case 4:
		return STRFTIME_L("Thu");
	case 5:
		return STRFTIME_L("Fri");
	case 6:
		return STRFTIME_L("Sat");
	default:
		__builtin_unreachable();
	}
}

static const STRFTIME_CHAR* GetWeekday(const struct tm* tm)
{
	switch (tm->tm_wday % 7)
	{
	case 0:
		return STRFTIME_L("Sunday");
	case 1:
		return STRFTIME_L("Monday");
	case 2:
		return STRFTIME_L("Tuesday");
	case 3:
		return STRFTIME_L("Wednesday");
	case 4:
		return STRFTIME_L("Thursday");
	case 5:
		return STRFTIME_L("Friday");
	case 6:
		return STRFTIME_L("Saturday");
	default:
		__builtin_unreachable();
	}
}

static const STRFTIME_CHAR* GetMonthAbbreviated(const struct tm* tm)
{
	switch (tm->tm_mon % 12)
	{
	case 0:
		return STRFTIME_L("Jan");
	case 1:
		return STRFTIME_L("Feb");
	case 2:
		return STRFTIME_L("Mar");
	case 3:
		return STRFTIME_L("Apr");
	case 4:
		return STRFTIME_L("May");
	case 5:
		return STRFTIME_L("Jun");
	case 6:
		return STRFTIME_L("Jul");
	case 7:
		return STRFTIME_L("Aug");
	case 8:
		return STRFTIME_L("Sep");
	case 9:
		return STRFTIME_L("Oct");
	case 10:
		return STRFTIME_L("Nov");
	case 11:
		return STRFTIME_L("Dec");
	default:
		__builtin_unreachable();
	}
}

static const STRFTIME_CHAR* GetMonth(const struct tm* tm)
{
	switch (tm->tm_mon % 12)
	{
	case 0:
		return STRFTIME_L("January");
	case 1:
		return STRFTIME_L("February");
	case 2:
		return STRFTIME_L("March");
	case 3:
		return STRFTIME_L("April");
	case 4:
		return STRFTIME_L("May");
	case 5:
		return STRFTIME_L("June");
	case 6:
		return STRFTIME_L("July");
	case 7:
		return STRFTIME_L("August");
	case 8:
		return STRFTIME_L("Sepember");
	case 9:
		return STRFTIME_L("October");
	case 10:
		return STRFTIME_L("November");
	case 11:
		return STRFTIME_L("December");
	default:
		__builtin_unreachable();
	}
}

static size_t strftime_convert_uintmax(STRFTIME_CHAR* destination, uintmax_t value)
{
	size_t result = 1;
	uintmax_t copy = value;
	while (10 <= copy)
		copy /= 10, result++;
	for (size_t i = result; i != 0; i--)
		destination[i - 1] = STRFTIME_L('0') + value % 10,
						value /= 10;
	destination[result] = STRFTIME_L('\0');
	return result;
}

static size_t strftime_convert_int(STRFTIME_CHAR* destination, int value)
{
	if (value < 0)
	{
		*destination++ = STRFTIME_L('-');
		return 1 + strftime_convert_uintmax(destination, -(uintmax_t)(intmax_t)value);
	}
	return strftime_convert_uintmax(destination, (uintmax_t)value);
}

size_t STRFTIME(STRFTIME_CHAR* s,
				size_t max,
				const STRFTIME_CHAR* format,
				const struct tm* tm)
{
	const STRFTIME_CHAR* orig_format = format;
	size_t ret = 0;

#define OUTPUT_CHAR(expr)             \
	do                                \
	{                                 \
		if (ret == max)               \
			return errno = ERANGE, 0; \
		s[ret++] = (expr);            \
	} while (0)

#define OUTPUT_STRING(expr)                    \
	do                                         \
	{                                          \
		const STRFTIME_CHAR* out_str = (expr); \
		while (*out_str)                       \
			OUTPUT_CHAR(*out_str++);           \
	} while (0)

#define OUTPUT_STRFTIME(expr)                                     \
	do                                                            \
	{                                                             \
		int old_errno = errno;                                    \
		errno = 0;                                                \
		size_t subret = STRFTIME(s + ret, max - ret, (expr), tm); \
		if (!subret && errno)                                     \
			return 0;                                             \
		errno = old_errno;                                        \
		ret += subret;                                            \
	} while (0)

#define OUTPUT_INT_PADDED(valexpr, widthexpr, padexpr) \
	do                                                 \
	{                                                  \
		int val = (valexpr);                           \
		size_t width = (widthexpr);                    \
		STRFTIME_CHAR pad = (padexpr);                 \
		STRFTIME_CHAR str[sizeof(int) * 3];            \
		str[0] = STRFTIME_L('\0');                     \
		size_t got = strftime_convert_int(str, val);   \
		while (pad && got < width--)                   \
			OUTPUT_CHAR(pad);                          \
		OUTPUT_STRING(str);                            \
	} while (0)

#define OUTPUT_INT(valexpr) OUTPUT_INT_PADDED(valexpr, 0, STRFTIME_L('\0'))

#if defined(STRFTIME_IS_WCHAR)
#define OUTPUT_UNSUPPORTED()                                                                      \
	do                                                                                            \
	{                                                                                             \
		fprintf(stderr, "%s:%u: %s: error: unsupported format string \"%ls\" around \"%%%ls\"\n", \
				__FILE__, __LINE__, __STRINGIFY(STRFTIME), orig_format, specifiers_begun_at);     \
		return 0;                                                                                 \
	} while (0)
#else
#define OUTPUT_UNSUPPORTED()                                                                    \
	do                                                                                          \
	{                                                                                           \
		fprintf(stderr, "%s:%u: %s: error: unsupported format string \"%s\" around \"%%%s\"\n", \
				__FILE__, __LINE__, __STRINGIFY(STRFTIME), orig_format, specifiers_begun_at);   \
		return 0;                                                                               \
	} while (0)
#endif

#define OUTPUT_UNDEFINED() OUTPUT_UNSUPPORTED()

	STRFTIME_CHAR c;
	while ((c = *format++))
	{
		if (c != STRFTIME_L('%'))
		{
			OUTPUT_CHAR(c);
			continue;
		}
		const STRFTIME_CHAR* specifiers_begun_at = format;
		c = *format++;

		// Process any optional flags.
		STRFTIME_CHAR padding_char = STRFTIME_L(' ');
		bool plus_padding = false;
		while (true)
		{
			switch (c)
			{
			case STRFTIME_L('0'):
				padding_char = STRFTIME_L('0');
				break;
			case STRFTIME_L('+'):
				padding_char = STRFTIME_L('0');
				plus_padding = true;
				break;
			default:
				goto end_of_flags;
			}
			c = *format++;
		}
	end_of_flags:

		// TODO: Support the '+' flag!
		if (plus_padding)
			OUTPUT_UNSUPPORTED();

		// Process the optional minimum field width.
		size_t field_width = 0;
		while (STRFTIME_L('0') <= c && c <= STRFTIME_L('9'))
			field_width = field_width * 10 + c - STRFTIME_L('0'),
			c = *format++;

		// Process an optional E or O modifier.
		bool e_modifier = false;
		bool o_modifier = false;
		if (c == STRFTIME_L('E'))
			e_modifier = true,
			c = *format++;
		else if (c == STRFTIME_L('O'))
			o_modifier = true,
			c = *format++;

		// TODO: Support the E and O modifiers where marked with the E and O
		//       comments in the below switch.
		if (e_modifier || o_modifier)
		{
		}

		switch (c)
		{
		case STRFTIME_L('a'):
			OUTPUT_STRING(GetWeekdayAbbreviated(tm));
			break;
		case STRFTIME_L('A'):
			OUTPUT_STRING(GetWeekday(tm));
			break;
		case STRFTIME_L('b'):
			OUTPUT_STRING(GetMonthAbbreviated(tm));
			break;
		case STRFTIME_L('B'):
			OUTPUT_STRING(GetMonth(tm));
			break;
		case STRFTIME_L('c'): /*E*/
			OUTPUT_STRING(GetWeekday(tm));
			OUTPUT_STRING(STRFTIME_L(" "));
			OUTPUT_STRING(GetMonthAbbreviated(tm));
			OUTPUT_STRING(STRFTIME_L(" "));
			OUTPUT_INT_PADDED(tm->tm_mday, 2, STRFTIME_L('0'));
			OUTPUT_STRING(STRFTIME_L(" "));
			OUTPUT_INT_PADDED(tm->tm_hour, 2, STRFTIME_L('0'));
			OUTPUT_STRING(STRFTIME_L(":"));
			OUTPUT_INT_PADDED(tm->tm_min, 2, STRFTIME_L('0'));
			OUTPUT_STRING(STRFTIME_L(":"));
			OUTPUT_INT_PADDED(tm->tm_sec, 2, STRFTIME_L('0'));
			break;
		case STRFTIME_L('C'): /*E*/
			if (!field_width)
				field_width = 2;
			OUTPUT_INT_PADDED((tm->tm_year + 1900) / 100, field_width, padding_char);
			break;
		case STRFTIME_L('d'):
			OUTPUT_INT_PADDED(tm->tm_mday, 2, STRFTIME_L('0'));
			break; /*O*/
		case STRFTIME_L('D'):
			OUTPUT_STRFTIME(STRFTIME_L("%m/%d/%y"));
			break;
		case STRFTIME_L('e'):
			OUTPUT_INT_PADDED(tm->tm_mday, 2, STRFTIME_L(' '));
			break; /*O*/
		case STRFTIME_L('F'):
			// TODO: Revisit this.
			OUTPUT_UNSUPPORTED();
			break;
		case STRFTIME_L('g'):
			// TODO: These require a bit of intelligence.
			OUTPUT_UNSUPPORTED();
			break;
		case STRFTIME_L('G'):
			// TODO: These require a bit of intelligence.
			OUTPUT_UNSUPPORTED();
			break;
		case STRFTIME_L('h'):
			OUTPUT_STRFTIME(STRFTIME_L("%b"));
			break;
		case STRFTIME_L('H'):
			OUTPUT_INT_PADDED(tm->tm_hour, 2, STRFTIME_L('0'));
			break; /*O*/
		case STRFTIME_L('I'):
			OUTPUT_INT_PADDED(tm->tm_hour % 12 + 1, 2, STRFTIME_L('0'));
			break; /*O*/
		case STRFTIME_L('j'):
			OUTPUT_INT_PADDED(tm->tm_yday + 1, 3, STRFTIME_L('0'));
			break;
		case STRFTIME_L('m'):
			OUTPUT_INT_PADDED(tm->tm_mon + 1, 2, STRFTIME_L('0'));
			break; /*O*/
		case STRFTIME_L('M'):
			OUTPUT_INT_PADDED(tm->tm_min, 2, STRFTIME_L('0'));
			break; /*O*/
		case STRFTIME_L('n'):
			OUTPUT_CHAR(STRFTIME_L('\n'));
			break;
		case STRFTIME_L('p'):
			OUTPUT_STRING(tm->tm_hour < 12 ? STRFTIME_L("AM") : STRFTIME_L("PM"));
			break;
		case STRFTIME_L('r'):
			OUTPUT_STRFTIME(STRFTIME_L("%I:%M:%S %p"));
			break;
		case STRFTIME_L('R'):
			OUTPUT_STRFTIME(STRFTIME_L("%H:%M"));
			break;
		case STRFTIME_L('S'):
			OUTPUT_INT_PADDED(tm->tm_sec, 2, STRFTIME_L('0'));
			break; /*O*/
		case STRFTIME_L('t'):
			OUTPUT_CHAR(STRFTIME_L('\t'));
			break;
		case STRFTIME_L('T'):
			OUTPUT_STRFTIME(STRFTIME_L("%H:%M:%S"));
			break;
		case STRFTIME_L('u'):
			OUTPUT_INT(tm->tm_yday);
			break;			  /*O*/
		case STRFTIME_L('U'): /*O*/
			// TODO: These require a bit of intelligence.
			OUTPUT_UNSUPPORTED();
			break;
		case STRFTIME_L('V'): /*O*/
			// TODO: These require a bit of intelligence.
			OUTPUT_UNSUPPORTED();
			break;
		case STRFTIME_L('w'):
			OUTPUT_INT(tm->tm_wday);
			break;			  /*O*/
		case STRFTIME_L('W'): /*O*/
			// TODO: These require a bit of intelligence.
			OUTPUT_UNSUPPORTED();
			break;
		case STRFTIME_L('x'):
			OUTPUT_STRFTIME(STRFTIME_L("%m/%d/%y"));
			break; /*E*/
		case STRFTIME_L('X'):
			OUTPUT_STRFTIME(STRFTIME_L("%H:%M:%S"));
			break; /*E*/
		case STRFTIME_L('y'):
			OUTPUT_INT_PADDED((tm->tm_year + 1900) % 100, 2, STRFTIME_L('0'));
			break; /*EO*/
		case STRFTIME_L('Y'):
			OUTPUT_INT(tm->tm_year + 1900);
			break; /*E*/
		case STRFTIME_L('z'):
			// TODO: struct tm doesn't have all this information available!
			break;
		case STRFTIME_L('Z'):
			// TODO: struct tm doesn't have all this information available!
			break;
		case STRFTIME_L('%'):
			OUTPUT_CHAR(STRFTIME_L('%'));
			break;
		default:
			OUTPUT_UNDEFINED();
			break;
		}
	}
	if (ret == max)
		return errno = ERANGE, 0;
	return s[ret] = STRFTIME_L('\0'), ret;
}
