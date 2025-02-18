#include <pci/pci.h>
#include <mm/vmm/paging.h>
#include <apic/lapic.h>
#include <printk/printk.h>

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
        printk("MSI capability not found for device @ %p\n", pciDeviceHeader);
        return;
    }

    // Read MSI Control Register
    volatile uint16_t *msiControl = (uint16_t *)((uint64_t)(msiCapAddr + 2));
    volatile uint16_t msiEnableBit = 1 << 0;

    // Enable MSI
    *msiControl |= msiEnableBit;

    // Set MSI Message Address and Data (Example configuration)
    volatile uint32_t *msiAddr = (uint32_t *)((uint64_t)(msiCapAddr + 4));
    volatile uint16_t *msiData = (uint16_t *)((uint64_t)(msiCapAddr + 8));

    *msiAddr = get_lapic_addr();
    *msiData = vector;
}

void enum_function(uint64_t deviceAddress, uint64_t function) {
    uint64_t offset = function << 12;
    uint64_t functionAddress = deviceAddress + offset;
    paging_map(
        (void *)((uint64_t)functionAddress & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)functionAddress & 0xFFFFFFFFFFFFF000),
        NULL
    );

    pci_dev_hdr_t *pciDeviceHeader = (pci_dev_hdr_t *)functionAddress;
    if (pciDeviceHeader->device_id == 0)
        return;
    if (pciDeviceHeader->device_id == 0xFFFF)
        return;

    printk("Vendor: %x; Device: %x\n",
        pciDeviceHeader->vendor_id,
        pciDeviceHeader->device_id);

    switch (pciDeviceHeader->class) {
    case 0x0C: // Serial Bus Controller
        switch (pciDeviceHeader->subclass) {
        case 0x03: // USB Controller
            switch (pciDeviceHeader->prog_if) {
            case 0x00: // UHCI Controller
                break;
            }
        }
        break;
    case 0x01: // Mass Storage Controller
        switch (pciDeviceHeader->subclass) {
        case 0x06: // Serial ATA Controller
            switch (pciDeviceHeader->prog_if) {
            case 0x01: // AHCI 1.0
                break;
            }
        }
        break;
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
}