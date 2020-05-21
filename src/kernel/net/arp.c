#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include "arp.h"

struct arp_packet *arp_create_packet(uint8_t *dmac, uint32_t dip, uint8_t smac, uint32_t sip, uint16_t op)
{
  struct arp_packet *ap = kmalloc(sizeof(struct arp_packet));
  ap->htype = htons(ARP_ETHERNET);
  ap->ptype = htons(ETH_P_IP);
  ap->hlen = 6;
  ap->plen = 4;
  ap->oper = htons(op);
  memcpy(ap->sha, smac, sizeof(ap->sha));
  ap->spa = htonl(sip);
  memcpy(ap->tha, dmac, sizeof(ap->tha));
  ap->tpa = htonl(dip);

  return ap;
}