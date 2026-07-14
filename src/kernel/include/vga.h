#ifndef VGA_H
#define VGA_H

#include <stdint.h>
#include <stddef.h>

#define VGA_TEXT_BUFFER ((uint16_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

//Black Background
#define VGA_ATTR_WHITE_ON_BLACK       0x07  // Default terminal text
#define VGA_ATTR_GREEN_ON_BLACK       0x02  // Matrix / Retro terminal look
#define VGA_ATTR_CYAN_ON_BLACK        0x03  // Clean UI look
#define VGA_ATTR_LIGHT_GREY_ON_BLACK  0x07
#define VGA_ATTR_YELLOW_ON_BLACK      0x0E  // Warnings
#define VGA_ATTR_WHITE_ON_BLUE        0x1F  // System Clock
#define VGA_ATTR_PURPLE_ON_BLACK      0x0D
#define VGA_ATTR_RED_ON_BLACK         0x0C

//White Background
#define VGA_ATTR_GREEN_ON_WHITE       0xF2
#define VGA_ATTR_YELLOW_ON_WHITE      0XFE
#define VGA_ATTR_RED_ON_WHITE         0xF4
#define VGA_ATTR_PURPLE_ON_WHITE      0xF5

// Alerts & Windows (Colored Backgrounds)
#define VGA_ATTR_WHITE_ON_BLUE        0x1F  // Classic BSOD / Norton Commander look
#define VGA_ATTR_WHITE_ON_RED         0x4F  // Critical Kernel Panic / Error screen
#define VGA_ATTR_BLACK_ON_WHITE       0xF0  // Inverted text / High contrast
#define VGA_ATTR_BLACK_ON_LIGHT_GREY  0x70  // Standard top/bottom menu bar look

// Text formatting helpers (for building custom attributes on the fly)
#define VGA_COLOR_BLACK   0
#define VGA_COLOR_WHITE   15
#define VGA_COLOR_LIGHTBLUE 0x91

#define VGA_BLOCK_CURSOR 0xDB

#define CURSOR_BLINK_RATE_TICKS 50

extern uint16_t virtual_text_buffer[VGA_WIDTH*VGA_HEIGHT];

void clear_vga_buffer();
void clear_line(size_t row);
void vga_putc(int c);
void vga_puts(const char* string);
void vga_puts_at(const char* string, int row, int col, uint8_t attribute);
void vga_print_int(uint32_t num);

void disable_hardware_cursor(void);
void erase_software_cursor(void);
void draw_software_cursor(void);
void set_software_cursor(int row, int col);
void vga_scroll(void);
void vga_cursor_tick(void);

void vga_backspace(void);

void vga_set_text_attribute(uint8_t attribute);
void vga_set_screen_attribute(uint8_t attribute);
void vga_set_cursor_attribute(uint8_t attribute);
void check_cursor_bounds(void);

void vga_set_default_text_attribute();

size_t get_current_cursor_position();

void vga_init(void);

//void graphics_init(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch);
//void vga_update_screen();

#endif
