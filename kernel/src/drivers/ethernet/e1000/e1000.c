#include <drivers/ethernet/e1000/e1000.h>
#include <logging/logging.h>
#include <mm/vmm/paging.h>
#include <mm/pmm/pmm.h>
#include <idt/idt.h>
#include <apic/ioapic.h>
#include <io/io.h>
#include <lai/helpers/pci.h>
#include <net/net.h>

uint16_t e1000_io_base;
uint64_t e1000_mem_base;
int e1000_io_type = -1; // 0 = MMIO, 1 = port IO
uint8_t e1000_mac_addr[6];

ethernet_dev_t *e1000_device;

e1000_rx_desc_t *rx_descs[E1000_NUM_RX_DESC];
e1000_tx_desc_t *tx_descs[E1000_NUM_TX_DESC];
uint16_t rx_cur;
uint16_t tx_cur;

int e1000_is_init = 0;

extern void e1000_irq();

uint32_t dev_e1000_read(uint16_t reg) {
    if (e1000_io_type == 0) {
        return *(volatile uint32_t *)(e1000_mem_base + reg);
    } else if (e1000_io_type == 1) {
        outl(e1000_io_base, reg);
        return inl(e1000_io_base + 4);
    } else {
        log_error("E1000", "IO type not initialized (dev_e1000_read)");
        return 0;
    }
}

void dev_e1000_write(uint16_t reg, uint32_t value) {
    if (e1000_io_type == 0) {
        *(volatile uint32_t *)(e1000_mem_base + reg) = value;
    } else if (e1000_io_type == 1) {
        outl(e1000_io_base, reg);
        outl(e1000_io_base + 4, value);
    } else {
        log_error("E1000", "IO type not initialized (dev_e1000_write)");
        return;
    }
}

int dev_e1000_detect_eeprom() {
    uint32_t val = 0;
    dev_e1000_write(REG_EEPROM, 0x1);
    int e1000_eeprom_exists = 0;

    for (int i = 0; i < 1000 && !e1000_eeprom_exists; i++) {
        val = dev_e1000_read(REG_EEPROM);
        if (val & 0x10)
            e1000_eeprom_exists = 1;
        else
            e1000_eeprom_exists = 0;
    }
    return e1000_eeprom_exists;
}

uint32_t dev_e1000_eeprom_read(uint8_t addr) {
    uint16_t data = 0;
    uint32_t temp = 0;
    if (dev_e1000_detect_eeprom()) {
        dev_e1000_write(REG_EEPROM, 1 | ((uint32_t)(addr) << 8));
        while (!((temp = dev_e1000_read(REG_EEPROM)) & (1 << 4)));
    } else {
        dev_e1000_write(REG_EEPROM, 1 | ((uint32_t)(addr) << 2));
        while (!((temp = dev_e1000_read(REG_EEPROM)) & (1 << 1)));
    }
    data = (uint16_t)((temp >> 16) & 0xFFFF);
    return data;
}

int dev_e1000_get_mac() {
    if (dev_e1000_detect_eeprom()) {
        uint32_t temp;
        temp = dev_e1000_eeprom_read(0);
        e1000_mac_addr[0] = temp & 0xFF;
        e1000_mac_addr[1] = temp >> 8;
        temp = dev_e1000_eeprom_read(1);
        e1000_mac_addr[2] = temp & 0xFF;
        e1000_mac_addr[3] = temp >> 8;
        temp = dev_e1000_eeprom_read(2);
        e1000_mac_addr[4] = temp & 0xFF;
        e1000_mac_addr[5] = temp >> 8;
    } else {
        uint8_t *mac8 = (uint8_t *)(e1000_mem_base + 0x5400);
        uint32_t *mac32 = (uint32_t *)(e1000_mem_base + 0x5400);
        if (mac32[0] != 0)
            for (int i = 0; i < 6; i++)
                e1000_mac_addr[i] = mac8[i];
        else
            return 0;
    }
    return 1;
}


void dev_e1000_init_rx() {
    uint8_t *ptr;
    struct e1000_rx_desc *descs;
    ptr = (uint8_t *)pmm_getpage();
    descs = (struct e1000_rx_desc *)ptr;
    for (int i = 0; i < E1000_NUM_RX_DESC; i++) {
        rx_descs[i] = (struct e1000_rx_desc *)((uint8_t *)descs + i * 16);
        // Allocate 3 pages (for 8208 bytes, allocated 12,288 bytes.)
        // Might be using a BIT too much memory
        rx_descs[i]->addr = (uint64_t)pmm_getpage();
        pmm_getpage();
        pmm_getpage();
        rx_descs[i]->status = 0;
    }

    dev_e1000_write(REG_TXDESCLO, (uint32_t)((uint64_t)ptr >> 32));
    dev_e1000_write(REG_TXDESCHI, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));

    dev_e1000_write(REG_RXDESCLO, (uint64_t)ptr);
    dev_e1000_write(REG_RXDESCHI, 0);

    dev_e1000_write(REG_RXDESCLEN, E1000_NUM_RX_DESC * 16);

    dev_e1000_write(REG_RXDESCHEAD, 0);
    dev_e1000_write(REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);
    rx_cur = 0;
    dev_e1000_write(REG_RCTRL,
        RCTL_EN | RCTL_SBP |
        RCTL_UPE | RCTL_MPE |
        RCTL_LBM_NONE | RTCL_RDMTS_HALF |
        RCTL_BAM | RCTL_SECRC |
        RCTL_BSIZE_8192);
}

void dev_e1000_init_tx() {    
    uint8_t *ptr;
    struct e1000_tx_desc *descs;

    ptr = (uint8_t *)pmm_getpage();

    descs = (struct e1000_tx_desc *)ptr;
    for (int i = 0; i < E1000_NUM_TX_DESC; i++) {
        tx_descs[i] = (struct e1000_tx_desc *)((uint8_t*)descs + i*16);
        tx_descs[i]->addr = 0;
        tx_descs[i]->cmd = 0;
        tx_descs[i]->status = TSTA_DD;
    }

    dev_e1000_write(REG_TXDESCHI, (uint32_t)((uint64_t)ptr >> 32) );
    dev_e1000_write(REG_TXDESCLO, (uint32_t)((uint64_t)ptr & 0xFFFFFFFF));

    dev_e1000_write(REG_TXDESCLEN, E1000_NUM_TX_DESC * 16);

    dev_e1000_write(REG_TXDESCHEAD, 0);
    dev_e1000_write(REG_TXDESCTAIL, 0);
    tx_cur = 0;
    dev_e1000_write(REG_TCTRL, TCTL_EN
        | TCTL_PSP
        | (15 << TCTL_CT_SHIFT)
        | (64 << TCTL_COLD_SHIFT)
        | TCTL_RTLC);
    
    //writeCommand(REG_TCTRL, 0b0110000000000111111000011111010);
    //writeCommand(REG_TIPG, 0x0060200A);
}

void dev_e1000_enable_irq() {
    dev_e1000_write(REG_IMASK ,0x1F6DC);
    dev_e1000_write(REG_IMASK ,0xff & ~4);
    dev_e1000_read(0xc0);
}

void dev_e1000_handle_recv() {
    uint16_t old_cur;
    int got_packet = 0;
 
    while ((rx_descs[rx_cur]->status & 0x1)) {
        got_packet = 1;
        uint8_t *buf = (uint8_t *)rx_descs[rx_cur]->addr;
        uint16_t len = rx_descs[rx_cur]->length;

        net_handle(e1000_device, (void *)buf, (int)len);
        
        // Here you should inject the received packet into your network stack
        rx_descs[rx_cur]->status = 0;
        old_cur = rx_cur;
        rx_cur = (rx_cur + 1) % E1000_NUM_RX_DESC;
        dev_e1000_write(REG_RXDESCTAIL, old_cur);
    }    
}

void dev_e1000_linkup() {
	uint32_t val = dev_e1000_read(REG_CTRL);
	dev_e1000_write(REG_CTRL, val | ECTRL_SLU);
}

void dev_e1000_send_data(const void *p_data, int p_len) {    
    tx_descs[tx_cur]->addr = (uint64_t)p_data;
    tx_descs[tx_cur]->length = p_len;
    tx_descs[tx_cur]->cmd = CMD_EOP | CMD_IFCS | CMD_RS;
    tx_descs[tx_cur]->status = 0;
    uint8_t old_cur = tx_cur;
    tx_cur = (tx_cur + 1) % E1000_NUM_TX_DESC;
    dev_e1000_write(REG_TXDESCTAIL, tx_cur);
    while (!(tx_descs[old_cur]->status & 0xff));
}

void e1000_irq_handler() {
    dev_e1000_write(REG_IMASK, 0x1);
    uint32_t status = dev_e1000_read(0xc0);
    if (!e1000_is_init) return;

    if (status & 0x04) {
        dev_e1000_linkup();
    } else if (status & 0x10) {

    } else if (status & 0x80) {
        dev_e1000_handle_recv();
    }
}

void dev_e1000_init(pci_hdr0_t *hdr, uint64_t cur_bus, uint64_t cur_dev, uint64_t cur_func) {
    if (e1000_is_init)
        return;
    
    acpi_resource_t irq_resource;
    int ret = lai_pci_route(&irq_resource, 0, cur_bus, cur_dev, cur_func);
    if (ret != LAI_ERROR_NONE) {
        log_error("E1000", "No IRQ pin route found");
        return;
    } else {
        idt_set_gate(e1000_irq, 0xA1, IDT_TA_InterruptGate, 0x08);
        ioapic_set_entry(ioapic_remap_irq(irq_resource.base), 0xA1);
    }

    if ((hdr->bar0 & 0x01) == 0) { // MMIO
        e1000_mem_base = hdr->bar0 & ~3;
        e1000_io_type = 0;
        hdr->hdr.command |= (1 << 1);

        for (int i = 0; i < 32; i++) {
            paging_map(
                (void *)((uint64_t)(e1000_mem_base + (i * 0x1000)) & 0xFFFFFFFFFFFFF000),
                (void *)((uint64_t)(e1000_mem_base + (i * 0x1000)) & 0xFFFFFFFFFFFFF000),
                (enum ptflag[]){ CacheDisabled, End }
            );
        }
    } else { // Port IO
        e1000_io_base = hdr->bar0 & ~1;
        e1000_io_type = 1;
        hdr->hdr.command |= (1 << 0);
    }

    // Bus mastering
    hdr->hdr.command |= (1 << 2);

    log_info("E1000", "%s @ %lx",
        e1000_io_type == 0 ? "MMIO address" : "Port IO address",
        e1000_io_type == 0 ? e1000_mem_base : e1000_io_base
    );

    log_info("E1000", "Int Line: %d, Int Pin: %d", hdr->interrupt_line, hdr->interrupt_pin);

    if (!dev_e1000_get_mac()) {
        log_error("E1000", "Could not get MAC address.");
    } else {
        log_info("E1000", "MAC address: %2x:%2x:%2x:%2x:%2x:%2x",
            e1000_mac_addr[0], e1000_mac_addr[1], e1000_mac_addr[2],
            e1000_mac_addr[3], e1000_mac_addr[4], e1000_mac_addr[5]);
    }

    dev_e1000_linkup();

    for (int i = 0; i < 0x80; i++) {
        dev_e1000_write(0x5200 + (i * 4), 0);
    }

    dev_e1000_enable_irq();

    dev_e1000_init_rx();
    dev_e1000_init_tx();

    e1000_device = ethernet_register_device("e1000", e1000_mac_addr, dev_e1000_send_data);

    e1000_is_init = 1;
    log_ok("E1000", "E1000 NIC initialized");
}