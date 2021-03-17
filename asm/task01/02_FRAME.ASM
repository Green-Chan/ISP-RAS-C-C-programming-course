.model tiny
.code
org 100h

X         equ 5
Y         equ 17

screen_width equ 80

start:    
          mov dh, 72h
          mov bx, offset frame_pattern
          call print_frame

          mov bx, offset frame_pattern2
          call print_frame

          ; print message

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

print_frame:
; Arguments of the function:
; dh -- color
; bx -- pointer to the frame pattern
;     Frame pattern structure:
;     dw first column, first row, frame width (inside), frame height (inside)
;     db s11, s12, s13, s14, s15
;     db s21, s22, s23, s24, s25
;     db s31, s32, s33, s34, s35
;     db s41, s42, s43, s44, s45
;     db s51, s52, s53, s54, s55
;     Where
;     s11 - first symbol in first row
;     s12 - second symbol in first row
;     s13 - middle symbols in first row (in amount of frame width)
;     s14 - second-to-last symbol in first row
;     s15 - last symbol in first row
;     s21, s22, s23, s24, s25 - the same in second row
;     s31, s32, s33, s34, s35 - the same in middle rows (in amount of frame heifht)
;     s41, s42, s43, s44, s45 - the same in second-to-last raw
;     s51, s52, s53, s54, s55 - the same in last raw

; ax, bx, cx, dx -- caller-saved registers
; di, si -- callee-saved registers

          ; save callee-saved registers
          push di
          push si

          ; prepare argument for print_frame_line call
          mov ax, 0b800h
          mov es, ax
          mov dh, 72h

          ; calculate position to start: di := (Y*screen_width+X)*2
          mov al, [bx+2]
          mov cl, screen_width
          mul cl
          mov di, ax
          add di, [bx]
          shl di, 1

          ; calculate the end position of the first
          ; raw minus 2 symbols: si := di + (Xsz + 2)*2.
          ; ! Not "Xsz*2 - 4" because the end position is
          ; ! begin position + (Xsz + 4)*2.
          mov si, [bx+4]
          inc si
          inc si
          shl si, 1
          add si, di

          ; calculate difference between end position
          ; and begin position of the next raw:
          ; cx := (screen_width - 4 - Xsz)*2
          mov cx, screen_width - 4
          sub cx, [bx + 4]
          shl cx, 1

          ; ax := Ysz
          mov ax, [bx + 6]

          add bx, 8

          call print_frame_line

          add di, cx
          add si, screen_width * 2
          add bx, 5

          call print_frame_line

          add di, cx
          add si, screen_width * 2
          add bx, 5

.frame_loop:
          call print_frame_line
          
          add di, cx
          add si, screen_width * 2

          dec ax
          cmp ax, 0
          jg .frame_loop

          add bx, 5

          call print_frame_line

          add di, cx
          add si, screen_width * 2
          add bx, 5

          call print_frame_line

          ; restore callee-saved registers
          pop si
          pop di
          ret

print_frame_line:
; Arguments of the function:
; es -- screen sector
; di -- begin position
; si -- end position minus 2 symbols (4 bytes)
; dh -- color
; bx -- row pattern begin

; After this function work
; -- es, si, bx and dh stay the same
; -- di becomes the end position
; Other registers:
; -- dl changes
          mov dl, [bx]
          mov es:[di], dx
          inc di
          inc di

          mov dl, [bx + 1]
          mov es:[di], dx
          inc di
          inc di

          mov dl, [bx + 2]
.frame_line_loop:
          mov es:[di], dx
          inc di
          inc di
          cmp di, si
          jl .frame_line_loop

          mov dl, [bx + 3]
          mov es:[di], dx
          inc di
          inc di

          mov dl, [bx + 4]
          mov es:[di], dx
          inc di
          inc di

          ret
           

msg db 'Hello mew ^_^$'

frame_pattern dw 5, 17, 20, 3
              db 020h, 0d5h, 0cdh, 0b8h, 020h
              db 0d5h, 0beh, 020h, 0d4h, 0b8h
              db 0b3h, 020h, 020h, 020h, 0b3h
              db 0d4h, 0b8h, 020h, 0d5h, 0beh
              db 020h, 0d4h, 0cdh, 0beh, 020h

frame_pattern2 dw 35, 17, 20, 3
              db 020h, '/', '-', '\', 020h
              db '/', 020h, 020h, 020h, '\'
              db '|', 020h, 020h, 020h, '|'
              db '\', 020h, 020h, 020h, '/'
              db 020h, '\', '-', '/', 020h

end start