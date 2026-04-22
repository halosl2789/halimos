[bits 16]
[extern kmain]
global start

start:

    cli
    lgdt [gdt_descriptor]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:init_32

[bits 32]
init_32:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    call kmain       
    jmp $

gdt_start:
    dq 0x0
gdt_code:
    dw 0xFFFF, 0x0, 0x9A00, 0x00CF
gdt_data:
    dw 0xFFFF, 0x0, 0x9200, 0x00CF
gdt_end:
gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start
