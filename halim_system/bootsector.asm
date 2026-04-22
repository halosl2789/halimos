[org 0x7C00]
bits 16

start:
    xor ax, ax
    mov ds, ax
    mov es, ax
    
    ; تحميل النظام (system.bin) إلى العنوان 0x8000
    mov ah, 0x02
    mov al, 10          ; تحميل 20 قطاع
    mov ch, 0
    mov dh, 0
    mov cl, 2           ; نبدأ من القطاع الثاني
    mov bx, 0x8000
    int 0x13
    
    jc disk_error
    jmp 0x8000          ; القفز إلى كود الـ 32-بت

disk_error:
    mov ah, 0x0E
    mov al, 'E'
    int 0x10
    hlt

times 510-($-$$) db 0
dw 0xAA55               ; التوقيع السحري