section .text
global _start
_start:
    ; Process 1 - burst 3ms
    ; In real OS this would do work
    ; For now just halt safely
    hlt
    jmp _start