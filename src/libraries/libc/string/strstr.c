/*
 * Copyright (c) 2011, 2012 Jonas 'Sortie' Termansen.
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
 * string/strstr.c
 * Locate a substring.
 */

#include <stdbool.h>
#include <string.h>

// TODO: This simple and hacky implementation runs in O(N^2) even though this
// problem can be solved in O(N).
char* strstr(const char* haystack, const char* needle)
{
	if (!needle[0])
		return (char*)haystack;
	for (size_t i = 0; haystack[i]; i++)
	{
		bool diff = false;
		for (size_t j = 0; needle[j]; j++)
		{
			if (haystack[i + j] != needle[j])
			{
				diff = true;
				break;
			}
		}
		if (diff)
			continue;
		return (char*)haystack + i;
	}
	return NULL;
}
