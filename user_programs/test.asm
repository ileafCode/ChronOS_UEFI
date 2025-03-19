[BITS 64]

; Basic test program for ChronOS64

_start:
    mov rax, 0  ; Test syscall (adds 10 to rbx and prints it out)
    mov rbx, 59 ; Random data
    int 0x80    ; Syscall

    mov rax, 0  ; Test syscall (adds 10 to rbx and prints it out)
    mov rbx, 25 ; Random data
    int 0x80    ; Syscall

    mov rax, 0   ; Test syscall (adds 10 to rbx and prints it out)
    mov rbx, 832 ; Random data
    int 0x80     ; Syscall

    mov rax, 0    ; Test syscall (adds 10 to rbx and prints it out)
    mov rbx, 5449 ; Random data
    int 0x80      ; Syscall

    jmp $       ; Currently just a forever loop because I don't have an exit syscall yet
[GLOBAL _start]