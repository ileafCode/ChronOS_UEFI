[bits 64]

%macro pushaq 0
    push rsp
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
    add rsp, 8
%endmacro

EXTERN process_set_pml4_to_kernel
EXTERN process_set_pml4_to_cur_proc

%macro int_stub 2
    pushaq
    call process_set_pml4_to_kernel

    mov rdi, rsp
    mov rsi, %2
    call %1

    call process_set_pml4_to_cur_proc
    popaq
    iretq
%endmacro

EXTERN lapic_address

%macro irq_stub 1
    pushaq
    call process_set_pml4_to_kernel
    mov rdi, rsp
    call %1

    push rax
    mov rax, [lapic_address]
    add rax, 0xB0
    mov dword [rax], 0
    pop rax

    call process_set_pml4_to_cur_proc
    popaq
%endmacro

; Syscall
EXTERN syscall_handler
syscall_req:
    pushaq
    call process_set_pml4_to_kernel

    mov rdi, rsp
    call syscall_handler

    call process_set_pml4_to_cur_proc
    popaq
    iretq
GLOBAL syscall_req

; IRQs
EXTERN timer_handler
timer_irq:
    irq_stub timer_handler
    iretq
GLOBAL timer_irq

EXTERN sci_handler
sci_irq:
    irq_stub sci_handler
    iretq
GLOBAL sci_irq

EXTERN kbd_ps2_handler
kbd_ps2_irq:
    irq_stub kbd_ps2_handler
    iretq
GLOBAL kbd_ps2_irq

EXTERN edu_irq_handler
edu_irq:
    irq_stub edu_irq_handler
    iretq
GLOBAL edu_irq

EXTERN e1000_irq_handler
e1000_irq:
    irq_stub e1000_irq_handler
    iretq
GLOBAL e1000_irq

EXTERN ahci_irq_handler
ahci_irq:
    irq_stub ahci_irq_handler
    iretq
GLOBAL ahci_irq

; Exceptions
EXTERN exception_handler

%macro DEFINE_EXCEPTION 1
exception%1:
    %if %1 != 8 && %1 != 10 && %1 != 11 && %1 != 12 && %1 != 13 && %1 != 14 && %1 != 17
        push $0
    %endif
    int_stub exception_handler, %1
GLOBAL exception%1
%endmacro

%assign i 0
%rep 32
    DEFINE_EXCEPTION i
    %assign i i+1
%endrep

