ENTRY(kernel_main)

SECTIONS
{
    . = 0x100000;

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