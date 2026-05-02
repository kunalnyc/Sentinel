#ifndef TSS_H
#define TSS_H

#include <stdint.h>

// 64-bit Task State Segment
struct TSS {
    uint32_t reserved0;
    uint64_t rsp0;        // kernel stack pointer (Ring 0)
    uint64_t rsp1;
    uint64_t rsp2;
    uint64_t reserved1;
    uint64_t ist[7];      // interrupt stack table
    uint64_t reserved2;
    uint16_t reserved3;
    uint16_t iomap_base;
} __attribute__((packed));

extern struct TSS kernel_tss;

void tss_init(uint64_t kernel_stack);
void tss_set_kernel_stack(uint64_t stack);

#endif