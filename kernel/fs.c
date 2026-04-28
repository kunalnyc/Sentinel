#include "fs.h"

// The entire filesystem lives in RAM
static slfs_t fs;

// In fs.c or kernel.c, declare these:
extern uint8_t _binary_programs_proc1_elf_start[];
extern uint8_t _binary_programs_proc1_elf_end[];
extern uint8_t _binary_programs_proc2_elf_start[];
extern uint8_t _binary_programs_proc2_elf_end[];
extern uint8_t _binary_programs_proc3_elf_start[];
extern uint8_t _binary_programs_proc3_elf_end[];
// ── String helpers (no stdlib) ─────────────────────────────
static int sl_strlen(const char *s)
{
    int i = 0;
    while(s[i]) i++;
    return i;
}

static int sl_strcmp(const char *a, const char *b)
{
    int i = 0;
    while(a[i] && b[i] && a[i] == b[i]) i++;
    return a[i] - b[i];
}

static void sl_strcpy(char *dst, const char *src, int max)
{
    int i = 0;
    while(src[i] && i < max - 1) { dst[i] = src[i]; i++; }
    dst[i] = 0;
}

static void sl_memcpy(uint8_t *dst, const uint8_t *src, uint32_t n)
{
    uint32_t i;
    for(i = 0; i < n; i++) dst[i] = src[i];
}

static void sl_memset(uint8_t *dst, uint8_t val, uint32_t n)
{
    uint32_t i;
    for(i = 0; i < n; i++) dst[i] = val;
}

// ── Internal helpers ───────────────────────────────────────
static int find_file(const char *name)
{
    int i;
    for(i = 0; i < SLFS_MAX_FILES; i++)
    {
        if(fs.files[i].type != SLFS_TYPE_EMPTY &&
           sl_strcmp(fs.files[i].name, name) == 0)
            return i;
    }
    return -1;
}

static int find_empty(void)
{
    int i;
    for(i = 0; i < SLFS_MAX_FILES; i++)
        if(fs.files[i].type == SLFS_TYPE_EMPTY)
            return i;
    return -1;
}

static uint32_t tick = 0;
static uint32_t get_time(void) { return tick++; }

// ── Public API ─────────────────────────────────────────────

void fs_init(void)
{
    // Zero entire filesystem
    uint32_t i;
    uint8_t *p = (uint8_t*)&fs;
    for(i = 0; i < sizeof(slfs_t); i++) p[i] = 0;

    fs.magic       = SLFS_MAGIC;
    fs.version     = 1;
    fs.total_files = SLFS_MAX_FILES;
    fs.used_files  = 0;

    // Create default files
    fs_create("readme.txt");
    fs_write("readme.txt",
        "SENTINELOS FILE SYSTEM V1.0. "
        "TRUST NOTHING. VERIFY EVERYTHING.",
        64);

    fs_create("kernel.log");
    fs_write("kernel.log",
        "KERNEL BOOT OK. ALL SYSTEMS ONLINE.",
        36);

    // Embed proc ELFs into filesystem
    extern uint8_t _binary_programs_proc1_elf_start[];
    extern uint8_t _binary_programs_proc1_elf_end[];
    extern uint8_t _binary_programs_proc2_elf_start[];
    extern uint8_t _binary_programs_proc2_elf_end[];
    extern uint8_t _binary_programs_proc3_elf_start[];
    extern uint8_t _binary_programs_proc3_elf_end[];

    uint32_t s1 = (uint32_t)(_binary_programs_proc1_elf_end
                           - _binary_programs_proc1_elf_start);
    uint32_t s2 = (uint32_t)(_binary_programs_proc2_elf_end
                           - _binary_programs_proc2_elf_start);
    uint32_t s3 = (uint32_t)(_binary_programs_proc3_elf_end
                           - _binary_programs_proc3_elf_start);

 fs_create("PROC1");
fs_write("PROC1", (char*)_binary_programs_proc1_elf_start, s1);

fs_create("PROC2");
fs_write("PROC2", (char*)_binary_programs_proc2_elf_start, s2);

fs_create("PROC3");
fs_write("PROC3", (char*)_binary_programs_proc3_elf_start, s3);
}

int fs_create(const char *name)
{
    if(sl_strlen(name) == 0)          return SLFS_ERR_NOTFOUND;
    if(find_file(name) >= 0)          return SLFS_ERR_EXISTS;
    int slot = find_empty();
    if(slot < 0)                      return SLFS_ERR_FULL;

    sl_memset((uint8_t*)&fs.files[slot], 0, sizeof(slfs_file_t));
    sl_strcpy(fs.files[slot].name, name, SLFS_MAX_FILENAME);
    fs.files[slot].type     = SLFS_TYPE_FILE;
    fs.files[slot].size     = 0;
    fs.files[slot].created  = get_time();
    fs.files[slot].modified = fs.files[slot].created;
    fs.used_files++;
    return SLFS_OK;
}

int fs_write(const char *name, const char *data, uint32_t len)
{
    int idx = find_file(name);
    if(idx < 0)                        return SLFS_ERR_NOTFOUND;
    if(len > SLFS_MAX_FILESIZE)        return SLFS_ERR_TOOBIG;

    sl_memcpy(fs.files[idx].data, (const uint8_t*)data, len);
    fs.files[idx].size     = len;
    fs.files[idx].modified = get_time();
    return SLFS_OK;
}

int fs_read(const char *name, char *buf, uint32_t buflen)
{
    int idx = find_file(name);
    if(idx < 0) return SLFS_ERR_NOTFOUND;

    uint32_t sz = fs.files[idx].size;
    if(sz >= buflen) sz = buflen - 1;
    sl_memcpy((uint8_t*)buf, fs.files[idx].data, sz);
    buf[sz] = 0;
    return (int)sz;
}

int fs_delete(const char *name)
{
    int idx = find_file(name);
    if(idx < 0) return SLFS_ERR_NOTFOUND;

    sl_memset((uint8_t*)&fs.files[idx], 0, sizeof(slfs_file_t));
    fs.used_files--;
    return SLFS_OK;
}

int fs_exists(const char *name)
{
    return find_file(name) >= 0 ? 1 : 0;
}

int fs_list(char out[][SLFS_MAX_FILENAME], int *count)
{
    *count = 0;
    int i;
    for(i = 0; i < SLFS_MAX_FILES; i++)
    {
        if(fs.files[i].type != SLFS_TYPE_EMPTY)
        {
            sl_strcpy(out[*count], fs.files[i].name, SLFS_MAX_FILENAME);
            (*count)++;
        }
    }
    return SLFS_OK;
}

uint32_t fs_used(void)  { return fs.used_files; }
uint32_t fs_total(void) { return SLFS_MAX_FILES; }