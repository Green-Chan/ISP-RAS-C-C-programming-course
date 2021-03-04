
.model tiny
.code

org 100h

start:    mov bx, offset msg   ; bx = msg;

cycle:                         ; while (true) {
          mov dl, [bx]         ;   dl = *bx; 
          cmp dl, '$'          ;   if (dl == '$')
          je exit              ;     break;
          
          mov ah, 06h          ;
          int 21h              ;   putchar(dl);

          inc bx               ;   bx++;
          jmp cycle            ; }

exit:     mov ax, 4c00h        ;
          int 21h              ; exit();

msg db 'Hello meow ^_^$'

end start

