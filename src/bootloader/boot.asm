[bits 16]
[org 0x7c00]

%define STAGE2_SEG 0x0000
%define STAGE2_OFFSET 0x8000
%define STAGE2_SECTORS 20

start:
    cli
    xor ax, ax
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov sp, 0x7c00
    sti

    mov [BOOT_DRIVE], dl           ; Save it just in case

    ; Load stage 2
    mov ah, 0x02
    mov al, STAGE2_SECTORS
    mov ch, 0
    mov cl, 2                      ; Sector 2
    mov dh, 0
    ; dl is untouched, still holds boot drive from BIOS

    mov bx, STAGE2_OFFSET
    int 0x13
    jc disk_error

    ; Jump to stage 2 (Pass boot drive in DL)
    jmp STAGE2_SEG:STAGE2_OFFSET

disk_error:
    mov si, disk_msg
.print:
    lodsb
    test al, al
    jz $
    mov ah, 0x0e
    int 0x10
    jmp .print

BOOT_DRIVE: db 0
disk_msg: db "Disk Error Stage 1", 0


times 510 - ($ - $$) db 0
dw 0xaa55
