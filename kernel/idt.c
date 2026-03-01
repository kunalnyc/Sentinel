#include "idt.h"

void idt_set_entry(int n, unsigned long long handler)
{
    idt[n].base_low  = handler & 0xFFFF;
    idt[n].base_high = (handler >> 16) & 0xFFFF;
    idt[n].selector  = 0x08;
    idt[n].zero      = 0;
    idt[n].type_attr = 0x8E;
}

void idt_init()
{
    idt_ptr.limit = (sizeof(struct IDTEntry) * 256) - 1;
   // TO:
    idt_ptr.base = (unsigned long long)&idt;

    // Load IDT into CPU
    __asm__ volatile("lidt %0" : : "m"(idt_ptr));
}