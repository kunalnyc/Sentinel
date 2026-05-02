; irq.asm — Proper IRQ handler stubs for SentinelOS
; Saves ALL registers, calls C handler, sends EOI, restores

BITS 64
section .text

global irq0_handler
extern timer_handler_c

irq0_handler:
    ; ── Save all registers ────────────────────────
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    ; ── Call C timer handler ──────────────────────
    call timer_handler_c

    ; ── Restore all registers ─────────────────────
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax

    iretq