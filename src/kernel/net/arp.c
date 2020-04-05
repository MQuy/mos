#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/rtl8139.h>
#include <kernel/net/ethernet.h>
#include "arp.h"

#define ARP_ETHERNET 0x0001
#define ARP_IPV4 0x0800
#define ARP_REQUEST 0x0001
#define ARP_REPLY 0x0002

int32_t arp_send_packet(uint8_t *dmac, uint8_t *dip)
{
  struct arp_packet *ap = kmalloc(sizeof(struct arp_packet));
  ap->htype = htons(ARP_ETHERNET);
  ap->ptype = htons(ETH_P_IP);
  ap->hlen = 6;
  ap->plen = 4;
  ap->oper = htons(ARP_REQUEST);
  memcpy(ap->sha, get_mac_address(), sizeof(ap->sha));
  ap->spa[0] = 0;
  ap->spa[1] = 0;
  ap->spa[2] = 0;
  ap->spa[3] = 0;
  memcpy(ap->tha, dmac, sizeof(ap->tha));
  memcpy(ap->tpa, dip, sizeof(ap->tpa));
  ethernet_send_packet(dmac, ap, sizeof(struct arp_packet), ETH_P_ARP);
}