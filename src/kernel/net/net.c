#include <kernel/memory/vmm.h>
#include <kernel/net/route.h>
#include <kernel/fs/vfs.h>
#include <kernel/fs/sockfs/sockfs.h>
#include <kernel/proc/task.h>
#include "net.h"

#define CHECKSUM_MASK 0xFFFF

extern struct process *current_process;

struct list_head lskbuf;
struct list_head lsocket;
struct net_device *current_netdev;

void push_rx_queue(uint8_t *data, uint32_t size)
{
  struct sk_buff *skb = kcalloc(1, sizeof(struct sk_buff));

  char *buf = kcalloc(1, size);
  memcpy(buf, data, size);
  skb->head = skb->data = buf;
  skb->tail = skb->end = buf + size;
  skb->len = size;

  list_add_tail(&skb->sibling, &lskbuf);
}

void socket_setup(int32_t family, enum socket_type type, int32_t protocal, struct vfs_file *file)
{
  struct socket *sock = SOCKET_I(file->f_dentry->d_inode);
  sock->file = file;
  sock->type = type;
  sock->state = SS_UNCONNECTED;
  INIT_LIST_HEAD(&sock->sibling);
  INIT_LIST_HEAD(&sock->tx_queue);
  INIT_LIST_HEAD(&sock->rx_queue);

  if (type == SOCK_DGRAM)
    sock->ops = &udp_proto_ops;
  else if (type == SOCK_RAW)
    sock->ops = &raw_proto_ops;

  list_add_tail(&sock->sibling, &lsocket);
}

struct socket *sockfd_lookup(uint32_t sockfd)
{
  struct vfs_file *file = current_process->files->fd[sockfd];
  return SOCKET_I(&file->f_dentry->d_inode);
}

struct sk_buff *alloc_skb(uint32_t header_size, uint32_t payload_size)
{
  struct sk_buff *skb = kcalloc(1, sizeof(struct sk_buff));

  // NOTE: MQ 2020-05-20 align starting header (udp, tcp or raw headers) by word
  uint32_t packet_size = header_size + payload_size + WORD_SIZE;
  skb->true_size = packet_size + sizeof(struct sk_buff);

  char *data = kcalloc(1, packet_size);
  skb->head = data;
  skb->data = skb->tail = WORD_ALIGN((uint32_t)data);
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

void net_init()
{
  INIT_LIST_HEAD(&lsocket);
  INIT_LIST_HEAD(&lskbuf);

  route_init();
}
