#ifndef MEMORY_H
#define MEMORY_H

// Page size = 4KB
#define PAGE_SIZE 4096

// Total pages we manage (for now 16MB of RAM)
#define TOTAL_PAGES 4096

// The bitmap - 1 bit per page
// 4096 pages / 8 bits per byte = 512 bytes
extern unsigned char page_bitmap[TOTAL_PAGES / 8];

// Function signatures
void memory_init();
unsigned int allocate_page();
void free_page(unsigned int page_address);

#endif