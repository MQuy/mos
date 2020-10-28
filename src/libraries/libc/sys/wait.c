#include <sys/wait.h>
#include <unistd.h>

_syscall4(waitid, idtype_t, id_t, struct infop *, int);
int waitid(idtype_t idtype, id_t id, struct infop *infop, int options)
{
	return syscall_waitid(idtype, id, infop, options);
}
