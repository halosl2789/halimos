OUTPUT_FORMAT("binary")
SECTIONS
{
    . = 0x8000;
    .text : { 
        boot.o(.text) 
        *(.text) 
    }
    .data : { *(.data) }
    .bss  : { *(.bss)  }
}