#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/wait.h>
#include <stdint.h>

volatile int32_t usr_interrupt = 0;

void fatal_error_signal(int sig)
{
	usr_interrupt = 1;
	signal(sig, SIG_DFL);
	raise(sig);
}

int main(void)
{
	struct sigaction usr_action;
	sigset_t block_mask;

	/* Establish the signal handler. */
	sigfillset(&block_mask);
	usr_action.sa_handler = fatal_error_signal;
	usr_action.sa_mask = block_mask;
	usr_action.sa_flags = 0;
	sigaction(SIGINT, &usr_action, NULL);

	raise(SIGINT);

	while (!usr_interrupt)
		;

	return 0;
}
