#include "scheduler.h"
#include "../security/trust.h"
#include <stdint.h>
// Actual definitions
struct Process process_table[MAX_PROCESSES];
int process_count = 0;
int current_process = 0;
uint64_t kernel_rsp_save = 0;


// Assembly context switch
extern void context_switch(uint64_t *kernel_rsp_save,
                           uint64_t proc_rip,
                           uint64_t proc_rsp);
// forward declaration
void println(char *str);

void scheduler_init()
{
    int i = 0;
    while(i < MAX_PROCESSES)
    {
        process_table[i].pid   = -1;
        process_table[i].state = PROCESS_DEAD;
        i++;
    }
}

int create_process(uint64_t entry_point, uint64_t token, uint32_t burst_time)
{
    // SECURITY CHECK FIRST - before anything else!
    // Find process in trust registry by token
    int trusted = 0;
    int i;
    for(i = 0; i < MAX_PROCESSES; i++)
    {
        if(process_table[i].pid == -1)
            break;
    }

    // Verify token exists in trust registry
    trusted = verify_process(token, trust_registry[0].hash);

    if(!trusted)
    {
       // Silent rejection - no println in 64-bit kernel println("SECURITY: Process rejected - unknown token!");
        return -1; // REJECTED
    }

    // Token verified - create process
    process_table[i].pid            = process_count++;
    process_table[i].state          = PROCESS_READY;
    process_table[i].rip            = entry_point;
    process_table[i].identity_token = token;
    process_table[i].burst_time     = burst_time;      // real value now
    process_table[i].remaining_time = burst_time;

    return process_table[i].pid;
}

void schedule()
{
    int i;
    for(i = 0; i < MAX_PROCESSES; i++)
    {
        if(process_table[i].state == PROCESS_READY)
        {
            current_process = i;
            process_table[i].state = PROCESS_RUNNING;

            // Set up process stack
            uint64_t *proc_stack = (uint64_t*)&process_table[i].stack[1020];
            
            // Push a safe halt loop as return address
            // When process rets, it lands here and halts
            extern void process_exit_handler(void);
            *proc_stack = (uint64_t)process_exit_handler;
            
            uint64_t proc_rsp = (uint64_t)proc_stack;
            proc_rsp &= ~0xFULL;

            context_switch(&kernel_rsp_save,
                           process_table[i].rip,
                           proc_rsp);

            process_table[i].state = PROCESS_DEAD;
            return;
        }
    }
}

void schedule_sjf(void)
{
    // Find READY process with smallest burst_time
    int best = -1;
    uint32_t min_burst = 0xFFFFFFFF;

    int i;
    for(i = 0; i < MAX_PROCESSES; i++)
    {
        if(process_table[i].state == PROCESS_READY)
        {
            if(process_table[i].burst_time < min_burst)
            {
                min_burst = process_table[i].burst_time;
                best = i;
            }
        }
    }

    if(best == -1) return;  // no ready processes

    current_process = best;
    process_table[best].state = PROCESS_RUNNING;

    uint64_t *proc_stack = (uint64_t*)&process_table[best].stack[1020];
    extern void process_exit_handler(void);
    *proc_stack = (uint64_t)process_exit_handler;

    uint64_t proc_rsp = (uint64_t)proc_stack;
    proc_rsp &= ~0xFULL;

    context_switch(&kernel_rsp_save,
                   process_table[best].rip,
                   proc_rsp);

    process_table[best].state = PROCESS_DEAD;
}