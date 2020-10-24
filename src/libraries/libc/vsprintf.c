#include <ctype.h>
#include <dprint.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/cdefs.h>
#include <vsprintf.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)

#define do_div(n, base) ({                               \
	unsigned long __upper, __low, __high, __mod, __base; \
	__base = (base);                                     \
	asm(""                                               \
		: "=a"(__low), "=d"(__high)                      \
		: "A"(n));                                       \
	__upper = __high;                                    \
	if (__high)                                          \
	{                                                    \
		__upper = __high % (__base);                     \
		__high = __high / (__base);                      \
	}                                                    \
	asm("divl %2"                                        \
		: "=a"(__low), "=d"(__mod)                       \
		: "rm"(__base), "0"(__low), "1"(__upper));       \
	asm(""                                               \
		: "=A"(n)                                        \
		: "a"(__low), "d"(__high));                      \
	__mod;                                               \
})

size_t simple_strnlen(const char *s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{
	unsigned long result = 0, value;

	if (!base)
	{
		base = 10;
		if (*cp == '0')
		{
			base = 8;
			cp++;
			if ((toupper(*cp) == 'X') && isxdigit(cp[1]))
			{
				cp++;
				base = 16;
			}
		}
	}
	else if (base == 16)
	{
		if (cp[0] == '0' && toupper(cp[1]) == 'X')
			cp += 2;
	}
	while (isxdigit(*cp) &&
		   (value = isdigit(*cp) ? *cp - '0' : toupper(*cp) - 'A' + 10) < base)
	{
		result = result * base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long simple_strtol(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoul(cp + 1, endp, base);
	return simple_strtoul(cp, endp, base);
}

unsigned long long simple_strtoull(const char *cp, char **endp, unsigned int base)
{
	unsigned long long result = 0, value;

	if (!base)
	{
		base = 10;
		if (*cp == '0')
		{
			base = 8;
			cp++;
			if ((toupper(*cp) == 'X') && isxdigit(cp[1]))
			{
				cp++;
				base = 16;
			}
		}
	}
	else if (base == 16)
	{
		if (cp[0] == '0' && toupper(cp[1]) == 'X')
			cp += 2;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp - '0' : (islower(*cp) ? toupper(*cp) : *cp) - 'A' + 10) < base)
	{
		result = result * base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long long simple_strtoll(const char *cp, char **endp, unsigned int base)
{
	if (*cp == '-')
		return -simple_strtoull(cp + 1, endp, base);
	return simple_strtoull(cp, endp, base);
}

static int skip_atoi(const char **s)
{
	int i = 0;

	while (isdigit(**s))
		i = i * 10 + *((*s)++) - '0';
	return i;
}

#define ZEROPAD 1  /* pad with zero */
#define SIGN 2	   /* unsigned/signed long */
#define PLUS 4	   /* show plus */
#define SPACE 8	   /* space if plus */
#define LEFT 16	   /* left justified */
#define SPECIAL 32 /* 0x */
#define LARGE 64   /* use 'ABCDEF' instead of 'abcdef' */

static char *number(char *buf, char *end, unsigned long long num, int base, int size, int precision, int type)
{
	char c, sign, tmp[66];
	const char *digits;
	static const char small_digits[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	static const char large_digits[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	int i;

	digits = (type & LARGE) ? large_digits : small_digits;
	if (type & LEFT)
		type &= ~ZEROPAD;
	if (base < 2 || base > 36)
		return NULL;
	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;
	if (type & SIGN)
	{
		if ((signed long long)num < 0)
		{
			sign = '-';
			num = -(signed long long)num;
			size--;
		}
		else if (type & PLUS)
		{
			sign = '+';
			size--;
		}
		else if (type & SPACE)
		{
			sign = ' ';
			size--;
		}
	}
	if (type & SPECIAL)
	{
		if (base == 16)
			size -= 2;
		else if (base == 8)
			size--;
	}
	i = 0;
	if (num == 0)
		tmp[i++] = '0';
	else
		while (num != 0)
			tmp[i++] = digits[do_div(num, base)];
	if (i > precision)
		precision = i;
	size -= precision;
	if (!(type & (ZEROPAD + LEFT)))
	{
		while (size-- > 0)
		{
			if (buf <= end)
				*buf = ' ';
			++buf;
		}
	}
	if (sign)
	{
		if (buf <= end)
			*buf = sign;
		++buf;
	}
	if (type & SPECIAL)
	{
		if (base == 8)
		{
			if (buf <= end)
				*buf = '0';
			++buf;
		}
		else if (base == 16)
		{
			if (buf <= end)
				*buf = '0';
			++buf;
			if (buf <= end)
				*buf = digits[33];
			++buf;
		}
	}
	if (!(type & LEFT))
	{
		while (size-- > 0)
		{
			if (buf <= end)
				*buf = c;
			++buf;
		}
	}
	while (i < precision--)
	{
		if (buf <= end)
			*buf = '0';
		++buf;
	}
	while (i-- > 0)
	{
		if (buf <= end)
			*buf = tmp[i];
		++buf;
	}
	while (size-- > 0)
	{
		if (buf <= end)
			*buf = ' ';
		++buf;
	}
	return buf;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int len;
	unsigned long long num;
	int i, base;
	char *str, *end, c;
	const char *s;

	int flags; /* flags to number() */

	int field_width; /* width of output field */
	int precision;	 /* min. # of digits for integers; max
				   number of chars for from string */
	int qualifier;	 /* 'h', 'l', or 'L' for integer fields */
					 /* 'z' support added 23/7/1999 S.H.    */
					 /* 'z' changed to 'Z' --davidm 1/25/99 */

	/* Reject out-of-range values early */
	if (unlikely((int)size < 0))
	{
		/* There can be only one.. */
		return 0;
	}

	str = buf;
	end = buf + size - 1;

	if (end < buf - 1)
	{
		end = ((void *)-1);
		size = end - buf + 1;
	}

	for (; *fmt; ++fmt)
	{
		if (*fmt != '%')
		{
			if (str <= end)
				*str = *fmt;
			++str;
			continue;
		}

		/* process flags */
		flags = 0;
	repeat:
		++fmt; /* this also skips first '%' */
		switch (*fmt)
		{
		case '-':
			flags |= LEFT;
			goto repeat;
		case '+':
			flags |= PLUS;
			goto repeat;
		case ' ':
			flags |= SPACE;
			goto repeat;
		case '#':
			flags |= SPECIAL;
			goto repeat;
		case '0':
			flags |= ZEROPAD;
			goto repeat;
		}

		/* get field width */
		field_width = -1;
		if (isdigit(*fmt))
			field_width = skip_atoi(&fmt);
		else if (*fmt == '*')
		{
			++fmt;
			/* it's the next argument */
			field_width = va_arg(args, int);
			if (field_width < 0)
			{
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		/* get the precision */
		precision = -1;
		if (*fmt == '.')
		{
			++fmt;
			if (isdigit(*fmt))
				precision = skip_atoi(&fmt);
			else if (*fmt == '*')
			{
				++fmt;
				/* it's the next argument */
				precision = va_arg(args, int);
			}
			if (precision < 0)
				precision = 0;
		}

		/* get the conversion qualifier */
		qualifier = -1;
		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' ||
			*fmt == 'Z' || *fmt == 'z')
		{
			qualifier = *fmt;
			++fmt;
			if (qualifier == 'l' && *fmt == 'l')
			{
				qualifier = 'L';
				++fmt;
			}
		}

		/* default base */
		base = 10;

		switch (*fmt)
		{
		case 'c':
			if (!(flags & LEFT))
			{
				while (--field_width > 0)
				{
					if (str <= end)
						*str = ' ';
					++str;
				}
			}
			c = (unsigned char)va_arg(args, int);
			if (str <= end)
				*str = c;
			++str;
			while (--field_width > 0)
			{
				if (str <= end)
					*str = ' ';
				++str;
			}
			continue;

		case 's':
			s = va_arg(args, char *);
			if ((unsigned long)s < PAGE_SIZE)
				s = "<NULL>";

			len = simple_strnlen(s, precision);

			if (!(flags & LEFT))
			{
				while (len < field_width--)
				{
					if (str <= end)
						*str = ' ';
					++str;
				}
			}
			for (i = 0; i < len; ++i)
			{
				if (str <= end)
					*str = *s;
				++str;
				++s;
			}
			while (len < field_width--)
			{
				if (str <= end)
					*str = ' ';
				++str;
			}
			continue;

		case 'p':
			if (field_width == -1)
			{
				field_width = 2 * sizeof(void *);
				flags |= ZEROPAD;
			}
			str = number(str, end,
						 (unsigned long)va_arg(args, void *),
						 16, field_width, precision, flags);
			continue;

		case 'n':
			/* FIXME:
				* What does C99 say about the overflow case here? */
			if (qualifier == 'l')
			{
				long *ip = va_arg(args, long *);
				*ip = (str - buf);
			}
			else if (qualifier == 'Z' || qualifier == 'z')
			{
				size_t *ip = va_arg(args, size_t *);
				*ip = (str - buf);
			}
			else
			{
				int *ip = va_arg(args, int *);
				*ip = (str - buf);
			}
			continue;

		case '%':
			if (str <= end)
				*str = '%';
			++str;
			continue;

			/* integer number formats - set up the flags and "break" */
		case 'o':
			base = 8;
			break;

		case 'X':
			flags |= LARGE;
		case 'x':
			base = 16;
			break;

		case 'd':
		case 'i':
			flags |= SIGN;
		case 'u':
			break;

		default:
			if (str <= end)
				*str = '%';
			++str;
			if (*fmt)
			{
				if (str <= end)
					*str = *fmt;
				++str;
			}
			else
			{
				--fmt;
			}
			continue;
		}
		if (qualifier == 'L')
			num = va_arg(args, long long);
		else if (qualifier == 'l')
		{
			num = va_arg(args, unsigned long);
			if (flags & SIGN)
				num = (signed long)num;
		}
		else if (qualifier == 'Z' || qualifier == 'z')
		{
			num = va_arg(args, size_t);
		}
		else if (qualifier == 'h')
		{
			num = (unsigned short)va_arg(args, int);
			if (flags & SIGN)
				num = (signed short)num;
		}
		else
		{
			num = va_arg(args, unsigned int);
			if (flags & SIGN)
				num = (signed int)num;
		}
		str = number(str, end, num, base,
					 field_width, precision, flags);
	}
	if (str <= end)
		*str = '\0';
	else if (size > 0)
		/* don't write out a null byte if the buf size is zero */
		*end = '\0';
	/* the trailing null byte doesn't count towards the total
	* ++str;
	*/
	return str - buf;
}

int vscnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
	int i;

	i = vsnprintf(buf, size, fmt, args);
	return (i >= size) ? (size - 1) : i;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return i;
}

int scnprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, size, fmt, args);
	va_end(args);
	return (i >= size) ? (size - 1) : i;
}

int vsprintf(char *buf, const char *fmt, va_list args)
{
	return vsnprintf(buf, INT_MAX, fmt, args);
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsnprintf(buf, INT_MAX, fmt, args);
	va_end(args);
	return i;
}

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

static size_t parse_scanset(bool scanset[256], const char *spec)
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

int vcbscanf(void *fp,
			 int (*fgetc)(void *),
			 int (*ungetc)(int, void *),
			 const char *restrict format,
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
					*va_arg(ap, unsigned char *) = uintmaxval;
				else
					*va_arg(ap, signed char *) = intmaxval;
				break;
			case TYPE_SHORT:
				if (un)
					*va_arg(ap, unsigned short *) = uintmaxval;
				else
					*va_arg(ap, signed short *) = intmaxval;
				break;
			case TYPE_INT:
				if (un)
					*va_arg(ap, unsigned int *) = uintmaxval;
				else
					*va_arg(ap, signed int *) = intmaxval;
				break;
			case TYPE_LONG:
				if (un)
					*va_arg(ap, unsigned long *) = uintmaxval;
				else
					*va_arg(ap, signed long *) = intmaxval;
				break;
			case TYPE_LONGLONG:
				if (un)
					*va_arg(ap, unsigned long long *) = uintmaxval;
				else
					*va_arg(ap, signed long long *) = intmaxval;
				break;
			case TYPE_PTRDIFF:
				*va_arg(ap, ptrdiff_t *) = intmaxval;
				break;
			case TYPE_SIZE:
				if (un)
					*va_arg(ap, size_t *) = uintmaxval;
				else
					*va_arg(ap, ssize_t *) = intmaxval;
				break;
			case TYPE_MAX:
				if (un)
					*va_arg(ap, uintmax_t *) = uintmaxval;
				else
					*va_arg(ap, intmax_t *) = intmaxval;
				break;
			case TYPE_PTR:
				*va_arg(ap, void **) = (void *)(uintptr_t)uintmaxval;
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
				return errno = EINVAL, matched_items ? matched_items : EOF;
			}
			if (!field_width)
				field_width = string ? SIZE_MAX : 1;
			char **strptr = NULL;
			char *str = NULL;
			size_t strsize = 0;
			if (!discard)
			{
				if (allocate)
				{
					strptr = va_arg(ap, char **);
					strsize = 16;
					str = (char *)malloc(strsize);
					if (!str)
						return matched_items ? matched_items : EOF;
				}
				else
					str = va_arg(ap, char *);
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
						char *newstr = (char *)reallocarray(str, strsize, 2);
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
					char *newstr = realloc(str, i + string);
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
				*va_arg(ap, signed char *) = bytes_parsed;
				break;
			case TYPE_SHORT:
				*va_arg(ap, signed short *) = bytes_parsed;
				break;
			case TYPE_INT:
				*va_arg(ap, signed int *) = bytes_parsed;
				break;
			case TYPE_LONG:
				*va_arg(ap, signed long *) = bytes_parsed;
				break;
			case TYPE_LONGLONG:
				*va_arg(ap, signed long long *) = bytes_parsed;
				break;
			case TYPE_PTRDIFF:
				*va_arg(ap, ptrdiff_t *) = bytes_parsed;
				break;
			case TYPE_SIZE:
				*va_arg(ap, ssize_t *) = bytes_parsed;
				break;
			case TYPE_MAX:
				*va_arg(ap, intmax_t *) = bytes_parsed;
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
			return errno = EINVAL, matched_items ? matched_items : EOF;
		}
		else
			return errno = EINVAL, matched_items ? matched_items : EOF;
	}
	return matched_items;
}

struct vsscanf_input
{
	union
	{
		const char *str;
		const unsigned char *ustr;
	};
	size_t offset;
};

static int vsscanf_fgetc(void *fp)
{
	struct vsscanf_input *input = (struct vsscanf_input *)fp;
	if (!input->ustr[input->offset])
		return EOF;
	return (int)input->ustr[input->offset++];
}

static int vsscanf_ungetc(int c, void *fp)
{
	struct vsscanf_input *input = (struct vsscanf_input *)fp;
	if (c == EOF && !input->ustr[input->offset])
		return c;
	assert(input->offset);
	input->offset--;
	assert(input->ustr[input->offset] == (unsigned char)c);
	return c;
}

int vsscanf(const char *str, const char *format, va_list ap)
{
	struct vsscanf_input input;
	input.str = str;
	input.offset = 0;
	return vcbscanf(&input, vsscanf_fgetc, vsscanf_ungetc, format, ap);
}

int sscanf(const char *buf, const char *fmt, ...)
{
	va_list args;
	int i;

	va_start(args, fmt);
	i = vsscanf(buf, fmt, args);
	va_end(args);
	return i;
}
