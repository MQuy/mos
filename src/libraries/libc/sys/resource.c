#include <assert.h>
#include <sys/resource.h>

int getrlimit(int resource, struct rlimit *rlp)
{
	assert_not_reached();
	__builtin_unreachable();
}

int setrlimit(int resource, const struct rlimit *rlp)
{
	assert_not_reached();
	__builtin_unreachable();
}
