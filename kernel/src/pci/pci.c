#include <pci/pci.h>
#include <mm/vmm/paging.h>
#include <apic/lapic.h>
#include <logging/logging.h>

// All drivers
#include <drivers/edu_qemu/edu.h>
#include <drivers/ethernet/e1000/e1000.h>

uint32_t pci_find_capability(pci_hdr0_t *pciDeviceHeader, uint8_t capID) {
    uint8_t ptr = pciDeviceHeader->capabilities_ptr;
    while (ptr) {
        uint64_t capHeaderAddr = (uint64_t)pciDeviceHeader + ptr;
        uint8_t *capHeader = (uint8_t *)capHeaderAddr;

        if (capHeader[0] == capID) {
            return capHeaderAddr;
        }

        ptr = capHeader[1]; // Next capability pointer
    }
    return 0; // Capability not found
}

void pci_enable_msi(pci_dev_hdr_t *pciDeviceHeader, uint8_t vector) {
    // Find the MSI Capability
    // 0x05 is the ID for MSI
    volatile uint32_t msiCapAddr = pci_find_capability((pci_hdr0_t *)pciDeviceHeader, 0x05);
    if (!msiCapAddr) {
        printk("MSI capability not found for device @ %lx\n", pciDeviceHeader);
        return;
    }
    
    volatile uint16_t *msiControl = (uint16_t *)((uint64_t)(msiCapAddr + 2));
    uint16_t msiControlValue = *msiControl;

    int is64Bit = (msiControlValue & (1 << 7)) ? 1 : 0;

    *msiControl |= 1;

    volatile uint32_t *msiAddr = (uint32_t *)(msiCapAddr + 4);
    volatile uint16_t *msiData = (uint16_t *)(msiCapAddr + 8);

    *msiAddr = get_lapic_addr() | (get_lapic_id() << 12);
    *msiData = vector;

    if (is64Bit) {
        volatile uint32_t *msiAddrHi = (uint32_t *)(msiCapAddr + 8);
        volatile uint16_t *msiDataHi = (uint16_t *)(msiCapAddr + 12);
        *msiAddrHi = 0;
        *msiDataHi = vector;
    }
}

uint64_t current_bus, current_device, current_func;

void enum_function(uint64_t deviceAddress, uint64_t function) {
    uint64_t offset = function << 12;
    uint64_t functionAddress = deviceAddress + offset;
    paging_map(
        (void *)((uint64_t)functionAddress & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)functionAddress & 0xFFFFFFFFFFFFF000),
        NULL
    );

    current_func = function;

    pci_dev_hdr_t *pciDeviceHeader = (pci_dev_hdr_t *)functionAddress;
    if (pciDeviceHeader->device_id == 0)
        return;
    if (pciDeviceHeader->device_id == 0xFFFF)
        return;

    //log_info("PCI", "Vendor: %4x; Device: %4x",
    //    pciDeviceHeader->vendor_id,
    //    pciDeviceHeader->device_id
    //);
    
    // Broad devices
    switch (pciDeviceHeader->class) {
    case 0x0C: // Serial Bus Controller
        switch (pciDeviceHeader->subclass) {
        case 0x03: // USB Controller
            switch (pciDeviceHeader->prog_if) {
            case 0x00: // UHCI Controller
                log_info("PCI", "Found a UHCI controller");
                break;
            }
        }
        break;
    case 0x01: // Mass Storage Controller
        switch (pciDeviceHeader->subclass) {
        case 0x06: // Serial ATA Controller
            switch (pciDeviceHeader->prog_if) {
            case 0x01: // AHCI 1.0
                log_info("PCI", "Found an AHCI controller");
                break;
            }
        }
        break;
    }

    // Specific devices
    if (pciDeviceHeader->vendor_id == 0x1234 &&
        pciDeviceHeader->device_id == 0x11E8) { // The EDU device in QEMU
        pci_enable_msi(pciDeviceHeader, 0xA0);
        dev_edu_init((pci_hdr0_t *)pciDeviceHeader);
    }
    if (pciDeviceHeader->vendor_id == 0x8086 &&
        pciDeviceHeader->device_id == 0x100E) { // e1000 for QEMU
        dev_e1000_init((pci_hdr0_t *)pciDeviceHeader, current_bus, current_device, current_func);
    }
}

void enum_dev(uint64_t busAddress, uint32_t device) {
    uint32_t offset = device << 15;

    uint64_t deviceAddress = busAddress + offset;
    paging_map(
        (void *)((uint64_t)deviceAddress & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)deviceAddress & 0xFFFFFFFFFFFFF000),
        NULL
    );

    current_device = (uint64_t)device;

    pci_dev_hdr_t *pciDeviceHeader = (pci_dev_hdr_t *)deviceAddress;

    if (pciDeviceHeader->device_id == 0)
        return;
    if (pciDeviceHeader->device_id == 0xFFFF)
        return;

    for (uint64_t function = 0; function < 8; function++)
        enum_function(deviceAddress, function);
}

void enum_bus(uint64_t baseAddress, uint32_t bus) {
    uint64_t offset = bus << 20;
    uint64_t busAddress = baseAddress + offset;

    paging_map(
        (void *)((uint64_t)busAddress & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)busAddress & 0xFFFFFFFFFFFFF000),
        NULL
    );

    current_bus = (uint64_t)bus;

    pci_dev_hdr_t *pciDeviceHeader = (pci_dev_hdr_t *)busAddress;

    if (pciDeviceHeader->device_id == 0)
        return;
    if (pciDeviceHeader->device_id == 0xFFFF)
        return;

    for (uint32_t device = 0; device < 32; device++)
        enum_dev(busAddress, device);
}

void pci_init() {
    mcfg_t *mcfg = (mcfg_t *)acpi_find_table("MCFG");
    if (!mcfg)
        return;
    
    int entries = ((mcfg->h.len) - sizeof(mcfg_t)) / sizeof(dev_cfg_t);
    for (int t = 0; t < entries; t++) {
        dev_cfg_t *dev_cfg = (dev_cfg_t *)((uint64_t)mcfg +
            sizeof(mcfg_t) + (sizeof(dev_cfg_t) * t));

        for (uint64_t bus = dev_cfg->start_bus; bus < dev_cfg->end_bus; bus++)
            enum_bus(dev_cfg->base_addr, bus);
    }

    log_ok("PCI", "PCI initialized");
}