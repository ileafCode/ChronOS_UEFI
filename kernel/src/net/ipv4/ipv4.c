#include <net/ipv4/ipv4.h>
#include <net/net.h>
#include <logging/logging.h>

void ipv4_send(ethernet_dev_t *device, ipv4_packet_t *data) {

}

void ipv4_handle(ethernet_dev_t *device, ipv4_packet_t *data, int length) {
    if (length < sizeof(ipv4_packet_t)) {
        return; // Packet is too small to process
    }

    log_info("IPV4", "A packet of type %08d is coming in", data->protocol);
}