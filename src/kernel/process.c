#include <memory.h>
#include <process.h>
#include <idt.h>
#include <vga.h>
#include <string.h>

pcb_t* head_process = NULL;
pcb_t* tail_process = NULL;
pcb_t* current_process = NULL;
int next_pid = 1;

uint32_t* pid1_stack_base = 0;
uint32_t pid1_stack_pointer = 0;

int first_switch = 1;

void kernel_idle_loop()
{
    while(1){
        asm volatile("hlt");
    }
}

void process_init()
{
    next_pid = 1;
    vga_puts("WE REACHED PROCESS_INIT\n"); 
    pcb_t* kernel_process = (pcb_t*)mem_allocate(sizeof(pcb_t));
    my_memset(kernel_process, 0, sizeof(pcb_t));
    if (!kernel_process){
        vga_puts("[PANIC] Kernel Process PID1 not initialized");
        while(1){
            asm volatile("cli; hlt");
        }
    }
    uint8_t* kernel_process_stack = (uint8_t*)mem_allocate(PROCESS_STACK_SIZE);
    my_memset(kernel_process_stack, 0, PROCESS_STACK_SIZE);
    if (!kernel_process_stack){
        vga_puts("[CRITICAL ERROR!] Kernel Process stack not forged!");
        while(1){
            asm volatile("cli; hlt");
        }
    }
    kernel_process->pid = 0;
    kernel_process->state = PROCESS_STATE_RUNNING;
    kernel_process->stack_base = (uint32_t*)kernel_process_stack;
    pid1_stack_base = kernel_process->stack_base;

    uint32_t* kernel_process_stack_pointer = (uint32_t*)((uint8_t*)kernel_process_stack + PROCESS_STACK_SIZE);
    // Kernel process execution frame
    *(--kernel_process_stack_pointer) = 0x202;
    *(--kernel_process_stack_pointer) = GDT_CODE_SEGMENT;
    *(--kernel_process_stack_pointer) = (uint32_t)kernel_idle_loop;

    // Save GPRs
    *(--kernel_process_stack_pointer) = 0;
    *(--kernel_process_stack_pointer) = 0;
    *(--kernel_process_stack_pointer) = 0;
    *(--kernel_process_stack_pointer) = 0;
    *(--kernel_process_stack_pointer) = 0;
    *(--kernel_process_stack_pointer) = 0;
    *(--kernel_process_stack_pointer) = 0;
    *(--kernel_process_stack_pointer) = 0;
    kernel_process->esp = (uint32_t)kernel_process_stack_pointer;
    pid1_stack_pointer = kernel_process->esp;
    
    kernel_process->waiting_on_process = -1;

    head_process = kernel_process;
    tail_process = kernel_process;
    kernel_process->next_process = kernel_process;
    
    vga_puts("Address of next_pid: ");
    vga_print_hex((uint32_t)&next_pid); 
    vga_puts("\n");

    current_process = kernel_process;
}

pcb_t* process_create(void(*entry_point)(void))
{    
    pcb_t* new_process = (pcb_t*)mem_allocate(sizeof(pcb_t));
    my_memset(new_process, 0, sizeof(pcb_t));
    if (!new_process){
        vga_puts("[CRITICAL ERROR] mem_allocate failed for PCB!\n");
        while(1){
            asm volatile("hlt");
        }
    }
    uint8_t* process_stack = (uint8_t*)mem_allocate(PROCESS_STACK_SIZE);
    *(uint32_t*)process_stack = 0xDEADBEEF;
    my_memset(process_stack, 0, PROCESS_STACK_SIZE);
    if (process_stack == NULL){
        vga_puts("[CRITICAL ERROR] mem_allocate failed for process stack!\n");
        while(1){
            asm volatile("hlt");
        }
    }

    new_process->pid = next_pid++;
    new_process->state = PROCESS_STATE_READY;
    new_process->stack_base =(uint32_t*)process_stack;
    
    uint32_t process_stack_bottom = (uint32_t)process_stack;
    uint32_t process_stack_top = process_stack_bottom + PROCESS_STACK_SIZE;
    uint32_t* process_stack_pointer = (uint32_t*)process_stack_top; 
 
    // Execution Frame
    *(--process_stack_pointer) = 0x202;
    *(--process_stack_pointer) = GDT_CODE_SEGMENT;
    *(--process_stack_pointer) = (uint32_t)entry_point;
    
    // Save GPRs
    // Order for pusha/popa: EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX
    *(--process_stack_pointer) = 0; // EDI
    *(--process_stack_pointer) = 0; // ESI
    *(--process_stack_pointer) = 0; // EBP
    *(--process_stack_pointer) = 0; // ESP
    *(--process_stack_pointer) = 0; // EBX
    *(--process_stack_pointer) = 0; // EDX
    *(--process_stack_pointer) = 0; // ECX
    *(--process_stack_pointer) = 0; // EAX
    new_process->esp = (uint32_t)process_stack_pointer;
    
    new_process->waiting_on_process = -1;
    
    asm volatile("cli");
    if (head_process == NULL){
        head_process = new_process;
        tail_process = new_process;
        new_process->next_process = head_process;
    }else{
        new_process->next_process = head_process;
        tail_process->next_process = new_process;
        tail_process = new_process;
    }
    // Inside process_create, at the very end
    vga_puts("Created PID: "); 
    vga_print_hex(new_process->pid);
    vga_puts("\n");
    vga_puts("Stack: ");
    vga_print_hex((uint32_t)new_process->stack_base);
    vga_puts("\n");

    vga_puts("DEBUG: Validating stack setup for PID: ");
    vga_print_hex(new_process->pid);
    vga_puts("\n");
    vga_puts("CS value at stack_ptr[9]: ");
    vga_print_hex(process_stack_pointer[9]);
    vga_puts("\n");
    vga_puts("Process State: ");
    vga_print_hex((uint32_t)new_process->state);
    vga_puts("\n");

    asm volatile("sti");
    return new_process;
}

uint32_t scheduler_switch_task(uint32_t current_esp)
{
    //vga_puts("Current running process: ");
    //vga_print_hex(current_process->pid);
    //vga_puts("\n");
    if (current_process == NULL) {
        vga_puts("[PANIC] current_process is NULL!\n");
        while(1) asm volatile("cli; hlt");
    }
    
    if (current_esp > 0x00100000){
        if (current_process->state == PROCESS_STATE_RUNNING){
            current_process->state = PROCESS_STATE_READY;
            current_process->esp = current_esp;
        }
    }
        
    int current_is_valid = 0;
    pcb_t* check = head_process;
    do {
        if (check == current_process){
            current_is_valid = 1;
            break;
        }
        check = check->next_process;
    }while(check != head_process);

    if (!current_is_valid){
        current_process = head_process;
    }

    // Round-Robin: Find next READY or RUNNING process 
    if (head_process == NULL){
        vga_puts("No processes left!\n");
        while(1){
            asm volatile("cli; hlt");
        }
    }

    pcb_t* next_process = current_process->next_process;
    int found = 0;
    pcb_t* temp = head_process;
    int total_procs = 0;
    do {
        total_procs++;
        temp = temp->next_process;
    }while(temp != head_process);

    for (int i = 0; i < total_procs; i++){
        if (next_process->state == PROCESS_STATE_READY || next_process->state == PROCESS_STATE_RUNNING){      
            found = 1;
            break;
        }
        next_process = next_process->next_process;
    }
    if (!found) {
        vga_puts("SCHEDULER ERROR: No ready processes!\n");
        while(1) asm volatile("cli; hlt");
    }
    current_process = next_process;

    // Diagnostic print right before check
    //vga_puts("Checking PID ");
    //vga_pcrint_hex(current_process->pid);
    //vga_puts(" with ESP: ");
    //vga_print_hex(current_process->esp);
    //vga_puts("\n");

    current_process->state = PROCESS_STATE_RUNNING;
    //vga_puts("[SCHED] Switched to PID: ");
    //vga_print_hex(current_process->pid);
    //vga_puts("\n");
    //
    return current_process->esp;
}

void process_reaper(void)
{
    while(1){
        asm volatile("cli");
        if (head_process == NULL) return;
    
        pcb_t* comparison_process = head_process;
        pcb_t* previous_process = tail_process;
            
        int processed_count = 0;
        int initial_proc_count = 0;
        pcb_t* temp = head_process;
        do {
            initial_proc_count++;
            temp = temp->next_process;
        }while(temp != head_process);

        while(processed_count < initial_proc_count && head_process != NULL){
            pcb_t* next_node = comparison_process->next_process;

            if (comparison_process->state == PROCESS_STATE_TERMINATED){
                if (comparison_process == comparison_process->next_process){
                    clear_vga_buffer();
                    vga_puts("[PANIC] Kernel tried to terminate all processes!\n");
                    while(1){
                        asm volatile("cli; hlt");
                    }
                }
                if (comparison_process == head_process){
                    head_process = next_node;
                }
                if (comparison_process == tail_process){
                    tail_process = previous_process;
                }
            
                previous_process->next_process = next_node;
                mem_free(comparison_process->stack_base);
                mem_free(comparison_process);

                if (head_process == comparison_process){
                    head_process = NULL;
                    break;
                }
                    
                comparison_process = next_node;
                }else{
                    previous_process = comparison_process;
                    comparison_process = next_node;
            }
            processed_count++;
        }
        asm volatile("sti");
    }   
}

void exit_process()
{
    asm volatile("cli");
    
    current_process->state = PROCESS_STATE_TERMINATED;

    pcb_t* parent_process = head_process;
    if (parent_process != NULL){
        pcb_t* start = parent_process;
        do {
            if (parent_process->waiting_on_process == current_process->pid){
                parent_process->state = PROCESS_STATE_READY;
                parent_process->waiting_on_process = -1;
            }
            parent_process = parent_process->next_process;
        }while(parent_process != start);
    }

    // Trigger immediate schedule switch, I'll later change it to a yield() function
    asm volatile("int $0x20");
    while(1){
        asm volatile("cli; hlt");
    }
}
