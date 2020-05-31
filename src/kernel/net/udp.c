#include <include/errno.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/net/net.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/ip.h>
#include "udp.h"

#define CHECKSUM_MASK 0xFFFF

int32_t udp_validate_header(struct udp_packet *udp)
{
  return 0;
}

void udp_set_checksum(struct udp_packet *udp_packet, uint16_t udp_len, uint32_t source_ip, uint32_t dest_ip)
{
  struct ip4_pseudo_header *ip4_pseudo_header = kcalloc(1, sizeof(ip4_pseudo_header));
  ip4_pseudo_header->source_ip = htonl(source_ip);
  ip4_pseudo_header->dest_ip = htonl(dest_ip);
  ip4_pseudo_header->zeros = 0;
  ip4_pseudo_header->protocal = IP4_PROTOCAL_UDP;
  ip4_pseudo_header->udp_length = htons(udp_len);

  uint32_t ip4_checksum_start = packet_checksum_start(ip4_pseudo_header, sizeof(struct ip4_pseudo_header));
  uint32_t udp_checksum_start = packet_checksum_start(udp_packet, udp_len);
  uint32_t checksum = ip4_checksum_start + udp_checksum_start;

  while (checksum > CHECKSUM_MASK)
    checksum = (checksum & CHECKSUM_MASK) + (checksum >> 16);

  udp_packet->checksum = ~checksum & CHECKSUM_MASK;
}

void udp_build_header(struct udp_packet *udp, uint16_t packet_len, uint32_t source_ip, uint16_t source_port, uint32_t dest_ip, uint16_t dest_port)
{
  uint16_t msg_len = packet_len - sizeof(struct udp_packet);

  udp->source_port = htons(source_port);
  udp->dest_port = htons(dest_port);
  udp->length = htons(msg_len);
  udp->checksum = 0;

  udp_set_checksum(udp, packet_len, source_ip, dest_ip);
}

int udp_bind(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  memcpy(&isk->ssin, myaddr, sockaddr_len);
  return 0;
}

// NOTE: MQ 2020-05-21 We don't need to perform routing, only supporting one router
int udp_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  memcpy(&isk->dsin, vaddr, sockaddr_len);
  return 0;
}

int udp_sendmsg(struct socket *sock, void *msg, size_t msg_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  struct sk_buff *skb = alloc_skb(MAX_UDP_HEADER, msg_len);
  skb->sk = sock->sk;

  // increase tail -> copy msg into data-tail space
  skb_put(skb, msg_len);
  memcpy(skb->data, msg, msg_len);

  // decrease data -> copy udp header into new expanding newdata-olddata
  skb_push(skb, sizeof(struct udp_packet));
  skb->h.udph = (struct udp_packet *)skb->data;
  udp_build_header(skb->h.udph, skb->len, isk->ssin.sin_addr, isk->ssin.sin_port, isk->dsin.sin_addr, isk->dsin.sin_port);

  // decrease data -> copy ip4 header into new expending newdata-olddata
  skb_push(skb, sizeof(struct ip4_packet));
  skb->nh.iph = (struct ip4_packet *)skb->data;
  ip4_build_header(skb->nh.iph, skb->len, IP4_PROTOCAL_UDP, isk->ssin.sin_addr, isk->dsin.sin_addr);

  ip4_sendmsg(sock, skb);
  return 0;
}

int udp_recvmsg(struct socket *sock, void *msg, size_t msg_len)
{
  struct sock *sk = sock->sk;
  struct sk_buff *skb;

  while (!skb)
  {
    skb = list_first_entry_or_null(&sk->rx_queue, struct sk_buff, sibling);
    if (!skb)
      update_thread(sk->owner_thread, THREAD_WAITING);
  }

  list_del(&skb->sibling);
  memcpy(msg, skb->h.udph + sizeof(struct udp_packet), msg_len);
  return 0;
}

int udp_handler(struct socket *sock, struct sk_buff *skb)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  int32_t ret = ethernet_rcv(skb);
  if (ret < 0)
    return ret;

  ret = ip4_rcv(skb);
  if (ret < 0)
    return ret;

  if (skb->nh.iph->protocal != IP4_PROTOCAL_UDP)
    return -EPROTO;

  struct udp_packet *udp = (struct udp_packet *)skb->data;

  if (skb->nh.iph->dest_ip == isk->ssin.sin_addr && udp->dest_port == isk->ssin.sin_port &&
      skb->nh.iph->source_ip == isk->dsin.sin_addr && udp->source_port == isk->dsin.sin_port)
  {
    skb->h.udph = udp;
    skb_pull(skb, sizeof(struct udp_packet));

    struct sk_buff *clone_skb = kcalloc(1, sizeof(struct sk_buff));
    memcpy(clone_skb, skb, sizeof(struct sk_buff));

    list_add_tail(&clone_skb->sibling, &sock->sk->rx_queue);
    update_thread(sock->sk->owner_thread, THREAD_READY);
  }
  return 0;
}

struct proto_ops udp_proto_ops = {
    .family = PF_INET,
    .bind = udp_bind,
    .connect = udp_connect,
    .sendmsg = udp_sendmsg,
    .recvmsg = udp_recvmsg,
    .handler = udp_handler,
};
