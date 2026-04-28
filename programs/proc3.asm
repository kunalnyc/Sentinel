BITS 64
section .text
global _start

_start:
    mov rax, 0xB8000
    mov word [rax], 0x0A50
    mov word [rax+2], 0x0A33
    ret