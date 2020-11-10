/*
 * Copyright (c) 2012, 2014, 2016, 2020 Jonas 'Sortie' Termansen.
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
 * stdio/vcbscanf.c
 * Input format conversion.
 */

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum scan_type
{
	TYPE_SHORT,
	TYPE_SHORTSHORT,
	TYPE_INT,
	TYPE_LONG,
	TYPE_LONGLONG,
	TYPE_SIZE,
	TYPE_PTRDIFF,
	TYPE_MAX,
	TYPE_PTR,
};

static int debase(unsigned char c, int base)
{
	if (c == '0')
		return 0;
	int ret = -1;
	if ('0' <= c && c <= '9')
	{
		ret = c - '0' + 0;
	}
	if ('a' <= c && c <= 'f')
	{
		ret = c - 'a' + 10;
	}
	if ('A' <= c && c <= 'F')
	{
		ret = c - 'A' + 10;
	}
	if (base <= ret)
		return -1;
	return ret;
}

static size_t parse_scanset(bool scanset[256], const char* spec)
{
	bool negate = spec[0] == '^';
	size_t offset = negate ? 1 : 0;
	for (size_t i = 0; i < 256; i++)
		scanset[i] = negate;
	if (spec[offset] == ']')
	{
		offset++;
		scanset[(unsigned char)']'] = !negate;
	}
	for (; spec[offset] && spec[offset] != ']'; offset++)
	{
		unsigned char c = (unsigned char)spec[offset];
		// Only allow ASCII in the scanset besides negation.
		if (128 < c)
			return offset;
		if (spec[offset + 1] == '-' &&
			spec[offset + 2] &&
			spec[offset + 2] != ']')
		{
			unsigned char to = (unsigned char)spec[offset + 2];
			for (int i = c; i <= to; i++)
				scanset[i] = !negate;
			offset += 2;
		}
		else
			scanset[(unsigned char)spec[offset]] = !negate;
	}
	return offset;
}

int vcbscanf(void* fp,
			 int (*fgetc)(void*),
			 int (*ungetc)(int, void*),
			 const char* restrict format,
			 va_list ap)
{
	int matched_items = 0;
	uintmax_t bytes_parsed = 0;
	int ic = 0;
	while (*format)
	{
		if (isspace((unsigned char)*format))
		{
			do
				format++;
			while (isspace((unsigned char)*format));
			while (true)
			{
				ic = fgetc(fp);
				if (ic == EOF)
					break;
				bytes_parsed++;
				if (!isspace(ic))
				{
					ungetc(ic, fp);
					bytes_parsed--;
					break;
				}
			}
			continue;
		}
		else if (format[0] != '%' || format[1] == '%')
		{
			if (*format == '%')
				format++;
			unsigned char c = *format++;
			ic = fgetc(fp);
			if (ic == EOF)
				return matched_items ? matched_items : EOF;
			bytes_parsed++;
			if (ic != c)
			{
				ungetc(ic, fp);
				bytes_parsed--;
				break;
			}
			continue;
		}
		format++;
		bool discard = format[0] == '*';
		if (discard)
			format++;
		size_t field_width = 0;
		while ('0' <= *format && *format <= '9')
			field_width = field_width * 10 + *format++ - '0';
		bool allocate = false;
		if (*format == 'm')
		{
			allocate = true;
			format++;
		}
		enum scan_type scan_type = TYPE_INT;
		switch (*format++)
		{
		case 'h':
			scan_type = *format == 'h' ? (format++, TYPE_SHORTSHORT) : TYPE_SHORT;
			break;
		case 'j':
			scan_type = TYPE_MAX;
			break;
		case 'l':
			scan_type = *format == 'l' ? (format++, TYPE_LONGLONG) : TYPE_LONG;
			break;
		case 'L':
			scan_type = TYPE_LONGLONG;
			break;
		case 't':
			scan_type = TYPE_PTRDIFF;
			break;
		case 'z':
			scan_type = TYPE_SIZE;
			break;
		default:
			format--;
		}
		if (*format == 'd' || *format == 'i' || *format == 'o' ||
			*format == 'u' || *format == 'x' || *format == 'X' ||
			*format == 'p')
		{
			int base;
			bool is_unsigned;
			switch (*format++)
			{
			case 'd':
				base = 10;
				is_unsigned = false;
				break;
			case 'i':
				base = 0;
				is_unsigned = false;
				break;
			case 'o':
				base = 8;
				is_unsigned = true;
				break;
			case 'u':
				base = 10;
				is_unsigned = true;
				break;
			case 'p':
				if (scan_type != TYPE_INT)
					return errno = EINVAL, matched_items ? matched_items : EOF;
				scan_type = TYPE_PTR;
			case 'X':
			case 'x':
				base = 16;
				is_unsigned = true;
				break;
			default:
				__builtin_unreachable();
			}
			if (allocate)
				return errno = EINVAL, matched_items ? matched_items : EOF;
			bool parsed_int = false;
			uintmax_t int_value = 0;
			bool negative = false;
			bool has_prefix = false;
			bool maybe_base16 = false;
			if (!field_width)
				field_width = SIZE_MAX;
			size_t i = 0;
			for (; i < field_width; i++)
			{
				ic = fgetc(fp);
				if (ic == EOF)
					break;
				bytes_parsed++;
				if (i == 0 && isspace(ic))
				{
					i--;
					continue;
				}
				if (ic == '-' && i == 0 && !has_prefix)
				{
					negative = true;
					has_prefix = true;
				}
				else if (ic == '+' && i == 0 && !has_prefix)
				{
					negative = false;
					has_prefix = true;
				}
				else if ((ic == 'x' || ic == 'X') &&
						 (base == 0 || base == 16) &&
						 (has_prefix ? i == 2 : i == 1) &&
						 int_value == 0)
				{
					maybe_base16 = true;
					parsed_int = false;
				}
				else if (ic == '0' && (has_prefix ? i == 1 : i == 0))
				{
					int_value = 0;
					parsed_int = true;
				}
				else
				{
					if (base == 0)
					{
						if (maybe_base16)
						{
							if (debase(ic, 16) < 0)
							{
								ungetc(ic, fp);
								bytes_parsed--;
								break;
							}
							base = 16;
						}
						else if (parsed_int)
							base = 8;
						else
							base = 10;
					}
					int cval = debase(ic, base);
					if (cval < 0)
					{
						ungetc(ic, fp);
						bytes_parsed--;
						break;
					}
					if (__builtin_mul_overflow(int_value, base, &int_value))
						int_value = UINTMAX_MAX;
					if (__builtin_add_overflow(int_value, cval, &int_value))
						int_value = UINTMAX_MAX;
					parsed_int = true;
				}
			}
			if (!parsed_int)
				return matched_items || i || ic != EOF ? matched_items : EOF;
			if (discard)
				continue;
			uintmax_t uintmaxval = int_value;
			if (negative)
				uintmaxval = -uintmaxval;
			intmax_t intmaxval = uintmaxval;
			bool un = is_unsigned;
			switch (scan_type)
			{
			case TYPE_SHORTSHORT:
				if (un)
					*va_arg(ap, unsigned char*) = uintmaxval;
				else
					*va_arg(ap, signed char*) = intmaxval;
				break;
			case TYPE_SHORT:
				if (un)
					*va_arg(ap, unsigned short*) = uintmaxval;
				else
					*va_arg(ap, signed short*) = intmaxval;
				break;
			case TYPE_INT:
				if (un)
					*va_arg(ap, unsigned int*) = uintmaxval;
				else
					*va_arg(ap, signed int*) = intmaxval;
				break;
			case TYPE_LONG:
				if (un)
					*va_arg(ap, unsigned long*) = uintmaxval;
				else
					*va_arg(ap, signed long*) = intmaxval;
				break;
			case TYPE_LONGLONG:
				if (un)
					*va_arg(ap, unsigned long long*) = uintmaxval;
				else
					*va_arg(ap, signed long long*) = intmaxval;
				break;
			case TYPE_PTRDIFF:
				*va_arg(ap, ptrdiff_t*) = intmaxval;
				break;
			case TYPE_SIZE:
				if (un)
					*va_arg(ap, size_t*) = uintmaxval;
				else
					*va_arg(ap, ssize_t*) = intmaxval;
				break;
			case TYPE_MAX:
				if (un)
					*va_arg(ap, uintmax_t*) = uintmaxval;
				else
					*va_arg(ap, intmax_t*) = intmaxval;
				break;
			case TYPE_PTR:
				*va_arg(ap, void**) = (void*)(uintptr_t)uintmaxval;
				break;
			}
			matched_items++;
		}
		else if (*format == 's' || *format == '[' || *format == 'c' ||
				 *format == 'C' || *format == 'S')
		{
			bool scanset[256];
			bool string;
			bool use_scanset;
			switch (*format++)
			{
			case 'S':
				if (scan_type != TYPE_INT)
					return errno = EINVAL, matched_items ? matched_items : EOF;
				scan_type = TYPE_LONG;
			case 's':
				string = true;
				use_scanset = false;
				break;
			case '[':
				string = true;
				use_scanset = true;
				break;
			case 'C':
				if (scan_type != TYPE_INT)
					return errno = EINVAL, matched_items ? matched_items : EOF;
				scan_type = TYPE_LONG;
			case 'c':
				string = false;
				use_scanset = false;
				break;
			default:
				__builtin_unreachable();
			}
			if (use_scanset)
			{
				size_t offset = parse_scanset(scanset, format);
				if (format[offset] != ']')
					return errno = EINVAL, matched_items ? matched_items : EOF;
				format += offset + 1;
			}
			if (scan_type != TYPE_INT)
			{
				fprintf(stderr,
						"%s:%u: error: scanf does not support wide strings\n",
						__FILE__, __LINE__);
				return errno = EINVAL, matched_items ? matched_items : EOF;
			}
			if (!field_width)
				field_width = string ? SIZE_MAX : 1;
			char** strptr = NULL;
			char* str = NULL;
			size_t strsize = 0;
			if (!discard)
			{
				if (allocate)
				{
					strptr = va_arg(ap, char**);
					strsize = 16;
					str = (char*)malloc(strsize);
					if (!str)
						return matched_items ? matched_items : EOF;
				}
				else
					str = va_arg(ap, char*);
			}
			size_t i = 0;
			while (i < field_width)
			{
				ic = fgetc(fp);
				if (ic == EOF)
					break;
				bytes_parsed++;
				if (string && (use_scanset ? !scanset[ic] : isspace(ic)))
				{
					if (!use_scanset && !i)
						continue;
					ungetc(ic, fp);
					bytes_parsed--;
					break;
				}
				if (!discard)
				{
					if (allocate && i + string == strsize)
					{
						char* newstr = (char*)reallocarray(str, strsize, 2);
						if (!newstr)
						{
							free(str);
							return matched_items ? matched_items : EOF;
						}
						str = newstr;
						strsize *= 2;
					}
					str[i++] = (char)ic;
				}
				else
					i++;
			}
			if (string ? !i : i < field_width)
			{
				if (!discard && allocate)
					free(str);
				return matched_items || i || ic != EOF ? matched_items : EOF;
			}
			if (!discard)
			{
				if (string)
					str[i] = '\0';
				if (allocate)
				{
					char* newstr = realloc(str, i + string);
					str = newstr ? newstr : str;
					*strptr = str;
				}
				if (string || i == field_width)
					matched_items++;
			}
		}
		else if (*format == 'n')
		{
			format++;
			if (allocate)
				return errno = EINVAL, matched_items ? matched_items : EOF;
			switch (scan_type)
			{
			case TYPE_SHORTSHORT:
				*va_arg(ap, signed char*) = bytes_parsed;
				break;
			case TYPE_SHORT:
				*va_arg(ap, signed short*) = bytes_parsed;
				break;
			case TYPE_INT:
				*va_arg(ap, signed int*) = bytes_parsed;
				break;
			case TYPE_LONG:
				*va_arg(ap, signed long*) = bytes_parsed;
				break;
			case TYPE_LONGLONG:
				*va_arg(ap, signed long long*) = bytes_parsed;
				break;
			case TYPE_PTRDIFF:
				*va_arg(ap, ptrdiff_t*) = bytes_parsed;
				break;
			case TYPE_SIZE:
				*va_arg(ap, ssize_t*) = bytes_parsed;
				break;
			case TYPE_MAX:
				*va_arg(ap, intmax_t*) = bytes_parsed;
				break;
			case TYPE_PTR:
				__builtin_unreachable();
			}
		}
		else if (*format == 'a' || *format == 'A' ||
				 *format == 'e' || *format == 'E' ||
				 *format == 'f' || *format == 'F' ||
				 *format == 'g' || *format == 'G')
		{
			fprintf(stderr, "%s:%u: error: scanf does not support \"%%%c\")\n",
					__FILE__, __LINE__, *format);
			return errno = EINVAL, matched_items ? matched_items : EOF;
		}
		else
			return errno = EINVAL, matched_items ? matched_items : EOF;
	}
	return matched_items;
}
