#include <sys/stat.h>
#include <unistd.h>

int fstat(int fildes, struct stat *buf)
{
	return _fstat(fildes, buf);
}
