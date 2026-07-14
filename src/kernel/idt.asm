[bits 32]

global load_idt
global isr0_divide_by_zero
extern exception_handler

global isr21_keyboard
extern keyboard_handler

global isr20_pit_timer
extern timer_handler

; Load the IDT pointer into the CPU
load_idt:
    mov eax, [esp + 4]  ; Get the pointer address passed from C
    lidt [eax]          ; Load IDT
    ret

; Interrupt Service Routine for Division by Zero (ISR 0)
isr0_divide_by_zero:
    pushad              ; Push EDI, ESI, EBP, ESP, EBX, EDX, ECX, EAX

    call exception_handler

    popad               ; Restore registers
    iretd               ; Interrupt Return (crucial for returning back to normal code)


isr21_keyboard:
    pusha
    call keyboard_handler
    popa
    iret

isr20_pit_timer:
    pusha
    call timer_handler
    popa
    iret
