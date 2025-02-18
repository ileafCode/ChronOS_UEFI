[bits 64]
load_gdt:   
    lgdt [rdi]      ; load GDT, rdi (1st argument) contains the gdt_ptr
    mov ax, 0x40    ; TSS segment is 0x40
    ltr ax          ; load TSS
    mov ax, 0x10    ; kernel data segment is 0x10
    mov ds, ax      ; load kernel data segment in data segment registers
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    pop rdi        ; pop the return address
    mov rax, 0x08   ; kernel code segment is 0x08
    push rax       ; push the kernel code segment
    push rdi       ; push the return address again
    retfq
GLOBAL load_gdt