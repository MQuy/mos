/*
 * Copyright (c) 2013 Jonas 'Sortie' Termansen.
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
 * pwd/getpwuid.c
 * Searchs the passwd database for a user with the given numeric user id in a
 * thread-insecure manner.
 */

#include <errno.h>
#include <pwd.h>
#include <stdlib.h>

struct passwd* getpwuid(uid_t uid)
{
	static struct passwd result_object;
	static char* buf = NULL;
	static size_t buflen = 0;
	if (!buf)
	{
		size_t new_buflen = 64;
		if (!(buf = (char*)malloc(new_buflen)))
			return NULL;
		buflen = new_buflen;
	}
	struct passwd* result;
	int errnum;
retry:
	errnum = getpwuid_r(uid, &result_object, buf, buflen, &result);
	if (errnum == ERANGE)
	{
		size_t new_buflen = 2 * buflen;
		char* new_buf = (char*)realloc(buf, new_buflen);
		if (!new_buf)
			return NULL;
		buf = new_buf;
		buflen = new_buflen;
		goto retry;
	}
	if (errnum < 0)
		return errno = errnum, (struct passwd*)NULL;
	return result;
}
