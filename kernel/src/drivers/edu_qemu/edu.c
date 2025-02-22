#include <drivers/edu_qemu/edu.h>
#include <logging/logging.h>
#include <mm/vmm/paging.h>
#include <idt/idt.h>

volatile void *edu_memory;

extern void edu_irq();

uint32_t dev_edu_read(uint32_t reg_offset) {
    return *(volatile uint32_t *)((uint64_t)(edu_memory) + reg_offset);
}

void dev_edu_write(uint32_t reg_offset, uint32_t data) {
    *(volatile uint32_t *)((uint64_t)(edu_memory) + reg_offset) = data;
}

uint64_t dev_edu_read64(uint32_t reg_offset) {
    return *(volatile uint64_t *)((uint64_t)(edu_memory) + reg_offset);
}

void dev_edu_write64(uint32_t reg_offset, uint64_t data) {
    *(volatile uint64_t *)((uint64_t)(edu_memory) + reg_offset) = data;
}

int edu_interrupt = 0;
void edu_irq_handler() {
    uint32_t status = dev_edu_read(EDU_INT_STATUS_REG);
    log_info("EDU", "irq status: %d", status);
    dev_edu_write(EDU_INT_ACK_REG, status);
    edu_interrupt = 1;
}

uint32_t dev_edu_factorial(int fact) {
    dev_edu_write64(EDU_INT_RAISE_REG, (uint64_t)edu_memory + EDU_STATUS_REG);
    dev_edu_write(EDU_FACTORIAL_REG, fact);

    while ((dev_edu_read(EDU_STATUS_REG) & EDU_STATUS_COMPUTING));
    return dev_edu_read(EDU_FACTORIAL_REG);
}

int dev_edu_is_live() {
    dev_edu_write(EDU_LIVENESS_REG, 2);
    int alive_value = (int)dev_edu_read(EDU_LIVENESS_REG);
    if (alive_value == (~2)) 
        return 1; // Is alive
    return 0; // Is not alive
}

void dev_edu_do_intr(uint32_t val) {
    dev_edu_write(EDU_INT_RAISE_REG, val);
}

void dev_edu_wait_intr() {
    while (edu_interrupt == 0)
        log_info("EDU", "lol ");
    edu_interrupt = 0;
}

void dev_edu_init(pci_hdr0_t *hdr) {
    paging_map(
        (void *)((uint64_t)hdr->bar0 & 0xFFFFFFFFFFFFF000),
        (void *)((uint64_t)hdr->bar0 & 0xFFFFFFFFFFFFF000),
        NULL
    );

    edu_memory = (volatile uint32_t *)((uint64_t)hdr->bar0);

    log_info("EDU", "Testing the EDU...");

    // Doing a few tests
    if (!dev_edu_is_live()) {
        log_error("EDU", "EDU is not alive");
        return;
    }

    log_info("EDU", "EDU is alive");

    if (dev_edu_factorial(5) != 120) {
        log_error("EDU", "EDU doesn't know how to calculate factorial");
        return;
    }

    idt_set_gate(edu_irq, 0xA0, IDT_TA_InterruptGate, 0x08);
    log_ok("EDU", "EDU is initialized");
}