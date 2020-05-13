#ifndef NET_ROUTE_H
#define NET_ROUTE_H

#include <stdint.h>
#include <include/list.h>

#define NUD_INCOMPLETE 0x01
#define NUD_REACHABLE 0x02
#define NUD_STALE 0x04
#define NUD_DELAY 0x08
#define NUD_PROBE 0x10
#define NUD_FAILED 0x20

struct neighbour
{
  uint8_t ha[6];
  uint8_t ip[4];
  uint8_t nud_state;
  struct net_device *dev;
  struct list_head sibling;
};

struct rtable
{
  uint8_t ip_dst[4];
  uint8_t netmask[4];
  uint8_t gateway[4];
  struct net_device *dev;
};

void route_init();

#endif