#ifndef _LIBC_TERMIO_H
#define _LIBC_TERMIO_H

#include <sys/ioctl.h>
#include <termios.h>

int tcgetattr(int fd, struct termios *term);

#endif
