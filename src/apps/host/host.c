#include "host.h"

#include <arpa/inet.h>
#include <if_ether.h>
#include <math.h>
#include <sockios.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAX_DNS_LEN 512

uint32_t ioctl_device_addr(int type)
{
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	ioctl(fd, type, (unsigned long)&ifr);
	close(fd);

	return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
}

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

void getaddrinfo(const char *domain, uint32_t *ip)
{
	int fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	struct sockaddr_in local_in;
	local_in.sin_addr = ioctl_device_addr(SIOCGIFADDR);
	local_in.sin_port = 53;
	bind(fd, (struct sockaddr *)&local_in, sizeof(struct sockaddr_in));

	struct sockaddr_in remote_in;
	remote_in.sin_addr = ioctl_device_addr(SIOCGIFDNSADDR);
	remote_in.sin_port = 53;
	connect(fd, (struct sockaddr *)&remote_in, sizeof(struct sockaddr_in));

	struct dns_packet *dns = calloc(1, MAX_DNS_LEN);
	int dns_len = sizeof(struct dns_packet);
	dns_build_header(dns, rand());
	dns_build_questions(dns, domain, &dns_len);

	send(fd, dns, dns_len);

	memset(dns, 0, MAX_DNS_LEN);
	recv(fd, dns, MAX_DNS_LEN);
	dns_parse_answers(dns, ip);
}

int main(int argc, char *argv[])
{
	char *domain = argv[1];

	uint32_t ip;
	getaddrinfo(domain, &ip);

	char text_ip[16] = {0};
	inet_ntop(ip, text_ip, sizeof(text_ip));
	write(1, text_ip, sizeof(text_ip) - 1);
	write(1, "\n", 1);

	return 0;
}
