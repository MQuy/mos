#ifndef _LIBC_NETDB_H
#define _LIBC_NETDB_H

#include <sys/socket.h>

struct hostent
{
	char *h_name;
	char **h_aliases;
	int h_addrtype;
	int h_length;
	char **h_addr_list;
};

struct netent
{
	char *n_name;
	char **n_aliases;
	int n_addrtype;
	uint32_t n_net;
};

struct protoent
{
	char *p_name;
	char **p_aliases;
	int p_proto;
};

struct servent
{
	char *s_name;
	char **s_aliases;
	int s_port;
	char *s_proto;
};

struct addrinfo
{
	int ai_flags;
	int ai_family;
	int ai_socktype;
	int ai_protocol;
	socklen_t ai_addrlen;
	struct sockaddr *ai_addr;
	char *ai_canonname;
	struct addrinfo *ai_next;
};

#endif
