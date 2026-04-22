[org 0x7C00]
bits 16

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    
    
    mov ah, 0x02
    mov al, 10          
    mov ch, 0
    mov dh, 0
    mov cl, 2           
    mov bx, 0x8000
    int 0x13
    
    jc disk_error
    jmp 0x8000          

disk_error:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    hlt

times 510-($-$$) db 0
dw 0xAA55               
