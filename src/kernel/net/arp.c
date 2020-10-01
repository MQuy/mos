#include "arp.h"

#include <memory/vmm.h>
#include <net/net.h>
#include <net/sk_buff.h>
#include <shared/errno.h>
#include <shared/if_ether.h>
#include <system/sysapi.h>
#include <utils/string.h>

int arp_validate_packet(struct arp_packet *ap)
{
	if (ap->htype != htons(ARP_ETHERNET) || ap->ptype != htons(ETH_P_IP) || ap->hlen != 6 || ap->plen != 4)
		return -EPROTO;

	return 0;
}

struct arp_packet *arp_create_packet(uint8_t *source_mac, uint32_t source_ip, uint8_t *dest_mac, uint32_t dest_ip, uint16_t op)
{
	struct arp_packet *ap = kcalloc(1, sizeof(struct arp_packet));
	ap->htype = htons(ARP_ETHERNET);
	ap->ptype = htons(ETH_P_IP);
	ap->hlen = 6;
	ap->plen = 4;
	ap->oper = htons(op);
	memcpy(ap->sha, source_mac, 6);
	ap->spa = htonl(source_ip);
	memcpy(ap->tha, dest_mac, 6);
	ap->tpa = htonl(dest_ip);

	return ap;
}

int arp_rcv(struct sk_buff *skb)
{
	int32_t ret;

	struct arp_packet *ap = (struct arp_packet *)skb->data;
	ret = arp_validate_packet(ap);
	if (ret < 0)
		return ret;

	skb->nh.arph = ap;

	return 0;
}

int arp_send(uint8_t *source_mac, uint32_t source_ip, uint8_t *dest_mac, uint32_t dest_ip, uint16_t type)
{
	uint32_t sockfd = sys_socket(PF_PACKET, SOCK_DGRAM, ETH_P_ARP);
	struct socket *sock = sockfd_lookup(sockfd);
	struct net_device *dev = sock->sk->dev;

	if (dev->state & NETDEV_STATE_OFF)
		return -EBUSY;

	struct sockaddr_ll addr_remote;
	addr_remote.sll_protocol = ETH_P_ARP;
	addr_remote.sll_pkttype = PACKET_BROADCAST;
	memcpy(addr_remote.sll_addr, dest_mac, 6);
	sock->ops->connect(sock, (struct sockaddr *)&addr_remote, sizeof(struct sockaddr_ll));

	struct arp_packet *sarp = arp_create_packet(source_mac, source_ip, dest_mac, dest_ip, type);
	sock->ops->sendmsg(sock, sarp, sizeof(struct arp_packet));
	sock->ops->shutdown(sock);
	return 0;
}
