#include <idt.h>
#include <vga.h>

struct IdtDescriptor32bit idt_entries[IDT_ENTRIES];
struct Idt_pointer idt_pointer;

extern void isr0_divide_by_zero();
extern void isr20_pit_timer();
extern void isr21_keyboard();

void outb(uint16_t port, uint8_t value)
{
    asm volatile("outb %b0, %w1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port)
{
    uint8_t result;
    asm volatile("inb %w1, %b0" : "=a"(result) : "Nd"(port));
    return result;
}

void io_wait(void)
{
    outb(0x80, 0);
}

void send_eoi(uint8_t irq) // ------> make sure i send the actual irq number (1, 2, 3 ....) not the interrupt vector entry
{
    // if the interrupt came from the slave pic (IRQs 8 - 15)
    if (irq >= 8){
        outb(SLAVE_PIC_COMMAND_PORT, PIC_EOI);
    }
    outb(MASTER_PIC_COMMAND_PORT, PIC_EOI);
}

void remap_pic(void)
{
    // Start initialization in cascade mode
    outb(MASTER_PIC_COMMAND_PORT, 0x11);
    io_wait();
    outb(SLAVE_PIC_COMMAND_PORT, 0x11);
    io_wait();

    // Remap the pic entries
    outb(MASTER_PIC_DATA_PORT, 0x20);
    io_wait();
    outb(SLAVE_PIC_DATA_PORT, 0x28);
    io_wait();

    outb(MASTER_PIC_DATA_PORT, 0x04);
    io_wait();
    outb(SLAVE_PIC_DATA_PORT, 0x02);
    io_wait();

    //Set mode to 8086 mode
    outb(MASTER_PIC_DATA_PORT, 0x01);
    io_wait();
    outb(SLAVE_PIC_DATA_PORT, 0x01);
    io_wait();

    // Clear all masks
    outb(MASTER_PIC_DATA_PORT, 0x00);
    outb(SLAVE_PIC_DATA_PORT, 0x00);
}

void set_idt_gate(struct IdtDescriptor32bit* idt_entry, uint32_t address_of_the_isr_handler, uint16_t selector, uint8_t attributes)
{
    idt_entry->low_16_bits = (address_of_the_isr_handler & 0xFFFF);
    idt_entry->segment_selector = selector;
    idt_entry->reserved = 0;
    idt_entry->attributes = attributes;
    idt_entry->high_16_bits = (address_of_the_isr_handler >> 16) & 0xFFFF;
}

void exception_handler() 
{
    const char* error_msg = " EXCEPTION TRIGGERED: DIVISION BY ZERO! ";
    
    vga_puts_at("  ======================================  ", 10, 20, VGA_ATTR_WHITE_ON_RED);
    vga_puts_at("  KERNEL PANIC: UNHANDLED EXCEPTION     ", 11, 20, VGA_ATTR_WHITE_ON_RED);
    vga_puts_at("  --------------------------------------  ", 12, 20, VGA_ATTR_WHITE_ON_RED);
    vga_puts_at(error_msg,                                       13, 22, VGA_ATTR_WHITE_ON_RED);
    vga_puts_at("  System Halting.                       ", 14, 20, VGA_ATTR_WHITE_ON_RED);
    vga_puts_at("  ======================================  ", 15, 20, VGA_ATTR_WHITE_ON_RED);

    // Disable interrupts and freeze the CPU
    asm volatile("cli");

    while(1){
        asm volatile("hlt");
    };
}

void initialize_idt()
{
    remap_pic();

    idt_pointer.size_of_the_idt = (sizeof(struct IdtDescriptor32bit) * IDT_ENTRIES) - 1;
    idt_pointer.address_of_the_idt = (uint32_t)&idt_entries;

    for (int i = 0; i < IDT_ENTRIES; i++){
        set_idt_gate(&idt_entries[i], 0, 0, 0);
    }

    set_idt_gate(&idt_entries[0], (uint32_t)isr0_divide_by_zero, GDT_CODE_SEGMENT, KERNEL_INTERRUPT_FLAG_32BIT);
    set_idt_gate(&idt_entries[32], (uint32_t)isr20_pit_timer, GDT_CODE_SEGMENT, KERNEL_INTERRUPT_FLAG_32BIT);
    set_idt_gate(&idt_entries[33], (uint32_t)isr21_keyboard, GDT_CODE_SEGMENT, KERNEL_INTERRUPT_FLAG_32BIT);


    load_idt((uint32_t)&idt_pointer);

    asm volatile("sti");
}
