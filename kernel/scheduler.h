#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>

// Process states
#define PROCESS_RUNNING  0
#define PROCESS_READY    1
#define PROCESS_BLOCKED  2
#define PROCESS_DEAD     3

// Maximum processes
#define MAX_PROCESSES 64

// Process Control Block (64-bit)
struct Process {
    int      pid;
    int      state;

    // 64-bit registers
    uint64_t rip;       // instruction pointer (was eip)
    uint64_t rsp;       // stack pointer       (was esp)
    uint64_t rbp;       // base pointer        (was ebp)
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rflags;

    // Process stack (64-bit aligned)
    uint64_t stack[1024];

    // Security
    uint64_t identity_token;
};

// Process table
extern struct Process process_table[MAX_PROCESSES];
extern int process_count;
extern int current_process;
extern uint64_t kernel_rsp_save;
// Function signatures
void scheduler_init();
int create_process(uint64_t entry_point, uint64_t token);
void schedule();

#endif