#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/wait.h>
#include <stdint.h>

volatile int32_t usr_interrupt = 0;

void synch_signal(int sig)
{
	usr_interrupt = 1;
}

/* The child process executes this function. */
void child_function(void)
{
	/* Let parent know you’re done. */
	kill(getppid(), SIGUSR1);
	exit(0);
}

int main(void)
{
	struct sigaction usr_action;
	sigset_t block_mask;
	pid_t child_id;

	/* Establish the signal handler. */
	sigfillset(&block_mask);
	usr_action.sa_handler = synch_signal;
	usr_action.sa_mask = block_mask;
	usr_action.sa_flags = 0;
	sigaction(SIGUSR1, &usr_action, NULL);

	/* Create the child process. */
	child_id = fork();
	if (child_id == 0)
		child_function(); /* Does not return. */

	/* Busy wait for the child to send a signal. */
	while (!usr_interrupt)
		;

	return 0;
}
