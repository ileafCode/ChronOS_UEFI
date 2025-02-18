[bits 64]

%macro pushaq 0
    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro popaq 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx
    pop rax
%endmacro

%macro int_stub 1
    pushaq

    mov rdi, rsp
    call %1

    popaq
    iretq
%endmacro

EXTERN lapic_address

%macro irq_stub 1
    pushaq
    call %1
    popaq

    push rax
    mov rax, [lapic_address]
    add rax, 0xB0
    mov dword [rax], 0
    pop rax

    iretq
%endmacro

; IRQs
EXTERN kbd_ps2_handler
kbd_ps2_irq:
    irq_stub kbd_ps2_handler
GLOBAL kbd_ps2_irq

; Exceptions
EXTERN div0_handler
div0_int:
    int_stub div0_handler
GLOBAL div0_int

EXTERN pf_handler
pf_int:
    int_stub pf_handler
GLOBAL pf_int
