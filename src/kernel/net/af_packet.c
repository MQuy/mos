#include <include/if_ether.h>
#include <include/errno.h>
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
  return 0;
}

int packet_sendmsg(struct socket *sock, void *msg, size_t msg_len)
{
  if (sock->state == SS_DISCONNECTED)
    return -ESHUTDOWN;

  struct packet_sock *psk = pkt_sk(sock->sk);
  struct sk_buff *skb = alloc_skb(sizeof(struct ethernet_packet), msg_len);
  skb->sk = sock->sk;
  skb->dev = psk->sk.dev;

  // increase tail -> copy msg into data-tail
  skb_put(skb, msg_len);
  memcpy(skb->data, msg, msg_len);

  if (sock->type != SOCK_RAW)
  {
    // decrease data -> copy ethernet header into newdata-olddata
    skb_push(skb, sizeof(struct ethernet_packet));
    skb->mac.eh = (struct ethernet_packet *)skb->data;
    ethernet_build_header(skb->mac.eh, sock->protocol, skb->dev->dev_addr, psk->sll.sll_addr);
  }
  else
    skb->mac.eh = (struct ethernet_packet *)skb->data;

  ethernet_sendmsg(skb);
  return 0;
}

int packet_recvmsg(struct socket *sock, void *msg, size_t msg_len)
{
  if (sock->state == SS_DISCONNECTED)
    return -ESHUTDOWN;

  struct sock *sk = sock->sk;
  struct sk_buff *skb = NULL;

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
  if (sock->type == SOCK_RAW)
    memcpy(msg, skb->mac.eh, msg_len);
  else
    memcpy(msg, (uint32_t)skb->mac.eh + sizeof(struct ethernet_packet), msg_len);
  return 0;
}

int packet_handler(struct socket *sock, struct sk_buff *skb)
{
  if (sock->state == SS_DISCONNECTED)
    return -ESHUTDOWN;

  struct sock *sk = sock->sk;
  struct ethernet_packet *eh = (struct ethernet_packet *)skb->data;

  if (eh->type != htons(sock->protocol))
    return -EPROTO;

  skb->mac.eh = eh;

  // TODO: MQ 2020-06-02 Clone skb net frame and adjust head, data, tail, end pointers
  struct sk_buff *clone_skb = kcalloc(1, sizeof(struct sk_buff));
  memcpy(clone_skb, skb, sizeof(struct sk_buff));

  list_add_tail(&clone_skb->sibling, &sk->rx_queue);
  update_thread(sk->owner_thread, THREAD_READY);
  return 0;
}

struct proto_ops packet_proto_ops = {
    .family = PF_PACKET,
    .connect = packet_connect,
    .sendmsg = packet_sendmsg,
    .recvmsg = packet_recvmsg,
    .shutdown = socket_shutdown,
    .handler = packet_handler,
};
