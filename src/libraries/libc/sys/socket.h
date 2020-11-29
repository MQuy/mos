#ifndef _LIBC_SOCKET_H
#define _LIBC_SOCKET_H 1

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#define AF_UNIX 1	 /* Unix domain sockets 		*/
#define AF_INET 2	 /* Internet IP Protocol 	*/
#define AF_PACKET 17 /* Packet family		*/

#define PF_UNIX AF_UNIX
#define PF_INET AF_INET
#define PF_PACKET AF_PACKET

#ifndef __socklen_t_defined
typedef __socklen_t socklen_t;
#define __socklen_t_defined
#endif

#define SHUT_RD 0	/* shut down the reading side */
#define SHUT_WR 1	/* shut down the writing side */
#define SHUT_RDWR 2 /* shut down both sides */

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
};

enum socket_type
{
	SOCK_STREAM = 1,
	SOCK_DGRAM = 2,
	SOCK_RAW = 3,
};

int socket(int family, enum socket_type type, int protocal);
int bind(int sockfd, struct sockaddr *addr, unsigned int addrlen);
int connect(int sockfd, struct sockaddr *addr, unsigned int addrlen);
int send(int sockfd, void *msg, size_t len);
int recv(int sockfd, void *msg, size_t len);

#endif
