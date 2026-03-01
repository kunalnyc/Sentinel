; SentinelOS 64-bit Bootloader
; Stages: Real Mode → VESA → Protected → Long Mode → Kernel

[BITS 16]
[ORG 0x7C00]

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    ; Print boot message
    mov si, msg_boot
    call print16

    ; Set VESA graphics mode
    call set_vesa

    ; Disable interrupts for mode switch
    cli

    ; Enable A20 line
    in  al, 0x92
    or  al, 2
    out 0x92, al

    ; Load GDT
    lgdt [gdt_ptr]

    ; Enter protected mode
    mov eax, cr0
    or  eax, 1
    mov cr0, eax

    jmp 0x08:stage32

; ─────────────────────
; VESA Mode Setup
; ─────────────────────
set_vesa:
    mov ax, 0x4F02
    mov bx, 0x4118    ; 800x600x32
    or  bx, 0x4000    ; linear framebuffer
    int 0x10

    ; Get mode info for framebuffer address
    mov ax, 0x4F01
    mov cx, 0x0118
    mov di, vbe_mode_info
    int 0x10

    ret

; ─────────────────────
; Print in Real Mode
; ─────────────────────
print16:
    mov ah, 0x0E
.loop:
    lodsb
    test al, al
    jz .done
    int 0x10
    jmp .loop
.done:
    ret

; ─────────────────────
; GDT
; ─────────────────────
align 8
gdt_start:
    ; Null
    dq 0

    ; 32-bit code segment
    dw 0xFFFF, 0x0000
    db 0x00, 0x9A, 0xCF, 0x00

    ; 32-bit data segment
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00

    ; 64-bit code segment
    dw 0x0000, 0x0000
    db 0x00, 0x98, 0x20, 0x00

    ; 64-bit data segment
    dw 0xFFFF, 0x0000
    db 0x00, 0x92, 0xCF, 0x00
gdt_end:

gdt_ptr:
    dw gdt_end - gdt_start - 1
    dd gdt_start

; ─────────────────────
; Data
; ─────────────────────
msg_boot        db 'SentinelOS 64 Loading...', 13, 10, 0
vbe_mode_info   times 256 db 0

times 510-($-$$) db 0
dw 0xAA55

; ─────────────────────
; Stage 2 - 32-bit
; Protected Mode
; Sets up paging for
; Long Mode
; ─────────────────────
[BITS 32]
stage32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    ; Clear page tables area
    mov edi, 0x1000
    xor eax, eax
    mov ecx, 0x3000 / 4
    rep stosd

    ; Set up PML4 → PDPT → PD → PT
    ; PML4 at 0x1000
    mov dword [0x1000], 0x2003  ; points to PDPT
    mov dword [0x1004], 0

    ; PDPT at 0x2000
    mov dword [0x2000], 0x3003  ; points to PD
    mov dword [0x2004], 0

    ; PD at 0x3000 - map first 2MB
    mov dword [0x3000], 0x0083  ; 2MB page, present, writable
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

    ; Enable paging → activates Long Mode
    mov eax, cr0
    or  eax, (1 << 31) | 1
    mov cr0, eax

    ; Jump to 64-bit code segment
    jmp 0x18:stage64

; ─────────────────────
; Stage 3 - 64-bit
; Long Mode!
; ─────────────────────
[BITS 64]
stage64:
    mov ax, 0x20
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov rsp, 0x90000

    ; Jump to kernel
    mov rax, 0x100000
    jmp rax