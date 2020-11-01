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
 * inttypes/wcstoimax.c
 * Converts integers represented as strings to binary representation.
 */

#define STRTOL wcstoimax
#define STRTOL_CHAR wchar_t
#define STRTOL_UCHAR wint_t
#define STRTOL_L(x) L##x
#define STRTOL_ISSPACE iswspace
#define STRTOL_INT intmax_t
#define STRTOL_UNSIGNED_INT uintmax_t
#define STRTOL_INT_MIN INTMAX_MIN
#define STRTOL_INT_MAX INTMAX_MAX
#define STRTOL_INT_IS_UNSIGNED false

#include "../stdlib/strtol.c"
