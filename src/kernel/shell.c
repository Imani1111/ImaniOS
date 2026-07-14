#include <shell.h>
#include <string.h>
#include <vga.h>
#include <keyboard.h>
#include <file_system.h>
#include <timer.h>
#include <uptime_display.h>


void read_command(char* buffer, int max_size)
{ 
    int index = 0;
    char c;

    while(1){
        c = get_char();

        if (c == '\n' || c == '\r'){
            buffer[index] = '\0';
            vga_putc('\n');
            break;
        }
        else if (c == '\b'){
            if(index > 0){
                index--;
                buffer[index] = '\0';
                vga_backspace();
            }
        }
        else{
            if (index < max_size - 1){
                buffer[index] = c;
                index++;
                vga_putc(c);
            }
        }
    }
}


void execute_command(char* command, char* args)
{
    // Command: help
    if (my_strcmp(command, "help") == 0) {
        vga_puts("Available commands:\n");
        vga_puts("  help     - Show this manual\n");
        vga_puts("  clear    - Clear the screen\n");
        vga_puts("  echo     - Print text to the screen\n");
        vga_puts("  ls       - List directory contents\n");
        vga_puts("  touch    - Create an empty file (e.g., touch file.txt)\n");
        vga_puts("  write    - Write data directly into a file (e.g., write <filename> <data>)\n");
        vga_set_text_attribute(VGA_ATTR_YELLOW_ON_BLACK);
        vga_puts("  Write command overwrites file contents, it doesnt append!!!!\n");
        vga_set_default_text_attribute();
        vga_puts("  mkdir    - Create a directory (e.g., mkdir docs)\n");
        vga_puts("  cat      - View file contents (e.g., cat file.txt)\n");
        vga_puts("  remove   - Delete specified entry (e.g., remove <entryname>)\n");
        vga_puts("  uptime   - Display current system uptime\n");
    }
    // Command: clear
    else if (my_strcmp(command, "clear") == 0) {
        clear_vga_buffer();
        set_software_cursor(3, 0);
        draw_software_cursor(); 
    }
    // Command: echo
    else if (my_strcmp(command, "echo") == 0) {
        if (args[0] == '\0') {
            vga_putc('\n');
        } else {
            vga_puts(args);
            vga_putc('\n');
        }
    }
    // Command: ls
    else if (my_strcmp(command, "ls") == 0){
        fs_list_directory();
    }
    // Command: touch <filename>
    else if (my_strcmp(command, "touch") == 0) {
        if (args[0] == '\0') {
            vga_puts("Error: Missing filename.\n");
        } else {
            if (fs_create_file(args) == 0) {
                vga_puts("File ");
                vga_puts(args);
                vga_puts(" created!\n");
            } else {
                vga_puts("Error: Failed to create file.\n");
            }
        }
    }
    // Command: mkdir <dirname>
    else if (my_strcmp(command, "mkdir") == 0) {
        if (args[0] == '\0') {
            vga_puts("Error: Missing directory name.\n");
        } else {
            if (fs_create_dir(args) == 0) {
                vga_puts("Directory created.\n");
            } else {
                vga_puts("Error: Failed to create directory.\n");
            }
        }
    }
    // Command: cat <filename>
    else if (my_strcmp(command, "cat") == 0) {
        if (args[0] == '\0') {
            vga_puts("Error: Missing filename.\n");
        } else {
            char file_buffer[512];
            // Clear buffer to guarantee clean, null-terminated string printing
            for(int i = 0; i < 512; i++) file_buffer[i] = 0;

            int32_t bytes_read = fs_read_file(args, file_buffer);
            if (bytes_read >= 0) {
                vga_puts(file_buffer);
                vga_putc('\n');
            } else {
                vga_puts("Error: Could not read file.\n");
            }
        }
    }
    // Command: write <filename> <text>
    else if (my_strcmp(command, "write") == 0) {
        if (args[0] == '\0') {
            vga_puts("Usage: write <filename> <text>\n");
        } else {
            // 1. Isolate the filename from the text content inside args
            char filename[48];
            int i = 0;

            // Copy until we hit a space or the end of the args string
            while (args[i] != '\0' && args[i] != ' ' && i < 47) {
                filename[i] = args[i];
                i++;
            }
            filename[i] = '\0'; // Null terminate the filename

            // 2. Check if a space exists separating filename and text
            if (args[i] == ' ') {
                // The actual text content starts right after that space
                const char* text_content = &args[i + 1];
                uint32_t text_len = 0;

                // Safely find string length (cap at 512 bytes for our sector size)
                while (text_content[text_len] != '\0' && text_len < 512) {
                    text_len++;
                } 
                if (fs_write_file(filename, text_content, text_len) == 0) {
                    vga_puts("Data written successfully.\n");
                } else {
                    vga_puts("Error: Could not write to file.\n");
                }
            }
            else {
                vga_puts("Error: Missing text content. Usage: write <filename> <text>\n");
            }
        }
    }
    // Command "remove"
    else if (my_strcmp(command, "remove") == 0){
        if (args[0] == '\0'){
            vga_puts("Error: Missing entry name\n");
        }
        else{
            if (fs_delete_entry(args) == 0){
                vga_puts("Entry: ");
                vga_puts(args);
                vga_puts(" deleted succesfully!\n");
            }else{
                vga_puts("Error: Failed to remove entry\n");
            }
        }
     }
    // Command "uptime"
    else if(my_strcmp(command, "uptime") == 0){
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
   
                vga_puts(timer_buffer);
                vga_putc('\n');
    }

    // Command Not Found
    else {
        vga_puts("Error: Command '");
        vga_puts(command);
        vga_puts("' not found.\n");
    }
}


void shell_main(void)
{
    char input_buffer[256];
    char command[64];
    char args[192];
   
    clear_line(1);
    clear_line(2);
    clear_line(3);
    clear_line(4);
    clear_line(5);
    clear_line(6);
    clear_line(7);
    clear_line(8);
    clear_line(9);
    clear_line(10);
    clear_line(11);
    clear_line(14);
    clear_line(15);
    set_software_cursor(3, 0);
    vga_puts("--- Kernel Shell Initialized ---\n");
    vga_set_text_attribute(VGA_ATTR_RED_ON_BLACK);
    vga_puts("YOU ARE IN **RING 0** PLEASE DONT DO ANYTHING STUPID!\n");
    vga_set_default_text_attribute();
    vga_puts("");
    vga_puts("Type 'help' for a list of commands.\n\n");

    while(1){
        vga_set_text_attribute(VGA_ATTR_PURPLE_ON_BLACK);
        vga_set_cursor_attribute(VGA_COLOR_WHITE);
        vga_puts("[kernel@imanios /]$ ");
        
        vga_set_text_attribute(VGA_ATTR_WHITE_ON_BLACK);
        read_command(input_buffer, 256);
        if (input_buffer[0] == '\0'){
            continue;
        }

        parse_cmd(input_buffer, command, args);
        vga_set_text_attribute(VGA_ATTR_GREEN_ON_BLACK);
        execute_command(command, args);
    }
}
