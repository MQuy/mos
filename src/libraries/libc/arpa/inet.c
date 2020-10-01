#include "inet.h"

#include <stdio.h>
#include <string.h>

char *inet_ntop(uint32_t src, char *dst, uint16_t len)
{
	const char fmt[] = "%u.%u.%u.%u";
	char tmp[sizeof "255.255.255.255"];
	uint8_t *sp = (uint8_t *)&src;
	if (sprintf(tmp, fmt, sp[3], sp[2], sp[1], sp[0]) >= len)
		return NULL;

	return strcpy(dst, tmp);
}
