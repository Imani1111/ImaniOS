[bits 32]
[global _start]
[extern kernel_main]

section .text.prologue

_start:
    ; 1. Preserve EBX (our framebuffer address from stage2) before doing anything else
    push ebx

    ; 2. Zero out the BSS section completely
    extern _bss_start
    extern _bss_end
    
    mov edi, _bss_start
    mov ecx, _bss_end
    sub ecx, edi            ; Calculate size of BSS in bytes
    xor al, al              ; Write 0s
    rep stosb               ; Fill memory with 0

    ; 3. Restore the framebuffer address and push it as the first argument to kernel_main
    pop ebx
    push ebx                ; Push the framebuffer pointer onto the stack

    ; 4. Call our C main function
    call kernel_main   
    
    ; If kernel returns, halt
    cli
.hang:
    hlt
    jmp .hang
