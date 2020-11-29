#include <assert.h>
#include <sys/vfs.h>

int statfs(const char *path, struct statfs *buf)
{
	assert_not_reached();
	__builtin_unreachable();
}

int fstatfs(int fd, struct statfs *buf)
{
	assert_not_reached();
	__builtin_unreachable();
}
