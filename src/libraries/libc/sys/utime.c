#include <assert.h>
#include <sys/utime.h>

int utime(const char *path, const struct utimbuf *times)
{
	assert_not_reached();
	__builtin_unreachable();
}
