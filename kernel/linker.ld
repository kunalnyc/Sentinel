ENTRY(_start)

SECTIONS
{
    . = 0x100000;

    .multiboot : ALIGN(8)
    {
        *(.multiboot)
    }

    .text : ALIGN(4096)
    {
        *(.text)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        *(.bss)
    }
}