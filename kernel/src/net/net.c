#include <net/net.h>
#include <logging/logging.h>
#include <net/arp/arp.h>
#include <string/string.h>
#include <drivers/ethernet/e1000/e1000.h>
#include <net/ipv4/ipv4.h>

/* Swap the bytes of a 16-bit value */
static inline uint16_t swap16(uint16_t x) {
    return (x >> 8) | (x << 8);
}

/* Swap the bytes of a 32-bit value */
static inline uint32_t swap32(uint32_t x) {
    return ((x >> 24) & 0xff) |
           ((x >> 8)  & 0xff00) |
           ((x << 8)  & 0xff0000) |
           ((x << 24) & 0xff000000);
}

uint16_t ntohs(uint16_t netshort) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return swap16(netshort);
#else
    return netshort;
#endif
}

uint16_t htons(uint16_t hostshort) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return swap16(hostshort);
#else
    return hostshort;
#endif
}

uint32_t ntohl(uint32_t netlong) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return swap32(netlong);
#else
    return netlong;
#endif
}

uint32_t htonl(uint32_t hostlong) {
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    return swap32(hostlong);
#else
    return hostlong;
#endif
}

uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void net_handle(ethernet_dev_t *device, ethernet_packet_t *data, int length) {
    if (!memcmp(data->destination, device->mac, 6) || !memcmp(data->destination, broadcast_mac, 6)) {
        if (ntohs(data->type) == 0x806) { // ARP
            arp_handle(device, (arp_packet_t *)(&data->payload));
        } else if (ntohs(data->type) == 0x800) {
            ipv4_handle(device, (ipv4_packet_t *)(&data->payload), length);
            //log_info("NET", "Other protocol: %x", ntohs(data->type));
        }
    }
    //arp_recieve_data((arp_packet_t *)data, length);
}

void net_init() {
    //log_ok("NET", "Networking initialized");
}