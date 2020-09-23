#include <kernel/memory/vmm.h>
#include <kernel/utils/string.h>

char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	void *new = kcalloc(len, sizeof(char));

	if (new == NULL)
		return NULL;

	return (char *)memcpy(new, s, len);
}
