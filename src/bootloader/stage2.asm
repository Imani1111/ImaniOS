[bits 16]
[org 0x8000]

%define KERNEL_TMP_SEG    0x1000   ; 0x1000:0x0000 = 0x10000
%define KERNEL_DST_ADDR   0x100000 ; 1MB Destination
%define KERNEL_SECTORS    40       ; Total sectors to read

stage2_start:
    xor ax, ax
    mov ds, ax
    mov es, ax

    ; Save boot drive from DL into a secure variable
    mov [boot_drive], dl                     

    ; 1. Enable A20 Line via Fast A20
    in al, 0x92
    or al, 2
    out 0x92, al

    ; 2. Load Kernel (Read 1 sector at a time loop to cross track boundaries safely)
    mov bx, KERNEL_TMP_SEG
    mov es, bx
    xor bx, bx                     ; Initial buffer target = 0x1000:0x0000
    
    mov cl, 22                     ; Start sector
    mov bp, KERNEL_SECTORS         ; Loop counter for remaining sectors

.read_loop:
    mov ah, 0x02
    mov al, 1                      ; Read exactly 1 sector
    mov ch, 0                      ; Cylinder 0
    mov dl, [boot_drive]           ; Restore boot drive
    mov dh, 0                      ; Head 0
    int 0x13
    jc stage2_error

    inc cl                         ; Move to next sector
    
    ; Advance the ES segment register by 0x20 (512 bytes) for the next sector
    mov ax, es
    add ax, 0x20
    mov es, ax
    
    dec bp
    jnz .read_loop                 ; Keep looping until all sectors are read

    ; NEW: Get VBE Graphics Mode Info & Switch Modes
    ;xor ax, ax
    ;mov es, ax

    ;mov ax, 0x4F01
    ;mov cx, 0x4118
    ;mov di, vbe_info_block
    ;int 0x10
    ;cmp ax, 0x004F
    ;jne stage2_error

    ;mov eax, [vbe_info_block + 0x28]
    ;mov [vbe_framebuffer], eax

    ;mov eax, 0x4F02
    ;mov bx, 0x4118
    ;int 0x10
    ;cmp ax, 0x004F
    ;jne stage2_error
    
    ; 3. Enter Protected Mode
    cli
    lgdt [gdt_descriptor]          
    
    mov eax, cr0
    or eax, 0x1                    ; Set PE bit
    mov cr0, eax

    jmp CODE_SEG:init_pm           ; Far jump to 32-bit mode

stage2_error:
    mov si, stage2_err_msg
.loop:
    lodsb
    test al, al
    jz $
    mov ah, 0x0e
    int 0x10
    jmp .loop

boot_drive: db 0
stage2_err_msg: db "Disk Error Stage 2", 0
; NEW: VBE Variables
;align 4
;vbe_framebuffer: dd 0       ; This will hold our physical graphics pointer
;vbe_info_block:  times 256 db 0

; --- GDT ---
align 4
gdt_start:
    dd 0x0, 0x0                    ; Null descriptor
gdt_code:
    dw 0xffff, 0x0
    db 0x0, 10011010b, 11001111b, 0x0
gdt_data:
    dw 0xffff, 0x0
    db 0x0, 10010010b, 11001111b, 0x0
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

[bits 32]
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000               ; Setup stack safely away from 1MB
    mov esp, ebp

    ; 4. Move Kernel to 0x100000
    cld                            ; CLEAR DIRECTION FLAG (Ensures we copy forwards!)
    mov esi, 0x10000               ; Source (0x1000:0x0000)
    mov edi, KERNEL_DST_ADDR       ; Destination (1MB)
    mov ecx, (KERNEL_SECTORS * 512) / 4
    rep movsd                      

    ; 5. Extract VBE info to pass to C kernel (System V cdecl / 32-bit stack convention)
    
    ; Argument 4: Pitch (Offset 0x10, 2 bytes)
    ;movzx eax, word [vbe_info_block + 0x10]
    ;push eax

    ; Argument 3: Height (Offset 0x14, 2 bytes)
    ;movzx eax, word [vbe_info_block + 0x14]
    ;push eax

    ; Argument 2: Width (Offset 0x12, 2 bytes)
    ;movzx eax, word [vbe_info_block + 0x12]
    ;push eax

    ; Argument 1: Linear Framebuffer Pointer (Offset 0x28, 4 bytes)
    ;mov eax, [vbe_framebuffer]
    ;push eax

    ; Launch ImaniOS!
    jmp KERNEL_DST_ADDR
