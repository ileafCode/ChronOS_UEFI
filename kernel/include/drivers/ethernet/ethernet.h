
#ifndef _KERNEL_ETHERNET_H
#define _KERNEL_ETHERNET_H

#include <stdint.h>

typedef struct ethernet_packet {
	uint8_t destination[6];
	uint8_t source[6];
	uint16_t type;
	uint8_t payload[];
} __attribute__((packed)) __attribute__((aligned(2))) ethernet_packet_t;

void ethernet_send_packet(uint8_t *destination, void *data, int length, uint16_t protocol);

#endif