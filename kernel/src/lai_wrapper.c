#include <lai/host.h>
#include <mm/pmm/pmm.h>
#include <mm/vmm/paging.h>
#include <printk/printk.h>
#include <acpi/acpi.h>
#include <utils/panic.h>
#include <shmall_wrapper.h>
#include <logging/logging.h>
#include <string/string.h>
#include <timers/hpet/hpet.h>
#include <io/io.h>

void *laihost_malloc(size_t size) {
    return kmalloc(size);
}

void laihost_free(void *ptr, size_t size) {
    kfree(ptr);
}

void *laihost_realloc(void *ptr, size_t newsize, size_t oldsize) {
    if (ptr)
        kfree(ptr);
    return kmalloc(newsize);
}

void laihost_log(int type, const char *str) {
    log_info("LAI", "%s", str);
}

void laihost_panic(const char *msg) {
    kernel_panic((char *)msg);
    printk("from LAI\n");
    while (1);
}

void *laihost_scan(const char *signature, size_t idx) {
    return acpi_find_nth_table(signature, idx);
}

void *laihost_map(size_t addr, size_t count) {
    for (int i = 0; i < count; i++) {
        paging_map(
            (void *)((uint64_t)(addr + (i * 0x1000)) & 0xFFFFFFFFFFFFF000),
            (void *)((uint64_t)(addr + (i * 0x1000)) & 0xFFFFFFFFFFFFF000),
            PAGE_NORMAL
        );
    }
}

void laihost_unmap(void *addr, size_t count) {
    for (int i = 0; i < count; i++) {
        paging_unmap(
            (void *)((uint64_t)(addr + (i * 0x1000)) & 0xFFFFFFFFFFFFF000)
        );
    }
}

void laihost_outb(uint16_t p, uint8_t i) {
    outb(p, i);
}
void laihost_outw(uint16_t p, uint16_t i) {
    outw(p, i);
}
void laihost_outd(uint16_t p, uint32_t i) {
    outl(p, i);
}
uint8_t laihost_inb(uint16_t p) {
    return inb(p);
}
uint16_t laihost_inw(uint16_t p) {
    return inw(p);
}
uint32_t laihost_ind(uint16_t p) {
    return inl(p);
}

void laihost_sleep(uint64_t ms) {
    hpet_usleep(ms * 1000);
}

uint64_t laihost_timer(void) {
    return hpet_get_ticks() / 10;
}

#define CONFIG_ADDRESS 0xCF8
#define CONFIG_DATA    0xCFC

uint8_t laihost_pci_readb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    if (offset >= 256) {
        return 0xFF;
    }
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(CONFIG_ADDRESS, address);
    uint32_t data = inl(CONFIG_DATA);
    return (uint8_t)((data >> ((offset & 3) * 8)) & 0xFF);
}

uint16_t laihost_pci_readw(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    if (offset >= 256 || (offset & 1)) {
        return 0xFFFF;
    }
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(CONFIG_ADDRESS, address);
    uint32_t data = inl(CONFIG_DATA);
    return (uint16_t)((data >> ((offset & 2) * 8)) & 0xFFFF);
}

uint32_t laihost_pci_readd(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset) {
    if (offset >= 256 || (offset & 3)) {
        return 0xFFFFFFFF;
    }
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(CONFIG_ADDRESS, address);
    return inl(CONFIG_DATA);
}


void laihost_pci_writeb(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint8_t val) {
    if (offset >= 256) {
        return;
    }
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(CONFIG_ADDRESS, address);
    uint32_t data = inl(CONFIG_DATA);
    data &= ~(0xFF << ((offset & 3) * 8));
    data |= ((uint32_t)val << ((offset & 3) * 8));
    outl(CONFIG_DATA, data);
}

void laihost_pci_writew(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint16_t val) {
    if (offset >= 256 || (offset & 1)) {
        return;
    }
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(CONFIG_ADDRESS, address);
    uint32_t data = inl(CONFIG_DATA);
    data &= ~(0xFFFF << ((offset & 2) * 8));
    data |= ((uint32_t)val << ((offset & 2) * 8));
    outl(CONFIG_DATA, data);
}

void laihost_pci_writed(uint16_t seg, uint8_t bus, uint8_t slot, uint8_t fun, uint16_t offset, uint32_t val) {
    if (offset >= 256 || (offset & 3)) {
        return;
    }
    uint32_t address = (uint32_t)((bus << 16) | (slot << 11) |
                                  (fun << 8) | (offset & 0xFC) | ((uint32_t)0x80000000));
    outl(CONFIG_ADDRESS, address);
    outl(CONFIG_DATA, val);
}
