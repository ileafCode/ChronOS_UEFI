
#ifndef _KERNEL_ETHERNET_H
#define _KERNEL_ETHERNET_H

#include <stdint.h>

#define ETHERNET_PROTOCOL_IPV4 0x0800
#define ETHERNET_HW_TYPE 1

typedef struct ethernet_dev {
	char name[12];
	uint8_t mac[6];
	void (*send_packet)(const void*, int);
} __attribute__((packed)) ethernet_dev_t;

typedef struct ethernet_packet {
	uint8_t destination[6];
	uint8_t source[6];
	uint16_t type;
	uint8_t payload[];
} __attribute__((packed)) __attribute__((aligned(2))) ethernet_packet_t;

void ethernet_send_packet(ethernet_dev_t *device, uint8_t *destination, void *data, int length, uint16_t protocol);
ethernet_dev_t *ethernet_register_device(char *name, uint8_t *mac_address, void (*send_packet)(const void*, int));

#endif