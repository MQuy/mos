#include <include/errno.h>
#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/net/net.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/neighbour.h>
#include "raw.h"

#define RAW_HEADER_SIZE (sizeof(struct ethernet_packet) + sizeof(struct ip4_packet))

int raw_bind(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  memcpy(&isk->ssin, myaddr, sockaddr_len);
  return 0;
}

int raw_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  memcpy(&isk->dsin, vaddr, sockaddr_len);

  sock->state = SS_CONNECTED;
  return 0;
}

int raw_sendmsg(struct socket *sock, void *msg, size_t msg_len)
{
  if (sock->state == SS_DISCONNECTED)
    return -ESHUTDOWN;

  struct inet_sock *isk = inet_sk(sock->sk);
  struct sk_buff *skb = alloc_skb(RAW_HEADER_SIZE, msg_len);
  skb->sk = sock->sk;
  skb->dev = isk->sk.dev;

  // increase tail -> copy msg into data-tail space
  skb_put(skb, msg_len);
  memcpy(skb->data, msg, msg_len);

  // decase data -> copy ip4 header into newdata-olddata
  skb_push(skb, sizeof(struct ip4_packet));
  skb->nh.iph = (struct ip4_packet *)skb->data;
  ip4_build_header(skb->nh.iph, skb->len, IP4_PROTOCAL_ICMP, isk->ssin.sin_addr, isk->dsin.sin_addr, 0);

  ip4_sendmsg(sock, skb);
  return 0;
}

int raw_recvmsg(struct socket *sock, void *msg, size_t msg_len)
{
  if (sock->state == SS_DISCONNECTED)
    return -ESHUTDOWN;

  struct sock *sk = sock->sk;
  struct sk_buff *skb;

  while (!skb)
  {
    skb = list_first_entry_or_null(&sk->rx_queue, struct sk_buff, sibling);
    if (!skb)
    {
      update_thread(sk->owner_thread, THREAD_WAITING);
      schedule();
    }
  }

  list_del(&skb->sibling);
  memcpy(msg, (uint32_t)skb->mac.eh + sizeof(struct ethernet_packet), msg_len);
  return 0;
}

int raw_handler(struct socket *sock, struct sk_buff *skb)
{
  if (sock->state == SS_DISCONNECTED)
    return -ESHUTDOWN;

  struct inet_sock *isk = inet_sk(sock->sk);
  struct net_device *dev = sock->sk->dev;
  struct ethernet_packet *eh = (struct ethernet_packet *)skb->data;

  if (htons(eh->type) != ETH_P_IP)
    return -EPROTO;

  skb->mac.eh = eh;
  skb_pull(skb, sizeof(struct ethernet_packet));

  struct ip4_packet *iph = (struct ip4_packet *)skb->data;
  if (iph->source_ip == isk->dsin.sin_addr && iph->dest_ip == dev->local_ip)
  {
    skb->nh.iph = iph;

    struct sk_buff *clone_skb = kcalloc(1, sizeof(struct sk_buff));
    memcpy(clone_skb, skb, sizeof(struct sk_buff));

    list_add_tail(&clone_skb->sibling, &sock->sk->rx_queue);
    update_thread(sock->sk->owner_thread, THREAD_READY);
  }
  return 0;
}

struct proto_ops raw_proto_ops = {
    .family = PF_INET,
    .bind = raw_bind,
    .connect = raw_connect,
    .sendmsg = raw_sendmsg,
    .recvmsg = raw_recvmsg,
    .shutdown = socket_shutdown,
    .handler = raw_handler,
};
