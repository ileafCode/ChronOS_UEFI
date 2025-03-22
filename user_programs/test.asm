[BITS 64]

; Basic test program for ChronOS64

_start:
    mov rax, 1                   ; mmap syscall
    mov rdi, 0xC0000000          ; fb
    mov rsi, 2                   ; page flags (present, rw)
    mov rdx, 1                   ; page count
    int 0x80                     ; address in rax

    mov dword [rax], 0xFF00FF00
    jmp $               ; Currently just a forever loop because I don't have an exit syscall yet
[GLOBAL _start]