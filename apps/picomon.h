#ifndef PICOMON_H
#define PICOMON_H

#include <stdint.h>

// --- System Call Table Addresses ---
// These match the 'j' instructions in start.S
#define ADDR_PUTC  0x10000004
#define ADDR_GETC  0x10000008
#define ADDR_PRINT 0x1000000C
#define ADDR_EXEC  0x10000010
#define ADDR_LS    0x10000014

// --- Helper Macros to Call Raw Addresses ---
// This casts the address to a function pointer and calls it
#define SYSCALL_VOID_CHAR(addr, c)   ((void (*)(char))(addr))(c)
#define SYSCALL_CHAR_VOID(addr)      ((char (*)(void))(addr))()
#define SYSCALL_VOID_STR(addr, s)    ((void (*)(char*))(addr))(s)

// --- User Friendly API ---

static inline void putc(char c) {
    SYSCALL_VOID_CHAR(ADDR_PUTC, c);
}

static inline char getc(void) {
    return SYSCALL_CHAR_VOID(ADDR_GETC);
}

static inline void print(char *s) {
    SYSCALL_VOID_STR(ADDR_PRINT, s);
}

static inline void exec(char *filename) {
    SYSCALL_VOID_STR(ADDR_EXEC, filename);
}

static inline void ls(void) {
    SYSCALL_VOID_STR(ADDR_LS, "");
}

// Simple Read Line function for games
static inline void readline(char *buf, int max) {
    int i = 0;
    while (i < max - 1) {
        char c = getc();
        if (c == '\r') {
            putc('\r'); putc('\n');
            break;
        }
        // Handle Backspace
        if (c == 127 || c == 8) {
            if (i > 0) {
                i--;
                putc('\b'); putc(' '); putc('\b');
            }
        } else {
            putc(c); // Echo
            buf[i++] = c;
        }
    }
    buf[i] = 0;
}

// Compare strings (since we don't have stdlib in apps yet)
static inline int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) { s1++; s2++; }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

#endif

