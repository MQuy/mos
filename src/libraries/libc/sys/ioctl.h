#ifndef _LIBC_IOCTL_H
#define _LIBC_IOCTL_H

#include <termios.h>

int ioctl(int fd, unsigned int cmd, unsigned long arg);

#endif
