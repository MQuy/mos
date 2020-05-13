#ifndef NET_UDP_H
#define NET_UDP_H

#include <stdint.h>

struct __attribute__((packed)) udp_packet
{
  uint16_t source_port;
  uint16_t dest_port;
  uint16_t length;
  uint16_t checksum;
  uint8_t payload[];
};

#endif