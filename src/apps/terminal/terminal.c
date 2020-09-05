#include <libc/stdlib.h>
#include <libc/string.h>
#include <libc/unistd.h>
#include <libc/wait.h>
#include <stdint.h>

int main(void)
{
	int fdm, fds, rc;
	char input[150] = {0};

	setsid();

	fdm = posix_openpt(O_RDWR);
	fds = open(ptsname(fdm), O_RDWR, 0);

	if (fork())
	{
		char msg[] = "hello world\027, from masterr\177\n";
		write(fdm, msg, sizeof(msg) - 1);	  // ptm write
		read(fdm, input, sizeof(input) - 1);  // pts echo back
		memset(input, 0, sizeof(input));
		rc = read(fdm, input, sizeof(input) - 1);  // pts write back
	}
	else
	{
		char msg[] = "let's end this conversation\n";
		write(fdm, msg, sizeof(msg) - 1);  // pts write
		rc = read(fds, input, sizeof(input) - 1);
	}

	return rc;
}
