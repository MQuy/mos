#include <libc-pointer-arith.h>
#include <string.h>
#include <sys/cdefs.h>

size_t strspn(const char *str, const char *accept)
{
	if (accept[0] == '\0')
		return 0;
	if (unlikely(accept[1] == '\0'))
	{
		const char *a = str;
		for (; *str == *accept; str++)
			;
		return str - a;
	}

	/* Use multiple small memsets to enable inlining on most targets.  */
	unsigned char table[256];
	unsigned char *p = memset(table, 0, 64);
	memset(p + 64, 0, 64);
	memset(p + 128, 0, 64);
	memset(p + 192, 0, 64);

	unsigned char *s = (unsigned char *)accept;
	/* Different from strcspn it does not add the NULL on the table
     so can avoid check if str[i] is NULL, since table['\0'] will
     be 0 and thus stopping the loop check.  */
	do
		p[*s++] = 1;
	while (*s);

	s = (unsigned char *)str;
	if (!p[s[0]])
		return 0;
	if (!p[s[1]])
		return 1;
	if (!p[s[2]])
		return 2;
	if (!p[s[3]])
		return 3;

	s = (unsigned char *)PTR_ALIGN_DOWN(s, 4);

	unsigned int c0, c1, c2, c3;
	do
	{
		s += 4;
		c0 = p[s[0]];
		c1 = p[s[1]];
		c2 = p[s[2]];
		c3 = p[s[3]];
	} while ((c0 & c1 & c2 & c3) != 0);

	size_t count = s - (unsigned char *)str;
	return (c0 & c1) == 0 ? count + c0 : count + c2 + 2;
}
