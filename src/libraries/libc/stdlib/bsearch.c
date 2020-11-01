/*
 * Copyright (c) 2012, 2016 Jonas 'Sortie' Termansen.
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
 * stdlib/bsearch.c
 * Binary search.
 */

#include <stdint.h>
#include <stdlib.h>

/*
 * Semantics of binary search:
 *
 * bsearch(key, base, 0, size, compare) may return NULL.
 *
 * If 0 <= i < nmemb
 * and ptr = base + size * i
 * and compare(key, ptr) = 0 then
 * bsearch(key, base, nmemb, size, compare) may return ptr.
 *
 * If 0 <= i < nmemb
 * and ptr = base + size * i
 * and compare(key, ptr) < 0 then
 * bsearch(key, base, nmemb, size, compare) may return
 * bsearch(key, base, i, size, compare).
 *
 * If 0 <= i < nmemb
 * and ptr = base + size * i
 * and compare(key, ptr) > 0 then
 * bsearch(key, base, nmemb, size, compare) may return
 * bsearch(key, base + size * (i + 1), nemb - (i + 1), size, compare).
 */

void* bsearch(const void* key,
			  const void* base_ptr,
			  size_t nmemb,
			  size_t size,
			  int (*compare)(const void*, const void*))
{
	const unsigned char* base = (const unsigned char*)base_ptr;
	while (nmemb)
	{
		size_t i = nmemb / 2;
		const unsigned char* candidate = base + i * size;
		int rel = compare(key, candidate);
		if (rel < 0) /* key smaller than candidate */
			nmemb = i;
		else if (0 < rel) /* key greater than candidate */
		{
			base = candidate + size;
			nmemb -= i + 1;
		}
		else /* key equal to candidate */
			return (void*)candidate;
	}
	return NULL;
}
