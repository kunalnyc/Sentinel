[BITS 32]

extern kernel_main

section .multiboot
align 8
mb_start:
    dd 0xE85250D6
    dd 0
    dd mb_end - mb_start
    dd 0x100000000 - (0xE85250D6 + 0 + (mb_end - mb_start))
    align 8
    dw 0
    dw 0
    dd 8
mb_end:

section .text
global _start
_start:
    mov esp, 0x90000

    ; Save magic and mb_ptr
    mov [saved_magic], eax
    mov [saved_mbptr], ebx

    ; We need to switch to real mode to call BIOS for VESA
    ; But we're already in protected mode from GRUB
    ; So instead - write VBE mode info to known address
    ; and let kernel use it

    ; Clear page tables
    mov edi, 0x1000
    xor eax, eax
    mov ecx, 0x4000 / 4
    rep stosd

    ; PML4 at 0x1000 - map first 4GB
    mov dword [0x1000], 0x2003
    mov dword [0x1004], 0

    ; PDPT at 0x2000 - map first 1GB
    mov dword [0x2000], 0x3003
    mov dword [0x2004], 0

    ; PD at 0x3000 - map first 512MB with 2MB pages
    mov eax, 0x0083
    mov ecx, 256
    mov edi, 0x3000
.map_pd:
    mov dword [edi], eax
    mov dword [edi+4], 0
    add eax, 0x200000
    add edi, 8
    loop .map_pd

    ; Load PML4
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

    xor rdi, rdi
    xor rsi, rsi
    mov edi, [rel saved_magic]
    mov esi, [rel saved_mbptr]

    call kernel_main

.halt:
    hlt
    jmp .halt