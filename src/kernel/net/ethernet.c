#include <include/ctype.h>
#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/devices/rtl8139.h>
#include <kernel/net/net.h>
#include "ethernet.h"

void ethernet_build_header(struct ethernet_packet *packet, uint16_t protocal, uint8_t *smac, uint8_t *dmac)
{
  packet->type = htons(protocal);
  memcpy(packet->dmac, dmac, sizeof(packet->dmac));
  memcpy(packet->smac, smac, sizeof(packet->smac));
}

void ethernet_sendmsg(struct sk_buff *skb)
{
  skb_push(skb, sizeof(struct ethernet_packet));

  skb->mac.eh = skb->data;
  ethernet_build_header(skb->mac.eh, ETH_P_IP, skb->dev->dev_addr);
}
