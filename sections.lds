/* software/sections.lds 
   ORIGIN tells our linker to calculate all offsets beginning 
     at the address.
*/
MEMORY {
    ram : ORIGIN = 0x10000000, LENGTH = 0x80000 /* 512KB SRAM */
}

/* use alignment to make sure we end on a 32bit (word) boundary */
SECTIONS {
    .text : {
        *(.text.start)
        *(.text*)
        . = ALIGN(4);
    } > ram

    .rodata : {
        *(.rodata*)
        . = ALIGN(4);
    } > ram

    .data : {
        *(.data*)
        . = ALIGN(4);
    } > ram

    .bss : {
        *(.bss*)
        . = ALIGN(4);
    } > ram
}

