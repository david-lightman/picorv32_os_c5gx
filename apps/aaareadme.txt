================================================================================
   PicoMon OS - Project Documentation & Developer Guide
   Platform: PicoRV32 (RISC-V Softcore) on Altera Cyclone V GX
   Date: January 17, 2026
================================================================================

1. PROJECT SUMMARY
--------------------------------------------------------------------------------
This is a custom, bare-metal Operating System built from scratch.
It consists of two distinct parts:
1. THE KERNEL (software/): Manages hardware, SD card (FAT32), and loads apps.
2. USER APPS (apps/): Standalone C programs loaded dynamically from the SD card.

Key Features:
- Filesystem: FAT32 (via ChaN's FatFs) over bit-banged SPI.
- Loader: Loads .bin files from SD card to address 0x10008000 and executes them.
- System Calls: Apps talk to the Kernel via a fixed Jump Table (0x10000004).
- Runtime: Custom crt0.S handles stack setup/teardown for C apps.

================================================================================
2. QUICK START: HOW TO BUILD & RUN
================================================================================

A. Building the Kernel (The OS)
   1. cd software
   2. make clean && make
      (Output: kernel.bin, padded to 512-byte alignment)

B. Building Apps (Your C Programs)
   1. cd apps
   2. make
      (Output: *.bin files for every *.c file found)

C. Flashing / Deploying (The Workflow)
   Run ./flash.sh from the root directory. It has two modes:

   [1] Flash Kernel: Unmounts disk, writes 'kernel.bin' to Sector 1 (Raw DD).
       -> DO THIS when you change main.c, start.S, or drivers.

   [2] Install Apps: Mounts disk, copies 'apps/*.bin' to the FAT32 partition.
       -> DO THIS when you compile a new game or tool.

   NOTE: You usually do [1] once, and [2] many times.

D. Running on FPGA
   1. Connect UART terminal (115200 baud).
   2. Press FPGA Reset button.
   3. Type 'ls' to see files.
   4. Type 'exec appname.bin' to run a program.

================================================================================
3. DEVELOPING USER APPS (THE "MAGIC")
================================================================================
To write a new program (e.g., 'spaceship.c'), you must understand the
Environment. We are running WITHOUT a standard OS (Linux/Windows), so we
fake it.

[ The Components ]

1. apps/crt0.S (C Runtime Zero)
   - PROBLEM: When the Kernel jumps to your App, the CPU is in a specific state.
     If your C 'main()' executes immediately, it might corrupt registers or
     fail to return correctly.
   - SOLUTION: crt0.S is the *true* entry point. It:
     a. Saves the OS Return Address (ra) and Stack Pointer (sp).
     b. Calls 'main()'.
     c. Restores (ra) and (sp) when main returns.
     d. Executes 'ret' to go back to the OS Monitor.

2. apps/picomon.h (The API)
   - PROBLEM: Your App doesn't know where 'putc' or 'print' are located in RAM.
   - SOLUTION: We defined a "Jump Table" in the Kernel at 0x10000004.
     This header defines macros that redirect function calls to those fixed
     addresses.
     Example: print("Hi") -> calls address 0x1000000C.

3. apps/Makefile (The Build Logic)
   - Links your code to start at 0x10008000 (User Space).
   - Automatically compiles 'crt0.S' and links it *in front* of your C code.
   - Links '-lgcc' to handle software math (multiply/divide).

[ How to Create a New App ]
1. Create 'apps/mytool.c'.
2. Add '#include "picomon.h"' at the top.
3. Write 'void main() { ... }'.
4. Run 'make' inside the apps/ directory.

================================================================================
4. TROUBLESHOOTING GUIDE (READ THIS IN 6 MONTHS)
================================================================================

ERROR: "undefined reference to `__mulsi3`" or `__divsi3`
CAUSE: You used '*' or '/' in your code. The RV32I CPU has no hardware math.
FIX:   Edit Makefile and ensure '-lgcc' is added to the linker command.
       Example: $(CC) ... -o app.elf crt0.S app.c -lgcc

ERROR: App runs but crashes/hangs when I type "return" or exit.
CAUSE: crt0.S is missing or broken. The App destroyed the OS stack pointer.
FIX:   Ensure your Makefile puts 'crt0.S' as the FIRST dependency.

ERROR: App crashes *immediately* upon 'exec'.
CAUSE: 
   1. The App was compiled for the wrong address. (Check -Ttext=0x10008000).
   2. The FPGA is running an OLD Kernel without the Jump Table.
FIX:   Run ./flash.sh and select Option 1 (Flash Kernel).

ERROR: "No rule to make target `adventure.bin`"
CAUSE: You are running 'make' from the wrong directory.
FIX:   'cd apps' before running make.

ERROR: 'ls' shows files like '._game.bin'
CAUSE: macOS clutter (resource forks).
FIX:   Run 'dot_clean apps/' on your Mac before flashing.

ERROR: "exec game" says "Error opening file: 0x04"
CAUSE: You forgot the extension. FatFs is strict.
FIX:   Type "exec game.bin".

================================================================================
5. MEMORY MAP REFERENCE
================================================================================
0x00000000  Boot ROM (Hardware)
0x10000000  Kernel Entry (_init)
0x10000004  Jump Table (putc)  <-- Apps call this
0x10000008  Jump Table (getc)
0x1000000C  Jump Table (print)
0x10000010  Jump Table (exec)
0x10000014  Jump Table (ls)
...
0x10008000  User App Load Address (crt0.S starts here)
...
0x10080000  Top of Stack (Grows Down)
