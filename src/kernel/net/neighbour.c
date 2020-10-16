#include "neighbour.h"

#include <include/if_ether.h>
#include <memory/vmm.h>
#include <net/arp.h>
#include <net/net.h>
#include <system/sysapi.h>
#include <utils/string.h>

struct list_head lneighbour;

uint8_t *get_mac_addr_from_ip(uint32_t ip)
{
	struct neighbour *nb;
	list_for_each_entry(nb, &lneighbour, sibling)
	{
		if (nb->ip == ip && nb->nud_state & NUD_REACHABLE)
			return nb->dev->dev_addr;
	}
	return NULL;
}

uint8_t *lookup_mac_addr_from_ip(uint32_t ip)
{
	uint8_t *maddr = get_mac_addr_from_ip(ip);

	if (maddr)
		return maddr;

	uint32_t sockfd = sys_socket(PF_PACKET, SOCK_DGRAM, ETH_P_ARP);
	struct socket *sock = sockfd_lookup(sockfd);
	struct net_device *dev = sock->sk->dev;

	if (dev->state & NETDEV_STATE_OFF)
		return NULL;

	struct sockaddr_ll addr_remote;
	addr_remote.sll_protocol = ETH_P_ARP;
	addr_remote.sll_pkttype = PACKET_BROADCAST;
	memcpy(addr_remote.sll_addr, dev->broadcast_addr, 6);
	sock->ops->connect(sock, (struct sockaddr *)&addr_remote, sizeof(struct sockaddr_ll));

	struct arp_packet *sarp = arp_create_packet(dev->dev_addr, dev->local_ip, dev->broadcast_addr, ip, ARP_REQUEST);
	sock->ops->sendmsg(sock, sarp, sizeof(struct arp_packet));

	struct arp_packet *rarp = kcalloc(1, sizeof(struct arp_packet));
	while (true)
	{
		sock->ops->recvmsg(sock, rarp, sizeof(struct arp_packet));
		if (rarp->oper == htons(ARP_REPLY) && rarp->spa == htonl(ip))
			break;
	}
	sock->ops->shutdown(sock);
	struct neighbour *nb = kcalloc(1, sizeof(struct neighbour));
	nb->dev = dev;
	nb->nud_state = NUD_REACHABLE;
	nb->ip = rarp->spa;
	memcpy(nb->ha, rarp->sha, 6);
	list_add_tail(&nb->sibling, &lneighbour);

	kfree(rarp);
	return nb->ha;
}

uint8_t *lookup_mac_addr_for_ethernet(struct net_device *dev, uint32_t ip)
{
	if ((dev->subnet_mask & ip) == (dev->local_ip && dev->subnet_mask))
		return lookup_mac_addr_from_ip(ip);
	else
		return dev->router_addr;
}

void neighbour_update_mapping(uint8_t *mac_address, uint32_t ip)
{
	struct neighbour *nb_iter, *prev_nb;
	list_for_each_entry(nb_iter, &lneighbour, sibling)
	{
		if (prev_nb)
		{
			list_del(&prev_nb->sibling);
			kfree(prev_nb);
			prev_nb = NULL;
		}
		if ((nb_iter->ip == ip || memcmp(nb_iter->ha, mac_address, 6) == 0) && nb_iter->nud_state & NUD_REACHABLE)
			prev_nb = nb_iter;
	}
	if (prev_nb)
	{
		list_del(&prev_nb->sibling);
		kfree(prev_nb);
	}

	struct neighbour *nb = kcalloc(1, sizeof(struct neighbour));
	nb->ip = ip;
	memcpy(nb->ha, mac_address, 6);
	nb->dev = get_current_net_device();
	nb->nud_state = NUD_REACHABLE;
	list_add_tail(&nb->sibling, &lneighbour);
}

void neighbour_init()
{
	INIT_LIST_HEAD(&lneighbour);
}
