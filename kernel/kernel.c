// Sentinel kernel main file
// Entry point - first code the CPU runs after bootloader
#include "../security/trust.h"
#include "../security/sha256.h"
#include "idt.h"
#include "memory.h"
#include "scheduler.h"
#include "keyboard.h"
// Actual definitions live here
struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

int cursor_row = 0;
int cursor_col = 0;

void print(char *str)
{
    char *video = (char *)0xB8000;
    int i = 0;

    while(str[i] != '\0')
    {
        int pos = (cursor_row * 80 + cursor_col) * 2;
        video[pos]     = str[i];
        video[pos + 1] = 0x0F;
        cursor_col++;
        i++;
    }
}
void println(char *str)
{
    print(str);
    cursor_row++;
    cursor_col = 0;
}

void clear_screen() {
    char *video = (char *)0xB8000; // VGA video memory starts at 0xB8000
    for (int i = 0; i < 80 * 25; i++) {
        video[i * 2] = ' '; // Clear character
        video[i * 2 + 1] = 0x0F; // Attribute byte (white on black)
    }
}

void kernel_main(void)
{
    clear_screen();
    println("SentinelOS v0.1 - Booting...");
    println("Kernel loaded at 0x100000");
    println("Initializing security layer...");

    // Initialize IDT
    idt_init();
    println("Interrupt Descriptor Table: ONLINE");

    // Register a trusted process
    unsigned char trusted_hash[32] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,
                                      17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};
    register_process(1001, "shell", trusted_hash, TRUST_SYSTEM);
    println("Trust Registry: ONLINE");
    memory_init();
    println("Memory Manager: ONLINE");

    // inside kernel_main after memory_init():
    unsigned char data[] = "SentinelOS";
    unsigned char hash[32];
    sha256_compute(data, 10, hash);
    println("SHA-256 Engine: ONLINE");
    // inside kernel_main after sha256:
    scheduler_init();
    println("Process Scheduler: ONLINE");
    // inside kernel_main:
    keyboard_init();
    println("Keyboard Driver: ONLINE");

    int pid1 = create_process(0x200000, 1001);
    int pid2 = create_process(0x300000, 9999);
    // test allocation
    unsigned int page = allocate_page();
    println("First free page allocated!");
    // Test 1 - verify a TRUSTED process
    if(verify_process(1001, trusted_hash))
        println("shell process: ALLOWED ✓");
    else
        println("shell process: REJECTED ✗");

    // Test 2 - verify an UNKNOWN process
    unsigned char fake_hash[32] = {0};
    if(verify_process(9999, fake_hash))
        println("unknown process: ALLOWED ✓");
    else
        println("unknown process: REJECTED ✗");

    while(1) {}
}