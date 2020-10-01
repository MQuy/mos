#ifndef LIBC_ARPA_INET_H
#define LIBC_ARPA_INET_H

#include <stdint.h>

char *inet_ntop(uint32_t src, char *dst, uint16_t len);

#endif
