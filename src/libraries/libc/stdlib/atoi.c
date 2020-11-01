/*
 * Copyright (c) 2011, 2014 Jonas 'Sortie' Termansen.
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
 * stdlib/atoi.c
 * Converts integers represented as strings to binary representation.
 */

#include <errno.h>
#include <limits.h>
#include <stdlib.h>

double atof(const char* str)
{
	return strtod(str, NULL);
}

int atoi(const char* str)
{
	long long_result = strtol(str, (char**)NULL, 10);
	if (long_result < INT_MIN)
		return errno = ERANGE, INT_MIN;
	if (INT_MAX < long_result)
		return errno = ERANGE, INT_MAX;
	return (int)long_result;
}

long atol(const char* str)
{
	return strtol(str, (char**)NULL, 10);
}

long long atoll(const char* str)
{
	return strtoll(str, (char**)NULL, 10);
}
