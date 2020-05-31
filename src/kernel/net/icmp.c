#include <include/errno.h>
#include <kernel/net/net.h>
#include <kernel/memory/vmm.h>
#include <kernel/system/sysapi.h>
#include "icmp.h"

int32_t icmp_validate_packet(struct icmp_packet *icmp)
{
  return 0;
}

struct icmp_packet *icmp_create_packet(uint8_t type, uint32_t rest_of_header)
{
  uint16_t icmp_packet_size = sizeof(struct icmp_packet) + 20;
  struct icmp_packet *icmp = kcalloc(1, icmp_packet_size);
  icmp->type = type;
  icmp->code = 0;
  icmp->rest_of_header = htonl(rest_of_header);
  icmp->checksum = singular_checksum(icmp, icmp_packet_size);

  return icmp;
}

void icmp_reply(uint32_t dest_ip, uint32_t rest_of_header)
{
  uint32_t sockfd = sys_socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
  struct socket *sock = sockfd_lookup(sockfd);

  struct sockaddr_in remote_sin;
  remote_sin.sin_addr = dest_ip;
  sock->ops->connect(sock, (struct sockaddr *)&remote_sin, sizeof(struct sockaddr_in));

  uint16_t icmp_packet_size = sizeof(struct icmp_packet) + 20;
  struct icmp_packet *icmp_packet = icmp_create_packet(ICMP_REPLY, rest_of_header);
  sock->ops->sendmsg(sock, icmp_packet, icmp_packet_size);
}

// Check icmp header valid, adjust skb *data
int32_t icmp_rcv(struct sk_buff *skb)
{
  if (skb->nh.iph->protocal != IP4_PROTOCAL_ICMP)
    return -EPROTO;

  int32_t ret;
  struct icmp_packet *icmp = (struct icmp_packet *)skb->data;

  ret = icmp_validate_packet(icmp);
  if (ret < 0)
    return ret;

  skb->h.icmph = icmp;
  return 0;
}
