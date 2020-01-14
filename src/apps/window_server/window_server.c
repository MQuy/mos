#include <stdint.h>
#include <stddef.h>
#include <include/ctype.h>
#include <include/fcntl.h>
#include <include/mman.h>
#include <libc/unistd.h>
#include <libc/stdlib.h>

#define SHARED_NAME "window_server"
#define SHARED_SIZE 32

int main()
{
  void *addr;
  int32_t fd;

  switch (fork())
  {
  case 0:
    fd = shm_open(SHARED_NAME, O_RDONLY, 0);
    addr = mmap(NULL, SHARED_SIZE, PROT_READ, MAP_SHARED, fd);
    break;

  default:
    fd = shm_open(SHARED_NAME, O_RDWR | O_CREAT, 0);
    ftruncate(fd, SHARED_SIZE);
    addr = mmap(NULL, SHARED_SIZE, PROT_WRITE, MAP_SHARED, fd);
    memcpy(addr, "hello world", 12);
    break;
  }

  return 0;
}