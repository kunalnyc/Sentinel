CC = gcc
CFLAGS = -m64 -ffreestanding -fno-builtin -nostdlib -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -fno-pic -fno-pie -no-pie -I.

# List all source files
C_SOURCES = kernel/kernel.c security/trust.c security/sha256.c kernel/idt.c \
            kernel/memory.c kernel/scheduler.c kernel/keyboard.c kernel/timer.c \
            kernel/graphics.c kernel/font.c 

C_OBJECTS = $(C_SOURCES:.c=.o)

all: sentinel.bin

kernel/boot.o: kernel/boot.asm
	nasm -f elf64 kernel/boot.asm -o kernel/boot.o

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

sentinel.bin: kernel/boot.o $(C_OBJECTS)
	$(CC) $(CFLAGS) -T kernel/linker.ld -o sentinel.bin $^

clean:
	rm -f sentinel.bin kernel/*.o bootloader/*.o bootloader/*.bin $(C_OBJECTS)

.PHONY: all clean