#include <include/errno.h>
#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/net/net.h>
#include <kernel/net/ethernet.h>
#include <kernel/net/neighbour.h>
#include "raw.h"

int raw_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  memcpy(&isk->dsin, vaddr, sockaddr_len);
}

int raw_sendmsg(struct socket *sock, char *msg, size_t msg_len)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  struct sk_buff *skb = alloc_skb(sizeof(struct ethernet_packet), msg_len);
  skb->sk = sock->sk;
  skb->dev = isk->sk.dev;

  // increase tail -> copy msg into data-tail space
  skb_put(skb, msg_len);
  skb->nh.iph = skb->data;
  memcpy(skb->data, msg, msg_len);

  // decrease data -> copy ethernet header into newdata-olddata
  skb_push(skb, sizeof(struct ethernet_packet));
  skb->mac.eh = skb->data;
  uint8_t *dmac = lookup_mac_addr_for_ethernet(skb->dev, isk->dsin.sin_addr);
  ethernet_build_header(skb->mac.eh, ETH_P_IP, skb->dev->base_addr, dmac);
}

int raw_recvmsg(struct socket *sock, char *msg, size_t msg_len)
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
  memcpy(msg, skb->mac.eh + sizeof(struct ethernet_packet), msg_len);
}

int raw_handler(struct socket *sock, struct sk_buff *skb)
{
  struct inet_sock *isk = inet_sk(sock->sk);
  struct net_device *dev = sock->sk->dev;
  struct ethernet_packet *eh = skb->data;

  if (htons(eh->type) != ETH_P_IP)
    return -EPROTO;

  skb->mac.eh = eh;
  skb_pull(skb, sizeof(struct ethernet_packet));

  struct ip4_packet *iph = skb->data;
  if (iph->source_ip == isk->dsin.sin_addr && iph->dest_ip == dev->local_ip)
  {
    skb->nh.iph = iph;

    struct sk_buff *clone_skb = kcalloc(1, sizeof(struct sk_buff));
    memcpy(clone_skb, skb, sizeof(struct sk_buff));

    list_add_tail(&clone_skb->sibling, &sock->sk->rx_queue);
    update_thread(sock->sk->owner_thread, THREAD_READY);
  }
}

struct proto_ops raw_proto_ops = {
    .family = PF_INET,
    .connect = raw_connect,
    .sendmsg = raw_sendmsg,
    .recvmsg = raw_recvmsg,
    .handler = raw_handler,
}