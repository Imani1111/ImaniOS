#ifndef IDT_H
#define IDT_H

#include <stdint.h>

#define IDT_ENTRIES 256
#define GDT_CODE_SEGMENT 0x08
#define KERNEL_INTERRUPT_FLAG_32BIT 0x8E

#define MASTER_PIC_COMMAND_PORT 0x20
#define MASTER_PIC_DATA_PORT 0x21
#define SLAVE_PIC_COMMAND_PORT 0xA0
#define SLAVE_PIC_DATA_PORT 0xA1
#define PIC_EOI 0x20

struct IdtDescriptor32bit {
    uint16_t low_16_bits;
    uint16_t segment_selector;
    uint8_t reserved;
    uint8_t attributes;
    uint16_t high_16_bits;
}__attribute__((packed));


struct Idt_pointer{
    uint16_t size_of_the_idt;
    uint32_t address_of_the_idt;
}__attribute__((packed));

extern void load_idt(uint32_t idt_pointer_addr);
void initialize_idt();
void set_idt_gate(struct IdtDescriptor32bit* idt_entries, uint32_t address_of_the_isr_handler, uint16_t selector, uint8_t attributes);

void outb(uint16_t port, uint8_t value);
uint8_t inb(uint16_t port);
void io_wait(void);
void send_eoi(uint8_t irq);

void remap_pic();
#endif
