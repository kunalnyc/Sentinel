CC = gcc
CFLAGS = -m64 -ffreestanding -fno-builtin -nostdlib -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -fno-pic -fno-pie
LDFLAGS = -nostdlib -no-pie

NASM = nasm
NASMFLAGS = -f elf64

all:
	$(NASM) -f bin bootloader/boot.asm -o bootloader/boot.bin
	$(NASM) $(NASMFLAGS) kernel/boot.asm -o kernel/boot.o
	$(CC) $(CFLAGS) $(LDFLAGS) -T kernel/linker.ld -o sentinel.bin kernel/boot.o kernel/kernel.c security/trust.c security/sha256.c kernel/idt.c kernel/memory.c kernel/scheduler.c kernel/keyboard.c kernel/timer.c kernel/graphics.c kernel/font.c

clean:
	rm -f sentinel.bin kernel/boot.o bootloader/boot.o bootloader/boot.bin