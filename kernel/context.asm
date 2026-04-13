; context.asm — SentinelOS context switch
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

    ; Save kernel RSP
    mov [rdi], rsp

    ; ── Switch to process ────────────────────────
    mov rsp, rdx        ; switch to process stack
    xor rbp, rbp        ; clear frame pointer
    xor rbx, rbx
    xor r12, r12
    xor r13, r13
    xor r14, r14
    xor r15, r15

    ; Push entry point and ret into it
    push rsi
    ret
EOF