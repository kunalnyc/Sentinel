#ifndef SCHEDULER_H
#define SCHEDULER_H

// Process states
#define PROCESS_RUNNING  0
#define PROCESS_READY    1
#define PROCESS_BLOCKED  2
#define PROCESS_DEAD     3

// Maximum processes
#define MAX_PROCESSES 64

// Process Control Block
struct Process {
    int           pid;
    int           state;
    unsigned int  eip;
    unsigned int  esp;
    unsigned int  ebp;
    unsigned int  eax;
    unsigned int  ebx;
    unsigned int  ecx;
    unsigned int  edx;
    unsigned int  eflags;
    unsigned int  stack[1024];
    unsigned int  identity_token;
};

// Process table
extern struct Process process_table[MAX_PROCESSES];
extern int process_count;
extern int current_process;

// Function signatures
void scheduler_init();
int create_process(unsigned int entry_point, unsigned int token);
void schedule();

#endif