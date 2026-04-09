#include "elf.h"
#include "memory.h"
#include "graphics.h"

// ─── Validate ELF Header ─────────────────────────────────────────────
int elf_validate(Elf64_Ehdr *header)
{
    // Check magic bytes: 0x7F 'E' 'L' 'F'
    if (header->e_ident[0] != ELF_MAGIC0 ||
        header->e_ident[1] != ELF_MAGIC1 ||
        header->e_ident[2] != ELF_MAGIC2 ||
        header->e_ident[3] != ELF_MAGIC3)
    {
        return ELF_ERR_MAGIC;
    }

    // Must be 64-bit
    if (header->e_ident[4] != ELFCLASS64)
        return ELF_ERR_CLASS;

    // Must be x86-64
    if (header->e_machine != EM_X86_64)
        return ELF_ERR_MACHINE;

    // Must be executable or PIE
    if (header->e_type != ET_EXEC && header->e_type != ET_DYN)
        return ELF_ERR_TYPE;

    return ELF_OK;
}

// ─── Load ELF Binary Into Memory ─────────────────────────────────────
int elf_load(uint8_t *elf_data, uint64_t *entry_out)
{
    // Cast the start of the data to an ELF header
    Elf64_Ehdr *header = (Elf64_Ehdr *)elf_data;

    // Validate first
    int result = elf_validate(header);
    if (result != ELF_OK)
        return result;

    // Walk program headers
    int loaded = 0;
    int i;
    for (i = 0; i < header->e_phnum; i++)
    {
        // Get pointer to this program header
        Elf64_Phdr *phdr = (Elf64_Phdr *)(elf_data + header->e_phoff
                                          + i * header->e_phentsize);

        // We only care about PT_LOAD segments
        if (phdr->p_type != PT_LOAD)
            continue;

        // Source: where in the ELF file this segment lives
        uint8_t *src = elf_data + phdr->p_offset;

        // Destination: virtual address this segment wants to live at
        uint8_t *dst = (uint8_t *)phdr->p_vaddr;

        // Copy file bytes into memory (filesz bytes)
        uint64_t j;
        for (j = 0; j < phdr->p_filesz; j++)
            dst[j] = src[j];

        // Zero out the rest (memsz - filesz) — this is the BSS region
        for (j = phdr->p_filesz; j < phdr->p_memsz; j++)
            dst[j] = 0;

        loaded++;
    }

    // No loadable segments found
    if (loaded == 0)
        return ELF_ERR_NO_LOAD;

    // Return the entry point
    *entry_out = header->e_entry;

    return ELF_OK;
}