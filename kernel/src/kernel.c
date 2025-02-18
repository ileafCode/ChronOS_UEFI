
#include <terminal/terminal.h>
#include <gdt/gdt.h>
#include <idt/idt.h>
#include <mm/pmm/pmm.h>
#include <mm/vmm/paging.h>
#include <efimem.h>
#include <printk/printk.h>
#include <bootinfo.h>
#include <debug/stacktrace/stacktrace.h>
#include <acpi/acpi.h>
#include <apic/lapic.h>
#include <apic/ioapic.h>
#include <pci/pci.h>
#include <logging/logging.h>

#include <timers/hpet/hpet.h>

extern void enable_sce();
extern void enable_optimizations();

void _start(boot_info_t *boot_info) {
    enable_optimizations();

    terminal_init(boot_info->framebuffer, boot_info->psf1_Font);
    terminal_clear();

    //while(1);
    gdt_init();
    idt_init();

    pmm_init(boot_info->mMap, boot_info->mMapSize, boot_info->mMapDescSize);
    paging_init(boot_info);

    if (boot_info->rsdp == 0) { // Fatal error
        terminal_set_fg_color_palette(9);
        printk("FATAL ERROR");
        terminal_set_fg_color_palette(15);
        printk(": no RSDP/XSDP found!\n");

        printk("This OS does NOT support computers without ACPI.\n");
        asm volatile ("cli;hlt");
    }

    acpi_init(boot_info);

    lapic_init();
    ioapic_init();

    hpet_init();
    pci_init();
    enable_sce();

    terminal_set_fg_color_palette(10);
    printk("\nDone\n\n");
    terminal_set_fg_color_palette(15);

    for (int i = 0; i < 8; i++) {
        terminal_set_fg_color_palette(i);
        terminal_set_bg_color_palette(i + 8);
        printk("# ");
    }
    printk("\n");
    terminal_set_fg_color_palette(15);
    terminal_set_bg_color_palette(0);

    terminal_set_fg_color_palette(15);
    terminal_set_bg_color_palette(0);

    while (1);
}