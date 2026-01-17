#ifndef SD_H
#define SD_H

#include <stdint.h>

// Initialize the SD Card. Returns 0 on success.
int sd_init(void);

// Read a single 512-byte sector.
// lba: Logical Block Address (Sector Number)
// buffer: Pointer to a 512-byte array
// Returns 0 on success.
int sd_readblock(uint32_t lba, uint8_t *buffer);

// add write for unlink, etc
int sd_writeblock(uint32_t lba, const uint8_t *buffer);

#endif
