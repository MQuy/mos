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
 * stdlib/mblen.c
 * Determine number of bytes in next multibyte character.
 */

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

// TODO: This function is unpure and should be removed.
int mblen(const char* s, size_t n)
{
	wchar_t wc;
	static mbstate_t ps;
	size_t result = mbrtowc(&wc, s, n, &ps);
	if (!s)
		memset(&ps, 0, sizeof(ps));
	if (result == (size_t)-1)
		return memset(&ps, 0, sizeof(ps)), -1;
	// TODO: Should ps be cleared to zero in this case?
	if (result == (size_t)-2)
		return -1;
	return (int)result;
}
