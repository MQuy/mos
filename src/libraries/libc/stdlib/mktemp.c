#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define TEMPLATE_SUFFIX "XXXXXX"

char *mktemp(char *template)
{
	int len_tmpt = strlen(template);
	if (len_tmpt < 6 || memcmp(template + len_tmpt - 6, TEMPLATE_SUFFIX, 6))
	{
		memset(template, 0, len_tmpt);
		return errno = EINVAL, template;
	}

	for (int i = len_tmpt - 6, len_charset = sizeof(FILENAME_CHARSET) - 1; i < len_tmpt; ++i)
	{
		template[i] = FILENAME_CHARSET[rand() % len_charset];
	}

	return template;
}
