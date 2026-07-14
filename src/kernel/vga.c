#include <vga.h>
#include <idt.h>
#include <font.h>

//uint16_t virtual_text_buffer[VGA_WIDTH * VGA_HEIGHT];
//void vga_update_screen(void);
//static uint32_t g_pitch = 1024 * 4;

// VGA color palette to 32-bit RGB (Format: 0xRRGGBB)
/*static const uint32_t vga_colors[16] = {
    0x000000, // 0: Black
    0x0000AA, // 1: Blue
    0x00AA00, // 2: Green
    0x00AAAA, // 3: Cyan
    0xAA0000, // 4: Red
    0xAA00AA, // 5: Magenta
    0xAA5500, // 6: Brown
    0xAAAAAA, // 7: Light Gray
    0x555555, // 8: Dark Gray
    0x5555FF, // 9: Light Blue
    0x55FF55, // 10: Light Green
    0x55FFFF, // 11: Light Cyan
    0xFF5555, // 12: Light Red
    0xFF55FF, // 13: Light Magenta
    0xFFFF55, // 14: Yellow
    0xFFFFFF  // 15: White
};*/

//static uint32_t* g_framebuffer = 0;

size_t cursor_row = 8;
size_t cursor_col = 0;

uint8_t current_screen_attribute = VGA_ATTR_WHITE_ON_BLACK;
static uint8_t current_cursor_attribute = VGA_COLOR_WHITE;
uint8_t current_text_attribute = VGA_ATTR_GREEN_ON_BLACK;

static volatile int software_cursor_enabled = 1;
static volatile int cursor_is_visible = 0;
uint16_t char_under_cursor = 0;

static volatile uint32_t blink_counter = 0;

void disable_hardware_cursor()
{
    outb(0x3D4, 0x0A);
    outb(0x3D5, 0x20);
}

void clear_vga_buffer()
{
    uint16_t blank_tile = ' ' | ((current_screen_attribute << 8) & 0xFF00);
    for (size_t i = VGA_WIDTH; i < VGA_WIDTH*VGA_HEIGHT; i++){
        VGA_TEXT_BUFFER[i] = blank_tile;
    }
    cursor_row = 2;
    cursor_col = 0;

    //vga_update_screen();
}

void clear_line(size_t row)
{
    uint16_t blank_tile = ' ' | ((current_screen_attribute << 8) & 0xFF00); 
    size_t index = row * VGA_WIDTH;

    for(int i = 0; i < VGA_WIDTH; i++){
        VGA_TEXT_BUFFER[index + i] = blank_tile;
    }
    //vga_update_screen();
}

void erase_software_cursor(void)
{
    if (cursor_is_visible){
        size_t index = cursor_row * VGA_WIDTH + cursor_col;
        VGA_TEXT_BUFFER[index] = char_under_cursor;
        cursor_is_visible = 0;
    }
    //vga_update_screen();
}

void draw_software_cursor(void)
{
    if (!cursor_is_visible && software_cursor_enabled){
        size_t index = cursor_row * VGA_WIDTH + cursor_col;
        
        char_under_cursor = VGA_TEXT_BUFFER[index];
        VGA_TEXT_BUFFER[index] = VGA_BLOCK_CURSOR | (( current_cursor_attribute << 8) & 0xFF00);
        cursor_is_visible = 1;
    }
}

void set_software_cursor(int row, int col)
{
    erase_software_cursor();

    if (row >= VGA_HEIGHT) row = VGA_HEIGHT - 1;
    if (col >= VGA_WIDTH)  col = VGA_WIDTH - 1;

    if (row == 0) row = 1;
    cursor_row = row;
    cursor_col = col;

    check_cursor_bounds();

    draw_software_cursor();
}

size_t get_current_cursor_position()
{
    return cursor_row * VGA_WIDTH + cursor_col;
}

void vga_scroll()
{
    erase_software_cursor();

    for (size_t row = 2; row < VGA_HEIGHT; row++){
        for (size_t col = 0; col < VGA_WIDTH; col++){
            VGA_TEXT_BUFFER[(row-1) * VGA_WIDTH + col] = VGA_TEXT_BUFFER[row * VGA_WIDTH + col];
        }
    }

    uint16_t blank_tile = ' ' | ((current_screen_attribute << 8) & 0xFF00);
    for (size_t col = 0; col < VGA_WIDTH; col++){
        size_t bottom_row_index = (VGA_HEIGHT - 1) * VGA_WIDTH + col;
        VGA_TEXT_BUFFER[bottom_row_index] = blank_tile; 
    }

    cursor_col = 0;
    cursor_row = VGA_HEIGHT - 1;
}

void check_cursor_bounds(void)
{
    // 1. If column wrapped past the right edge (e.g., exactly VGA_WIDTH)
    if (cursor_col >= VGA_WIDTH && cursor_col != (size_t)-1) {
        cursor_col = 0;
        cursor_row++;
    }
    
    // 2. If column underflowed below 0 (wrapped to max size_t)
    if (cursor_col == (size_t)-1) {
        if (cursor_row > 0) {
            cursor_row--;
            cursor_col = VGA_WIDTH - 1;
        } else {
            cursor_col = 0;
        }
    }

    // 3. If row underflowed below 0 (wrapped to max size_t)
    if (cursor_row == (size_t)-1) {
        cursor_row = 0;
    }

    // 4. If row went off the bottom of the screen
    if (cursor_row >= VGA_HEIGHT) {
        vga_scroll();
    }
}

void vga_backspace(void)
{
    erase_software_cursor();

    // 2. Clear out the OLD position completely so it doesn't leave a trail
    size_t old_index = cursor_row * VGA_WIDTH + cursor_col;
    VGA_TEXT_BUFFER[old_index] = ' ' | ((current_screen_attribute << 8) & 0xFF00);
    
    if (cursor_col > 0){
        cursor_col--;
    }
    else if(cursor_col == 0 && cursor_row > 1){
        cursor_col = VGA_WIDTH - 1;
        cursor_row--;
    }
    else{
        draw_software_cursor();
        return;
    }

    size_t index = cursor_row * VGA_WIDTH + cursor_col;
    VGA_TEXT_BUFFER[index] = ' ' | ((current_cursor_attribute << 8) & 0xFF00);

    draw_software_cursor();
}

void vga_putc(int c)
{
    erase_software_cursor();

    if (c == '\n'){
        cursor_row++;
        cursor_col = 0;
    }
    else if (c == '\r'){
        cursor_col = 0;
    }
    else if (c == '\t'){
        cursor_col = (cursor_col + 8) & ~(8 - 1);
    }
    else if (c == '\b'){
        vga_backspace();
    }
    else {
        size_t index = cursor_row * VGA_WIDTH + cursor_col;
        VGA_TEXT_BUFFER[index] = c | (current_text_attribute << 8);
        cursor_col++;
    }
   
    //vga_update_screen();
    check_cursor_bounds();
    draw_software_cursor();
}

void vga_puts(const char* string)
{
    size_t i = 0;
    while (string[i] != '\0'){
        vga_putc(string[i]);
        i++;
    }
    //vga_update_screen();
}

void vga_puts_at(const char* string, int row, int col, uint8_t attribute)
{
    size_t offset = row * VGA_WIDTH + col; 
    int cols_written = col;
    int i = 0;

    erase_software_cursor();

    while(string[i] != '\0'){
        if (offset >= VGA_WIDTH * VGA_HEIGHT) break;
        uint16_t combined = string[i] | ((attribute << 8) & 0xFF00);

        VGA_TEXT_BUFFER[offset] = combined;
        offset++;
        i++;
        cols_written++;
    }

    uint16_t blank_tile = ' ' | ((attribute << 8) & 0xFF00);
    while (cols_written < VGA_WIDTH) {
        if (offset >= VGA_WIDTH * VGA_HEIGHT) break;
        VGA_TEXT_BUFFER[offset] = blank_tile;
        offset++;
        cols_written++;
    }
}

void vga_print_int(uint32_t num)
{
    if (num == 0) {
        vga_puts("0");
        return;
    }

    char buf[12];
    int i = 0;

    // Extract digits in reverse order
    while (num > 0) {
        buf[i++] = (num % 10) + '0';
        num /= 10;
    }
    buf[i] = '\0'; 
    // Reverse the string in-place so it displays correctly
    int start = 0;
    int end = i - 1;
    while (start < end) {
        char temp = buf[start];
        buf[start] = buf[end];
        buf[end] = temp;
        start++;
        end--;
    }
 
    vga_puts(buf);
}

void vga_cursor_tick()
{
    if (!software_cursor_enabled) return;
    blink_counter++;

    if (blink_counter >= CURSOR_BLINK_RATE_TICKS){
        blink_counter = 0;

        if (cursor_is_visible){
            erase_software_cursor();
        }
        else{
            draw_software_cursor();
        }
    }
}

void vga_set_default_text_attribute()
{
    current_text_attribute = VGA_ATTR_GREEN_ON_BLACK;
}

void vga_set_screen_attribute(uint8_t attribute)
{
    current_screen_attribute = attribute;
}

void vga_set_text_attribute(uint8_t attribute)
{
    current_text_attribute = attribute;
}

void vga_set_cursor_attribute(uint8_t attribute)
{
    current_cursor_attribute = attribute;
}

void vga_init()
{
    disable_hardware_cursor();

    current_screen_attribute = VGA_ATTR_WHITE_ON_BLACK; 
    current_text_attribute = VGA_ATTR_GREEN_ON_BLACK;
    
    // Explicitly force initialization state
    software_cursor_enabled = 1; 
    cursor_is_visible = 0;
    blink_counter = 0;

    clear_vga_buffer();
    draw_software_cursor();
}

// YOU CAN UNCOMMENT THIS CODE IF YOU WANT TO TRY AND SET UP VGA GRAPHICS MODE 
// ALSO CHECK OUT STAGE2.ASM FOR THE GRAPHICS INITIALIZATION ASSEMBLY CODE

/*void graphics_init(uint32_t* framebuffer, uint32_t width, uint32_t height, uint32_t pitch)
{
    g_framebuffer = framebuffer;
    g_pitch = pitch;
}*/

/*void vga_update_screen()
{
    if (!g_framebuffer) return;
    
    for (int row = 0; row < VGA_HEIGHT; row++){
        for (int col = 0; col < VGA_WIDTH; col++){
            int offset = (row * VGA_WIDTH) + col;
            uint16_t cell = virtual_text_buffer[offset];
            char c = (char)(cell & 0x00FF);
            uint8_t attribute = (uint8_t)(cell >> 8);

            uint32_t fg_color = vga_colors[attribute & 0x0F];
            uint32_t bg_color = vga_colors[(attribute >> 4) & 0X0F];

            int screen_start_x = col * 8;
            int screen_start_y = row * 16;

            for (int cy = 0; cy < 16; cy++){
                uint8_t row_byte = font8x16[(uint8_t)c][cy];
                for (int cx = 0; cx < 8; cx++){
                    uint32_t color = (row_byte & (0x80 >> cx)) ? fg_color : bg_color;

                    int px = screen_start_x + cx;
                    int py = screen_start_y + cy;
    
                    if (px >= 0 && px < SCREEN_WIDTH && py >= 0 && py < SCREEN_HEIGHT) {
                        // Calculate memory offset using bytes per scanline (Pitch)
                        // Cast to uint8_t* to do byte-exact math, then write 32-bit color
                        uint8_t* pixel_row = (uint8_t*)g_framebuffer + (py * g_pitch);
                        uint32_t* pixel = (uint32_t*)(pixel_row + (px * 4)); // Assumes 32-bpp
                        
                        *pixel = color;
                    }
                }
            }
        }
    }
}*/
