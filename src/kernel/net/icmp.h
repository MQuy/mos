#ifndef NET_ICMP_H
#define NET_ICMP_H

#include <stdint.h>

#define ICMP_REPLY 0
#define ICMP_ECHO 0

struct __attribute__((packed)) icmp_packet
{
  uint8_t type;
  uint8_t code;
  uint16_t checksum;
  uint32_t rest_of_header;
  uint8_t payload[];
};

#endif