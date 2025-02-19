
#include <acpi/acpi.h>
#include <mm/vmm/paging.h>
#include <logging/logging.h>
#include <string/string.h>

rsdp_t *rsdp;
rsdt_t *rsdt;

sdt_hdr_t *acpi_find_table(const char *signature) {
    if (strlen(signature) != 4) {
        return NULL;
    }

    int entries = (rsdt->h.len - sizeof(rsdt->h)) / 4;
    for (int i = 0; i < entries; i++) {
        sdt_hdr_t *hdr = (sdt_hdr_t *)(uintptr_t)rsdt->table_ptr[i];
        if (!memcmp(hdr->signature, signature, 4)) {
            return hdr;
        }
    }

    return NULL;
}

void acpi_init(boot_info_t *boot_info) {
    rsdp = (rsdp_t *)boot_info->rsdp;
    printk(" --- RSDP info ---\n");
    printk("    RSDP addr: %lx\n", rsdp);
    printk("    RSDP OEMID: %s\n", rsdp->oemid);
    printk("    RSDP revision: %d\n", rsdp->revision);
    printk("    RSDT addr: %lx\n", rsdp->rsdt_addr);

    rsdt = (rsdt_t *)((uint64_t)rsdp->rsdt_addr);
    if (!rsdt) {
        log_error("ACPI", "No RSDP found. Halting");
        asm volatile ("cli;hlt");
    }

    paging_map((void *)(rsdt), (void *)(rsdt), NULL);
    log_info("ACPI", "Mapped RSDT to memory");

    int entries = (rsdt->h.len - sizeof(rsdt->h)) / 4;
    for (int i = 0; i < entries; i++) {
        sdt_hdr_t *hdr = (sdt_hdr_t *)((uint64_t)rsdt->table_ptr[i]);
        paging_map(
            (void *)((uint64_t)hdr & 0xFFFFFFFFFFFFF000),
            (void *)((uint64_t)hdr & 0xFFFFFFFFFFFFF000), 
            NULL
        );
    }
    log_info("ACPI", "Mapped every table to memory");

    log_ok("ACPI", "ACPI initialized\n");
}