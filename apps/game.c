#include "picomon.h"

void main() {
    print("\r\n=== WELCOME TO THE GAME ===\r\n");
    print("You are in a maze of twisty little passages, all alike.\r\n");
    print("Press 'q' to quit.\r\n");

    while (1) {
        char c = getc();
        if (c == 'q') break;
        putc(c); // Echo
    }
    print("\r\nBye!\r\n");
}

