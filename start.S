/* software/start.S */
.section .text.start
.global _start
.global run_with_context

/* Import C functions */
.global putc
.global getc
.global print
.global cmd_exec
.global cmd_ls

_start:
    j _init       /* 0x10000000 */
    j putc        /* 0x10000004 */
    j getc        /* 0x10000008 */
    j print       /* 0x1000000C */
    j cmd_exec    /* 0x10000010 */
    j cmd_ls      /* 0x10000014 */
    
    .align 4

_init:
    li sp, 0x10080000
    call main
/* ... rest of file ... */


loop:
    j loop

/* 
   run_with_context(uint32_t addr, UserContext *ctx)
   Jumps to 'addr'. When 'addr' returns, it goes to 'trap_entry'.
*/
run_with_context:
    /* 1. Save Kernel Registers */
    addi sp, sp, -64
    sw ra, 60(sp)
    sw s0, 56(sp)
    sw s1, 52(sp)
    sw s2, 48(sp)
    sw s3, 44(sp)
    sw s4, 40(sp)
    sw s5, 36(sp)
    sw s6, 32(sp)
    sw s7, 28(sp)
    sw s8, 24(sp)
    sw s9, 20(sp)
    sw s10, 16(sp)
    sw s11, 12(sp)

    /* 2. Set "Return Address" to our trap handler */
    la ra, trap_entry 

    /* 3. Load User State (Simplified for V1) */
    lw s0, 32(a1)
    lw s1, 36(a1)
    
    /* 4. Jump to User Code */
    mv t0, a0
    jr t0

/* User code returns here via 'ret' */
trap_entry:
    /* 1. Restore Kernel Registers */
    lw ra, 60(sp)
    lw s0, 56(sp)
    lw s1, 52(sp)
    lw s2, 48(sp)
    lw s3, 44(sp)
    lw s4, 40(sp)
    lw s5, 36(sp)  /* FIXED: This was 'sw' in the bad code */
    lw s6, 32(sp)
    lw s7, 28(sp)
    lw s8, 24(sp)
    lw s9, 20(sp)
    lw s10, 16(sp)
    lw s11, 12(sp)
    addi sp, sp, 64
    
    /* 2. Return to C Kernel */
    ret
