#include <stdint.h>

#define DNS_FLAG_QUERY 0
#define DNS_FLAG_RESPONSE 1
#define DNS_STANDARD_QUERY 0
#define DNS_A_RECORD 1
#define DNS_CNAME 5

struct dns_packet
{
	// header
	uint16_t id;
	uint8_t rd : 1;
	uint8_t tc : 1;
	uint8_t aa : 1;
	uint8_t op_code : 4;
	uint8_t qr : 1;
	uint8_t rcode : 4;
	uint8_t z : 3;
	uint8_t ra : 1;
	uint16_t qd_count;
	uint16_t an_count;
	uint16_t ns_count;
	uint16_t ar_count;
	// payload = question + answer + authority + additional
	uint8_t payload[];
};

struct dns_name
{
	uint8_t len;
	char name[];
};

struct dns_question_spec
{
	uint16_t qtype;
	uint16_t qclass;
};

struct dns_answer_spec
{
	uint16_t atype;
	uint16_t aclass;
	uint32_t ttl;
	uint16_t rd_length;
	uint8_t rdata[];
};
