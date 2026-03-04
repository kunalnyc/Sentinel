#ifndef IDT_H
#define IDT_H

#include <stdint.h>

// IDT entry structure
struct IDTEntry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t always0;
    uint8_t flags;
    uint16_t base_high;
} __attribute__((packed));

// IDT pointer structure for lidt instruction
struct IDTPointer {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// Function declarations
void idt_set_entry(int index, uint32_t base, uint16_t selector, uint8_t flags);
void idt_init(void);

#endif