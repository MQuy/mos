#include "af_packet.h"

#include <include/errno.h>
#include <include/if_ether.h>
#include <memory/vmm.h>
#include <net/ethernet.h>
#include <net/net.h>
#include <net/sk_buff.h>
#include <proc/task.h>
#include <utils/math.h>
#include <utils/string.h>

int packet_connect(struct socket *sock, struct sockaddr *vaddr, int sockaddr_len)
{
	struct packet_sock *psk = pkt_sk(sock->sk);
	memcpy(&psk->sll, vaddr, sockaddr_len);
	return 0;
}

int packet_sendmsg(struct socket *sock, void *msg, size_t msg_len)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct packet_sock *psk = pkt_sk(sock->sk);
	struct sk_buff *skb = skb_alloc(sizeof(struct ethernet_packet), msg_len);
	skb->sk = sock->sk;
	skb->dev = psk->sk.dev;

	// increase tail -> copy msg into data-tail
	skb_put(skb, msg_len);
	memcpy(skb->data, msg, msg_len);

	if (sock->type != SOCK_RAW)
	{
		// decrease data -> copy ethernet header into newdata-olddata
		skb_push(skb, sizeof(struct ethernet_packet));
		skb->mac.eh = (struct ethernet_packet *)skb->data;
		ethernet_build_header(skb->mac.eh, sock->protocol, skb->dev->dev_addr, psk->sll.sll_addr);
	}
	else
		skb->mac.eh = (struct ethernet_packet *)skb->data;

	ethernet_sendmsg(skb);
	return 0;
}

int packet_recvmsg(struct socket *sock, void *msg, size_t msg_len)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct sock *sk = sock->sk;
	struct sk_buff *skb = NULL;

	while (!skb)
	{
		skb = list_first_entry_or_null(&sk->rx_queue, struct sk_buff, sibling);
		if (!skb)
		{
			update_thread(sk->owner_thread, THREAD_WAITING);
			schedule();
		}
	}

	list_del(&skb->sibling);

	uint32_t payload_len;
	uint8_t *buff;
	if (sock->type != SOCK_RAW)
	{
		skb_pull(skb, sizeof(struct ethernet_packet));
		buff = (uint8_t *)skb->mac.eh + sizeof(struct ethernet_packet);
	}
	else
		buff = (uint8_t *)skb->mac.eh;
	payload_len = min(msg_len, skb->len);
	memcpy(msg, buff, payload_len);

	skb_free(skb);
	return payload_len;
}

int packet_handler(struct socket *sock, struct sk_buff *skb)
{
	if (sock->state == SS_DISCONNECTED)
		return -ESHUTDOWN;

	struct sock *sk = sock->sk;
	struct ethernet_packet *eh = (struct ethernet_packet *)skb->data;

	if (eh->type != htons(sock->protocol) && sock->protocol != ETH_P_ALL)
		return -EPROTO;

	skb->mac.eh = eh;

	list_add_tail(&skb->sibling, &sk->rx_queue);
	update_thread(sk->owner_thread, THREAD_READY);
	return 0;
}

struct proto_ops packet_proto_ops = {
	.family = PF_PACKET,
	.obj_size = sizeof(struct packet_sock),
	.ioctl = inet_ioctl,
	.connect = packet_connect,
	.sendmsg = packet_sendmsg,
	.recvmsg = packet_recvmsg,
	.shutdown = socket_shutdown,
	.handler = packet_handler,
};
