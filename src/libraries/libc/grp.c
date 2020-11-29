#include <assert.h>
#include <grp.h>
#include <stddef.h>

void endgrent()
{
	assert_not_reached();
}

struct group *getgrent()
{
	assert_not_reached();
	__builtin_unreachable();
}

struct group *getgrgid(gid_t gid)
{
	assert_not_reached();
	__builtin_unreachable();
}

int getgrgid_r(gid_t gid, struct group *grp, char *buffer,
			   size_t bufsize, struct group **result)
{
	assert_not_reached();
	__builtin_unreachable();
}

struct group *getgrnam(const char *name)
{
	assert_not_reached();
	__builtin_unreachable();
}

int getgrnam_r(const char *name, struct group *grp, char *buffer,
			   size_t bufsize, struct group **result)
{
	assert_not_reached();
	__builtin_unreachable();
}

void setgrent()
{
	assert_not_reached();
}
