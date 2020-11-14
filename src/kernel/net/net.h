#ifndef NET_H
#define NET_H

#include <fs/vfs.h>
#include <include/if_ether.h>
#include <include/list.h>

#define AF_UNIX 1	 /* Unix domain sockets 		*/
#define AF_INET 2	 /* Internet IP Protocol 	*/
#define AF_PACKET 17 /* Packet family		*/

#define PF_UNIX AF_UNIX
#define PF_INET AF_INET

#define PF_PACKET AF_PACKET

#define WORD_SIZE 4
#define WORD_MASK (~(WORD_SIZE - 1))
#define WORD_ALIGN(addr) (((addr) + WORD_SIZE - 1) & WORD_MASK)

#define CHECKSUM_MASK 0xFFFF

struct sk_buff;

/* Standard well-defined IP protocols.  */
enum
{
	IPPROTO_IP = 0,	   /* Dummy protocol for TCP		*/
	IPPROTO_ICMP = 1,  /* Internet Control Message Protocol	*/
	IPPROTO_TCP = 6,   /* Transmission Control Protocol	*/
	IPPROTO_UDP = 17,  /* User Datagram Protocol		*/
	IPPROTO_RAW = 255, /* Raw IP packets			*/
	IPPROTO_MAX
};

typedef unsigned short sa_family_t;

struct sockaddr
{
	sa_family_t sa_family; /* address family, AF_xxx	*/
	char sa_data[14];	   /* 14 bytes of protocol address	*/
};

struct sockaddr_in
{
	uint16_t sin_port; /* port in network byte order */
	uint32_t sin_addr; /* address in network byte order */
};

#define PACKET_HOST 0	   /* To us		*/
#define PACKET_BROADCAST 1 /* To all		*/
#define PACKET_MULTICAST 2 /* To group		*/
#define PACKET_OTHERHOST 3 /* To someone else 	*/
#define PACKET_OUTGOING 4  /* Outgoing of any type */
#define PACKET_LOOPBACK 5  /* MC/BRD frame looped back */
#define PACKET_USER 6	   /* To user space	*/
#define PACKET_KERNEL 7	   /* To kernel space	*/
/* Unused, PACKET_FASTROUTE and PACKET_LOOPBACK are invisible to user space */
#define PACKET_FASTROUTE 6 /* Fastrouted frame	*/

struct sockaddr_ll
{
	uint16_t sll_protocol; /* Physical-layer protocol */
	uint8_t sll_pkttype;   /* Packet type */
	uint8_t sll_addr[6];   /* Physical-layer address */
};

#define IFNAMSIZ 16

struct ifmap
{
	unsigned long mem_start;
	unsigned long mem_end;
	unsigned short base_addr;
	unsigned char irq;
	unsigned char dma;
	unsigned char port;
	/* 3 bytes spare */
};

struct ifreq
{
	char ifr_name[IFNAMSIZ]; /* Interface name */
	union
	{
		struct sockaddr ifr_addr;
		struct sockaddr ifr_dstaddr;
		struct sockaddr ifr_broadaddr;
		struct sockaddr ifr_netmask;
		struct sockaddr ifr_hwaddr;
		short ifr_flags;
		int ifr_ifindex;
		int ifr_metric;
		int ifr_mtu;
		struct ifmap ifr_map;
		char ifr_slave[IFNAMSIZ];
		char ifr_newname[IFNAMSIZ];
		char *ifr_data;
	};
};

enum socket_state
{
	SS_UNCONNECTED = 1,	  /* unconnected to any socket	*/
	SS_CONNECTING = 2,	  /* in process of connecting	*/
	SS_CONNECTED = 3,	  /* connected to socket		*/
	SS_DISCONNECTING = 4, /* in process of disconnecting	*/
	SS_DISCONNECTED = 5,
} socket_state;

enum socket_type
{
	SOCK_STREAM = 1,
	SOCK_DGRAM = 2,
	SOCK_RAW = 3,
};

struct socket
{
	uint16_t protocol;
	enum socket_type type;
	enum socket_state state;
	uint32_t flags;

	struct vfs_file *file;
	struct sock *sk;
	struct proto_ops *ops;

	struct list_head sibling;
};

struct sock
{
	struct socket *sock;
	struct net_device *dev;
	struct thread *owner_thread;
	struct list_head rx_queue;
	uint32_t rx_length;
	struct list_head tx_queue;
	struct list_head *send_head;
};

struct inet_sock
{
	struct sock sk;
	struct sockaddr_in ssin;
	struct sockaddr_in dsin;
};

struct packet_sock
{
	struct sock sk;
	struct sockaddr_ll sll;
};

static inline struct packet_sock *pkt_sk(struct sock *sk)
{
	return (struct packet_sock *)sk;
}

static inline struct inet_sock *inet_sk(struct sock *sk)
{
	return (struct inet_sock *)sk;
}

struct proto_ops
{
	int family;
	int obj_size;
	int (*bind)(struct socket *sock, struct sockaddr *myaddr, int sockaddr_len);
	int (*connect)(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len);
	int (*accept)(struct socket *sock, struct sockaddr *addr, int sockaddr_len);
	int (*ioctl)(struct socket *sock, unsigned int cmd, unsigned long arg);
	int (*listen)(struct socket *sock, int backlog);
	int (*shutdown)(struct socket *sock);
	int (*sendmsg)(struct socket *sock, void *msg, size_t msg_len);
	// TODO: MQ 2020-05-27
	// At the beging CONNECTED -> READY
	// At the end READ -> CONNECTED
	// To make sure each called recvmsg -> only one message
	int (*recvmsg)(struct socket *sock, void *msg, size_t msg_len);
	// NOTE: MQ 2020-05-24 Handling incoming messages to match and process further
	int (*handler)(struct socket *sock, struct sk_buff *skb);
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

enum netdev_state
{
	NETDEV_STATE_OFF = 1,
	NETDEV_STATE_UP = 1 << 1,
	NETDEV_STATE_CONNECTED = 1 << 2,  // interface connects and gets config (dhcp -> ip) from router
};

struct net_device
{
	uint32_t base_addr;
	uint8_t irq;

	char name[16];
	enum netdev_state state;
	struct list_head sibling;

	// ip & mac address
	uint8_t dev_addr[6];
	uint8_t broadcast_addr[6];
	uint8_t zero_addr[6];
	uint8_t router_addr[6];
	uint16_t mtu;
	uint32_t dns_server_ip;
	uint32_t dhcp_server_ip;
	uint32_t router_ip;
	uint32_t local_ip;
	uint32_t subnet_mask;
	uint32_t lease_time;
};

void net_init();
void net_rx_loop();
void net_switch();
void push_rx_queue(uint8_t *data, uint32_t size);
void socket_setup(int32_t family, enum socket_type type, int32_t protocal, struct vfs_file *file);
int socket_shutdown(struct socket *sock);
struct socket *sockfd_lookup(uint32_t fd);
uint16_t singular_checksum(void *packet, uint16_t size);
uint32_t packet_checksum_start(void *packet, uint16_t size);
uint16_t transport_calculate_checksum(void *segment, uint16_t segment_len, uint8_t protocal, uint32_t source_ip, uint32_t dest_ip);
char *inet_ntop(uint32_t src, char *dst, uint16_t len);
int inet_ioctl(struct socket *sock, unsigned int cmd, unsigned long arg);

void register_net_device(struct net_device *);
struct net_device *get_current_net_device();

// tcp.c
extern struct proto_ops tcp_proto_ops;

// udp.c
extern struct proto_ops udp_proto_ops;

// raw.c
extern struct proto_ops raw_proto_ops;

// af_packet.c
extern struct proto_ops packet_proto_ops;

#endif
