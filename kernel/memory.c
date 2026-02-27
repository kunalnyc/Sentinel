#include "memory.h"
// Actual definition lives here
unsigned char page_bitmap[TOTAL_PAGES / 8];
// Initialize memory manager
void memory_init()
{
    // Set all pages as free
    int i = 0;
    while(i < TOTAL_PAGES / 8)
    {
        page_bitmap[i] = 0;
        i++;
    }

    // Mark first 4MB as used (kernel space)
    // 4MB / 4KB = 1024 pages
    int j = 0;
    while(j < 128)  // 1024 pages / 8 bits = 128 bytes
    {
        page_bitmap[j] = 0xFF;  // 0xFF = 11111111 = all 8 pages used
        j++;
    }
}

unsigned int allocate_page()
{
    // loop through every byte in bitmap
    for(int i = 0; i < TOTAL_PAGES / 8; i++)
    {
        // loop through every bit in this byte
        for(int j = 0; j < 8; j++)
        {
            // check if this page is free
            if(!(page_bitmap[i] & (1 << j)))
            {
                // mark it as used
                page_bitmap[i] |= (1 << j);

                // return physical address of this page
                return (i * 8 + j) * PAGE_SIZE;
            }
        }
    }
    return 0; // no free pages!
}

void free_page(unsigned int page_address)
{
    // calculate page index
    unsigned int page_index = page_address / PAGE_SIZE;

    // calculate byte and bit in bitmap
    unsigned int byte_index = page_index / 8;
    unsigned int bit_index = page_index % 8;

    // mark this page as free
    page_bitmap[byte_index] &= ~(1 << bit_index);
}