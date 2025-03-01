#include <drivers/storage/ahci/ahci.h>
#include <logging/logging.h>
#include <mm/vmm/paging.h>
#include <lai/helpers/pci.h>
#include <idt/idt.h>
#include <apic/ioapic.h>
#include <shmall_wrapper.h>
#include <mm/pmm/pmm.h>
#include <timers/hpet/hpet.h>

extern void ahci_irq();

hba_mem_t *ahci_abar;
ahci_port_t *ports[32];
uint8_t portCount;

void dev_ahci_flush_writes(ahci_port_t *port) {
	volatile uint32_t dummy = port->hbaPort->cmdSts;
	dummy = dummy;
}

port_type_t dev_ahci_port_chk_type(hba_port_t *port) {
    uint32_t sataStatus = port->sataStatus;
    uint8_t interfacePowerManagement = (sataStatus >> 8) & 0b111;
    uint8_t deviceDetection = sataStatus & 0b111;

    if (deviceDetection != HBA_PORT_DEV_PRESENT)
        return None;
    if (interfacePowerManagement != HBA_PORT_IPM_ACTIVE)
        return None;
    
    switch (port->signature) {
    case SATA_SIG_ATAPI:
        return SATAPI;
    case SATA_SIG_ATA:
        return SATA;
    case SATA_SIG_PM:
        return PM;
    case SATA_SIG_SEMB:
        return SEMB;
    }
    return None;
}

void dev_ahci_probe_ports() {
    uint32_t portsImplemented = ahci_abar->portsImplemented;
    for (int i = 0; i < 32; i++) {
        if (portsImplemented & (1 << i)) {
            port_type_t portType = dev_ahci_port_chk_type(&ahci_abar->ports[i]);
            if (portType == SATA || portType == SATAPI) {
                ports[portCount] = kmalloc(sizeof(ahci_port_t));
                ports[portCount]->portType = portType;
                ports[portCount]->hbaPort = &ahci_abar->ports[i];
                ports[portCount]->portNumber = portCount;
                portCount++;
            }
        }
    }
}

void dev_ahci_stop_cmd(ahci_port_t *port) {
    port->hbaPort->cmdSts &= ~HBA_PxCMD_ST;
    dev_ahci_flush_writes(port);
    port->hbaPort->cmdSts &= ~HBA_PxCMD_FRE;
    dev_ahci_flush_writes(port);

    while (1) {
        if (port->hbaPort->cmdSts & HBA_PxCMD_FR)
            continue;
        if (port->hbaPort->cmdSts & HBA_PxCMD_CR)
            continue;
        break;
    }
}

void dev_ahci_start_cmd(ahci_port_t *port) {
    while (port->hbaPort->cmdSts & HBA_PxCMD_CR);
    port->hbaPort->cmdSts |= HBA_PxCMD_FRE;
    dev_ahci_flush_writes(port);
    port->hbaPort->cmdSts |= HBA_PxCMD_ST;
    dev_ahci_flush_writes(port);
}

void dev_ahci_port_configure(ahci_port_t *port) {
    dev_ahci_stop_cmd(port);

    void *newBase = pmm_getpage();
    paging_map(
        (void *)((uint64_t)(newBase) & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)(newBase) & 0xFFFFFFFFFFFFF000),
        (enum ptflag[]){ CacheDisabled, WriteThrough, End }
    );
    port->hbaPort->commandListBase = (uint32_t)((uint64_t)newBase);
    dev_ahci_flush_writes(port);
    port->hbaPort->commandListBaseUpper = (uint32_t)((uint64_t)newBase >> 32);
    dev_ahci_flush_writes(port);
    memset((void *)((uintptr_t)port->hbaPort->commandListBase), 0, 1024);

    void *fisBase = pmm_getpage();
    paging_map(
        (void *)((uint64_t)(fisBase) & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)(fisBase) & 0xFFFFFFFFFFFFF000),
        (enum ptflag[]){ CacheDisabled, WriteThrough, End }
    );
    port->hbaPort->fisBaseAddress = (uint32_t)((uint64_t)fisBase);
    dev_ahci_flush_writes(port);
    port->hbaPort->fisBaseAddressUpper = (uint32_t)((uint64_t)fisBase >> 32);
    dev_ahci_flush_writes(port);
    memset(fisBase, 0, 256);

    hba_cmd_hdr_t *cmdHeader = (hba_cmd_hdr_t *)((uint64_t)port->hbaPort->commandListBase + ((uint64_t)port->hbaPort->commandListBaseUpper << 32));

    for (int i = 0; i < 32; i++) {
        cmdHeader[i].prdtLength = 8;

        void *cmdTableAddress = pmm_getpage();
        paging_map(
            (void *)((uint64_t)(cmdTableAddress) & 0xFFFFFFFFFFFFF000),
            (void *)((uint64_t)(cmdTableAddress) & 0xFFFFFFFFFFFFF000),
            (enum ptflag[]){ CacheDisabled, WriteThrough, End }
        );
        uint64_t address = (uint64_t)cmdTableAddress + (i << 8);
        cmdHeader[i].commandTableBaseAddress = (uint32_t)(uint64_t)address;
        cmdHeader[i].commandTableBaseAddressUpper = (uint32_t)((uint64_t)address >> 32);
        memset(cmdTableAddress, 0, 256);
    }

    dev_ahci_start_cmd(port);
}

int dev_ahci_find_cmdslot(hba_port_t *port) {
    uint32_t slots = (port->sataActive | port->commandIssue);
    int cmdslots = (ahci_abar->hostCapability & 0x0f00) >> 8;
    for (int i = 0; i < cmdslots; i++) {
        if ((slots & 1) == 0)
            return i;
        slots >>= 1;
    }
    return -1;
}

int dev_ahci_read(ahci_port_t *port, uint64_t sector, uint32_t sectorCount) {
    uint32_t sectorL = (uint32_t)sector;
    uint32_t sectorH = (uint32_t)(sector >> 32);

    int slot = dev_ahci_find_cmdslot(port->hbaPort);
    if (slot == -1) {
        return -1;
    }

    port->hbaPort->interruptStatus = (uint32_t)-1;
    dev_ahci_flush_writes(port);

    hba_cmd_hdr_t *cmdHeader = (hba_cmd_hdr_t *)((uintptr_t)port->hbaPort->commandListBase);
    cmdHeader += slot;
    cmdHeader->commandFISLength = sizeof(FIS_REG_H2D) / sizeof(uint32_t); // command FIS size;
    cmdHeader->write = 0;
    cmdHeader->prdtLength = 1;

    hba_cmd_table_t *commandTable = (hba_cmd_table_t *)((uintptr_t)cmdHeader->commandTableBaseAddress |
                                                        ((uint64_t)cmdHeader->commandTableBaseAddressUpper << 32));
    memset(commandTable, 0, sizeof(hba_cmd_table_t) + (cmdHeader->prdtLength - 1) * sizeof(hba_prdt_entry_t));
    commandTable += slot;
    commandTable->prdtEntry[0].dataBaseAddress = (uint32_t)(uint64_t)port->buffer;
    commandTable->prdtEntry[0].dataBaseAddressUpper = (uint32_t)((uint64_t)port->buffer >> 32);
    commandTable->prdtEntry[0].byteCount = (sectorCount << 9) - 1; // 512 bytes per sector
    commandTable->prdtEntry[0].interruptOnCompletion = 1;

    FIS_REG_H2D *cmdFIS = (FIS_REG_H2D *)(&commandTable->commandFIS);
    cmdFIS->fisType = FIS_TYPE_REG_H2D;
    cmdFIS->commandControl = 1; // command
    cmdFIS->command = ATA_CMD_READ_DMA_EX;

    cmdFIS->lba0 = (uint8_t)sectorL;
    cmdFIS->lba1 = (uint8_t)(sectorL >> 8);
    cmdFIS->lba2 = (uint8_t)(sectorL >> 16);

    cmdFIS->lba3 = (uint8_t)sectorH;
    cmdFIS->lba4 = (uint8_t)(sectorH >> 8);
    cmdFIS->lba5 = (uint8_t)(sectorH >> 16);

    cmdFIS->deviceRegister = 1 << 6; // LBA mode
    cmdFIS->countLow = sectorCount & 0xFF;
    cmdFIS->countHigh = (sectorCount >> 8) & 0xFF;

    uint64_t spin = 0;
    while ((port->hbaPort->taskFileData & (ATA_DEV_BUSY | ATA_DEV_DRQ)) && spin < 10000000)
        spin++;
    if (spin == 10000000)
        return -1;

    port->hbaPort->commandIssue = 1U << slot;
    dev_ahci_flush_writes(port);

    while (1) {
        if ((port->hbaPort->commandIssue & (1U << slot)) == 0)
			break;
        if (port->hbaPort->interruptStatus & HBA_PxIS_TFES)
            return -1;
        asm volatile ("" ::: "memory");
    }
    return 0;
}

void dev_ahci_reset() {
    // Disable the controller
    ahci_abar->globalHostControl &= ~GHC_AE;
    hpet_usleep(50000); // 50ms

    // Initiate controller reset
    ahci_abar->globalHostControl |= GHC_HR;
    hpet_usleep(50000); // 50ms

    // Wait for reset bit to clear
    while (ahci_abar->globalHostControl & GHC_HR);

    // Enable the AHCI controller
    ahci_abar->globalHostControl |= GHC_AE;
    hpet_usleep(50000); // 50ms

    log_info("AHCI", "Controller has been reset");
}

void dev_ahci_bios_os_handoff() {
    // If AHCI BIOS/OS handoff is supported
    if (ahci_abar->hostCapabilitiesExtended & 0x01) {
        // Set the OS owned semaphore bit
        ahci_abar->biosHandoffCtrlSts |= (1 << 1);
        hpet_usleep(50000); // 50ms

        // Wait for BIOS to clear BIOS owned semaphore
        while (ahci_abar->biosHandoffCtrlSts & (1 << 0));
        hpet_usleep(50000); // 50ms

        // Wait for BIOS busy bit to clear
        while (ahci_abar->biosHandoffCtrlSts & (1 << 4));
        hpet_usleep(50000); // 50ms

        log_info("AHCI", "BIOS/OS handoff complete");
    }
}

void ahci_irq_handler() {
    uint32_t irq_status = ahci_abar->interruptStatus;
    ahci_abar->interruptStatus = irq_status;
}

ahci_port_t *dev_ahci_get_port(int idx) {
    return ports[idx];
}

void dev_ahci_init(pci_hdr0_t *hdr, uint64_t cur_bus, uint64_t cur_dev, uint64_t cur_func) {
    // Enable "Memory Space", "Bus Mastering" and disable "Interrupt Disable"
    hdr->hdr.command |= (1 << 1);
    hdr->hdr.command |= (1 << 2);
    hdr->hdr.command &= ~(1 << 10);

    ahci_abar = (hba_mem_t *)((uint64_t)hdr->bar5 & ~3);

    // Map BAR5 as uncachable
    paging_map(
        (void *)((uint64_t)(hdr->bar5 & ~3) & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)(hdr->bar5 & ~3) & 0xFFFFFFFFFFFFF000),
        (enum ptflag[]){ CacheDisabled, WriteThrough, End }
    );

    // Perform BIOS/OS handoff
    dev_ahci_bios_os_handoff();

    // Reset controller
    // dev_ahci_reset(); // For some reason it clears all the ports to 0 (null)

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
        log_info("AHCI", "Interrupt vector is %x", irq_num);
    }

    // Enable AHCI mode and interrupts
    ahci_abar->globalHostControl |= GHC_IE;
    ahci_abar->globalHostControl |= GHC_AE;

    for (int i = 0; i < 32; i++) {
        ports[i] = NULL;
    }

    // Probe ports
    dev_ahci_probe_ports();

    log_info("AHCI", "Found %d ports", portCount);

    // Initialize all ports
    for (int i = 0; i < portCount; i++) {
        ahci_port_t *port = ports[i];
        port->buffer = (uint8_t *)pmm_getpage();
        memset(port->buffer, 0, 0x1000);
        dev_ahci_port_configure(port);
    }

    log_ok("AHCI", "AHCI controller initialized");
}