
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
#include <utils/panic.h>
#include <shmall_wrapper.h>
#include <lai/helpers/sci.h>
#include <timers/hpet/hpet.h>
#include <net/net.h>
#include <fs/fat/ff.h>
#include <lai/helpers/pm.h>
#include <process/process.h>
#include <io/io.h>

extern void enable_sce();
extern void enable_optimizations();

FRESULT list_dir(const char *path)
{
    FRESULT res;
    DIR dir;
    FILINFO fno;
    int nfile, ndir;

    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        nfile = ndir = 0;
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Error or end of dir */
            if (fno.fattrib & AM_DIR) {            /* Directory */
                printk("     <DIR> %s\n", fno.fname);
                ndir++;
            } else {                               /* File */
                printk("%10u %s\n", fno.fsize, fno.fname);
                nfile++;
            }
        }
        f_closedir(&dir);
        printk("%d dirs, %d files.\n", ndir, nfile);
    } else {
        printk("Failed to open \"%s\". (%u)\n", path, res);
    }
    return res;
}

__attribute__((force_align_arg_pointer))
void _start(boot_info_t *boot_info) {
    enable_optimizations();

    terminal_init(boot_info->framebuffer, boot_info->psf1_Font);
    terminal_clear();

    gdt_init();
    idt_init();

    pmm_init(boot_info->mMap, boot_info->mMapSize, boot_info->mMapDescSize);
    paging_init(boot_info);

    if (boot_info->rsdp == 0) { // Fatal error
        kernel_panic("No RSDP/XSDP found.");
        printk("This OS does not support computers without ACPI.\n");
        asm volatile ("cli;hlt");
    }

    heap_init();

    acpi_init(boot_info);

    lapic_init();
    ioapic_init();

    hpet_init();

    lai_enable_acpi(1);

    net_init();
    pci_init();

    process_init();

    enable_sce();
    
    terminal_set_fg_color_palette(10);
    printk("\nDone\n\n");
    terminal_set_fg_color_palette(15);

    for (int i = 0; i < 8; i++) {
        terminal_set_fg_color_palette(i);
        terminal_set_bg_color_palette(i + 8);
        printk("# ");
    }
    terminal_set_fg_color_palette(15);
    terminal_set_bg_color_palette(0);

    printk("\n");

    ioapic_set_entry(ioapic_remap_irq(1), 0x21);

    int i = 0;

    while (1) {
        //outb(0xE9, 'K');
        printk("Kernel: %d\n", i++);
    }
}