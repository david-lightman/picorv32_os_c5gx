#include <stdint.h>
#include "ff.h" // fat32
#include "sd.h"

// --- Context Switching Types ---
typedef struct {
    uint32_t regs[32]; // x0-x31
} UserContext;

UserContext user_ctx; // Global storage for registers

// Defined in start.S
void run_with_context(uint32_t addr, UserContext *ctx);
void set_time(int y, int m, int d, int h, int min, int s); // Defined in diskio.c

// ==========================================
//  HARDWARE DEFINITIONS
// ==========================================
#define UART_DATA   (*(volatile uint32_t*)0x20000000)
#define UART_STATUS (*(volatile uint32_t*)0x20000004)
#define RX_READY    0x01
#define TX_BUSY     0x02

// TYPES & GLOBALS 
// ==========================================

// 'current_addr' remembers the last memory location we touched.
typedef struct {
    uint32_t current_addr;
} MonitorState;

MonitorState state = { .current_addr = 0x10000000 };

// Function Pointer Type:
// Every command function must accept a string argument (char *args)
// and return nothing (void).
typedef void (*cmd_func_t)(char *args);

// The Command Structure
typedef struct {
    const char *name;       // The command string (e.g., "peek")
    cmd_func_t  func;       // The function to call
    const char *help_text;  // Description for the help command
} Command;

// LOW-LEVEL DRIVERS
// ==========================================

void putc(char c) {
    while (UART_STATUS & TX_BUSY);
    UART_DATA = c;
}

// Non-blocking check for character
int has_char() {
    return (UART_STATUS & RX_READY);
}

char getc() {
    while (!has_char());
    return (char)UART_DATA;
}

void print(const char *str) {
    while (*str) putc(*str++);
}

// Print a 32-bit number as Hex (0x1234ABCD)
void print_hex(uint32_t val) {
    print("0x");
    for (int i = 28; i >= 0; i -= 4) {
        uint32_t nibble = (val >> i) & 0xF;
        putc(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
}

// Print a single byte as two hex characters
void print_byte (uint8_t b) {
    // top four bits MSB
    uint8_t hn = (b >> 4) & 0x0F;
    uint8_t ln = b & 0x0F;

    putc(hn < 10 ? '0' + hn : 'A' + hn - 10);
    putc(ln < 10 ? '0' + ln : 'A' + ln - 10);
}

// STRING HELPERS (No Standard Library!)
// ==========================================

// Compare two strings. Returns 0 if they match.
int k_strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

// Convert Hex String to Integer ("10" -> 16, "FF" -> 255)
uint32_t k_htoi(const char *s) {
    uint32_t val = 0;
    while (*s == ' ') s++; // Skip leading spaces
    while (*s) {
        char c = *s++;
        if (c >= '0' && c <= '9') val = (val << 4) | (c - '0');
        else if (c >= 'a' && c <= 'f') val = (val << 4) | (c - 'a' + 10);
        else if (c >= 'A' && c <= 'F') val = (val << 4) | (c - 'A' + 10);
        else break; 
    }
    return val;
}

// Convert String to Int
uint32_t k_atoi(char *s) {
    uint32_t val = 0;
    while (*s == ' ') s++;
    while (*s >= '0' && *s <= '9') {
        val = (val * 10) + (*s++ - '0');
    }
    return val;
}

// Print Integer with fixed width (e.g., print_dec(5, 2) -> "05")
void print_dec(int val, int width) {
    char buf[12];
    int idx = 0;
    if (val == 0) {
        while (width-- > 0) putc('0');
        return;
    }
    
    // Extract digits
    int temp = val;
    while (temp > 0) {
        buf[idx++] = (temp % 10) + '0';
        temp /= 10;
    }
    
    // Pad with leading zeros
    while (idx < width) {
        putc('0');
        width--;
    }
    
    // Print digits reversed
    while (idx > 0) putc(buf[--idx]);
}

// Import from diskio.c
extern DWORD get_fattime(void);
extern void set_time(int y, int m, int d, int h, int min, int s);



// COMMAND FUNCTIONS
// ==========================================
uint8_t disk_buf[512];  // buffer for SD card ops

void cmd_sd(char *args) {
    print("Initializing SD Card...\r\n");
    
    int res = sd_init();
    if (res != 0) {
        print("Init Failed! Error: ");
        print_hex(res); // -1 = CMD0 fail, -2 = Timeout
        print("\r\n");
        return;
    }
    print("Init OK.\r\n");

    print("Reading Sector 0 (MBR)...\r\n");
    res = sd_readblock(0, disk_buf);
    
    if (res != 0) {
        print("Read Failed! Error: ");
        print_hex(res);
        print("\r\n");
        return;
    }

    // Verify Signature (Last 2 bytes of MBR should be 0x55, 0xAA)
    print("Signature: ");
    print_byte(disk_buf[510]);
    print_byte(disk_buf[511]);
    print("\r\n");
    
    if (disk_buf[510] == 0x55 && disk_buf[511] == 0xAA) {
        print("Valid MBR found!\r\n");
    } else {
        print("Invalid Signature (Expected 55AA)\r\n");
    }
}

void cmd_date(char *args) {
    // print Help string
    if (k_strcmp(args, "-h") == 0 || k_strcmp(args, "--help") == 0) {
        print("Usage:\r\n");
        print("  date                       Show current time\r\n");
        print("  date YYYY MM DD HH MM SS   Set time\r\n");
        return;
    }

    // Show Time (No arguments)
    if (!*args) {
        uint32_t t = get_fattime();
        
        int year = ((t >> 25) & 0x7F) + 1980;
        int mon  = (t >> 21) & 0x0F;
        int day  = (t >> 16) & 0x1F;
        int hour = (t >> 11) & 0x1F;
        int min  = (t >> 5)  & 0x3F;
        int sec  = (t & 0x1F) * 2;

        print("Current Time: ");
        print_dec(year, 4); putc('-');
        print_dec(mon, 2);  putc('-');
        print_dec(day, 2);  putc(' ');
        print_dec(hour, 2); putc(':');
        print_dec(min, 2);  putc(':');
        print_dec(sec, 2);
        print("\r\n");
        return;
    }

    // Set Time (Parse Arguments)
    char *p = args;
    int y = k_atoi(p); while (*p && *p != ' ') p++;
    int m = k_atoi(p); while (*p && *p != ' ') p++;
    int d = k_atoi(p); while (*p && *p != ' ') p++;
    int h = k_atoi(p); while (*p && *p != ' ') p++;
    int min = k_atoi(p); while (*p && *p != ' ') p++;
    int s = k_atoi(p);

    if (y < 1980 || m < 1 || m > 12) {
        print("Invalid date format. Try: date -h\r\n");
        return;
    }

    set_time(y, m, d, h, min, s);
    print("Time updated.\r\n");
}


void cmd_peek(char *args) {
    // If the user typed an address, update our state.
    // If they typed nothing, use the old address 
    if (*args) {
        state.current_addr = k_htoi(args);
    }

    // Read from hardware
    uint32_t val = *(volatile uint32_t*)state.current_addr;

    print("READ  ");
    print_hex(state.current_addr);
    print(" -> ");
    print_hex(val);
    print("\r\n");

    // Auto-increment so the user can just type 'peek' again to see the next word
    state.current_addr += 4; 
}

void cmd_poke(char *args) {
    // We need to split "ADDR VALUE".
    // Or if just "VALUE" is provided, use current_addr.
    
    char *arg1 = args;
    char *arg2 = 0;

    // Find the space between arguments
    char *p = args;
    while (*p) {
        if (*p == ' ') {
            *p = 0;       // Replace space with NULL to split string
            arg2 = p + 1; // Second arg starts after the space
            break;
        }
        p++;
    }

    uint32_t write_addr;
    uint32_t write_val;

    if (arg2) {
        // User typed: poke 1000 FFFF
        write_addr = k_htoi(arg1);
        write_val  = k_htoi(arg2);
    } else {
        // User typed: poke FFFF (Implicitly at current_addr)
        write_addr = state.current_addr;
        write_val  = k_htoi(arg1);
    }

    *(volatile uint32_t*)write_addr = write_val;

    print("WRITE ");
    print_hex(write_addr);
    print(" <- ");
    print_hex(write_val);
    print("\r\n");

    // Update state
    state.current_addr = write_addr + 4;
}

#define USER_PROG_ADDR 0x10008000

void cmd_exec(char *args) {
    if (!*args) {
        print("Usage: exec <filename>\r\n");
        return;
    }

    FIL f;
    FRESULT res;
    UINT bytes_read;

    print("Loading "); print(args); print("...\r\n");

    // 1. Open File
    res = f_open(&f, args, FA_READ);
    if (res != FR_OK) {
        print("Error opening file: "); print_hex(res); print("\r\n");
        return;
    }

    // We load it to a safe offset (32KB into RAM) to avoid overwriting the Kernel
    uint8_t *load_addr = (uint8_t*)USER_PROG_ADDR;
    
    res = f_read(&f, load_addr, f_size(&f), &bytes_read);
    f_close(&f);

    if (res != FR_OK) {
        print("Read Error: "); print_hex(res); print("\r\n");
        return;
    }

    print("Loaded "); print_hex(bytes_read); print(" bytes to "); print_hex(USER_PROG_ADDR); print("\r\n");
    print("Executing...\r\n");

    // We use the trampoline to save registers before jumping
    run_with_context(USER_PROG_ADDR, &user_ctx);

    print("Program Returned.\r\n");
}

void cmd_cls(char *args) {
    print("\033[2J\033[H"); // ANSI Clear Screen
}

// Forward declaration for the table
extern const Command commands[];

void cmd_help(char *args) {
    print("Available Commands:\r\n");
    for (int i = 0; commands[i].name; i++) {
        print("  ");
        print(commands[i].name);
        print("\t : ");
        print(commands[i].help_text);
        print("\r\n");
    }
}

// hex dump command for the monitor
void cmd_dump(char *args) {
    // if args has text, use it, 
    //  otherwise, use state.current_addr.
    if (*args) {
        state.current_addr = k_htoi(args);
    }

    // We want to print 64 bytes total, in chunks of 16.
    for (int line = 0; line < 4; line++) {
        
        uint32_t row_addr = state.current_addr;

        // print header: "1000000000: "
        print_hex(row_addr);
        print(": ");

        // hex loop - we need to look at the next 16 bytes relative to row_addr
        for (int i = 0; i < 16; i++) {
            uint8_t val = *(volatile uint8_t*)(row_addr + i);

            print_byte(val); // print "AF"
            putc(' ');       // space between bytes
        }

        print ("|"); // divider

        // --- ASCII LOOP ---
        // Read the exact same memory again to print the text representation
        for (int i = 0; i < 16; i++) {
            uint8_t val = *(volatile uint8_t*)(row_addr + i);
            
            // Check if it's printable (A-Z, 0-9, punctuation)
            // 32 is Space, 126 is Tilde (~). Anything else is garbage/control codes.
            if (val >= 32 && val <= 126) {
                putc(val);
            } else {
                putc('.'); // Placeholder for non-printable chars
            }
        }

        print("|\r\n"); // End of row

        // Move our global state forward by 16 bytes for the next loop (or next command)
        state.current_addr += 16;
    }
    
}

FATFS fs;      // Filesystem object
FIL file;      // File object

void cmd_ls(char *args) {
    FRESULT res;
    DIR dir;
    FILINFO fno;

    print("Mounting...\r\n");
    res = f_mount(&fs, "", 1); // Mount immediately
    if (res != FR_OK) {
        print("Mount Error: "); print_hex(res); print("\r\n");
        return;
    }

    print("Listing /\r\n");
    res = f_opendir(&dir, "/");
    if (res == FR_OK) {
        while (1) {
            res = f_readdir(&dir, &fno);
            if (res != FR_OK || fno.fname[0] == 0) break; // Error or End of Dir
            
            print(fno.fname);
            if (fno.fattrib & AM_DIR) print("/");
            print("\r\n");
        }
        f_closedir(&dir);
    } else {
        print("OpenDir Error\r\n");
    }
}

void cmd_unlink(char *args) {
    if (!*args) {
        print("Usage: rm <filename>\r\n");
        return;
    }

    print("Deleting "); print(args); print("...\r\n");

    FRESULT res = f_unlink(args);

    if (res == FR_OK) {
        print("Deleted.\r\n");
    } else if (res == FR_NO_FILE) {
        print("File not found.\r\n");
    } else if (res == FR_WRITE_PROTECTED) {
        print("Error: Disk is Write Protected.\r\n");
    } else {
        print("Error: "); print_hex(res); print("\r\n");
    }
}



// COMMANDS 
// This is the "Engine" configuration. To add a command, add one line here.
const Command commands[] = {
    { "cls",    cmd_cls,  "Clear screen" },
    { "date",   cmd_date, "Show or set time" },
    { "dump",   cmd_dump, "[addr] Hex dump memory" },
    { "exec",   cmd_exec, "<addr> Run machine code" },
    { "help",   cmd_help, "Show this list" },
    { "ls",     cmd_ls,   "List directory contents" },
    { "peek",   cmd_peek, "[addr] Read memory" },
    { "poke",   cmd_poke, "[addr] val Write memory" },
    { "sd",     cmd_sd,   "Initialize and test SD card sector" },
    { "unlink", cmd_unlink, "<filename> unlink a file" },
    { 0, 0, 0 } // Sentinel (End of list marker)
};


// ENTRY
// ==========================================
void main() {
    char buffer[64];
    int idx = 0;

    cmd_cls(0);
    print("=== PicoMon v1.0 ===\r\n");
    print("> ");

    while (1) {
        char c = getc();

        if (c == '\r') {
            putc('\r'); putc('\n');
            buffer[idx] = 0; // Null-terminate the string

            // PARSER: Separate "command" from "arguments"
            char *cmd_str = buffer;
            char *arg_str = "";

            // Skip leading spaces
            while (*cmd_str == ' ') cmd_str++;

            // Find the first space after the command
            char *p = cmd_str;
            while (*p) {
                if (*p == ' ') {
                    *p = 0;        // Split command from args
                    arg_str = p + 1; // Args start here
                    while (*arg_str == ' ') arg_str++; // Skip spaces before args
                    break;
                }
                p++;
            }

            // Lookup in Command Table
            if (*cmd_str) {
                int found = 0;
                for (int i = 0; commands[i].name; i++) {
                    // Check if user input matches command name
                    if (k_strcmp(cmd_str, commands[i].name) == 0) {
                        commands[i].func(arg_str); // Execute function!
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    print("Unknown command. Try 'help'.\r\n");
                }
            }

            // Reset for next command
            idx = 0;
            print("> ");
        } 
        else if (c == 127 || c == 8) { // Handle Backspace
            if (idx > 0) {
                idx--;
                putc('\b'); putc(' '); putc('\b');
            }
        }
        else if (idx < 63) {
            buffer[idx++] = c;
            putc(c); // Echo
        }
    }
}
