
#ifndef _KERNEL_NET_H
#define _KERNEL_NET_H

#include <stdint.h>
#include <drivers/ethernet/ethernet.h>

uint16_t ntohs(uint16_t netshort);
uint16_t htons(uint16_t netshort);
uint32_t ntohl(uint32_t netlong);
uint32_t htonl(uint32_t netlong);

void net_handle(ethernet_dev_t *device, ethernet_packet_t *data, int length);
void net_init();

#endif