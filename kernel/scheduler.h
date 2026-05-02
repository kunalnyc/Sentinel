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

// Round Robin quantum — ticks per process (100Hz = 10ms per tick)
#define QUANTUM 10   // 10 ticks = 100ms per process

// Process Control Block (64-bit)
struct Process {
    int      pid;
    int      state;

    // 64-bit registers
    uint64_t rip;
    uint64_t rsp;
    uint64_t rbp;
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

    // Scheduling metadata
    uint32_t burst_time;      // total CPU time needed
    uint32_t remaining_time;  // for SRTF
    uint32_t quantum;         // ticks used in current slot (Round Robin)
    uint32_t total_ticks;     // total ticks consumed (stats)
};

// Process table
extern struct Process process_table[MAX_PROCESSES];
extern int process_count;
extern int current_process;
extern uint64_t kernel_rsp_save;

// Function signatures
void scheduler_init();
void process_exit_handler(void);
int  create_process(uint64_t entry_point, uint64_t token, uint32_t burst_time);
void schedule();
void schedule_sjf(void);
void schedule_tick(void);   // called from IRQ0 every timer tick

#endif