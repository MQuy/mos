#include <include/errno.h>
#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include "arp.h"

int32_t arp_validate_packet(struct arp_packet *ap)
{
  if (ap->htype != htons(ARP_ETHERNET) || ap->ptype != htons(ETH_P_IP) || ap->hlen != 6 || ap->plen != 4)
    return -EPROTO;

  return 0;
}

struct arp_packet *arp_create_packet(uint8_t *source_mac, uint32_t source_ip, uint8_t *dest_mac, uint32_t dest_ip, uint16_t op)
{
  struct arp_packet *ap = kmalloc(sizeof(struct arp_packet));
  ap->htype = htons(ARP_ETHERNET);
  ap->ptype = htons(ETH_P_IP);
  ap->hlen = 6;
  ap->plen = 4;
  ap->oper = htons(op);
  memcpy(ap->sha, source_mac, sizeof(ap->sha));
  ap->spa = htonl(source_ip);
  memcpy(ap->tha, dest_mac, sizeof(ap->tha));
  ap->tpa = htonl(dest_ip);

  return ap;
}

int32_t arp_rcv(struct sk_buff *skb)
{
  int32_t ret;

  struct arp_packet *ap = skb->data;
  ret = arp_validate_packet(ap);
  if (ret < 0)
    return ret;

  skb->nh.arph = ap;

  return 0;
}

void arp_gratuitous(struct sk_buff *skb)
{
}