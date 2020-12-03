#include <errno.h>
#include <limits.h>
#include <sys/mman.h>
#include <unistd.h>

_syscall1(mmap, struct mmap_args *);
void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	struct mmap_args args = {
		.addr = addr,
		.len = len,
		.prot = prot,
		.flags = flags,
		.fildes = fildes,
		.off = off};
	SYSCALL_RETURN_POINTER(syscall_mmap(&args));
}

_syscall2(munmap, void *, size_t);
int munmap(void *addr, size_t len)
{
	SYSCALL_RETURN(syscall_munmap(addr, len));
}
