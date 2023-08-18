ENTRY(_hw3_unicorn)

PHDRS 
{
       data_phdr PT_LOAD FLAGS(6); 
       text_phdr PT_LOAD FLAGS(3);
       rodata_phdr PT_LOAD FLAGS(5);
}

SECTIONS
{
    . = 0x60000;

    .data : 
    {
        *(.data)
        *(.bss)
    } : data_phdr

    . = 0x400000;
    .text :
    {
        *(.text)
    } : text_phdr

    . = 0x80000000;
    .rodata :
    {
        *(.rodata)
    } : rodata_phdr
}

EXTERN(white purple)
