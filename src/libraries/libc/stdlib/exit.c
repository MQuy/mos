#include <stdlib.h>
#include <unistd.h>

extern struct list_head lstream;

void exit(int status)
{
	fflush(NULL);
	FILE *iter;
	list_for_each_entry(iter, &lstream, sibling)
	{
		fclose(iter);
	}

	_exit(status);
}
