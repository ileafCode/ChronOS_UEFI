[BITS 64]

; Basic test program for ChronOS64
section .text

main:
    mov rax, 1
    mov rdi, 0xC0000000
    mov rsi, 2
    mov rdx, 4
    int 0x80

    mov rax, 2
    mov rdi, 0xC0000000
    mov rsi, 4
    int 0x80

    jmp $               ; Currently just a forever loop because I don't have an exit syscall yet
[GLOBAL main]