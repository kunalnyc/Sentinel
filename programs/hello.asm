; hello.asm — Minimal SentinelOS test program
; This runs inside the kernel, no syscalls, no libc
; It writes directly to VGA memory at 0xB8000

BITS 64

section .text
global _start

_start:
    ; Write 'H' to VGA text buffer at 0xB8000
    ; Each character = 2 bytes: [char][color]
    ; Color 0x0A = bright green on black

    mov rax, 0xB8000
    mov byte [rax + 0],  'H'
    mov byte [rax + 1],  0x0A
    mov byte [rax + 2],  'I'
    mov byte [rax + 3],  0x0A
    mov byte [rax + 4],  '!'
    mov byte [rax + 5],  0x0A

    ; Halt — spin forever
.hang:
    hlt
    jmp .hang