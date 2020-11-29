#include <assert.h>
#include <sys/utsname.h>

int uname(struct utsname *buf)
{
	assert_not_reached();
	__builtin_unreachable();
}
