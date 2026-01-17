CC = riscv64-unknown-elf-gcc
OBJCOPY = riscv64-unknown-elf-objcopy

CFLAGS = -march=rv32i -mabi=ilp32 -ffreestanding -nostdlib
LINKER_FLAGS = -Wl,-Ttext=0x10008000

# Explicitly list sources if wildcards fail
SRCS = $(wildcard *.c)
BINS = $(SRCS:.c=.bin)

all: $(BINS)

# The Pattern Rule
# Note: We force crt0.S to be the first source file passed to gcc
%.bin: %.c
	@echo "Building $@"
	$(CC) $(CFLAGS) $(LINKER_FLAGS) -o $*.elf crt0.S $< -lgcc
	$(OBJCOPY) -O binary $*.elf $@

clean:
	rm -f *.elf *.bin
