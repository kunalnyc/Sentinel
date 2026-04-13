; context.asm — SentinelOS context switch
; Simple direct call approach — safe for cooperative switching

BITS 64
section .text
global context_switch

; void context_switch(uint64_t *kernel_rsp_save, uint64_t proc_rip, uint64_t proc_rsp)
; rdi = address to save kernel RSP
; rsi = process entry RIP  
; rdx = process stack pointer

context_switch:
    ; ── Save kernel context ──────────────────────
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    pushfq

    ; Save kernel RSP so we can return later
    mov [rdi], rsp

    ; ── Set up process stack ─────────────────────
    mov rsp, rdx

    ; Clear registers for clean process start
    xor rax, rax
    xor rbx, rbx
    xor rcx, rcx
    xor rdx, rdx
    xor rbp, rbp
    xor r8,  r8
    xor r9,  r9
    xor r10, r10
    xor r11, r11
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15

    ; Jump directly to process entry point
    jmp rsi