; hello.asm — Minimal SentinelOS test program
; This runs inside the kernel, no syscalls, no libc
; It writes directly to VGA memory at 0xB8000

BITS 64

section .text
global _start

_start:
    ; Write directly to VGA at top-left
    mov rax, 0xB8000
    mov word [rax], 0x0A48    ; 'H' green
    mov word [rax+2], 0x0A49  ; 'I' green  
    mov word [rax+4], 0x0A21  ; '!' green
    ret