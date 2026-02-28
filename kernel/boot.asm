; SentinelOS Kernel Entry Point
; GRUB hands control in 32-bit protected mode
[BITS 32]

extern kernel_main

; Multiboot2 header
section .multiboot
align 8
mb_start:
    dd 0xE85250D6
    dd 0
    dd mb_end - mb_start
    dd -(0xE85250D6 + 0 + (mb_end - mb_start))
    dw 0
    dw 0
    dd 8
mb_end:

section .text
global _start
_start:
    ; Set up 32-bit stack first
    mov esp, 0x90000

    ; Set up page tables for Long Mode
    ; Clear page table area
    mov edi, 0x1000
    xor eax, eax
    mov ecx, 0x3000 / 4
    rep stosd

    ; PML4 at 0x1000
    mov dword [0x1000], 0x2003
    mov dword [0x1004], 0

    ; PDPT at 0x2000
    mov dword [0x2000], 0x3003
    mov dword [0x2004], 0

    ; PD at 0x3000 - 2MB page
    mov dword [0x3000], 0x0083
    mov dword [0x3004], 0

    ; Load PML4 into CR3
    mov eax, 0x1000
    mov cr3, eax

    ; Enable PAE
    mov eax, cr4
    or  eax, (1 << 5)
    mov cr4, eax

    ; Enable Long Mode in EFER
    mov ecx, 0xC0000080
    rdmsr
    or  eax, (1 << 8)
    wrmsr

    ; Enable paging
    mov eax, cr0
    or  eax, (1 << 31) | 1
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64_ptr]

    ; Jump to 64-bit code
    jmp 0x08:long_mode

; 64-bit GDT
align 8
gdt64:
    dq 0
    dq 0x00AF9A000000FFFF  ; 64-bit code
    dq 0x00CF92000000FFFF  ; 64-bit data
gdt64_end:

gdt64_ptr:
    dw gdt64_end - gdt64 - 1
    dd gdt64

[BITS 64]
long_mode:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000
    xor rbp, rbp

    call kernel_main

.halt:
    hlt
    jmp .halt