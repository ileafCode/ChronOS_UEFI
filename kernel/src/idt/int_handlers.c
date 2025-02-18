#include <idt/int_handlers.h>
#include <idt/idt.h>
#include <debug/stacktrace/stacktrace.h>
#include <printk/printk.h>
#include <io/io.h>

typedef struct int_reg_return {
    uint64_t r15, r14, r13, r12;
    uint64_t r11, r10,  r9,  r8;
    uint64_t rdi, rsi, rbp, rbx;
    uint64_t rdx, rcx, rax;
} __attribute__((packed)) int_reg_return_t;

void print_regs(int_reg_return_t *regs) {
    printk(" --- Registers --- \n");
    printk("    RAX: %16lx, RCX: %16lx, RDX: %16lx, RBX: %16lx\n",
        regs->rax, regs->rcx, regs->rdx, regs->rbx);
    printk("    RBP: %16lx, RSI: %16lx, RDI: %16lx\n",
        regs->rbp, regs->rsi, regs->rdi);
    printk("    R8:  %16lx, R9:  %16lx, R10: %16lx, R11: %16lx\n",
        regs->r8,  regs->r9,  regs->r10, regs->r11);
    printk("    R12: %16lx, R13: %16lx, R14: %16lx, R15: %16lx\n",
        regs->r12,  regs->r13,  regs->r14, regs->r15);
}

void div0_handler(int_reg_return_t *regs) {
    printk("Division by 0 exception\n");
    print_regs(regs);
    stacktrace_print();
    while (1);
}

void pf_handler() {
    printk("Page fault\n");
    stacktrace_print();
    while (1);
}

void kbd_ps2_handler() {
    uint8_t sc = inb(0x60);
    //printk("%x\n", sc);
}

void timer_handler() {
}

extern void div0_int();
extern void pf_int();

extern void timer_irq();
extern void kbd_ps2_irq();

void int_handlers_init() {
    idt_set_gate(div0_int, 0, IDT_TA_InterruptGate, 0x08);
    idt_set_gate(pf_int, 14, IDT_TA_InterruptGate, 0x08);
    idt_set_gate(timer_irq, 0x20, IDT_TA_InterruptGate, 0x08);
    idt_set_gate(kbd_ps2_irq, 0x21, IDT_TA_InterruptGate, 0x08);
}