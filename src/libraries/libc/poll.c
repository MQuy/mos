#include <errno.h>
#include <poll.h>
#include <unistd.h>

_syscall2(poll, struct pollfd *, uint32_t);
int poll(struct pollfd *fds, uint32_t nfds)
{
	SYSCALL_RETURN_ORIGINAL(syscall_poll(fds, nfds));
}
