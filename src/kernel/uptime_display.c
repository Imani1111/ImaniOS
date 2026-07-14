#include <timer.h>
#include <uptime_display.h>
#include <vga.h>
#include <string.h>

uint32_t last_printed_second = 99999;

void reverse(char* string, int length)
{
    int start = 0;
    int end = length - 1;
    while (start < end){
        char temp = string[start];
        string[start] = string[end];
        string[end] = temp;
        start++;
        end--;
    }
}

void utoa(uint32_t num, char* string)
{
    int i = 0;
    if (num == 0){
        string[i++] = '0';
        string[i] = '\0';
        return;
    }
    
    while(num != 0){
        int last_digit = num % 10;
        string[i++] = last_digit + '0';
        num = num / 10;
    }
    string[i] = '\0';
    reverse(string, i);
}

void pad_zero(char* string, int min_width)
{
    int len = 0;
    while(string[len] != '\0') len++;

    if (len >= min_width) return;
    int padding_needed = min_width - len;

    for (int i = len; i >= 0; i--){
        string[i + padding_needed] = string[i];
    }
    for (int i = 0; i < padding_needed; i++){
        string[i] = '0';
    }
}

void draw_system_uptime()
{
    Uptime_t system_uptime = get_system_uptime();

    char hour_str[12];
    char minutes_str[4];
    char seconds_str[4];
    char ms_str[5];

    utoa(system_uptime.hours, hour_str);
    utoa(system_uptime.minutes, minutes_str);
    utoa(system_uptime.seconds, seconds_str);
    utoa(system_uptime.milliseconds, ms_str);

    pad_zero(ms_str, 3);
    pad_zero(seconds_str, 2);
    pad_zero(minutes_str, 2);

    char timer_buffer[64];

    my_strcpy(timer_buffer, "SYSTEM UPTIME: ");
    my_strcat(timer_buffer, hour_str);
    my_strcat(timer_buffer, ":");
    my_strcat(timer_buffer, minutes_str);
    my_strcat(timer_buffer, ":");
    my_strcat(timer_buffer, seconds_str);
   
    clear_line(0);
    vga_puts_at(timer_buffer, 0, 0, VGA_ATTR_WHITE_ON_BLUE);
}

void draw_current_system_uptime()
{ 
    Uptime_t current_time = get_system_uptime();

    if (current_time.seconds != last_printed_second){
        draw_system_uptime();

        last_printed_second = current_time.seconds;
    }
}
