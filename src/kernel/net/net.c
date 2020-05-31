#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/neighbour.h>
#include <kernel/net/icmp.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/sockfs/sockfs.h>
#include <kernel/proc/task.h>
#include "net.h"

#define CHECKSUM_MASK 0xFFFF

extern struct process *current_process;
extern struct thread *current_thread;

struct process *net_process;
struct thread *net_thread;
struct list_head lrx_skb;
struct list_head lsocket;
struct net_device *current_netdev;

void push_rx_queue(uint8_t *data, uint32_t size)
{
  struct sk_buff *skb = alloc_skb(0, size);

  memcpy(skb->data, data, size);
  list_add_tail(&skb->sibling, &lrx_skb);
}

void sock_setup(struct socket *sock, int32_t family)
{
  struct sock *sk = sock->sk;
  if (family == PF_INET)
    sk = kcalloc(1, sizeof(struct inet_sock));
  else if (family == PF_PACKET)
    sk = kcalloc(1, sizeof(struct packet_sock));

  sk->dev = get_current_net_device();
  sk->owner_thread = current_thread;
  INIT_LIST_HEAD(&sk->rx_queue);

  sock->sk = sk;
}

void socket_setup(int32_t family, enum socket_type type, int32_t protocal, struct vfs_file *file)
{
  struct socket *sock = SOCKET_I(file->f_dentry->d_inode);
  sock->protocol = protocal;
  sock->file = file;
  sock->type = type;
  sock->state = SS_UNCONNECTED;

  if (family == PF_INET)
  {
    if (type == SOCK_DGRAM)
      sock->ops = &udp_proto_ops;
    else if (type == SOCK_RAW)
      sock->ops = &raw_proto_ops;
  }
  else if (family == PF_PACKET)
    sock->ops = &packet_proto_ops;

  list_add_tail(&sock->sibling, &lsocket);
  sock_setup(sock, family);
}

struct socket *sockfd_lookup(uint32_t sockfd)
{
  struct vfs_file *file = current_process->files->fd[sockfd];
  return SOCKET_I(&file->f_dentry->d_inode);
}

struct sk_buff *alloc_skb(uint32_t header_size, uint32_t payload_size)
{
  struct sk_buff *skb = kcalloc(1, sizeof(struct sk_buff));

  // NOTE: MQ 2020-05-20 padding starting header (udp, tcp or raw headers) by word
  uint32_t packet_size = header_size + payload_size + WORD_SIZE;
  skb->true_size = packet_size + sizeof(struct sk_buff);

  char *data = kcalloc(1, packet_size);
  skb->head = data;
  skb->data = skb->tail = WORD_ALIGN((uint32_t)data + header_size);
  skb->end = data + packet_size;
}

uint32_t packet_checksum_start(void *packet, uint16_t size)
{
  uint32_t checksum = 0;

  uint16_t ibytes = size;
  uint16_t *chunk = (uint16_t *)packet;
  while (ibytes > 1)
  {
    checksum += *chunk;

    ibytes -= 2;
    chunk += 1;
  }
  if (ibytes == 1)
    checksum += *(uint8_t *)chunk;

  while (checksum > CHECKSUM_MASK)
    checksum = (checksum & CHECKSUM_MASK) + (checksum >> 16);

  return checksum;
}

uint16_t singular_checksum(void *packet, uint16_t size)
{
  uint32_t checksum = packet_checksum_start(packet, size);
  return ~checksum & CHECKSUM_MASK;
}

void register_net_device(struct net_device *ndev)
{
  current_netdev = ndev;
}

struct net_device *get_current_net_device()
{
  return current_netdev;
}

bool is_broadcast_mac_address(uint8_t *maddr)
{
  if (memcmp(current_netdev->broadcast_addr, maddr, 6) == 0 || memcmp(current_netdev->zero_addr, maddr, 6) == 0)
    return true;
  else
    return false;
}

// 1. Check icmp request to local ip -> send ICMP reply
// 2. Check arp probe asking our mac address -> send arp reply
// 3. Check arp annoucement -> update neighbour arp
void net_default_rx_handler(struct sk_buff *skb)
{
  uint32_t ret = ethernet_rcv(skb);
  if (ret < 0)
    return;

  if (skb->mac.eh->type == htons(ETH_P_IP))
  {
    ret = ip4_rcv(skb);
    if (ret < 0)
      return ret;

    if (skb->nh.iph->protocal == IP4_PROTOCAL_ICMP)
    {
      ret = icmp_rcv(skb);
      if (ret < 0)
        return ret;

      if (skb->h.icmph->code == ICMP_ECHO && skb->h.icmph->type == ICMP_REQUEST &&
          skb->nh.iph->dest_ip == htonl(current_netdev->local_ip))
        icmp_reply(skb->nh.iph->source_ip, ntohl(skb->h.icmph->rest_of_header));
    }
  }
  else if (skb->mac.eh->type = ntohs(ETH_P_ARP))
  {
    ret = arp_rcv(skb);
    if (ret < 0)
      return ret;

    if (skb->nh.arph->tpa == current_netdev->local_ip)
      arp_send(current_netdev->dev_addr, current_netdev->local_ip, skb->nh.arph->sha, skb->nh.arph->spa, ARP_REPLY);
    else if (skb->nh.arph->tpa == skb->nh.arph->spa && is_broadcast_mac_address(skb->nh.arph->tha))
      neighbour_update_mapping(skb->nh.arph->sha, skb->nh.arph->spa);
  }
}

void net_rx_loop()
{
  while (true)
  {
    struct sk_buff *skb;
    struct sk_buff *prev_skb = NULL;

    list_for_each_entry(skb, &lrx_skb, sibling)
    {
      if (prev_skb)
        list_del(&prev_skb->sibling);

      struct socket *sock;
      list_for_each_entry(sock, &lsocket, sibling)
      {
        sock->ops->handler(sock, skb);
      }
      if ((current_netdev->state & NETDEV_STATE_CONNECTED) != 0)
        net_default_rx_handler(skb);

      prev_skb = skb;
    }
    if (prev_skb)
      list_del(&prev_skb->sibling);

    update_thread(current_thread, THREAD_WAITING);
    schedule();
  }
}

void net_switch()
{
  if (list_empty(&lrx_skb))
    return;
  if (net_thread != current_thread)
    update_thread(net_thread, THREAD_READY);
}

void net_init()
{
  INIT_LIST_HEAD(&lsocket);
  INIT_LIST_HEAD(&lrx_skb);

  neighbour_init();

  net_process = create_kernel_process("net", net_rx_loop, 0);
  net_thread = net_process->active_thread;
}
