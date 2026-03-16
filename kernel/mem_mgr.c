#include "mem_mgr.h"

// Heap pointer - starts at HEAP_START
static uint8_t  heap[HEAP_SIZE];
static uint32_t heap_ptr = 0;

// Allocation table
static mem_block_t blocks[MAX_ALLOCS];
static int         block_count = 0;

// Stats
static mem_stats_t stats;

// ── Helpers ──────────────────────────────────────────

static void mem_zero_block(mem_block_t *b)
{
    b->magic  = 0;
    b->size   = 0;
    b->in_use = 0;
    b->addr   = 0;
}

// ── Init ─────────────────────────────────────────────

void mem_init(void)
{
    int i;
    // Zero heap
    for(i = 0; i < HEAP_SIZE; i++) heap[i] = 0;

    // Zero blocks
    for(i = 0; i < MAX_ALLOCS; i++) mem_zero_block(&blocks[i]);

    heap_ptr    = 0;
    block_count = 0;

    stats.total_bytes = HEAP_SIZE;
    stats.used_bytes  = 0;
    stats.free_bytes  = HEAP_SIZE;
    stats.alloc_count = 0;
    stats.free_count  = 0;
    stats.peak_used   = 0;
}

// ── Alloc ─────────────────────────────────────────────

void* mem_alloc(uint32_t size)
{
    if(size == 0) return (void*)0;
    if(size > HEAP_SIZE - heap_ptr) return (void*)0;
    if(block_count >= MAX_ALLOCS) return (void*)0;

    // Align to 4 bytes
    uint32_t aligned = (size + 3) & ~3;

    // Record block
    blocks[block_count].magic  = BLOCK_MAGIC;
    blocks[block_count].size   = aligned;
    blocks[block_count].in_use = 1;
    blocks[block_count].addr   = heap_ptr;
    block_count++;

    // Get pointer into heap
    void *ptr = (void*)&heap[heap_ptr];
    heap_ptr += aligned;

    // Update stats
    stats.used_bytes  += aligned;
    stats.free_bytes   = HEAP_SIZE - stats.used_bytes;
    stats.alloc_count++;
    if(stats.used_bytes > stats.peak_used)
        stats.peak_used = stats.used_bytes;

    return ptr;
}

// ── Free ──────────────────────────────────────────────

void mem_free(void *ptr)
{
    if(!ptr) return;

    // Find matching block by address
    uint32_t offset = (uint32_t)((uint8_t*)ptr - heap);
    int i;
    for(i = 0; i < block_count; i++)
    {
        if(blocks[i].addr == offset && blocks[i].in_use)
        {
            blocks[i].in_use = 0;

            // Zero out the freed memory
            uint32_t j;
            for(j = 0; j < blocks[i].size; j++)
                heap[offset + j] = 0;

            stats.used_bytes -= blocks[i].size;
            stats.free_bytes  = HEAP_SIZE - stats.used_bytes;
            stats.free_count++;
            return;
        }
    }
}

// ── Stats ─────────────────────────────────────────────

mem_stats_t mem_get_stats(void)
{
    stats.free_bytes = HEAP_SIZE - stats.used_bytes;
    return stats;
}

// ── RAM bar ───────────────────────────────────────────
// Fills buf with ASCII progress bar, sets percent

void mem_get_bar(char *buf, int width, int *percent)
{
    if(stats.total_bytes == 0) { buf[0]=0; *percent=0; return; }

    *percent = (int)((stats.used_bytes * 100) / stats.total_bytes);
    int fill = (width * (*percent)) / 100;

    int i = 0;
    buf[i++] = '[';
    int j;
    for(j = 0; j < width; j++)
    {
        if(j < fill)      buf[i++] = '=';
        else if(j == fill) buf[i++] = '>';
        else               buf[i++] = ' ';
    }
    buf[i++] = ']';
    buf[i]   = 0;
}