#include "picomon.h"

// Entry point is always main
void main() {
    char buf[64];

    print("\033[2J\033[H"); // Clear Screen
    print("================================\r\n");
    print("   DUNGEON OF THE FPGA WIZARD   \r\n");
    print("================================\r\n");
    print("\r\n");
    print("You stand in a dark server room. LEDs blink ominously.\r\n");
    
    while (1) {
        print("\r\nWhat do you want to do? (look/go north/quit)\r\n> ");
        readline(buf, 64);

        if (strcmp(buf, "look") == 0) {
            print("You see a flashing 'CONFIG_DONE' light and a scary 'vmlinuz' file.\r\n");
        } 
        else if (strcmp(buf, "go north") == 0) {
            print("You trip over a JTAG cable and fall into the void.\r\n");
            print("GAME OVER.\r\n");
            break;
        }
        else if (strcmp(buf, "quit") == 0) {
            print("Returning to PicoMon...\r\n");
            break;
        }
        else {
            print("I don't understand '");
            print(buf);
            print("'.\r\n");
        }
    }
    
    // Function simply returns to the monitor
}

