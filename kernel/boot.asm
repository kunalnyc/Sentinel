[BITS 32]

extern kernel_main

section .multiboot
align 8
mb_start:
    dd 0xE85250D6
    dd 0
    dd mb_end - mb_start
    dd -(0xE85250D6 + 0 + (mb_end - mb_start))
    align 8
    dw 0
    dw 0
    dd 8
mb_end:

section .bss
align 16
resb 16384
stack_top:

section .text
global _start
_start:
    mov esp, stack_top

    ; Save magic and mb_ptr before anything
    mov [saved_magic], eax
    mov [saved_mbptr], ebx

    ; Clear page tables
    mov edi, 0x1000
    xor eax, eax
    mov ecx, 0x3000 / 4
    rep stosd

    ; PML4
    mov dword [0x1000], 0x2003
    mov dword [0x1004], 0

    ; PDPT
    mov dword [0x2000], 0x3003
    mov dword [0x2004], 0

    ; PD
    mov dword [0x3000], 0x0083
    mov dword [0x3004], 0

    ; Load CR3
    mov eax, 0x1000
    mov cr3, eax

    ; Enable PAE
    mov eax, cr4
    or eax, 0x20
    mov cr4, eax

    ; Enable Long Mode
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100
    wrmsr

    ; Enable paging
    mov eax, cr0
    or eax, 0x80000001
    mov cr0, eax

    lgdt [gdt_ptr]
    jmp 0x08:lmode

section .data
saved_magic dd 0
saved_mbptr dd 0

align 8
gdt:
    dq 0
    dq 0x00AF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:
gdt_ptr:
    dw gdt_end - gdt - 1
    dd gdt

[BITS 64]
lmode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000
    xor rbp, rbp

    ; Load arguments for kernel_main
    xor rdi, rdi
    xor rsi, rsi
    mov edi, [rel saved_magic]
    mov esi, [rel saved_mbptr]

    call kernel_main

.halt:
    hlt
    jmp .halt