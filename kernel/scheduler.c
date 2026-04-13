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

int create_process(uint64_t entry_point, uint64_t token)
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

            // Set up process stack - leave room for return address
            uint64_t *proc_stack = &process_table[i].stack[1020];
            
            // Align stack to 16 bytes
            uint64_t proc_rsp = (uint64_t)proc_stack & ~0xFULL;

            context_switch(&kernel_rsp_save,
                           process_table[i].rip,
                           proc_rsp);

            process_table[i].state = PROCESS_DEAD;
            return;
        }
    }
}