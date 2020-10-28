#include <sys/mman.h>
#include <unistd.h>

_syscall5(mmap, void *, size_t, uint32_t, uint32_t, int32_t);

void *mmap(void *addr, size_t len, int prot, int flags, int fildes, off_t off)
{
	// TODO: MQ 2020-10-28 off is not supported
	return (void *)syscall_mmap(addr, len, prot, flags, fildes);
}

_syscall2(munmap, void *, size_t);
int munmap(void *addr, size_t len)
{
	return syscall_munmap(addr, len);
}
