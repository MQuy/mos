#ifndef NET_ETHERNET_H
#define NET_ETHERNET_H

#include <stdint.h>

struct __attribute__((packed)) ethernet_packet
{
  uint8_t dmac[6];
  uint8_t smac[6];
  uint16_t type;
  uint8_t payload[];
};

int32_t ethernet_send_packet(uint8_t *dmac, void *payload, uint32_t size, uint16_t protocal);

#endif