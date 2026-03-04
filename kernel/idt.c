#include <stdint.h>
#include <stddef.h>  // for uintptr_t
#include "idt.h"

// Define the global IDT table
struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

void idt_set_entry(int index, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[index].base_low = base & 0xFFFF;
    idt[index].base_high = (base >> 16) & 0xFFFF;
    idt[index].selector = selector;
    idt[index].always0 = 0;
    idt[index].flags = flags;
}

void idt_init(void) {
    idt_ptr.limit = sizeof(struct IDTEntry) * 256 - 1;
    idt_ptr.base = (uint32_t)(uintptr_t)&idt;
    
    // Load IDT
    __asm__ volatile ("lidt (%0)" : : "r" (&idt_ptr));
}