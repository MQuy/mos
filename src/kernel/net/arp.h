#ifndef NET_ARP_H
#define NET_ARP_H

#include <stdint.h>

#define ARP_ETHERNET 0x0001
#define ARP_IPV4 0x0800
#define ARP_REQUEST 0x0001
#define ARP_REPLY 0x0002

struct __attribute__((packed)) arp_packet
{
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen, plen;
    uint16_t oper;
    uint8_t sha[6];
    uint32_t spa;
    uint8_t tha[6];
    uint32_t tpa;
};

struct arp_packet *arp_create_packet(uint8_t *source_mac, uint32_t source_ip, uint8_t *dest_mac, uint32_t dest_ip, uint16_t op);
int32_t arp_rcv(struct sk_buff *skb);
int32_t arp_send(uint8_t *source_mac, uint32_t source_ip, uint8_t *dest_mac, uint32_t dest_ip, uint16_t type);

#endif