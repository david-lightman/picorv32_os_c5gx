/* apps/app.S */
.section .text
.global _start

/* We call the Fixed Jump Table, NOT the dynamic function address */
.equ SYS_PUTC,  0x10000004
.equ SYS_PRINT, 0x1000000C

_start:
    /* Stack Setup */
    addi sp, sp, -16
    sw ra, 12(sp)

    /* Print 'H' */
    li a0, 0x48
    li t0, SYS_PUTC
    jalr ra, 0(t0)

    /* Print 'i' */
    li a0, 0x69
    li t0, SYS_PUTC
    jalr ra, 0(t0)

    /* Print " World" using the print syscall */
    la a0, msg
    li t0, SYS_PRINT
    jalr ra, 0(t0)

    /* Return */
    lw ra, 12(sp)
    addi sp, sp, 16
    ret

msg:
    .string " World!\r\n"
