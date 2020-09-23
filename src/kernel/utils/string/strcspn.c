#include <include/pointer.h>
#include <kernel/utils/string.h>

size_t strcspn(const char *str, const char *reject)
{
	if (reject[0] == '\0' ||
		reject[1] == '\0')
		return strchrnul(str, reject[0]) - str;

	/* Use multiple small memsets to enable inlining on most targets.  */
	unsigned char table[256];
	unsigned char *p = memset(table, 0, 64);
	memset(p + 64, 0, 64);
	memset(p + 128, 0, 64);
	memset(p + 192, 0, 64);

	unsigned char *s = (unsigned char *)reject;
	unsigned char tmp;
	do
		p[tmp = *s++] = 1;
	while (tmp);

	s = (unsigned char *)str;
	if (p[s[0]])
		return 0;
	if (p[s[1]])
		return 1;
	if (p[s[2]])
		return 2;
	if (p[s[3]])
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
	} while ((c0 | c1 | c2 | c3) == 0);

	size_t count = s - (unsigned char *)str;
	return (c0 | c1) != 0 ? count - c0 + 1 : count - c2 + 3;
}
