#include <include/ctype.h>
#include <include/if_ether.h>
#include <kernel/proc/task.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/devices/rtl8139.h>
#include <kernel/net/net.h>
#include <kernel/net/sk_buff.h>
#include "ethernet.h"

void ethernet_build_header(struct ethernet_packet *packet, uint16_t protocal, uint8_t *source_mac, uint8_t *dest_mac)
{
  packet->type = htons(protocal);
  memcpy(packet->dest_mac, dest_mac, sizeof(packet->dest_mac));
  memcpy(packet->source_mac, source_mac, sizeof(packet->source_mac));
}

void ethernet_sendmsg(struct sk_buff *skb)
{
  rtl8139_send_packet(skb->mac.eh, skb->len);
}

int ethernet_rcv(struct sk_buff *skb)
{
  skb->mac.eh = (struct ethernet_packet *)skb->data;
  skb_pull(skb, sizeof(struct ethernet_packet));

  return 0;
}
