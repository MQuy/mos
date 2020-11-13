/*
 * Copyright (c) 2012, 2014, 2015 Jonas 'Sortie' Termansen.
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
 * wchar/mbrtowc.c
 * Convert a multibyte sequence to a wide character.
 */

#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>

static size_t utf8_mbrtowc(wchar_t* restrict pwc,
						   const char* restrict s,
						   size_t n,
						   mbstate_t* restrict ps)
{
	size_t i;
	for (i = 0; !(i && ps->count == 0); i++)
	{
		// Handle the case where we were not able to fully decode a character,
		// but it is still possible to finish decoding given more bytes.
		if (n <= i)
			return (size_t)-2;

		char c = s[i];
		unsigned char uc = (unsigned char)c;

		// The initial state is that we expect a leading byte that informs us of
		// the length of this character sequence. The number of consecutive high
		// order bits tells us how many bytes make up this character (one
		// leading byte followed by zero or more continuation bytes).
		if (ps->count == 0)
		{
			if ((uc & 0b10000000) == 0b00000000) /* 0xxxxxxx */
			{
				ps->length = (ps->count = 0) + 1;
				ps->wch = (wchar_t)uc & 0b1111111;
			}
			else if ((uc & 0b11100000) == 0b11000000) /* 110xxxxx */
			{
				ps->length = (ps->count = 1) + 1;
				ps->wch = (wchar_t)uc & 0b11111;
			}
			else if ((uc & 0b11110000) == 0b11100000) /* 1110xxxx */
			{
				ps->length = (ps->count = 2) + 1;
				ps->wch = (wchar_t)uc & 0b1111;
			}
			else if ((uc & 0b11111000) == 0b11110000) /* 11110xxx */
			{
				ps->length = (ps->count = 3) + 1;
				ps->wch = (wchar_t)uc & 0b111;
			}
#if 0 /* 5-byte and 6-byte sequences are forbidden by RFC 3629 */
			else if ( (uc & 0b11111100) == 0b11111000 ) /* 111110xx */
			{
				ps->length = (ps->count = 4) + 1) + 1;
				ps->wch = (wchar_t) uc & 0b11;
			}
			else if ( (uc & 0b11111110) == 0b11111100 ) /* 1111110x */
			{
				ps->length = (ps->count = 5) + 1) + 1;
				ps->wch = (wchar_t) uc & 0b1;
			}
#endif
			else
				return errno = EILSEQ, (size_t)-1;
		}

		// The secondary state is that following a leading byte, we are
		// expecting a non-zero number of continuation byte bytes.
		else
		{
			// Verify this is a continuation byte.
			if ((uc & 0b11000000) != 0b10000000)
				return errno = EILSEQ, (size_t)-1;
			ps->wch = ps->wch << 6 | (uc & 0b00111111);
			ps->count--;
		}
	}

	// Reject the character if it was produced with an overly long sequence.
	if (ps->length == 1 && 1 << 7 <= ps->wch)
		return errno = EILSEQ, (size_t)-1;
	if (ps->length == 2 && 1 << (5 + 1 * 6) <= ps->wch)
		return errno = EILSEQ, (size_t)-1;
	if (ps->length == 3 && 1 << (4 + 2 * 6) <= ps->wch)
		return errno = EILSEQ, (size_t)-1;
	if (ps->length == 4 && 1 << (3 + 3 * 6) <= ps->wch)
		return errno = EILSEQ, (size_t)-1;
#if 0 /* 5-byte and 6-byte sequences are forbidden by RFC 3629 */
	if ( ps->length == 5 && 1 << (2 + 4 * 6) <= ps->wch )
		return errno = EILSEQ, (size_t) -1;
	if ( ps->length == 6 && 1 << (1 + 5 * 6) <= ps->wch )
		return errno = EILSEQ, (size_t) -1;
#endif

	// The definition of UTF-8 prohibits encoding character numbers between
	// U+D800 and U+DFFF, which are reserved for use with the UTF-16 encoding
	// form (as surrogate pairs) and do not directly represent characters.
	if (0xD800 <= ps->wch && ps->wch <= 0xDFFF)
		return errno = EILSEQ, (size_t)-1;
	// RFC 3629 limits UTF-8 to 0x0 through 0x10FFFF.
	if (0x10FFFF <= ps->wch)
		return errno = EILSEQ, (size_t)-1;

	wchar_t result = ps->wch;

	if (pwc)
		*pwc = result;

	ps->length = 0;
	ps->wch = 0;

	return result != L'\0' ? i : 0;
}

size_t mbrtowc(wchar_t* pwc,
			   const char* s,
			   size_t n,
			   mbstate_t* ps)
{
	static mbstate_t static_ps;
	if (!ps)
		ps = &static_ps;
	if (!s)
		s = "", n = 1;

	// TODO: Verify whether the current locale is UTF-8.
	return utf8_mbrtowc(pwc, s, n, ps);
}
