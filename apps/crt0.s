/* apps/crt0.S 
    An attempt to replace the C Runtime Zero - Startup File
*/
.section .text
.global _start

_start:
    /* the RISC-V ABI requires 16-byte stack alignment even when
         calling (4-byte) functions that do not use vector/SIMD instructions.
         So we ensure the stack is aligned before calling main().
     */
    addi sp, sp, -16 /* note: sw <src>, <dst> for store  -- not load */
    sw ra, 12(sp)   /* save return address register from our monitor onto the stack */
                    /*  we reserved 16 bytes.. so we save it to the top 4 bytes */

    /* 2. Call C Main */
    call main

    /* 3. Restore and Return to Monitor */
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

