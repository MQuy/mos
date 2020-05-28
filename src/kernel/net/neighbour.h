#ifndef NET_NEIGHBOUR_H
#define NET_NEIGHBOUR_H

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
  uint32_t ip;
  uint8_t nud_state;
  struct net_device *dev;
  struct list_head sibling;
};

void neighbour_init();
uint8_t *lookup_mac_addr_from_ip(uint32_t ip);
uint8_t *lookup_mac_addr_for_ethernet(struct net_device *dev, uint32_t ip);

#endif