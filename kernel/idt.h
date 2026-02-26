#ifndef IDT_H
#define IDT_H

// One entry in the IDT table
struct IDTEntry {
    unsigned short base_low;    // lower 16 bits of handler address
    unsigned short selector;    // kernel code segment = 0x08
    unsigned char  zero;        // always 0
    unsigned char  type_attr;   // type and attributes
    unsigned short base_high;   // upper 16 bits of handler address
} __attribute__((packed));

// IDT pointer - tells CPU where the IDT is
struct IDTPointer {
    unsigned short limit;   // size of IDT - 1
    unsigned int   base;    // address of IDT
} __attribute__((packed));

// 256 possible interrupts
extern struct IDTEntry idt[256];
extern struct IDTPointer idt_ptr;

// Function signatures
void idt_init();
void idt_set_entry(int n, unsigned int handler);

#endif