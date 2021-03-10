.model tiny
.code
org 100h

X         equ 5
Y         equ 17

Xsz       equ 20
Ysz       equ 3

start:    mov bx, 0b800h       ; Addr = es * 10h + di
          mov es, bx

          mov di, (Y*80+X+1)*2
          mov dh, 72h
          mov dl, 0c9h
          mov es:[di], dx 
          inc di
          inc di
          mov dl, 0cdh

l1:
          mov es:[di], dx
          inc di
          inc di
          cmp di, (Y*80+X)*2+2*2+Xsz*2
          jl l1

          mov dl, 0bbh
          mov es:[di], dx
          
          mov di, ((Y+1)*80+X)*2
          mov dl, 0c9h
          mov es:[di], dx
          inc di
          inc di

          mov dl, 0bch
          mov es:[di], dx
          inc di
          inc di

          mov dl, ' '
l2:
          mov es:[di], dx
          inc di
          inc di
          cmp di, ((Y+1)*80+X)*2+2*2+Xsz*2
          jl l2

          mov dl, 0c8h
          mov es:[di], dx
          inc di
          inc di

          mov dl, 0bbh
          mov es:[di], dx


          mov di, ((Y+2)*80+X)*2
          mov si, ((Y+2)*80+X)*2 + 2*2 + 2*Xsz + 2
l3:       
          mov dl, 0bah
          mov es:[di], dx
          inc di
          inc di

          mov dl, ' '
l4:       
          mov es:[di], dx
          inc di
          inc di
          cmp di, si
          jl l4

          mov dl, 0bah
          mov es:[di], dx

          add si, 80*2
          add di, ((Y+3)*80+X)*2 - (((Y+2)*80+X)*2 + 2*2 + 2*Xsz + 2)

          cmp di, ((Y+2)*80+X)*2 + 2*80*Ysz
          jl l3

          mov dl, 0c8h
          mov es:[di], dx
          inc di
          inc di

          mov dl, 0bbh
          mov es:[di], dx
          inc di
          inc di

          mov dl, ' '
l5:
          mov es:[di], dx
          inc di
          inc di
          cmp di, ((Y+2+Ysz)*80+X+2+Xsz)*2
          jl l5

          mov dl, 0c9h
          mov es:[di], dx
          inc di
          inc di

          mov dl, 0bch
          mov es:[di], dx

          mov di, ((Y+2+Ysz+1)*80+X+1)*2

          mov dl, 0c8h
          mov es:[di], dx
          inc di
          inc di

          mov dl, 0cdh
l6:
          mov es:[di], dx
          inc di
          inc di
          cmp di, ((Y+2+Ysz+1)*80+X+2+Xsz)*2
          jl l6

          mov dl, 0bch
          mov es:[di], dx
          inc di
          inc di


          mov bx, offset msg   ; bx = msg;
          mov di, ((Y+2)*80+X)*2 + 2*2

while_loop:                    ; while (true) {
          mov dl, [bx]         ;   dl = *bx; 
          cmp dl, '$'          ;   if (dl == '$')
          je exit              ;     break;

          mov es:[di], dx      ;   putchar(dl);
                               ;   // place on screen -- di
                               ;   // color           -- dh
                               ;   // symbol          -- dl

          inc bx               ;   bx++;
          inc di               ;   di++;
          inc di               ;   di++;
          jmp while_loop       ; }   

exit:     mov ax, 4c00h        ;
          int 21h              ; exit();

msg db 'Hello mew ^_^$'

end start