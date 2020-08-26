#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/wait.h>
#include <stdint.h>

int main()
{
	pid_t pid = fork();

	if (!pid)
	{
		while (true)
			;
	}
	else
	{
		sleep(1);
		kill(pid, SIGKILL);
		struct infop infop;
		waitid(P_PID, pid, &infop, WEXITED);
	}

	return 0;
}
