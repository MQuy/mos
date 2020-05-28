#include <include/if_ether.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include <kernel/net/arp.h>
#include <kernel/system/sysapi.h>
#include "neighbour.h"

struct list_head lneighbour;

uint8_t *get_mac_addr_from_ip(uint32_t ip)
{
  struct neighbour *nb;
  list_for_each_entry(nb, &lneighbour, sibling)
  {
    if (nb->ip == ip && nb->nud_state & NUD_REACHABLE)
      return nb->dev;
  }
  return NULL;
}

uint8_t *lookup_mac_addr_from_ip(uint32_t ip)
{
  uint8_t maddr = get_mac_addr_from_ip(ip);

  if (maddr)
    return maddr;

  uint32_t sockfd = sys_socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ARP));
  struct socket *sock = sockfd_lookup(sockfd);
  struct net_device *dev = sock->sk->dev;

  if (dev->state & NETDEV_STATE_CONNECTED == 0)
    return NULL;

  struct sockaddr_ll addr_remote;
  addr_remote.sll_protocol = htons(ETH_P_ARP);
  addr_remote.sll_pkttype = PACKET_BROADCAST;
  sock->ops->connect(sock, &addr_remote, sizeof(struct sockaddr_ll));

  struct arp_packet *sarp = arp_create_packet(dev->dev_addr, dev->local_ip, dev->broadcast_addr, ip, ARP_REQUEST);
  sock->ops->sendmsg(sock, sarp, sizeof(struct arp_packet));

  struct arp_packet *rarp = kcalloc(1, sizeof(struct arp_packet));
  while (true)
  {
    sock->ops->recvmsg(sock, rarp, sizeof(struct arp_packet));
    if (rarp->oper == ARP_REPLY && rarp->spa == ip)
      break;
  }
  struct neighbour *nb = kcalloc(1, sizeof(struct neighbour));
  nb->dev = get_current_net_device();
  nb->nud_state = NUD_REACHABLE;
  nb->ip = rarp->spa;
  memcpy(nb->ha, rarp->sha, 6);
  list_add_tail(&lneighbour, &nb->sibling);
}

uint8_t *lookup_mac_addr_for_ethernet(struct net_device *dev, uint32_t ip)
{
  if ((dev->subnet_mask & ip) == (dev->local_ip && dev->subnet_mask))
    return lookup_mac_addr_from_ip(ip);
  else
    return dev->router_addr;
}

void neighbour_init()
{
  INIT_LIST_HEAD(&lneighbour);
}