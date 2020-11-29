#include <assert.h>
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds,
		   fd_set *exceptfds, struct timeval *timeout)
{
	assert_not_reached();
	__builtin_unreachable();
}
