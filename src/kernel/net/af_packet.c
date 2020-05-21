#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include <kernel/net/ethernet.h>
#include "af_packet.h"

int packet_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
  struct packet_sock *psk = pkt_sk(sock->sk);
  memcpy(&psk->sll, vaddr, sockaddr_len);
}

int packet_sendmsg(struct socket *sock, char *msg, size_t msg_len)
{
  struct packet_sock *psk = pkt_sk(sock->sk);
  struct sk_buff *skb = alloc_skb(sock->sk, sizeof(struct ethernet_packet), msg_len);

  // increase tail -> copy msg into data-tail
  skb_put(skb, msg_len);
  memcpy(skb->data, msg, msg_len);

  if (sock->type == SOCK_RAW)
  {
    skb->mac.eh = skb->data;
  }
  else
  {
    // decrease data -> copy ethernet header into newdata-olddata
    skb_push(skb, sizeof(struct ethernet_packet));
    skb->mac.eh = skb->data;
    ethernet_build_header(skb->mac.eh, sock->protocol, skb->dev->dev_addr, psk->sll.sll_addr);
  }

  ethernet_sendmsg(skb);
}

int packet_recvmsg(struct socket *sock, char *msg, size_t msg_len)
{
}

struct proto_ops packet_proto_ops = {
    .family = PF_PACKET,
    .connect = packet_connect,
    .sendmsg = packet_sendmsg,
    .recvmsg = packet_recvmsg,
}