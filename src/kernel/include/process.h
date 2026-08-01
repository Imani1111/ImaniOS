#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <stddef.h>

#define PROCESS_STATE_READY 1
#define PROCESS_STATE_RUNNING 2
#define PROCESS_STATE_BLOCKED 3
#define PROCESS_STATE_WAITING 4
#define PROCESS_STATE_TERMINATED 5

#define PROCESS_STACK_SIZE 16384U

typedef struct PCB{
    uint32_t esp;
    uint32_t* stack_base;
    int state;
    int pid;
    struct PCB* next_process;
    int waiting_on_process;
}pcb_t;

extern pcb_t* current_process;
extern pcb_t* head_process;
extern pcb_t* tail_process;
extern int next_pid;

void kernel_idle_loop(void);
void process_init(void);
pcb_t* process_create(void(*entry_point)(void));
void scheduler_tick();
uint32_t scheduler_switch_task(uint32_t current_esp);
void process_reaper(void);
void exit_process(void);

#endif
