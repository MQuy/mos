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
  uint8_t dha[6];
  uint32_t dip;
  uint8_t nud_state;
  struct net_device *dev;
  struct list_head sibling;
};

struct rtable
{
  uint32_t dip;
  uint32_t netmask;
  uint32_t gateway;
  struct net_device *dev;
};

void route_init();

#endif