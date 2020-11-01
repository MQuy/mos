#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern struct list_head lstream;

struct cleanup_handler
{
	void (*func)(int status, void *arg);
	void *arg;
	struct list_head sibling;
};

LIST_HEAD(lhandler);

int on_exit(void (*func)(int, void *), void *arg)
{
	struct cleanup_handler *cp = calloc(1, sizeof(struct cleanup_handler));
	cp->func = func;
	cp->arg = NULL;
	list_add_tail(&cp->sibling, &lhandler);
	return 0;
}

int atexit(void (*func)())
{
	return on_exit(func, NULL);
}

void _clear_on_exit()
{
	struct cleanup_handler *iter, *next;
	list_for_each_entry_safe(iter, next, &lhandler, sibling)
	{
		list_del(&iter->sibling);
		free(iter);
	}
}

void _Exit(int status)
{
	exit(status);
}

bool in_exiting_phrase = false;

void exit(int status)
{
	// exit is called in
	if (in_exiting_phrase)
		_exit(status);

	in_exiting_phrase = true;

	struct cleanup_handler *iter, *next;
	list_for_each_entry_safe(iter, next, &lhandler, sibling)
	{
		iter->func(status, iter->arg);
	}

	// TODO: MQ 2020-11-01 Remove files created by tmpfile and close fd

	fflush(NULL);

	FILE *iter_file;
	list_for_each_entry(iter_file, &lstream, sibling)
	{
		fclose(iter_file);
	}

	_exit(status);
}
