# Compilers and flags
NASM = nasm
CC = gcc
LD = ld

CFLAGS = -m32 -ffreestanding -fno-pie -nostdlib -fno-builtin -fno-stack-protector -mpreferred-stack-boundary=2 -mno-sse -mno-sse2 -mno-mmx -mno-80387 -Wall -Wextra -Isrc/kernel/include
# --- UPDATED: Force flat binary layout for direct jump from bootloader ---
LDFLAGS = -m elf_i386 -T linker.ld --oformat binary

# Directories
SRC_DIR = src
BOOT_DIR = $(SRC_DIR)/bootloader
KERNEL_DIR = $(SRC_DIR)/kernel
BUILD_DIR = build

# File variables
BOOT_BIN = $(BUILD_DIR)/boot.bin
STAGE2_BIN = $(BUILD_DIR)/stage2.bin
KERNEL_BIN = $(BUILD_DIR)/kernel.bin
OS_IMAGE = os_image.img

# Sector Sizing Rules (Matching your ASM requirements)
STAGE2_SECTORS = 20
KERNEL_SECTORS = 40

# Robust mathematical sizing formulas
STAGE2_BYTES = $(shell echo $$(($(STAGE2_SECTORS) * 512)))
KERNEL_BYTES = $(shell echo $$(($(KERNEL_SECTORS) * 512)))

KERNEL_OBJS = $(BUILD_DIR)/kernel_entry.o \
              $(BUILD_DIR)/kernel_main.o \
              $(BUILD_DIR)/idt.o \
              $(BUILD_DIR)/idt_asm.o \
              $(BUILD_DIR)/keyboard.o \
              $(BUILD_DIR)/timer.o \
              $(BUILD_DIR)/vga.o \
              $(BUILD_DIR)/string.o \
              $(BUILD_DIR)/shell.o \
              $(BUILD_DIR)/file_system.o \
              $(BUILD_DIR)/uptime_display.o

.PHONY: all clean run

# Default target
all: $(OS_IMAGE)

$(OS_IMAGE): $(BOOT_BIN) $(STAGE2_BIN) $(KERNEL_BIN)
	@echo "==> Building final OS disk image..."
	truncate -s $(STAGE2_BYTES) $(STAGE2_BIN)
	truncate -s $(KERNEL_BYTES) $(KERNEL_BIN)
	cat $(BOOT_BIN) $(STAGE2_BIN) $(KERNEL_BIN) > $(OS_IMAGE)
	@echo "==> $(OS_IMAGE) built successfully."

$(BOOT_BIN): $(BOOT_DIR)/boot.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(STAGE2_BIN): $(BOOT_DIR)/stage2.asm | $(BUILD_DIR)
	$(NASM) -f bin $< -o $@

$(KERNEL_BIN): $(KERNEL_OBJS)
	$(LD) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/kernel_entry.o: $(BOOT_DIR)/kernel_entry.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/kernel_main.o: $(KERNEL_DIR)/kernel_main.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/idt.o: $(KERNEL_DIR)/idt.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/idt_asm.o: $(KERNEL_DIR)/idt.asm | $(BUILD_DIR)
	$(NASM) -f elf32 $< -o $@

$(BUILD_DIR)/keyboard.o: $(KERNEL_DIR)/keyboard.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/timer.o: $(KERNEL_DIR)/timer.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/vga.o: $(KERNEL_DIR)/vga.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/string.o: $(KERNEL_DIR)/string.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/shell.o: $(KERNEL_DIR)/shell.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/file_system.o: $(KERNEL_DIR)/file_system.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/uptime_display.o: $(KERNEL_DIR)/uptime_display.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(OS_IMAGE)
	qemu-system-i386 -hda $(OS_IMAGE) -d int -no-reboot

clean:
	rm -rf $(BUILD_DIR) $(OS_IMAGE)
