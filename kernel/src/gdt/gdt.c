#include <gdt/gdt.h>
#include <logging/logging.h>

__attribute__((aligned(0x1000)))
gdt_t default_gdt = {
    {0, 0, 0, 0x00, 0x00, 0},  /* 0x00 null  */
    {0, 0, 0, 0x9A, 0xA0, 0},  /* 0x08 kernel code (kernel base selector) */
    {0, 0, 0, 0x92, 0xA0, 0},  /* 0x10 kernel data */
    {0, 0, 0, 0x00, 0x00, 0},  /* 0x18 null (user base selector) */
    {0, 0, 0, 0x92, 0xA0, 0},  /* 0x20 user data */
    {0, 0, 0, 0x9A, 0xA0, 0},  /* 0x28 user code */
    {0, 0, 0, 0x92, 0xa0, 0},  /* 0x30 ovmf data */
    {0, 0, 0, 0x9a, 0xa0, 0},  /* 0x38 ovmf code */
    {0, 0, 0, 0x89, 0xa0, 0},  /* 0x40 tss low */
    {0, 0, 0, 0x00, 0x00, 0},  /* 0x48 tss high */
};

gdt_desc_t gdt_desc;
tss_t tss;

extern void load_gdt(gdt_desc_t *);

void gdt_init() {
    uint64_t tss_base = ((uint64_t)&tss);
    default_gdt.tss_low.base0 = tss_base & 0xffff;
    default_gdt.tss_low.base1 = (tss_base >> 16) & 0xff;
    default_gdt.tss_low.base2 = (tss_base >> 24) & 0xff;
    default_gdt.tss_low.limit0 = sizeof(tss);
    default_gdt.tss_high.limit0 = (tss_base >> 32) & 0xffff;
    default_gdt.tss_high.base0 = (tss_base >> 48) & 0xffff;

    gdt_desc.size = sizeof(gdt_t) - 1;
    gdt_desc.offset = (uint64_t)&default_gdt;
    load_gdt(&gdt_desc);
    log_ok("GDT", "GDT and TSS initialized");
}