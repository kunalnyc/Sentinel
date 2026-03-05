#ifndef IDT_H
#define IDT_H
#include <stdint.h>

struct IDTEntry {
    uint16_t base_low;
    uint16_t selector;
    uint8_t  ist;
    uint8_t  flags;
    uint16_t base_mid;
    uint32_t base_high;
    uint32_t reserved;
} __attribute__((packed));

struct IDTPointer {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

void idt_set_entry(int index, uint64_t base, uint16_t selector, uint8_t flags);
void idt_init(void);
#endif