BITS 64
section .text
global context_switch
global process_exit_handler
extern kernel_rsp_save

; void context_switch(uint64_t *kernel_rsp_save, uint64_t proc_rip, uint64_t proc_rsp)
context_switch:
    ; Save kernel context
    push rbp
    push rbx
    push r12
    push r13
    push r14
    push r15
    pushfq

    ; Save kernel RSP
    mov [rdi], rsp

    ; Switch to process stack
    mov rsp, rdx

    ; Push exit handler so when process rets it lands here
    lea rax, [rel process_exit_handler]
    push rax

    ; Clear registers
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

    ; Jump to process entry — ret will land in process_exit_handler
    jmp rsi

process_exit_handler:
    ; Process returned — restore kernel RSP and resume
    mov rsp, [rel kernel_rsp_save]
    popfq
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret