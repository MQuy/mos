/* Copyright (C) 1991-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Based on strlen implementation by Torbjorn Granlund (tege@sics.se),
   with help from Dan Sahlin (dan@sics.se) and
   commentary by Jim Blandy (jimb@ai.mit.edu);
   adaptation to memchr suggested by Dick Karpinski (dick@cca.ucsf.edu),
   and implemented by Roland McGrath (roland@ai.mit.edu).
   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.
   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>

void* memccpy(void* dest, const void* src, int c, size_t n)
{
	void* p = memchr(src, c, n);

	if (p != NULL)
		return mempcpy(dest, src, p - src + 1);

	memcpy(dest, src, n);
	return NULL;
}
