#include "tss.h"
#include "idt.h"
#include <stdint.h>

struct TSS kernel_tss;

// GDT TSS entry is at offset 0x28
// We write it as two 8-byte entries (16 bytes total for 64-bit TSS)
void tss_init(uint64_t kernel_stack)
{
    // Clear TSS
    uint8_t *p = (uint8_t*)&kernel_tss;
    uint32_t i;
    for(i = 0; i < sizeof(struct TSS); i++) p[i] = 0;

    // Set kernel stack for Ring 0
    kernel_tss.rsp0     = kernel_stack;
    kernel_tss.iomap_base = sizeof(struct TSS);

    // Install TSS descriptor into GDT at 0x28
    uint64_t base  = (uint64_t)&kernel_tss;
    uint32_t limit = sizeof(struct TSS) - 1;

    // GDT is at a fixed address in boot.asm — we patch it at runtime
    // TSS descriptor format (16 bytes):
    // bytes 0-1: limit low
    // bytes 2-4: base low
    // byte  5:   access (0x89 = present, type=9 = 64-bit TSS available)
    // byte  6:   granularity (limit high + flags)
    // byte  7:   base mid-high
    // bytes 8-11: base high
    // bytes 12-15: reserved

    // Find GDT base from GDTR
    uint8_t gdtr[10];
    __asm__ volatile("sgdt %0" : "=m"(gdtr));
    uint64_t gdt_base = *(uint64_t*)(gdtr + 2);

    // TSS entry is at GDT offset 0x28 (5th entry * 8 bytes)
    uint64_t *tss_entry = (uint64_t*)(gdt_base + 0x28);

    // Low 8 bytes
    tss_entry[0] = (uint64_t)(limit & 0xFFFF)
                 | ((base & 0xFFFFFF) << 16)
                 | ((uint64_t)0x89 << 40)
                 | (((uint64_t)(limit >> 16) & 0xF) << 48)
                 | (((base >> 24) & 0xFF) << 56);

    // High 8 bytes — base bits 63:32
    tss_entry[1] = (base >> 32) & 0xFFFFFFFF;

    // Load TSS — selector 0x28, RPL=0
    __asm__ volatile("ltr %0" : : "r"((uint16_t)0x28));
}

void tss_set_kernel_stack(uint64_t stack)
{
    kernel_tss.rsp0 = stack;
}