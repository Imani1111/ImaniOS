#include <timer.h>
#include <idt.h>
#include <uptime_display.h>
#include <vga.h>

extern void vga_cursor_tick(void);
volatile uint32_t timer_ticks = 0;
static uint32_t timer_frequency = 100;  // Default fallback for getting system uptime 

void initialize_timer(uint32_t target_frequency)
{
    timer_frequency = target_frequency;

    uint32_t divisor = PIT_BASE_FREQUENCY /  target_frequency;
    if (divisor == 0 ) divisor = 1;
    if (divisor > 65535) divisor = 65535;

    uint8_t command = PIT_CMD_CHANNEL_0 |
                      PIT_CMD_BOTH      |
                      PIT_CMD_MODE_3    |
                      PIT_CMD_BINARY;

    uint8_t low_byte = (divisor & 0xFF);
    uint8_t high_byte = (divisor >> 8) & 0xFF;

    outb(PIT_PORT_COMMAND, command);
    outb(PIT_PORT_CHANNEL_0, low_byte);
    outb(PIT_PORT_CHANNEL_0, high_byte);
}

void timer_handler(void)
{
    timer_ticks++;

    vga_cursor_tick();
    
    // If PIT is 100Hz, this triggers exactly once per second
    if (timer_ticks % 100 == 0) { 
        draw_system_uptime(); 
    }

    //vga_update_screen();

    send_eoi(0);
}

Uptime_t get_system_uptime()
{
    Uptime_t system_uptime;
    uint32_t current_ticks = timer_ticks;

    uint32_t total_ms = (current_ticks * 1000) / timer_frequency;
    system_uptime.milliseconds = total_ms % 1000;

    uint32_t total_seconds = total_ms / 1000;
    system_uptime.seconds = total_seconds % 60;

    uint32_t total_minutes = total_seconds / 60;
    system_uptime.minutes = total_minutes % 60;

    uint32_t total_hours = total_minutes / 60;
    system_uptime.hours = total_hours;
    
    return system_uptime;
}

void stall_cpu(uint32_t ticks)
{
    uint32_t start_ticks = timer_ticks;
    while((timer_ticks - start_ticks) < ticks){
        asm volatile("pause");
    }
}
