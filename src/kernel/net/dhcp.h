#ifndef NET_DHCP_H
#define NET_DHCP_h

#include <stdint.h>

#define DHCP_REQUEST 1
#define DHCP_REPLY 2
#define DHCP_ETHERNET 1
#define DHCP_MAGIC 0x63825363

struct __attribute__((packed)) dhcp_packet
{
  uint8_t op;
  uint8_t htype;
  uint8_t hlen;
  uint8_t hops;
  uint32_t xip;
  uint16_t secs;
  uint16_t flags;
  uint32_t ciaddr;
  uint32_t yiaddr;
  uint32_t siaddr;
  uint32_t giaddr;
  uint8_t chaddr[16];
  uint8_t sname[64];
  uint8_t file[128];
  uint32_t magic;
  uint8_t options[];
};

#endif