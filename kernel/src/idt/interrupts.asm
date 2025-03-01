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

%macro int_stub 2
    pushaq

    mov rdi, rsp
    mov rsi, %2
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
EXTERN timer_handler
timer_irq:
    irq_stub timer_handler
GLOBAL timer_irq

EXTERN sci_handler
sci_irq:
    irq_stub sci_handler
GLOBAL sci_irq

EXTERN kbd_ps2_handler
kbd_ps2_irq:
    irq_stub kbd_ps2_handler
GLOBAL kbd_ps2_irq

EXTERN edu_irq_handler
edu_irq:
    irq_stub edu_irq_handler
GLOBAL edu_irq

EXTERN e1000_irq_handler
e1000_irq:
    irq_stub e1000_irq_handler
GLOBAL e1000_irq

EXTERN ahci_irq_handler
ahci_irq:
    irq_stub ahci_irq_handler
GLOBAL ahci_irq

; Exceptions
EXTERN exception_handler

%macro DEFINE_EXCEPTION 1
    exception%1:
        int_stub exception_handler, %1
    GLOBAL exception%1
%endmacro

%assign i 0
%rep 32
    DEFINE_EXCEPTION i
    %assign i i+1
%endrep
