#include <idt/int_handlers.h>
#include <idt/idt.h>
#include <debug/stacktrace/stacktrace.h>
#include <printk/printk.h>
#include <io/io.h>
#include <utils/panic.h>

typedef struct int_reg_return {
    uint64_t r15, r14, r13, r12;
    uint64_t r11, r10,  r9,  r8;
    uint64_t rdi, rsi, rbp, rbx;
    uint64_t rdx, rcx, rax;
} __attribute__((packed)) int_reg_return_t;

const char *exception_messages[32] = {
    "Division By Zero",                  // 0
    "Debug",                              // 1
    "Non-Maskable Interrupt",             // 2
    "Breakpoint",                         // 3
    "Overflow",                            // 4
    "Bound Range Exceeded",               // 5
    "Invalid Opcode",                      // 6
    "Device Not Available (No Math Coprocessor)", // 7
    "Double Fault",                        // 8
    "Coprocessor Segment Overrun (deprecated)", // 9
    "Invalid TSS",                         // 10
    "Segment Not Present",                 // 11
    "Stack-Segment Fault",                 // 12
    "General Protection Fault",            // 13
    "Page Fault",                          // 14
    "Reserved (15)",                       // 15
    "x87 Floating-Point Exception",        // 16
    "Alignment Check",                     // 17
    "Machine Check",                       // 18
    "SIMD Floating-Point Exception",       // 19
    "Virtualization Exception",            // 20
    "Control Protection Exception",        // 21
    "Reserved (22)",                       // 22
    "Reserved (23)",                       // 23
    "Reserved (24)",                       // 24
    "Reserved (25)",                       // 25
    "Reserved (26)",                       // 26
    "Reserved (27)",                       // 27
    "Hypervisor Injection Exception",      // 28
    "VMM Communication Exception",         // 29
    "Security Exception",                  // 30
    "Reserved (31)"                        // 31
};

const char *exception_verses[32] = {
    "Proverbs 16:18",         // 0: Division By Zero
    "Luke 12:2",              // 1: Debug
    "1 Thessalonians 5:2",     // 2: Non-Maskable Interrupt
    "Psalm 46:10",            // 3: Breakpoint
    "Ecclesiastes 1:8",       // 4: Overflow
    "Job 38:11",              // 5: Bound Range Exceeded
    "Matthew 7:23",           // 6: Invalid Opcode
    "Luke 14:28",             // 7: Device Not Available (No Math Coprocessor)
    "Lamentations 3:47",      // 8: Double Fault
    "Ecclesiastes 1:9",       // 9: Coprocessor Segment Overrun (deprecated)
    "James 1:8",              // 10: Invalid TSS
    "Matthew 25:8",           // 11: Segment Not Present
    "Psalm 55:22",            // 12: Stack-Segment Fault
    "Proverbs 4:23",          // 13: General Protection Fault
    "John 14:2",              // 14: Page Fault
    "",                       // 15: Reserved (15)
    "James 1:5",              // 16: x87 Floating-Point Exception
    "1 Corinthians 14:40",    // 17: Alignment Check
    "1 Corinthians 3:13",     // 18: Machine Check
    "Ecclesiastes 3:1",       // 19: SIMD Floating-Point Exception
    "1 Corinthians 13:12",    // 20: Virtualization Exception
    "Ephesians 6:11",         // 21: Control Protection Exception
    "",                       // 22: Reserved (22)
    "",                       // 23: Reserved (23)
    "",                       // 24: Reserved (24)
    "",                       // 25: Reserved (25)
    "",                       // 26: Reserved (26)
    "",                       // 27: Reserved (27)
    "Psalm 101:7",            // 28: Hypervisor Injection Exception
    "Colossians 4:6",         // 29: VMM Communication Exception
    "Psalm 4:8",              // 30: Security Exception
    ""                        // 31: Reserved (31)
};

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

void exception_handler(int_reg_return_t *regs, uint64_t exception) {
    kernel_panic(exception_messages[exception]);
    print_regs(regs);
    stacktrace_print();
    printk("\n%s\n", exception_verses[exception]);
    
    while (1);
}

void kbd_ps2_handler() {
    uint8_t sc = inb(0x60);
    printk("%x\n", sc);
}

void timer_handler() {
}

extern void exception0();
extern void exception1();
extern void exception2();
extern void exception3();
extern void exception4();
extern void exception5();
extern void exception6();
extern void exception7();
extern void exception8();
extern void exception9();
extern void exception10();
extern void exception11();
extern void exception12();
extern void exception13();
extern void exception14();
extern void exception15();
extern void exception16();
extern void exception17();
extern void exception18();
extern void exception19();
extern void exception20();
extern void exception21();
extern void exception22();
extern void exception23();
extern void exception24();
extern void exception25();
extern void exception26();
extern void exception27();
extern void exception28();
extern void exception29();
extern void exception30();
extern void exception31();

extern void timer_irq();
extern void kbd_ps2_irq();

void int_handlers_init() {
    void (*exceptions[32])() = {
        exception0, exception1, exception2, exception3,
        exception4, exception5, exception6, exception7,
        exception8, exception9, exception10, exception11,
        exception12, exception13, exception14, exception15,
        exception16, exception17, exception18, exception19,
        exception20, exception21, exception22, exception23,
        exception24, exception25, exception26, exception27,
        exception28, exception29, exception30, exception31
    };

    for (int i = 0; i < 32; i++) {
        idt_set_gate(exceptions[i], i, IDT_TA_InterruptGate, 0x08);
    }

    idt_set_gate(timer_irq, 0x20, IDT_TA_InterruptGate, 0x08);
    idt_set_gate(kbd_ps2_irq, 0x21, IDT_TA_InterruptGate, 0x08);
}