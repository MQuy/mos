#ifndef LIBC_UNISTD_H
#define LIBC_UNISTD_H

#include <stdint.h>

#define __NR_exit 1
#define __NR_fork 2
#define __NR_read 3
#define __NR_write 4
#define __NR_open 5
#define __NR_close 6
#define __NR_pipe 42

#define _syscall0(name)                       \
  static inline int32_t syscall_##name()      \
  {                                           \
    int32_t ret;                              \
    __asm__ __volatile__("int $0x7F"          \
                         : "=a"(ret)          \
                         : "0"(__NR_##name)); \
    return ret;                               \
  }
#define _syscall1(name, type1)                           \
  static inline int32_t syscall_##name(type1 arg1)       \
  {                                                      \
    int32_t ret;                                         \
    __asm__ __volatile__("int $0x7F"                     \
                         : "=a"(ret)                     \
                         : "0"(__NR_##name), "b"(arg1)); \
    return ret;                                          \
  }

#define _syscall2(name, type1, type2)                               \
  static inline int32_t syscall_##name(type1 arg1, type2 arg2)      \
  {                                                                 \
    int32_t ret;                                                    \
    __asm__ __volatile__("int $0x7F"                                \
                         : "=a"(ret)                                \
                         : "0"(__NR_##name), "b"(arg1), "c"(arg2)); \
    return ret;                                                     \
  }

#define _syscall3(name, type1, type2, type3)                                   \
  static inline int32_t syscall_##name(type1 arg1, type2 arg2, type3 arg3)     \
  {                                                                            \
    int32_t ret;                                                               \
    __asm__ __volatile__("int $0x7F"                                           \
                         : "=a"(ret)                                           \
                         : "0"(__NR_##name), "b"(arg1), "c"(arg2), "d"(arg3)); \
    return ret;                                                                \
  }

_syscall0(fork);
static inline int32_t fork()
{
  return syscall_fork();
}

_syscall1(exit, int32_t);
static inline void exit(int32_t code)
{
  syscall_exit(code);
}

_syscall3(read, uint32_t, char *, uint32_t);
static inline int32_t read(uint32_t fd, char *buf, uint32_t size)
{
  return syscall_read(fd, buf, size);
}

_syscall3(write, uint32_t, const char *, uint32_t);
static inline int32_t write(uint32_t fd, const char *buf, uint32_t size)
{
  return syscall_write(fd, buf, size);
}

_syscall3(open, const char *, int32_t, int32_t);
static inline int32_t open(const char *path, int32_t flag, int32_t mode)
{
  return syscall_open(path, flag, mode);
}

_syscall1(close, uint32_t);
static inline int32_t close(uint32_t fd)
{
  return syscall_close(fd);
}

_syscall1(pipe, int32_t *);
static inline int32_t pipe(int32_t *fildes)
{
  return syscall_pipe(fildes);
}

#endif