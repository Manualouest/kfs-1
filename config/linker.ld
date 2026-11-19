ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5); /* 4 (Read) + 1 (Execute) = R E */
    data PT_LOAD FLAGS(6); /* 4 (Read) + 2 (Write)   = RW  */
}

SECTIONS
{
    . = 1M;

    .text ALIGN(4K) :
    {
        KEEP(*(.multiboot))
        *(.text)
        *(.rodata*)
    } :text

    .data ALIGN(4K) :
    {
        *(.data)
    } :data

    .bss ALIGN(4K) :
    {
        *(COMMON)
        *(.bss)
    } :data
}
