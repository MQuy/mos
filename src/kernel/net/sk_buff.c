#include "sk_buff.h"

#include <memory/vmm.h>
#include <net/net.h>
#include <utils/string.h>

struct sk_buff *skb_alloc(uint32_t header_size, uint32_t payload_size)
{
	struct sk_buff *skb = kcalloc(1, sizeof(struct sk_buff));

	// NOTE: MQ 2020-05-20 padding starting header (udp, tcp or raw headers) by word
	uint32_t packet_size = header_size + payload_size + WORD_SIZE;
	skb->true_size = packet_size + sizeof(struct sk_buff);

	uint8_t *data = kcalloc(1, packet_size);
	skb->head = data;
	skb->data = skb->tail = (uint8_t *)WORD_ALIGN((uint32_t)data + header_size);
	skb->end = data + packet_size;
	return skb;
}

struct sk_buff *skb_clone(struct sk_buff *skb)
{
	struct sk_buff *skb_new = kcalloc(1, sizeof(struct sk_buff));
	memcpy(skb_new, skb, sizeof(struct sk_buff));

	uint32_t packet_size = skb->true_size - sizeof(struct sk_buff);
	uint8_t *packet = kcalloc(1, skb->true_size - sizeof(struct sk_buff));
	memcpy(packet, skb->head, packet_size);

	skb_new->head = packet;
	skb_new->data = packet + (skb->data - skb->head);
	skb_new->tail = packet + (skb->tail - skb->head);
	skb_new->end = packet + packet_size;

	return skb_new;
}

void skb_free(struct sk_buff *skb)
{
	kfree(skb->head);
	kfree(skb);
}
