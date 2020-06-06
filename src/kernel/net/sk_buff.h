#ifndef NET_SK_BUFF_H
#define NET_SK_BUFF_H

#include <stdint.h>
#include <include/list.h>

struct udp_packet;
struct icmp_packet;
struct ip4_packet;
struct arp_packet;
struct ethernet_packet;

struct sk_buff
{
  struct sock *sk;
  struct net_device *dev;
  struct list_head sibling;
  uint32_t len, true_size;

  union {
    struct udp_packet *udph;
    struct icmp_packet *icmph;
    uint8_t *raw;
  } h;

  union {
    struct ip4_packet *iph;
    struct arp_packet *arph;
    uint8_t *raw;
  } nh;

  union {
    struct ethernet_packet *eh;
    uint8_t *raw;
  } mac;

  uint8_t *head;
  uint8_t *data;
  uint8_t *tail;
  uint8_t *end;
};

static inline void skb_reserve(struct sk_buff *skb, uint32_t len)
{
  skb->data += len;
  skb->tail += len;
}

static inline void skb_put(struct sk_buff *skb, uint32_t len)
{
  skb->tail += len;
  skb->len += len;
};

static inline void skb_push(struct sk_buff *skb, uint32_t len)
{
  skb->data -= len;
  skb->len += len;
}

static inline void skb_pull(struct sk_buff *skb, uint32_t len)
{
  skb->data += len;
  skb->len -= len;
}

struct sk_buff *skb_alloc(uint32_t header_size, uint32_t payload_size);
struct sk_buff *skb_clone(struct sk_buff *skb);
void skb_free(struct sk_buff *skb);

#endif
