CC     = gcc
AS     = nasm
LD     = ld

CFLAGS = -m64 -ffreestanding -fno-builtin -nostdlib -mcmodel=kernel \
         -mno-red-zone -mno-mmx -mno-sse -mno-sse2 \
         -fno-pic -fno-pie -no-pie -I.

# ── Kernel sources ────────────────────────────────────────────────────
C_SOURCES = kernel/kernel.c kernel/idt.c kernel/memory.c \
            kernel/scheduler.c kernel/keyboard.c kernel/timer.c \
            kernel/graphics.c kernel/font.c kernel/shell.c \
            kernel/boot_anim.c kernel/fs.c kernel/mouse.c \
            kernel/mem_mgr.c kernel/elf.c \
            security/trust.c security/sha256.c

C_OBJECTS = $(C_SOURCES:.c=.o)

# ── Default target ────────────────────────────────────────────────────
all: sentinel.bin programs/hello.elf

# ── Kernel boot object ────────────────────────────────────────────────
kernel/boot.o: kernel/boot.asm
	$(AS) -f elf64 kernel/boot.asm -o kernel/boot.o

# ── Kernel C objects ──────────────────────────────────────────────────
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── Kernel binary ─────────────────────────────────────────────────────
sentinel.bin: kernel/boot.o $(C_OBJECTS)
	$(CC) $(CFLAGS) -T kernel/linker.ld -o sentinel.bin $^

# ── User programs ─────────────────────────────────────────────────────
programs/hello.o: programs/hello.asm
	$(AS) -f elf64 programs/hello.asm -o programs/hello.o

programs/hello.elf: programs/hello.o programs/hello.ld
	$(LD) -T programs/hello.ld -o programs/hello.elf programs/hello.o

# ── Clean ─────────────────────────────────────────────────────────────
clean:
	rm -f sentinel.bin
	rm -f kernel/*.o
	rm -f bootloader/*.o bootloader/*.bin
	rm -f $(C_OBJECTS)
	rm -f programs/*.o programs/*.elf

.PHONY: all clean