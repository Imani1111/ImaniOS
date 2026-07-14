# 🪐 ImaniOS (32-Bit x86 Operating System)

[![Kernel Architecture](https://img.shields.io/badge/Architecture-x86__32%20%28i386%29-blueviolet?style=for-the-badge)](https://wiki.osdev.org/)
[![Build Tool](https://img.shields.io/badge/Build%20Tool-GNU%20Make%20%2F%20GCC-blue?style=for-the-badge)](https://www.gnu.org/software/make/)
[![Platform](https://img.shields.io/badge/Emulator-QEMU%20%2F%20Bochs-orange?style=for-the-badge)](https://www.qemu.org/)
[![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)](LICENSE)

**ImaniOS** is a custom, bare-metal 32-bit operating system kernel built from scratch for the x86 architecture. It features a multi-stage bootloader, a structured Interrupt Descriptor Table (IDT), dynamic drivers for the PIT timer and keyboard, a virtual file system (VFS), and an interactive shell with custom memory-mapped VGA mechanics.

---

## 🛠️ Features

* **Multi-Stage Bootloader:** Written in x86 Assembly (`boot.asm` and `stage2.asm`) to transition the CPU safely from 16-bit Real Mode to 32-bit Protected Mode.
* **Interrupt Descriptor Table (IDT):** Complete software-hardware bridge mapping CPU interrupts (0–31) and PIC IRQs (such as the keyboard and system timer).
* **VGA Text Mode Graphic Driver:** Direct hardware-framebuffer manipulation supporting customized terminal outputs and font configurations.
* **Programmable Interval Timer (PIT):** Configured to 100Hz to handle system uptime and precise CPU stalling sequences.
* **Interactive Kernel Shell & VFS:** A built-in virtual file system formatter and interactive shell interface running as the kernel user space environment.

---

## 📂 Project Directory Tree

This repository is structured as follows:

```text
os/
├── build/                      # Compiled object files (.o) and intermediate binaries
├── src/                        # Core OS Source Code
│   ├── bootloader/             # Assembly Bootloader Sequence
│   │   ├── boot.asm            # MBR (Sector 1) - Initial boot sector
│   │   ├── stage2.asm          # Stage 2 Bootloader (Enters 32-bit Protected Mode)
│   │   └── kernel_entry.asm    # Assembly wrapper that calls kernel_main()
│   │
│   └── kernel/                 # 32-Bit C Kernel and Drivers
│       ├── kernel_main.c       # Kernel entry point and initialization sequence
│       ├── file_system.c       # Simple virtual file system storage routines
│       ├── idt.c & idt.asm     # C and Assembly handlers for the IDT and ISRs
│       ├── keyboard.c          # PS/2 Keyboard input driver
│       ├── shell.c             # Interactive kernel shell interface
│       ├── string.c            # Custom implementation of standard string operations
│       ├── timer.c             # PIT Timer configurations for hardware delays
│       ├── uptime_display.c    # Formatted system uptime tracking
│       ├── vga.c               # Color-coded VGA text output driver
│       └── include/            # Internal Kernel Headers
│           ├── file_system.h
│           ├── font.h          # Custom VGA font definitions
│           ├── idt.h
│           ├── keyboard.h
│           ├── shell.h
│           ├── string.h
│           ├── timer.h
│           ├── uptime_display.h
│           └── vga.h
│
├── linker.ld                   # Links kernel sections in physical memory addresses
├── Makefile                    # Automated build script for compiling and assembling
└── os_image.img                # Final bootable disk image


How to compile and run:
TOOLS REQUIRED:
    -make
    -gcc
    -ld
    -nasm
    -qemu-system-i386
    -truncate & cat ---> part of GNU CoreUtils


Compile by doing: make 
Run by doing: make run
Clean build file by doing: make clean
