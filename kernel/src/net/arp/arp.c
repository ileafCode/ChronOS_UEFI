#include <net/arp/arp.h>
#include <net/net.h>
#include <logging/logging.h>
#include <string/string.h>
#include <drivers/ethernet/ethernet.h>
#include <drivers/ethernet/e1000/e1000.h>

void arp_handle(ethernet_dev_t *device, arp_packet_t *data) {
    char dest_hw_address[6];
    char dest_protocol_address[4];
    memcpy(dest_hw_address, data->src_hardware_addr, 6);
    memcpy(dest_protocol_address, data->src_protocol_addr, 4);

    if (ntohs(data->protocol) == ETHERNET_PROTOCOL_IPV4) {
        if (ntohs(data->hardware_type) == ARP_REQUEST) {
            log_info("ARP", "Got ARP_REQUEST. Sending data...");
            memcpy(data->src_hardware_addr, device->mac, 6);
        
            // Set IP to 192.168.1.100
            data->src_protocol_addr[0] = 192;
            data->src_protocol_addr[1] = 168;
            data->src_protocol_addr[2] = 1;
            data->src_protocol_addr[3] = 100;

            memcpy(data->dst_hardware_addr, dest_hw_address, 6);
            memcpy(data->dst_protocol_addr, dest_protocol_address, 4);

            data->opcode = htons(ARP_REPLY);

            data->hardware_addr_len = 6;
            data->protocol_addr_len = 4;

            data->hardware_type = htons(ETHERNET_HW_TYPE);

            data->protocol = htons(ETHERNET_PROTOCOL_IPV4);

            ethernet_send_packet(device, dest_hw_address, data, sizeof(arp_packet_t), 0x806);
            log_info("ARP", "Data is sent!");
        } else if (ntohs(data->hardware_type) == ARP_REPLY) {
            log_info("ARP", "Got ARP_REPLY from:");
            log_info("ARP", "MAC address: %2x:%2x:%2x:%2x:%2x:%2x",
                data->src_hardware_addr[0], data->src_hardware_addr[1], data->src_hardware_addr[2],
                data->src_hardware_addr[3], data->src_hardware_addr[4], data->src_hardware_addr[5]);
        }
    }
}