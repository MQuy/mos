#include <include/ctype.h>
#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/utils/math.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/net.h>
#include "ip.h"

#define IP4_TTL 0x80

struct ip4_packet *ip4_build_header(struct ip4_packet *packet, uint16_t packet_size, uint8_t protocal, uint32_t sip, uint32_t dip)
{
  packet->version = 4;
  packet->ihl = 5;
  packet->type_of_service = 0;
  packet->total_length = htons(packet_size);
  packet->identification = htons(0);
  packet->flags = 0;
  packet->fragment_offset = 0;
  packet->time_to_live = IP4_TTL;
  packet->protocal = protocal;
  packet->source_ip = htonl(sip);
  packet->dest_ip = htonl(dip);
  packet->header_checksum = singular_checksum(packet, sizeof(struct ip4_packet));

  return packet;
}

void ip4_sendmsg(struct socket *sock, struct sk_buff *skb)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  // NOTE: MQ 2020-05-21 We don't need to perform routing, only support one router
  skb->dev = isk->sk.dev;

  skb_push(skb, sizeof(struct ethernet_packet));
  skb->mac.eh = skb->data;
  ethernet_build_header(skb->mac.eh, ETH_P_IP, skb->dev->dev_addr, isk->dsin.sin_addr);

  ethernet_sendmsg(skb);
}