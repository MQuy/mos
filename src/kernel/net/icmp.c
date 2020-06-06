#include <include/errno.h>
#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include <kernel/net/sk_buff.h>
#include <kernel/net/ip.h>
#include <kernel/system/sysapi.h>
#include "icmp.h"

int icmp_validate_packet(struct icmp_packet *icmp)
{
  return 0;
}

struct icmp_packet *icmp_create_packet(struct icmp_packet *icmp, uint8_t type, uint32_t rest_of_header, uint8_t *payload, uint32_t payload_len)
{
  uint16_t icmp_packet_size = sizeof(struct icmp_packet) + payload_len;
  icmp->type = type;
  icmp->code = 0;
  icmp->rest_of_header = htonl(rest_of_header);
  if (payload_len > 0 && payload)
    memcpy(icmp->payload, payload, payload_len);
  icmp->checksum = singular_checksum(icmp, icmp_packet_size);

  return icmp;
}

void icmp_reply(uint32_t source_ip,
                uint8_t *dest_mac, uint32_t dest_ip,
                uint32_t identification,
                uint32_t rest_of_header,
                uint8_t *payload, uint32_t payload_len)
{
  uint32_t sockfd = sys_socket(PF_PACKET, SOCK_DGRAM, ETH_P_IP);
  struct socket *sock = sockfd_lookup(sockfd);

  struct sockaddr_ll remote_sin;
  memcpy(remote_sin.sll_addr, dest_mac, 6);
  sock->ops->connect(sock, (struct sockaddr *)&remote_sin, sizeof(struct sockaddr_ll));

  uint32_t total_len = sizeof(struct ip4_packet) + sizeof(struct icmp_packet) + payload_len;
  uint8_t *buff = kcalloc(1, total_len);
  struct ip4_packet *iph = (struct ip4_packet *)buff;
  ip4_build_header(iph, total_len, IP4_PROTOCAL_ICMP, source_ip, dest_ip, identification);

  struct icmp_packet *icmp_packet = (struct icmp_packet *)(buff + sizeof(struct ip4_packet));
  icmp_create_packet(icmp_packet, ICMP_REPLY, rest_of_header, payload, payload_len);

  sock->ops->sendmsg(sock, iph, total_len);
  sock->ops->shutdown(sock);

  kfree(buff);
}

// Check icmp header valid, adjust skb *data
int icmp_rcv(struct sk_buff *skb)
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
