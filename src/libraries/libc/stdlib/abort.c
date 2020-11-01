#include <list.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

extern struct list_head lstream;

void abort()
{
	fflush(NULL);

	FILE *iter;
	list_for_each_entry(iter, &lstream, sibling)
	{
		fclose(iter);
	}

	sigset_t set = sigmask(SIGABRT);
	sigprocmask(SIG_UNBLOCK, &set, NULL);
	raise(SIGABRT);
	_exit(127);
}
