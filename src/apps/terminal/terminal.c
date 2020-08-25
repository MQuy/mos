#include <libc/stdlib.h>
#include <libc/unistd.h>
#include <libc/wait.h>
#include <stdint.h>

int main()
{
	pid_t pid = fork();

	if (!pid)
	{
		exit(0);
	}
	else
	{
		struct infop infop;
		waitid(P_PID, pid, &infop, WEXITED);
	}

	return 0;
}
