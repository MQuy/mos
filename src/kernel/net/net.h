#ifndef NET_H
#define NET_H

#include <include/list.h>
#include <kernel/fs/vfs.h>
#include <kernel/net/ip.h>
#include <kernel/net/arp.h>

#define AF_INET 2 /* Internet IP Protocol 	*/
#define PF_INET AF_INET

typedef unsigned short sa_family_t;
struct sockaddr
{
  sa_family_t sa_family; /* address family, AF_xxx	*/
  char sa_data[14];      /* 14 bytes of protocol address	*/
};

enum socket_state
{
  SS_FREE = 0,     /* not allocated		*/
  SS_UNCONNECTED,  /* unconnected to any socket	*/
  SS_CONNECTING,   /* in process of connecting	*/
  SS_CONNECTED,    /* connected to socket		*/
  SS_DISCONNECTING /* in process of disconnecting	*/
} socket_state;

enum socket_type
{
  SOCK_STREAM = 1,
  SOCK_DGRAM = 2,
  SOCK_RAW = 3,
};

struct socket
{
  enum socket_type type;
  enum socket_state state;
  unsigned long flags;
  struct proto_ops *ops;
  struct file *file;

  struct list_head tx_queue;
  struct list_head rx_queue;
};

struct proto_ops
{
  int family;
  int (*release)(struct socket *sock);
  int (*bind)(struct socket *sock,
              struct sockaddr *myaddr,
              int sockaddr_len);
  int (*connect)(struct socket *sock,
                 struct sockaddr *vaddr,
                 int sockaddr_len, int flags);
  int (*accept)(struct socket *sock,
                struct socket *newsock, int flags);
  int (*getname)(struct socket *sock,
                 struct sockaddr *addr,
                 int *sockaddr_len, int peer);
  int (*listen)(struct socket *sock, int len);
  int (*shutdown)(struct socket *sock, int flags);
  int (*sendmsg)(char *msg, struct socket *sock, size_t total_len);
  int (*recvmsg)(char *msg, struct socket *sock, size_t total_len, int flags);
};

struct socket_alloc
{
  struct socket socket;
  struct vfs_inode inode;
};

static inline struct socket *SOCKET_I(struct vfs_inode *inode)
{
  return &container_of(inode, struct socket_alloc, inode)->socket;
}

static inline struct vfs_inode *SOCK_INODE(struct socket *socket)
{
  return &container_of(socket, struct socket_alloc, socket)->inode;
}

struct net_device
{
  uint32_t base_addr;
  uint32_t mem_start;
  uint32_t mem_end;
  uint32_t irq;

  char name[16];
  uint8_t state;
  struct list_head sibling;

  uint8_t dev_addr[6];
  uint8_t broadcast[6];
};

struct sk_buff
{
  struct socket *sk;
  struct net_device *dev;
  struct list_head sibling;
  uint32_t len, data_len;

  union {
    struct udp_packet *uh;
    struct icmp_packet *icmph;
    uint8_t *raw;
  } h;

  union {
    struct ip4_packet *iph;
    struct arp_packet *arph;
    uint8_t *raw;
  } nh;

  union {
    uint8_t *raw;
  } mac;

  uint8_t *head;
  uint8_t *data;
  uint8_t *tail;
  uint8_t *end;
};

void inet_init();
void push_rx_queue(uint8_t *data, uint32_t size);

#endif
