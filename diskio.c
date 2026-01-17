#include "ff.h"
#include "diskio.h"
#include "sd.h"

// Check Status (Always OK for now)
DSTATUS disk_status(BYTE pdrv) {
    return 0;
}

// Initialize Disk
DSTATUS disk_initialize(BYTE pdrv) {
    if (sd_init() == 0) return 0;
    return STA_NOINIT;
}

// Read Sector(s)
DRESULT disk_read(BYTE pdrv, BYTE *buff, LBA_t sector, UINT count) {
    // FatFs might ask for multiple sectors, so we loop
    for (int i = 0; i < count; i++) {
        if (sd_readblock(sector + i, buff + (i * 512)) != 0) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}

// Write sectors
DRESULT disk_write(BYTE pdrv, const BYTE *buff, LBA_t sector, UINT count) {
    for (int i = 0; i < count; i++) {
        if (sd_writeblock(sector + i, buff + (i * 512)) != 0) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}

// IOCTL (Required for some FatFs features, minimal implementation)
DRESULT disk_ioctl(BYTE pdrv, BYTE cmd, void *buff) {
    return RES_OK;
}

// Timekeeping - needed for file timestamps

// Default: Jan 17, 2026 12:00:00
// Packed Format: ((Year-1980)<<25) | (Month<<21) | (Day<<16) | (Hour<<11) | (Min<<5) | (Sec/2)
volatile DWORD current_fattime = 
    ((DWORD)(2026 - 1980) << 25) | 
    ((DWORD)1 << 21) | 
    ((DWORD)17 << 16) | 
    ((DWORD)12 << 11);

DWORD get_fattime(void) {
    return current_fattime;
}

void set_time(int y, int m, int d, int h, int min, int s) {
    current_fattime = ((DWORD)(y - 1980) << 25) | 
                      ((DWORD)m << 21) | 
                      ((DWORD)d << 16) | 
                      ((DWORD)h << 11) | 
                      ((DWORD)min << 5) | 
                      ((DWORD)(s / 2));
}
