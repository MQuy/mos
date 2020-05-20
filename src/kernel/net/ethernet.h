#ifndef NET_ETHERNET_H
#define NET_ETHERNET_H

#include <stdint.h>

struct sk_buff;

struct __attribute__((packed)) ethernet_packet
{
  uint8_t dmac[6];
  uint8_t smac[6];
  uint16_t type;
  uint8_t payload[];
};

void ethernet_sendmsg(struct sk_buff *skb);

#endif