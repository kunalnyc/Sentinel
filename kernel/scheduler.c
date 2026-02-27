#include "scheduler.h"
#include "../security/trust.h"

// Actual definitions
struct Process process_table[MAX_PROCESSES];
int process_count = 0;
int current_process = 0;

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

int create_process(unsigned int entry_point, unsigned int token)
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
        println("SECURITY: Process rejected - unknown token!");
        return -1; // REJECTED
    }

    // Token verified - create process
    process_table[i].pid            = process_count++;
    process_table[i].state          = PROCESS_READY;
    process_table[i].eip            = entry_point;
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
            // In future: context switch here
            return;
        }
    }
}