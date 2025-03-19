
#include <acpi/acpi.h>
#include <mm/vmm/paging.h>
#include <logging/logging.h>
#include <string/string.h>
#include <lai/core.h>
#include <lai/helpers/sci.h>
#include <utils/panic.h>
#include <idt/idt.h>
#include <apic/ioapic.h>
#include <process/process.h>

rsdp_t *rsdp;

rsdt_t *rsdt;
xsdt_t *xsdt; // NULL if not supported

// TODO fix xsdt support and try on a real machine

extern void sci_irq();
void sci_handler() {
    log_info("ACPI", "SCI interrupt happened!... What now?\n");
}

sdt_hdr_t *acpi_find_table(const char *signature) {
    if (strlen(signature) != 4) {
        return NULL;
    }

    int entries;
    if (xsdt) {
        entries = (xsdt->h.len - sizeof(xsdt->h)) / 8;
    } else {
        entries = (rsdt->h.len - sizeof(rsdt->h)) / 4;
    }

    for (int i = 0; i < entries; i++) {
        sdt_hdr_t *hdr;// = (sdt_hdr_t *)(uintptr_t)rsdt->table_ptr[i];
        if (xsdt) {
            hdr = (sdt_hdr_t *)(uintptr_t)xsdt->table_ptr[i];
        } else {
            hdr = (sdt_hdr_t *)(uintptr_t)rsdt->table_ptr[i];
        }

        if (!memcmp(hdr->signature, signature, 4)) {
            return hdr;
        }
    }

    return NULL;
}

sdt_hdr_t *acpi_find_nth_table(const char *signature, int idx) {
    if (strlen(signature) != 4) {
        return NULL;
    }

    int entries;
    if (xsdt) {
        entries = (xsdt->h.len - sizeof(xsdt->h)) / 8;
    } else {
        entries = (rsdt->h.len - sizeof(rsdt->h)) / 4;
    }

    for (int i = 0; i < entries; i++) {
        sdt_hdr_t *hdr;// = (sdt_hdr_t *)(uintptr_t)rsdt->table_ptr[i];
        if (xsdt) {
            hdr = (sdt_hdr_t *)(uintptr_t)xsdt->table_ptr[i];
        } else {
            hdr = (sdt_hdr_t *)(uintptr_t)rsdt->table_ptr[i];
        }
        
        if (memcmp(hdr->signature, signature, 4) == 0) {
            if (idx == 0) {
                return hdr;
            }
            idx--;
        }
    }

    return NULL;
}

void acpi_init(boot_info_t *boot_info) {
    rsdp = (rsdp_t *)boot_info->rsdp;

    if (rsdp->revision > 0) {
        xsdt = (xsdt_t *)((uint64_t)rsdp->xsdt_addr);
        paging_map((void *)(xsdt), (void *)(xsdt), PAGE_NORMAL);
        log_info("ACPI", "Mapped XSDT to memory");
    } else {
        rsdt = (rsdt_t *)((uint64_t)rsdp->rsdt_addr);
        paging_map((void *)(rsdt), (void *)(rsdt), PAGE_NORMAL);
        log_info("ACPI", "Mapped RSDT to memory");
    }

    int entries;// = (rsdt->h.len - sizeof(rsdt->h)) / 4;
    if (xsdt) {
        entries = (xsdt->h.len - sizeof(xsdt->h)) / 8;
    } else {
        entries = (rsdt->h.len - sizeof(rsdt->h)) / 4;
    }

    for (int i = 0; i < entries; i++) {
        sdt_hdr_t *hdr = (sdt_hdr_t *)((uint64_t)rsdt->table_ptr[i]);
        paging_map(
            (void *)((uint64_t)hdr & 0xFFFFFFFFFFFFF000),
            (void *)((uint64_t)hdr & 0xFFFFFFFFFFFFF000), 
            PAGE_NORMAL
        );
    }
    log_info("ACPI", "Mapped every table to memory");

    acpi_fadt_t *fadt = (acpi_fadt_t *)acpi_find_table("FACP");
    if (!fadt) {
        kernel_panic("No FADT");
        printk("From acpi.c\n");
        while (1);
    }

    idt_set_gate(sci_irq, 0x40, IDT_TA_InterruptGate, 0x08);
    ioapic_set_entry(ioapic_remap_irq(fadt->sci_irq), 0x40);

    lai_set_acpi_revision(rsdp->revision);
    lai_create_namespace();

    log_ok("ACPI", "ACPI initialized");
}