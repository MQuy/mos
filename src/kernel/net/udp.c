#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ip.h>
#include "udp.h"

#define MAX_UDP_HEADER (sizeof(struct ethernet_packet) + sizeof(struct ip4_packet) + sizeof(struct udp_packet))
#define CHECKSUM_MASK 0xFFFF

void udp_set_checksum(struct udp_packet *udp_packet, uint16_t udp_size, uint32_t source_ip, uint32_t dest_ip)
{
  struct ip4_pseudo_header *ip4_pseudo_header = kcalloc(1, sizeof(ip4_pseudo_header));
  ip4_pseudo_header->source_ip = htonl(source_ip);
  ip4_pseudo_header->dest_ip = htonl(dest_ip);
  ip4_pseudo_header->zeros = 0;
  ip4_pseudo_header->protocal = IP4_PROTOCAL_UDP;
  ip4_pseudo_header->udp_length = htons(udp_size);

  uint32_t ip4_checksum_start = packet_checksum_start(ip4_pseudo_header, sizeof(struct ip4_pseudo_header));
  uint32_t udp_checksum_start = packet_checksum_start(udp_packet, udp_size);
  uint32_t checksum = ip4_checksum_start + udp_checksum_start;

  while (checksum > CHECKSUM_MASK)
    checksum = (checksum & CHECKSUM_MASK) + (checksum >> 16);

  udp_packet->checksum = ~checksum & CHECKSUM_MASK;
}

void udp_build_header(struct sk_buff *skb, void *msg, uint16_t msg_len)
{
  struct socket *sk = skb->sk;
  struct udp_packet *udp = skb->h.udph;

  udp->source_port = htons(sk->sport);
  udp->dest_port = htons(sk->dport);
  udp->length = htons(msg_len);
  udp->checksum = 0;
  memcpy(udp->payload, msg, msg_len);

  udp_set_checksum(udp, sizeof(struct udp_packet) + msg_len, 0, 0xffffffff);
}

int udp_bind(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len)
{
  struct sockaddr_in *usin = (struct sockaddr_in *)myaddr;
  sock->saddr = usin->sin_addr.s_addr;
  sock->sport = usin->sin_port;
}

int udp_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
  struct sockaddr_in *usin = (struct sockaddr_in *)vaddr;
  sock->daddr = usin->sin_addr.s_addr;
  sock->dport = usin->sin_port;
  // TODO: MQ 2020-05-21 We don't need to perform routing, only supporting one router
  sock->dev = get_current_net_device();
}

int udp_sendmsg(struct socket *sock, char *msg, size_t msg_len)
{
  struct sk_buff *skb = alloc_skb(MAX_UDP_HEADER, msg_len);

  skb_put(skb, msg_len);
  skb_push(skb, sizeof(struct udp_packet));

  skb->h.udph = skb->data;
  udp_build_header(skb->h.udph, msg, msg_len);
  ip4_sendmsg(sock, skb);
}

int udp_recvmsg(struct socket *sock, char *msg, size_t msg_len)
{
}

struct proto_ops udp_proto_ops = {
    .family = PF_INET,
    .bind = udp_bind,
    .connect = udp_connect,
    .sendmsg = udp_sendmsg,
    .recvmsg = udp_recvmsg,
};