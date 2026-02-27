CC = gcc
CFLAGS = -m32 -ffreestanding -fno-builtin -nostdlib
NASM = nasm
NASMFLAGS = -f elf32

all:
	$(NASM) $(NASMFLAGS) kernel/boot.asm -o kernel/boot.o
	$(CC) $(CFLAGS) -T kernel/linker.ld -o sentinel.bin kernel/boot.o kernel/kernel.c security/trust.c security/sha256.c kernel/idt.c kernel/memory.c kernel/scheduler.c kernel/keyboard.c kernel/timer.c

clean:
	rm -f sentinel.bin kernel/boot.o