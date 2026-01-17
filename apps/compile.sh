# 1. Compile to Object
riscv64-unknown-elf-gcc -c -march=rv32i -mabi=ilp32 -ffreestanding -nostdlib -o adventure.o adventure.c

# 2. Link to Binary (Start at 0x10008000)
# We need a tiny entry point for the app too, but usually GCC handles 'main' if we are careful.
# Let's use a one-liner linker command:
riscv64-unknown-elf-ld -Ttext=0x10008000 -o adventure.elf adventure.o

# 3. Extract Binary
riscv64-unknown-elf-objcopy -O binary adventure.elf adventure.bin

