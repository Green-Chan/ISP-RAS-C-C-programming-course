.model tiny
.code
org 100h

X         equ 5
Y         equ 17

screen_width equ 80

start:
          xor bx, bx
          mov es, bx
          mov bx, 9 * 4

          cli
          mov ax, es:[bx]
          mov word ptr old_09, ax
          mov ax, es:[bx+2]
          mov word ptr old_09+2, ax
          
          mov es:[bx], offset new_09
          mov ax, cs
          mov es:[bx+2], ax
          sti

          mov ax, 3100h
          mov dx, offset end_of_program
          shr dx, 4
          inc dx
;          int 21h
 
.forever_cycle:
          inc al
          jmp .forever_cycle

new_09    proc

          push ds
          push es
          push di
          push si
          push dx
          push cx
          push bx
          push ax

          mov ax, cs
          mov ds, ax

          mov ax, 0b800h
          mov es, ax

          mov dh, 72h
          mov bx, offset frame_pattern
          call print_frame

          ; print number

          mov di, ((Y+2)*80+X)*2 + 2*2
          pop ax
          push ax
          mov bx, 2
          call print_number

          mov dl, 'b'
          mov es:[di], dx
          inc di
          inc di


          mov di, ((Y+3)*80+X)*2 + 2*2
          mov bx, 8
          call print_number

          mov dl, 'o'
          mov es:[di], dx
          inc di
          inc di


          mov di, ((Y+4)*80+X)*2 + 2*2
          mov bx, 16
          call print_number

          mov dl, 'h'
          mov es:[di], dx
          inc di
          inc di

.new09_exit:

          in al, 60h
          in al, 61h
          mov ah, al
          or al, 80h
          out 61h, al
          mov al, ah
          out 61h, al

          mov al, 20h
          out 20h, al

          pop ax
          pop bx
          pop cx
          pop dx
          pop si
          pop di
          pop es
          pop ds

          db 0eah   ; jmp far command code
old_09 dd 0         ; address where to jump
new_09    endp


;------------------------------------------------------------
;------------------------------------------------------------

print_number   proc
; Print a number in binary (base 2), octal (base 8) or
; hexadecimal (base 16) notation to screen.
; Arguments of the function:
; es -- screen sector
; di -- begin position (where to print)
; dh -- color
; ax -- number to print
; bx -- base (if not 2, 8, or 16 function do nothing)
;
; After this function work
; -- ax, es and dh stay the same
; -- di becomes the end position
; Other registers:
; -- dl changes
; bx, cx -- caller-saved registers
; si, bp -- callee-saved register

          ; save callee-saved registers
          push si
          push bp

          cmp bx, 2
          je .binary

          cmp bx, 8
          je .octal

          cmp bx, 16
          je .hex

          ; bad base
          pop bp
          pop si
          ret

.binary:
          mov cl, 15
          mov ch, 1

          jmp .print_number_preparation

.octal:
          mov cl, 15
          mov ch, 3

          jmp .print_number_preparation

.hex:
          mov cl, 12
          mov ch, 4
          
.print_number_preparation:
          cmp ax, 0
          jne .print_notzero_number
          mov dl, '0'
          mov es:[di], dx
          inc di
          inc di
          pop bp
          pop si
          ret

.print_notzero_number:
          dec bx
          mov si, ax

.print_number_skip_spaces:
          shr ax, cl
          and ax, bx
          add ax, offset digits

          cmp ax, offset digits
          jne .print_number_print

          sub cl, ch
          mov ax, si

          jmp .print_number_skip_spaces

.print_number_print:
          mov bp, ax
          mov dl, [bp]
          mov es:[di], dx
          inc di
          inc di

          cmp cl, 0
          mov ax, si
          je .print_number_end

          sub cl, ch
          shr ax, cl
          and ax, bx
          add ax, offset digits
          jmp .print_number_print

.print_number_end:
          pop bp
          pop si
          ret

print_number   endp

;------------------------------------------------------------
;------------------------------------------------------------

print_frame    proc
; Arguments of the function:
; es -- screen sector
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
; es stay the same

          ; save callee-saved registers
          push di
          push si

          ; prepare argument for print_frame_line call
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

print_frame    endp

;------------------------------------------------------------
;------------------------------------------------------------

print_frame_line    proc
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
           
print_frame_line    endp


msg db 'Hello mew ^_^$'

digits db '0123456789ABCDEF'

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

end_of_program:
end start