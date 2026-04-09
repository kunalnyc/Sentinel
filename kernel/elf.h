#ifndef ELF_H
#define ELF_H

#include <stdint.h>

// ELF Magic
#define ELF_MAGIC0 0x7F
#define ELF_MAGIC1 'E'
#define ELF_MAGIC2 'L'
#define ELF_MAGIC3 'F'

// ELF Classes
#define ELFCLASS64  2

// ELF Data Encoding
#define ELFDATA2LSB 1   // Little endian

// ELF Types
#define ET_EXEC     2   // Executable file
#define ET_DYN      3   // Shared object (PIE executables)

// ELF Machine
#define EM_X86_64   62  // AMD x86-64

// Program Header Types
#define PT_NULL     0   // Ignore
#define PT_LOAD     1   // Loadable segment — we care about this
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4

// Program Header Flags
#define PF_X        0x1  // Executable
#define PF_W        0x2  // Writable
#define PF_R        0x4  // Readable

// ─── ELF64 Header ───────────────────────────────────────────────────
typedef struct {
    uint8_t  e_ident[16];   // Magic, class, data, version, OS/ABI
    uint16_t e_type;        // Object file type (ET_EXEC etc.)
    uint16_t e_machine;     // Architecture (EM_X86_64)
    uint32_t e_version;     // ELF version (always 1)
    uint64_t e_entry;       // Entry point virtual address
    uint64_t e_phoff;       // Program header table offset
    uint64_t e_shoff;       // Section header table offset
    uint32_t e_flags;       // Processor-specific flags
    uint16_t e_ehsize;      // ELF header size
    uint16_t e_phentsize;   // Program header entry size
    uint16_t e_phnum;       // Number of program headers
    uint16_t e_shentsize;   // Section header entry size
    uint16_t e_shnum;       // Number of section headers
    uint16_t e_shstrndx;    // Section name string table index
} __attribute__((packed)) Elf64_Ehdr;

// ─── ELF64 Program Header ────────────────────────────────────────────
typedef struct {
    uint32_t p_type;        // Segment type (PT_LOAD etc.)
    uint32_t p_flags;       // Segment flags (PF_R, PF_W, PF_X)
    uint64_t p_offset;      // Offset in file
    uint64_t p_vaddr;       // Virtual address in memory
    uint64_t p_paddr;       // Physical address (usually same as vaddr)
    uint64_t p_filesz;      // Size of segment in file
    uint64_t p_memsz;       // Size of segment in memory (>= filesz)
    uint64_t p_align;       // Alignment
} __attribute__((packed)) Elf64_Phdr;

// ─── ELF Loader Result Codes ─────────────────────────────────────────
#define ELF_OK              0
#define ELF_ERR_MAGIC      -1   // Not an ELF file
#define ELF_ERR_CLASS      -2   // Not 64-bit
#define ELF_ERR_MACHINE    -3   // Not x86-64
#define ELF_ERR_TYPE       -4   // Not executable
#define ELF_ERR_NO_LOAD    -5   // No loadable segments

// ─── Function Declarations ────────────────────────────────────────────
int  elf_validate(Elf64_Ehdr *header);
int  elf_load(uint8_t *elf_data, uint64_t *entry_out);

#endif