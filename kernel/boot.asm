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
    mov [saved_magic], eax
    mov [saved_mbptr], ebx

    ; Clear page tables
    mov edi, 0x1000
    xor eax, eax
    mov ecx, 0x5000 / 4
    rep stosd

    ; PML4 at 0x1000
    ; Entry 0 → PDPT at 0x2000 (first 512GB)
    mov dword [0x1000], 0x2003
    mov dword [0x1004], 0

    ; PDPT at 0x2000
    ; Map all 4 entries = 4GB total using 1GB pages
    ; 1GB page flag = 0x83 | (1<<7) = 0x83 already has PS bit
    ; For 1GB pages in PDPT: bit 7 must be set
    mov dword [0x2000], 0x0000083  ; 0GB-1GB
    mov dword [0x2004], 0
    mov dword [0x2008], 0x40000083 ; 1GB-2GB
    mov dword [0x200C], 0
    mov dword [0x2010], 0x80000083 ; 2GB-3GB
    mov dword [0x2014], 0
    mov dword [0x2018], 0xC0000083 ; 3GB-4GB (covers 0xE0000000!)
    mov dword [0x201C], 0

    mov eax, 0x1000
    mov cr3, eax

    ; Enable PAE + PSE
    mov eax, cr4
    or eax, 0x30
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