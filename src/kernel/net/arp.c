#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/devices/rtl8139.h>
#include <kernel/net/ethernet.h>
#include "arp.h"

int32_t arp_send_packet(uint8_t *dmac, uint32_t dip, uint32_t sip, uint16_t op)
{
  struct arp_packet *ap = kmalloc(sizeof(struct arp_packet));
  ap->htype = htons(ARP_ETHERNET);
  ap->ptype = htons(ETH_P_IP);
  ap->hlen = 6;
  ap->plen = 4;
  ap->oper = htons(op);
  memcpy(ap->sha, get_mac_address(), sizeof(ap->sha));
  ap->spa = htonl(sip);
  memcpy(ap->tha, dmac, sizeof(ap->tha));
  ap->tpa = htonl(dip);
  ethernet_send_packet(dmac, ap, sizeof(struct arp_packet), ETH_P_ARP);
  return 0;
}