#ifndef NET_ARP
#define NET_ARP

#include <stdint.h>

struct __attribute__((packed)) arp_packet
{
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen, plen;
    uint16_t oper;
    uint8_t sha[6];
    uint8_t spa[4];
    uint8_t tha[6];
    uint8_t tpa[4];
};

int32_t arp_send_packet(uint8_t *dmac, uint8_t *dip);

#endif