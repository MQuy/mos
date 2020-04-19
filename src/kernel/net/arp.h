#ifndef NET_ARP
#define NET_ARP

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

int32_t arp_send_packet(uint8_t *dmac, uint32_t dip, uint32_t sip, uint16_t op);

#endif