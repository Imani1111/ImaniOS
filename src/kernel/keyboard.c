#include <keyboard.h>
#include <idt.h>
#include <vga.h>
#include <timer.h>
#include <uptime_display.h>

static char kbd_buffer[KEYBOARD_BUFFER_SIZE];
static volatile uint8_t kbd_head = 0;
static volatile uint8_t kbd_tail = 0;

extern size_t cursor_row;
extern size_t cursor_col;

static int shift_state = 0;

int keyboard_focus = 0;

static const char ascii_characters[128] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
  '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0,  'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',   0,
 '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/',   0, '*',   0, ' ',
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0, '-',   0,   0,   0, '+',   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

static const char ascii_shifted_characters[128] = {
    0,  27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b',
  '\t', 'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n',
    0,  'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~',   0,
  '|', 'Z', 'X', 'C', 'V', 'B', 'N', 'M', '<', '>', '?',   0, '*',   0, ' ',
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0, '-',   0,   0,   0, '+',   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0
};

char get_char(void)
{
    while (kbd_head == kbd_tail){
        asm volatile("hlt");
    }

    char c = kbd_buffer[kbd_tail];
    kbd_tail = (kbd_tail + 1) % KEYBOARD_BUFFER_SIZE;
    return c;
}

void keyboard_handler(void)
{
    uint8_t keyboard_code = inb(KEYBOARD_DATA_PORT);
    
    uint8_t clean_index = keyboard_code & 0x7F;
    int is_release = (keyboard_code & 0x80);

    // 1. TRACK SHIFT STATE
    if (clean_index == KEY_LSHIFT) {
        if (is_release) shift_state &= ~1; else shift_state |= 1;
        send_eoi(1);
        return;
    }
    if (clean_index == KEY_RSHIFT) {
        if (is_release) shift_state &= ~2; else shift_state |= 2;
        send_eoi(1);
        return;
    } 
 
    if (!is_release) {
        switch (clean_index) {
            // Raw movement arrows still require manual cursor management
            case KEY_LEFT:
                erase_software_cursor();
                if (cursor_col == 0) {
                    if (cursor_row > 0) {
                        cursor_row--;
                        cursor_col = VGA_WIDTH - 1;
                    }
                } else {
                    cursor_col--;
                }
                check_cursor_bounds();
                draw_software_cursor();
                break;

            case KEY_RIGHT:
                erase_software_cursor();
                if (cursor_col == VGA_WIDTH - 1) {
                    cursor_row++;
                    cursor_col = 0;
                } else {
                    cursor_col++;
                }
                check_cursor_bounds();
                draw_software_cursor();
                break;

            case KEY_UP:
                erase_software_cursor();
                if (cursor_row > 0) {
                    cursor_row--;
                }
                check_cursor_bounds();
                draw_software_cursor();
                break;

            case KEY_DOWN:
                erase_software_cursor();
                cursor_row++;
                check_cursor_bounds();
                draw_software_cursor();
                break;
 
            default:
                if (clean_index < sizeof(ascii_characters)){
                int shift_pressed = (shift_state != 0);
                char ascii = shift_pressed? ascii_shifted_characters[clean_index] : ascii_characters[clean_index];
                if (ascii != 0 && ascii != '0'){
                    uint8_t next_head = (kbd_head + 1) % KEYBOARD_BUFFER_SIZE;
                    if (next_head != kbd_tail){
                        kbd_buffer[kbd_head] = ascii;
                        kbd_head = next_head;
                        }
                    }
                }
                // Default key handling is done by read_command() in shell logic
                break;
        }
    }



    send_eoi(1);
}
