#include <net/arp/arp.h>
#include <net/net.h>
#include <logging/logging.h>
#include <string/string.h>
#include <drivers/ethernet/ethernet.h>
#include <drivers/ethernet/e1000/e1000.h>

void arp_handle(arp_packet_t *data) {
    char dest_hw_address[6];
    char dest_protocol_address[4];
    memcpy(dest_hw_address, data->src_hardware_addr, 6);
    memcpy(dest_protocol_address, data->src_protocol_addr, 4);

    if (ntohs(data->hardware_type) == ARP_REQUEST && ntohs(data->protocol) == 0x800) {
        log_info("ARP", "Got ARP_REQUEST. Sending data...");
        memcpy(data->src_hardware_addr, dev_e1000_get_mac_addr(), 6);
        
        // Set IP to 192.168.8.183
        data->src_protocol_addr[0] = 192;
        data->src_protocol_addr[1] = 168;
        data->src_protocol_addr[2] = 8;
        data->src_protocol_addr[3] = 183;

        memcpy(data->dst_hardware_addr, dest_hw_address, 6);
        memcpy(data->dst_protocol_addr, dest_protocol_address, 4);

        data->opcode = htons(ARP_REPLY);

        data->hardware_addr_len = 6;
        data->protocol_addr_len = 4;

        data->hardware_type = htons(1);

        data->protocol = htons(0x800);

        ethernet_send_packet(dest_hw_address, data, sizeof(arp_packet_t), 0x806);
        log_info("ARP", "Data is sent!");
    }
}