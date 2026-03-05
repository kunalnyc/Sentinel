#include <stdint.h>
#include "idt.h"

struct IDTEntry idt[256];
struct IDTPointer idt_ptr;

void idt_set_entry(int index, uint64_t base, uint16_t selector, uint8_t flags)
{
    idt[index].base_low  = base & 0xFFFF;
    idt[index].base_mid  = (base >> 16) & 0xFFFF;
    idt[index].base_high = (base >> 32) & 0xFFFFFFFF;
    idt[index].selector  = selector;
    idt[index].ist       = 0;
    idt[index].flags     = flags;
    idt[index].reserved  = 0;
}

void idt_init(void)
{
    idt_ptr.limit = sizeof(struct IDTEntry) * 256 - 1;
    idt_ptr.base  = (uint64_t)&idt;
    __asm__ volatile("lidt (%0)" : : "r"(&idt_ptr));
}