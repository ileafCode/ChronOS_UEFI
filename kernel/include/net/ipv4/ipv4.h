#ifndef _KERNEL_NET_IPV4_H
#define _KERNEL_NET_IPV4_H

#include <stdint.h>
#include <drivers/ethernet/ethernet.h>

typedef struct ipv4_packet {
	uint8_t  version_ihl;
	uint8_t  dscp_ecn;
	uint16_t length;
	uint16_t ident;
	uint16_t flags_fragment;
	uint8_t  ttl;
	uint8_t  protocol;
	uint16_t checksum;
	uint32_t source;
	uint32_t destination;
	uint8_t  payload[];
} __attribute__ ((packed)) ipv4_packet_t;

void ipv4_handle(ethernet_dev_t *device, ipv4_packet_t *data, int length);

#endif