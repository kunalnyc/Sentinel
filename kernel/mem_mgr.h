#ifndef MEM_MGR_H
#define MEM_MGR_H

#include <stdint.h>

#define HEAP_START      0x200000      // 2MB mark
#define HEAP_SIZE       (4 * 1024 * 1024)  // 4MB heap
#define BLOCK_MAGIC     0xDEADBEEF
#define MAX_ALLOCS      256

// Single allocation record
typedef struct {
    uint32_t magic;
    uint32_t size;
    uint32_t in_use;
    uint32_t addr;
} mem_block_t;

// Memory stats
typedef struct {
    uint32_t total_bytes;
    uint32_t used_bytes;
    uint32_t free_bytes;
    uint32_t alloc_count;
    uint32_t free_count;
    uint32_t peak_used;
} mem_stats_t;

// Public API
void      mem_init(void);
void*     mem_alloc(uint32_t size);
void      mem_free(void *ptr);
mem_stats_t mem_get_stats(void);
void      mem_get_bar(char *buf, int width, int *percent);

#endif