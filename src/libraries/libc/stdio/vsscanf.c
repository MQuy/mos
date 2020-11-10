/*
 * Copyright (c) 2012, 2014 Jonas 'Sortie' Termansen.
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
 * stdio/vsscanf.c
 * Input format conversion.
 */

#include <assert.h>
#include <stdio.h>

struct vsscanf_input
{
	union
	{
		const char* str;
		const unsigned char* ustr;
	};
	size_t offset;
};

static int vsscanf_fgetc(void* fp)
{
	struct vsscanf_input* input = (struct vsscanf_input*)fp;
	if (!input->ustr[input->offset])
		return EOF;
	return (int)input->ustr[input->offset++];
}

static int vsscanf_ungetc(int c, void* fp)
{
	struct vsscanf_input* input = (struct vsscanf_input*)fp;
	if (c == EOF && !input->ustr[input->offset])
		return c;
	assert(input->offset);
	input->offset--;
	assert(input->ustr[input->offset] == (unsigned char)c);
	return c;
}

int vsscanf(const char* str, const char* format, va_list ap)
{
	struct vsscanf_input input;
	input.str = str;
	input.offset = 0;
	return vcbscanf(&input, vsscanf_fgetc, vsscanf_ungetc, format, ap);
}
