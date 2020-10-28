#include <poll.h>
#include <unistd.h>

_syscall2(poll, struct pollfd *, uint32_t);
int poll(struct pollfd *fds, uint32_t nfds)
{
	return syscall_poll(fds, nfds);
}
