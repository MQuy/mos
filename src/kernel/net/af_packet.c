#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/proc/task.h>
#include <kernel/net/net.h>
#include <kernel/net/ethernet.h>
#include "af_packet.h"

extern struct thread *current_thread;

int packet_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
  struct packet_sock *psk = pkt_sk(sock->sk);
  memcpy(&psk->sll, vaddr, sockaddr_len);
}

int packet_sendmsg(struct socket *sock, char *msg, size_t msg_len)
{
  struct packet_sock *psk = pkt_sk(sock->sk);
  struct sk_buff *skb = alloc_skb(sizeof(struct ethernet_packet), msg_len);
  skb->sk = sock->sk;

  // increase tail -> copy msg into data-tail
  skb_put(skb, msg_len);
  memcpy(skb->data, msg, msg_len);

  if (sock->type != SOCK_RAW)
  {
    // decrease data -> copy ethernet header into newdata-olddata
    skb_push(skb, sizeof(struct ethernet_packet));
    skb->mac.eh = skb->data;
    ethernet_build_header(skb->mac.eh, sock->protocol, skb->dev->dev_addr, psk->sll.sll_addr);
  }
  else
    skb->mac.eh = skb->data;

  ethernet_sendmsg(skb);
}

int packet_recvmsg(struct socket *sock, char *msg, size_t msg_len)
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
  if (sock->type == SOCK_RAW)
    memcpy(msg, skb->mac.eh, msg_len);
  else
    memcpy(msg, skb->mac.eh + sizeof(struct ethernet_packet), msg_len);
}

int packet_handler(struct socket *sock, struct sk_buff *skb)
{
  struct sock *sk = sock->sk;
  struct ethernet_packet *eh = skb->data;

  if (eh->type != sock->protocol)
    return;

  skb->mac.eh = eh;
  if (sock->type != SOCK_RAW)
  {
    if (sock->protocol == htons(ETH_P_ARP))
    {
      skb_push(skb, sizeof(struct arp_packet));
      skb->nh.arph = skb->data;
    }
  }

  struct sk_buff *clone_skb = kcalloc(1, sizeof(struct sk_buff));
  memcpy(clone_skb, skb, sizeof(struct sk_buff));

  list_add_tail(&clone_skb->sibling, &sk->rx_queue);
  update_thread(sk->owner_thread, THREAD_READY);
}

struct proto_ops packet_proto_ops = {
    .family = PF_PACKET,
    .connect = packet_connect,
    .sendmsg = packet_sendmsg,
    .recvmsg = packet_recvmsg,
    .handler = packet_handler,
}