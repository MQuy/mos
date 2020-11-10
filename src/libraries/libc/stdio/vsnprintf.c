/*
 * Copyright (c) 2011, 2012, 2013, 2014, 2015 Jonas 'Sortie' Termansen.
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
 * stdio/vsnprintf.c
 * Prints a formatted string to the supplied buffer.
 */

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

struct vsnprintf
{
	char* str;
	size_t size;
	size_t written;
};

static size_t vsnprintf_callback(void* ctx, const char* string, size_t length)
{
	struct vsnprintf* state = (struct vsnprintf*)ctx;
	if (1 <= state->size && state->written < state->size)
	{
		size_t available = state->size - state->written;
		size_t possible = length < available ? length : available;
		memcpy(state->str + state->written, string, possible);
		state->written += possible;
	}
	return length;
}

int vsnprintf(char* restrict str,
			  size_t size,
			  const char* restrict format,
			  va_list list)
{
	struct vsnprintf state;
	state.str = str;
	state.size = size ? size - 1 : 0;
	state.written = 0;
	int result = vcbprintf(&state, vsnprintf_callback, format, list);
	if (1 <= size)
		state.str[state.written] = '\0';
	return result;
}
