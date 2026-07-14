OUTPUT_FORMAT("binary")
ENTRY(_start)

SECTIONS
{
    /* The kernel will be loaded at 1MB physical address */
    . = 0x100000;

    .text : ALIGN(4096)
    {
        /* Ensure the entry point code from kernel_entry.asm is at the very beginning */
        *(.text.prologue)
        *(.text)
    }

    .rodata : ALIGN(4096)
    {
        *(.rodata*)
    }

    .data : ALIGN(4096)
    {
        *(.data)
    }

    .bss : ALIGN(4096)
    {
        _bss_start = .;
        *(COMMON)
        *(.bss)
        _bss_end = .;
    }
}
