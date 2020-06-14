#include "dns.h"

#include <kernel/memory/vmm.h>
#include <kernel/net/net.h>
#include <kernel/system/sysapi.h>
#include <kernel/utils/math.h>
#include <kernel/utils/printf.h>
#include <kernel/utils/string.h>

#define MAX_DNS_LEN 512

void dns_build_header(struct dns_packet *dns, uint16_t identifier)
{
	dns->id = htons(identifier);
	dns->qr = DNS_FLAG_QUERY;
	dns->op_code = DNS_STANDARD_QUERY;
	dns->aa = 0;
	dns->tc = 0;
	dns->rd = 1;
	dns->ra = 0;
	dns->z = 0;
	dns->rcode = 0;
	dns->qd_count = htons(1);
	dns->an_count = 0;
	dns->ns_count = 0;
	dns->ar_count = 0;
}

void dns_build_questions(struct dns_packet *dns, const char *domain, int *dns_len)
{
	char *iter = strdup(domain);
	char *domain_part;
	char *question = (char *)dns + sizeof(struct dns_packet);
	uint8_t domain_part_len;

	while ((domain_part = strsep(&iter, ".")))
	{
		domain_part_len = strlen(domain_part);
		*question++ = domain_part_len;
		strcpy(question, domain_part);
		question += domain_part_len;
		*dns_len += domain_part_len + 1;
	}

	*question++ = 0;
	struct dns_question_spec *question_spec = (struct dns_question_spec *)question;
	question_spec->qtype = htons(1);
	question_spec->qclass = htons(1);

	*dns_len += 1 + sizeof(struct dns_question_spec);
}

void dns_traverse_name(uint8_t **name)
{
	while (true)
	{
		if (**name >= 64)
		{
			*name += 2;
			break;
		}
		else
			*name += 1 + **name;

		if (**name == 0)
		{
			*name += 1;
			break;
		}
	}
}

void dns_parse_answers(struct dns_packet *dns, uint32_t *remote_ip)
{
	uint8_t *buf = dns->payload;
	// skip questions
	for (int qi = 0; qi < htons(dns->qd_count); ++qi)
	{
		dns_traverse_name(&buf);
		buf += sizeof(struct dns_question_spec);
	}
	// answers
	for (int ai = 0; ai < htons(dns->an_count); ++ai)
	{
		dns_traverse_name(&buf);
		struct dns_answer_spec *answer_spec = (struct dns_answer_spec *)buf;
		if (htons(answer_spec->atype) == DNS_A_RECORD && htons(answer_spec->aclass) == 1)
		{
			memcpy(remote_ip, answer_spec->rdata, 4);
			*remote_ip = htonl(*remote_ip);
		}
		buf += sizeof(struct dns_answer_spec) + htons(answer_spec->rd_length);
	}
}

// NOTE: MQ 2020-06-13
// create a udp socket
//  -> bind to local ip/address
//  -> connect to dns server ip/address (from dhcp)
// build dns header
// convert domain into question name -> build dns question
// link dns header and dns question -> send udp + dns
// parse response -> dns header
// skip dns question?
// each answer -> check A record -> IP address
void getaddrinfo(const char *domain, uint32_t *ip)
{
	int fd = sys_socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	struct socket *sock = sockfd_lookup(fd);
	struct net_device *dev = get_current_net_device();

	struct sockaddr_in local_in;
	local_in.sin_addr = dev->local_ip;
	local_in.sin_port = 53;
	sock->ops->bind(sock, (struct sockaddr *)&local_in, sizeof(struct sockaddr_in));

	struct sockaddr_in remote_in;
	remote_in.sin_addr = dev->dns_server_ip;
	remote_in.sin_port = 53;
	sock->ops->connect(sock, (struct sockaddr *)&remote_in, sizeof(struct sockaddr_in));

	struct dns_packet *dns = kcalloc(1, MAX_DNS_LEN);
	int dns_len = sizeof(struct dns_packet);
	dns_build_header(dns, rand());
	dns_build_questions(dns, domain, &dns_len);

	sock->ops->sendmsg(sock, dns, dns_len);

	memset(dns, 0, MAX_DNS_LEN);
	sock->ops->recvmsg(sock, dns, MAX_DNS_LEN);
	dns_parse_answers(dns, ip);
	DEBUG &&debug_println(DEBUG_INFO, "[dns]: %s - %d.%d.%d.%d", domain, *ip >> 24, (*ip >> 16) & 0xff, (*ip >> 8) & 0xff, *ip & 0xff);
}
