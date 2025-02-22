#include <drivers/ethernet/ethernet.h>
#include <drivers/ethernet/e1000/e1000.h>
#include <net/net.h>
#include <shmall_wrapper.h>
#include <string/string.h>

void ethernet_send_packet(uint8_t *destination, void *data, int length, uint16_t protocol) {
    uint8_t *source_mac = dev_e1000_get_mac_addr();
    ethernet_packet_t *packet = (ethernet_packet_t *)kmalloc(sizeof(ethernet_packet_t) + length);
    void *packet_data = (void*)packet + sizeof(ethernet_packet_t);

    memcpy(packet->destination, destination, 6);
    memcpy(packet->source, source_mac, 6);

    memcpy(packet_data, data, length);

    packet->type = htons(protocol);
    dev_e1000_send_data((const void *)packet, length + sizeof(ethernet_packet_t));

    kfree(packet);
}