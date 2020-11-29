#include <assert.h>
#include <mntent.h>

FILE *setmntent(const char *filename, const char *type)
{
	assert_not_reached();
	__builtin_unreachable();
}

struct mntent *getmntent(FILE *stream)
{
	assert_not_reached();
	__builtin_unreachable();
}

int addmntent(FILE *stream, const struct mntent *mnt)
{
	assert_not_reached();
	__builtin_unreachable();
}

int endmntent(FILE *streamp)
{
	assert_not_reached();
	__builtin_unreachable();
}

char *hasmntopt(const struct mntent *mnt, const char *opt)
{
	assert_not_reached();
	__builtin_unreachable();
}
