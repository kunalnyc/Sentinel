# ⬡ SentinelOS

<div align="center">

```
        ◆
       ╱│╲
      ╱ │ ╲
     ╱  │  ╲
    ◆───┼───◆
     ╲  │  ╱
      ╲ │ ╱
       ╲│╱
        ◆
   S E N T I N E L
```

### *"Trust Nothing. Verify Everything."*

[![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)
![Build](https://img.shields.io/badge/build-PASSING-00FF64?style=for-the-badge)
![Arch](https://img.shields.io/badge/arch-x86__64-FFC832?style=for-the-badge)
![Version](https://img.shields.io/badge/version-0.1--alpha-0096FF?style=for-the-badge)

**A 64-bit operating system where every process must earn its right to execute.**  
Built from scratch in Assembly + C. Forerunner-inspired UI. Zero-trust kernel architecture.

</div>

---

## ◈ What Is SentinelOS?

SentinelOS is a **bare-metal 64-bit operating system** built from scratch — no Linux, no POSIX, no borrowed kernel. Every line of code, from the bootloader to the security layer, was written by hand.

The core idea is simple and absolute:

> *No process is trusted by default. Every thread, every driver, and every byte of code must prove its identity before touching the CPU.*

While traditional operating systems authenticate once and trust forever, SentinelOS implements a **Continuous Verification Model** — identity is re-checked at every critical boundary.

---

## ◈ Boot Sequence

```
┌─────────────────────────────────────────────────────────────┐
│                     SENTINEL BOOT FLOW                      │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  BIOS / UEFI                                                │
│      │                                                      │
│      ▼                                                      │
│  GRUB Multiboot2  ──────────────────── loads sentinel.bin  │
│      │                                                      │
│      ▼                                                      │
│  boot.asm [32-bit]  ──── sets up 4-level page tables       │
│      │                   enables PAE + Long Mode            │
│      │                   loads 64-bit GDT                   │
│      ▼                                                      │
│  boot.asm [64-bit]  ──── jumps to kernel_main              │
│      │                                                      │
│      ▼                                                      │
│  kernel_main()      ──── initializes all subsystems        │
│                          launches Forerunner UI             │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

## ◈ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                        USER SPACE                           │
│   ┌──────────────┐  ┌──────────────┐  ┌──────────────┐     │
│   │  Process A   │  │  Process B   │  │  Process C   │     │
│   │  ID: 0x7A3F  │  │  ID: 0x9C2B  │  │  ID: 0x4D81  │     │
│   └──────┬───────┘  └──────┬───────┘  └──────┬───────┘     │
└──────────┼─────────────────┼─────────────────┼─────────────┘
           │                 │                 │
┌──────────▼─────────────────▼─────────────────▼─────────────┐
│                    VERIFICATION GATE                        │
│         Identity Check ◆ Hash Verify ◆ Permission          │
└──────────────────────────┬──────────────────────────────────┘
                           │
┌──────────────────────────▼──────────────────────────────────┐
│                      KERNEL SPACE                           │
│  ┌───────────┐  ┌───────────┐  ┌───────────┐  ┌─────────┐  │
│  │ Scheduler │  │  Memory   │  │    IDT    │  │  Trust  │  │
│  │  Shield   │  │   Guard   │  │ Guardian  │  │Registry │  │
│  └───────────┘  └───────────┘  └───────────┘  └─────────┘  │
│                                                             │
│  ┌───────────┐  ┌───────────┐  ┌───────────────────────┐   │
│  │ SHA-256   │  │ Keyboard  │  │     Graphics / Font   │   │
│  │  Engine   │  │  Driver   │  │     Mode 13h + VESA   │   │
│  └───────────┘  └───────────┘  └───────────────────────┘   │
└─────────────────────────────────────────────────────────────┘
           │
┌──────────▼──────────────────────────────────────────────────┐
│                       HARDWARE                              │
│     CPU (x86-64) ◆ RAM ◆ VGA ◆ PS/2 ◆ PIT Timer           │
└─────────────────────────────────────────────────────────────┘
```

---

## ◈ Key Features

### 🔐 Trust Registry
Every process receives a unique cryptographic identity at creation:

```c
struct process_identity {
    uint32_t pid;
    uint8_t  hash[32];          // SHA-256 of executable
    uint32_t trust_level;       // 0=Kernel 1=System 2=User 3=Guest
    uint64_t creation_tick;     // Timestamp at birth
};
```

### 🛡️ Verification Gate
Hardware/software hybrid barrier between userland and kernel:
- **Identity Tokens** — cryptographically signed process descriptors
- **Integrity Checks** — real-time SHA-256 hashing of executable segments  
- **Execution Boundaries** — strict memory segment enforcement

### ⚡ Hardware-Enforced Security
- **IDT Protection** — interrupt gates verify process identity before handling
- **Memory Guard** — physical pages tagged with process ownership
- **Process Shield** — context switches validate both exiting and entering processes

### 🎨 Forerunner UI
High-contrast geometric sci-fi aesthetic:

| Element | Color | Hex |
|---|---|---|
| Background | Deep Space Black | `#050514` |
| Primary | Forerunner Gold | `#FFC832` |
| Status OK | Verified Green | `#00FF64` |
| Accent | Neon Cyan | `#00FFDC` |
| Alert | Alert Red | `#FF1E1E` |
| Text | Hard Silver | `#B4B4C8` |

---

## ◈ Project Structure

```
Sentinel/
├── bootloader/
│   └── boot.asm          # Real mode → Protected → Long Mode
├── kernel/
│   ├── boot.asm          # Multiboot2 header + 32→64 transition
│   ├── kernel.c          # Main kernel logic + Forerunner UI
│   ├── idt.c / idt.h     # Interrupt Descriptor Table + Guardian
│   ├── memory.c / .h     # Physical memory manager + Guard
│   ├── scheduler.c / .h  # Identity-aware process scheduler
│   ├── graphics.c / .h   # Mode 13h VGA + Bochs VBE driver
│   ├── font.c / .h       # Bitmap font engine
│   ├── keyboard.c / .h   # PS/2 keyboard driver
│   ├── timer.c / .h      # PIT 100Hz timer
│   └── linker.ld         # Kernel linker script
├── security/
│   ├── trust.c / .h      # Trust Registry + Identity management
│   └── sha256.c / .h     # Kernel-space SHA-256 engine
├── iso/
│   └── boot/grub/        # GRUB bootloader config
└── README.md
```

---

## ◈ Quick Start

### Prerequisites

**Ubuntu / Debian:**
```bash
sudo apt install nasm gcc qemu-system-x86 grub-pc-bin xorriso git
```

**Arch Linux:**
```bash
sudo pacman -S nasm gcc qemu grub xorriso git
```

### Build & Run

```bash
# Clone
git clone https://github.com/yourusername/SentinelOS.git
cd SentinelOS

# Build
make clean && make

# Create bootable ISO
cp sentinel.bin iso/boot/
grub-mkrescue -o sentinel.iso iso/

# Launch in QEMU
qemu-system-x86_64 -cdrom sentinel.iso -m 256M -boot d
```

---

## ◈ System Requirements

| Component | Minimum | Recommended |
|---|---|---|
| CPU | x86-64 (Intel / AMD) | Any modern 64-bit |
| RAM | 64 MB | 256 MB+ |
| Display | VGA 320x200 | VESA 800x600+ |
| Storage | 10 MB | 50 MB |

---

## ◈ Roadmap

```
[✓] Phase 1 — Foundation
    ✓ Custom 3-stage bootloader
    ✓ 64-bit Long Mode kernel
    ✓ 4-level page tables
    ✓ IDT + interrupt handling
    ✓ Physical memory manager
    ✓ Process scheduler
    ✓ PS/2 keyboard + PIT timer
    ✓ Mode 13h graphics + font engine
    ✓ Forerunner UI

[◈] Phase 2 — Security Core (in progress)
    ◈ Trust Registry
    ◈ Verification Gate
    ◈ SHA-256 integration
    ◈ VESA high-resolution framebuffer

[ ] Phase 3 — User Space
    ○ ELF binary loading
    ○ System call interface
    ○ Process isolation

[ ] Phase 4 — Advanced
    ○ Networking stack
    ○ Encrypted storage
    ○ TPM 2.0 integration
    ○ AVX-512 crypto acceleration
```

---

## ◈ Contributing

We welcome architects who value absolute security.

1. Fork the repository
2. Create your branch: `git checkout -b feature/YourFeature`
3. Commit: `git commit -m 'Add YourFeature'`
4. Push: `git push origin feature/YourFeature`
5. Open a Pull Request

---

## ◈ License

Distributed under the **MIT License**. See `LICENSE` for details.

---

<div align="center">

```
        ◆
       ╱│╲
      ╱ │ ╲
     ◆──┼──◆
      ╲ │ ╱
       ╲│╱
        ◆
```

**SentinelOS — Where Trust Is Earned, Not Given.**

[Report Bug](../../issues) · [Request Feature](../../issues) · [Discussions](../../discussions)

</div>