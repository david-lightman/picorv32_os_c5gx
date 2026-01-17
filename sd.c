#include "sd.h"

#define SD_PORT   (*(volatile uint32_t*)0x30000000)
#define PIN_SCK   1
#define PIN_MOSI  2
#define PIN_CS    4

static uint8_t spi_byte(uint8_t out) {
    uint8_t in = 0;
    for (int i = 7; i >= 0; i--) {
        int bit = (out >> i) & 1;
        int mosi_mask = bit ? PIN_MOSI : 0;
        SD_PORT = mosi_mask; 
        SD_PORT = mosi_mask | PIN_SCK;
        if (SD_PORT & 1) in |= (1 << i);
    }
    return in;
}

static uint8_t sd_cmd(uint8_t cmd, uint32_t arg, uint8_t crc) {
    SD_PORT = PIN_MOSI; 
    spi_byte(0xFF);
    spi_byte(cmd | 0x40);
    spi_byte(arg >> 24); spi_byte(arg >> 16); spi_byte(arg >> 8); spi_byte(arg);
    spi_byte(crc);
    uint8_t r = 0xFF;
    for (int i = 0; i < 100; i++) {
        r = spi_byte(0xFF);
        if ((r & 0x80) == 0) break;
    }
    return r;
}

int sd_init(void) {
    SD_PORT = PIN_CS | PIN_MOSI;
    for (int i = 0; i < 10; i++) spi_byte(0xFF);
    int retries = 100;
    while (sd_cmd(0, 0, 0x95) != 0x01 && retries-- > 0) spi_byte(0xFF); 
    if (retries <= 0) return -1;
    sd_cmd(8, 0x1AA, 0x87);
    retries = 20000;
    while (retries--) {
        sd_cmd(55, 0, 0xFF);
        if (sd_cmd(41, 0x40000000, 0xFF) == 0x00) return 0;
    }
    return -2;
}

int sd_readblock(uint32_t lba, uint8_t *buffer) {
    if (sd_cmd(17, lba, 0xFF) != 0x00) return -1;
    int timeout = 20000;
    while (spi_byte(0xFF) != 0xFE && timeout-- > 0);
    if (timeout <= 0) return -2;
    for (int i = 0; i < 512; i++) *buffer++ = spi_byte(0xFF);
    spi_byte(0xFF); spi_byte(0xFF);
    SD_PORT = PIN_CS | PIN_MOSI;
    spi_byte(0xFF);
    return 0;
}

/* 
int sd_writebock(uint32_t lba, const uint8_t *buffer);
Sends CMD24 (Write Block). 
The protocol is strict: 
    Send Command -> Send Data Token (0xFE) -> Send 512 Bytes 
                 -> Wait for "Accepted" response -> Wait for Busy signal to stop.
*/
int sd_writeblock(uint32_t lba, const uint8_t *buffer) {
    //  Send CMD24 (Write Single Block)
    if (sd_cmd(24, lba, 0xFF) != 0x00) return -1; // Command rejected

    // Send Start Token (0xFE) slightly different than read
    spi_byte(0xFF); // One byte gap
    spi_byte(0xFE); 

    //  Write 512 Bytes
    for (int i = 0; i < 512; i++) {
        spi_byte(*buffer++);
    }

    // Send Dummy CRC
    spi_byte(0xFF);
    spi_byte(0xFF);

    //  Check Data Response
    // The card responds immediately. Format: xxx00101 (0x05) = Accepted
    uint8_t resp = spi_byte(0xFF);
    if ((resp & 0x1F) != 0x05) return -2; // Write Error (CRC or Write Error)

    //  Wait for Busy (Card pulls line low while writing internally)
    int timeout = 1000000; // Writes take time!
    while (spi_byte(0xFF) == 0x00) {
        if (timeout-- <= 0) return -3; // Timeout
    }

    //  Cleanup
    SD_PORT = PIN_CS | PIN_MOSI; // Release CS
    spi_byte(0xFF);
    
    return 0;
}
