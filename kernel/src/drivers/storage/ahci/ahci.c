#include <drivers/storage/ahci/ahci.h>
#include <logging/logging.h>
#include <mm/vmm/paging.h>
#include <lai/helpers/pci.h>
#include <idt/idt.h>
#include <apic/ioapic.h>

extern void ahci_irq();

hba_mem_t *ahci_abar;

void dev_ahci_reset() {
    // Disable the controller
    ahci_abar->globalHostControl &= ~GHC_AE;

    // Initiate controller reset
    ahci_abar->globalHostControl |= GHC_HR;

    // Wait for reset bit to clear
    while (ahci_abar->globalHostControl & GHC_HR);

    // Enable the AHCI controller
    ahci_abar->globalHostControl |= GHC_AE;

    log_info("AHCI", "Controller has been reset");
}

void dev_ahci_bios_os_handoff() {
    // If AHCI BIOS/OS handoff is supported
    if (ahci_abar->hostCapabilitiesExtended & 0x01) {
        // Set the OS owned semaphore bit
        ahci_abar->biosHandoffCtrlSts |= (1 << 1);

        // Wait for BIOS to clear BIOS owned semaphore
        while (ahci_abar->biosHandoffCtrlSts & (1 << 0));

        // Wait for BIOS busy bit to clear
        while (ahci_abar->biosHandoffCtrlSts & (1 << 4));

        log_info("AHCI", "BIOS/OS handoff complete");
    }
}

void ahci_irq_handler() {

}

void dev_ahci_init(pci_hdr0_t *hdr, uint64_t cur_bus, uint64_t cur_dev, uint64_t cur_func) {
    // Enable "Memory Space" and disable "Interrupt Disable"
    hdr->hdr.command |= (1 << 1);
    hdr->hdr.command &= ~(1 << 10);

    ahci_abar = (hba_mem_t *)((uint64_t)hdr->bar5 & ~3);

    // Map BAR5 as uncachable
    paging_map(
        (void *)((uint64_t)(hdr->bar5 & ~3) & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)(hdr->bar5 & ~3) & 0xFFFFFFFFFFFFF000),
        (enum ptflag[]){ CacheDisabled, End }
    );

    // Perform BIOS/OS handoff
    dev_ahci_bios_os_handoff();

    // Reset controller
    dev_ahci_reset();

    // Set-up interrupts
    acpi_resource_t irq_resource;
    int ret = lai_pci_route(&irq_resource, 0, cur_bus, cur_dev, cur_func);
    if (ret != LAI_ERROR_NONE) {
        log_error("AHCI", "No IRQ pin route found");
        return;
    } else {
        uint8_t irq_num = pci_alloc_int_handler();
        idt_set_gate(ahci_irq, irq_num, IDT_TA_InterruptGate, 0x08);
        ioapic_set_entry(ioapic_remap_irq(irq_resource.base), irq_num);
        log_info("AHCI", "Set int handler to %d", irq_num);
    }

    // Enable AHCI mode and interrupts
    ahci_abar->globalHostControl |= GHC_IE;
    ahci_abar->globalHostControl |= GHC_AE;

    log_ok("AHCI", "AHCI controller initialized");
}