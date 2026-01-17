# software/Makefile
CC = riscv64-unknown-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy

# ADDED: -mno-relax to use absolute addresses in the linker script
#  this should fix the issue with global addresses not being set correctly
#CFLAGS = -march=rv32i -mabi=ilp32 -O2 -ffreestanding -nostdlib -mno-relax
#CFLAGS = -march=rv32i -mabi=ilp32 -O2 -ffreestanding -nostdlib -mno-relax -fno-pic
CFLAGS = -march=rv32i -mabi=ilp32 -O2 -ffreestanding -nostdlib -mno-relax -fno-pic -msmall-data-limit=0


all: kernel.bin

kernel.bin: main.c sd.c diskio.c ff.c stdlib.c start.S sections.lds
	$(CC) $(CFLAGS) -Wl,-Bstatic,-T,sections.lds -o kernel.elf start.S main.c sd.c diskio.c ff.c stdlib.c -lgcc
	$(OBJCOPY) -O binary kernel.elf kernel.bin 
	@# Pad to next 512-byte boundary for SD card sector alignment
	truncate -s %512 kernel.bin

clean:
	rm -f *.elf *.bin
