#include <vga.h>
#include <keyboard.h>
#include <process.h>

void snake_main()
{
    clear_vga_buffer();
    while (1){
        vga_puts("[DEBUG] Snake process successfully started!\n");

        char c = get_char();

        vga_puts("I received key: ");
        vga_print_hex((uint32_t)c);
        vga_putc('\n');

        if (c == 27){
            keyboard_focus = 0;

            pcb_t* temp = head_process;
            do {
                if (temp->pid == 5) {
                    vga_puts("--- Parent (PID 5) Context at Child Exit ---\n");
                    vga_puts("State: "); vga_print_hex(temp->state); vga_puts("\n");
                    vga_puts("Waiting on PID: "); vga_print_hex(temp->waiting_on_process); vga_puts("\n");
        
                    // If temp->esp points to its saved stack frame when it went to sleep:
                    uint32_t* parent_stack = (uint32_t*)temp->esp;
                    if (parent_stack != NULL) {
                        vga_puts("Parent Saved EIP: "); vga_print_hex(parent_stack[8]); vga_puts("\n");
                        vga_puts("Parent Saved CS:  "); vga_print_hex(parent_stack[9]); vga_puts("\n");
                        vga_puts("Parent Saved ESP: "); vga_print_hex(parent_stack[3]); vga_puts("\n");
                    }

                    break;
                }
                temp = temp->next_process;
            } while (temp != head_process);

            //while(1) asm volatile("cli; hlt");
            exit_process();
        }
    }
}
