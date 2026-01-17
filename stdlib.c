#include <stdint.h>
#include <stddef.h>

// --- Memory Functions ---

void *memset(void *dst, int c, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    while (n--) *d++ = c;
    return dst;
}

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *d = (uint8_t *)dst;
    const uint8_t *s = (const uint8_t *)src;
    while (n--) *d++ = *s++;
    return dst;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    const uint8_t *p1 = (const uint8_t *)s1;
    const uint8_t *p2 = (const uint8_t *)s2;
    while (n--) {
        if (*p1 != *p2) return *p1 - *p2;
        p1++; p2++;
    }
    return 0;
}

// --- String Functions ---

char *strchr(const char *s, int c) {
    while (*s != (char)c) {
        if (!*s++) return 0;
    }
    return (char *)s;
}

// Check if FatFs needs this (sometimes it uses strlen)
size_t strlen(const char *s) {
    const char *p = s;
    while (*p) p++;
    return p - s;
}

