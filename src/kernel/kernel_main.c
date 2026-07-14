#include <idt.h>
#include <keyboard.h>
#include <timer.h>
#include <vga.h>
#include <string.h>
#include <uptime_display.h>
#include <shell.h>
#include <file_system.h>

void type_writer(const char* str, uint32_t delay) {
    for (int i = 0; str[i] != '\0'; i++) {
        vga_putc(str[i]);
        stall_cpu(delay);
    }
}

void kernel_main() {
    vga_init();
    initialize_idt();
    initialize_timer(100);
    set_software_cursor(2, 0);

    vga_puts("========================================================\n");
    vga_puts("  IMANI-OS Kernel Boot Environment [Version 0.1.4a]\n");
    vga_puts("  (c) 2026 Imani Technologies. All Rights Reserved.\n");
    vga_puts("========================================================\n\n");
    stall_cpu(150);

    // --- Hardware Diagnostics & Loading Stage ---
    vga_puts("[  OK  ] Detecting x86 CPU architecture...\n");
    stall_cpu(80);
    vga_puts("[  OK  ] Memory Map: 640KB Low / 1048576KB High RAM detected.\n");
    stall_cpu(100);

    vga_puts("[ INFO ] IDT: Gates mapped & Loaded.\n");
    vga_puts("[ INFO ] PIC: Master/Slave Remapped (IRQ 0x20-0x2F).\n");
    stall_cpu(100);

   
    vga_puts("[  OK  ] PIT Timer: Calibrated to 100Hz.\n");
    stall_cpu(200);
    
    clear_vga_buffer();

    // Safely unmask ONLY the timer (bit 0) and keyboard (bit 1)
    outb(0x21, 0xF8);
    
    vga_puts("[  OK  ] VGA Driver: 80x25 Text Mode Initialized.\n");
    vga_puts("[  OK  ] PS/2 Keyboard Driver: Registered.\n");
    stall_cpu(150);

    vga_puts_at(" !!! SYSTEM NOTICE: INITIALIZING IMANI-CORE SUITE !!! ", 4, 0, VGA_ATTR_RED_ON_WHITE);
    vga_putc('\n');
    vga_putc('\n');

    type_writer("Booting ImaniOS! . . .", 80);
    
    stall_cpu(300);

    clear_vga_buffer();
    
    vga_puts_at(" [!] DO NOT POWER OFF OR PRESS KEYS DURING COLD BOOT [!] ", 7, 0, VGA_ATTR_RED_ON_WHITE);
    vga_puts_at(" >> LOADING KERNEL INTERACTIVE SHELL ENVIRONMENT << ", 8, 0, VGA_ATTR_YELLOW_ON_WHITE);

    
    // This is for anyone who has this kernel and wants to test if the interrupt vector table 0 - 31 works
    // You can uncomment this to test ISR 0 --> the divide by zero error
    // Warning! --> This will cause a kernel panic and will require reboot!!!!
            //int volatile local_var = 0;
            //int volatile trigger_fault = 10 / local_var; 
            //(void)trigger_fault;

    asm volatile("sti"); 
    vga_puts("\n\n[  OK  ] Hardware Interrupts (STI) enabled successfully.\n");
    vga_puts("[ INFO ] System tick handler: Active (Cursor Blinking).\n");

    stall_cpu(250);
    vga_puts("[ INFO ] Mounting Virtual File System (VFS)...\n");
    fs_format();
    vga_puts("[  OK  ] File system formatted and mounted successfully.\n");
    
    stall_cpu(200);
    vga_puts("Launching interactive shell...\n");
    stall_cpu(100);
    
    shell_main();
}
