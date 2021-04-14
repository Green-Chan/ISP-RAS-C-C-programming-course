.model tiny
.code
org 100h

X            equ 5
Y            equ 16
F_scan_code  equ 33
M_scan_code  equ 50

screen_width equ 80


C_1   equ 4561	; ~ 1193180 / 261.63
CS1   equ 4305	; ~ 1193180 / 277.18
D_1   equ 4063	; ~ 1193180 / 293.66
DS1   equ 3835	; ~ 1193180 / 311.13
E_1   equ 3620	; ~ 1193180 / 329.63
F_1   equ 3417	; ~ 1193180 / 349.23
FS1   equ 3225	; ~ 1193180 / 369.99
G_1   equ 3045	; ~ 1193180 / 392.00
GS1   equ 2873	; ~ 1193180 / 415.30
A_1   equ 2712	; ~ 1193180 / 440.00
AS1   equ 2560	; ~ 1193180 / 466.16
B_1   equ 2416	; ~ 1193180 / 493.88
A_1   equ 2712	; ~ 1193180 / 440.00
C_2   equ 2280	; ~ 1193180 / 523.25
CS2   equ 2152	; ~ 1193180 / 554.36
D_2   equ 2032	; ~ 1193180 / 587.32
DS2   equ 1918	; ~ 1193180 / 622.26
E_2   equ 1810	; ~ 1193180 / 659.26

PAUSE equ 0

SOUND_LENGTH  equ  4

FALSE equ 0
TRUE  equ 1




start:
          ; set new keyboard interrupt handler
          mov ax, offset new_09         ; new handler
          mov bx, 9                     ; keyboard interrupt
          mov di, offset old_09         ; where to save old handler
          call save_and_rewrite_handler

          ; set new timer interrupt handler
          mov ax, offset new_1c         ; new handler
          mov bx, 1ch                   ; keyboard interrupt
          mov di, offset old_1c         ; where to save old handler
          call save_and_rewrite_handler

          ; stay resident
          mov ax, 3100h
          mov dx, offset end_of_program
          shr dx, 4
          inc dx
          int 21h

;------------------------------------------------------------
;------------------------------------------------------------

new_09    proc

          push ds
          push es
          push di
          push si
          push dx
          push cx
          push bx
          push ax

          in al, 60h
          cmp al, F_scan_code
          je .print_frame_and_register

          cmp al, M_scan_code
          je .on_off_music

          pop ax
          pop bx
          pop cx
          pop dx
          pop si
          pop di
          pop es
          pop ds

          db 0eah   ; jmp far command code
old_09    dd 0      ; address where to jump
new_09    endp


.print_frame_and_register: 
          
          call send_EOIs

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


          mov di, ((Y+3)*80+X)*2 + 2*2
          mov bx, 8
          call print_number

          mov dl, 'o'
          mov es:[di], dx


          mov di, ((Y+4)*80+X)*2 + 2*2
          mov bx, 16
          call print_number

          mov dl, 'h'
          mov es:[di], dx

          mov di, ((Y+5)*80+X)*2 + 2*2
          call print_dec

.new09_exit:
          pop ax
          pop bx
          pop cx
          pop dx
          pop si
          pop di
          pop es
          pop ds

          iret

.on_off_music:

          call send_EOIs

          mov ax, cs
          mov ds, ax

          cmp word ptr is_playing_music, TRUE
          jne .turn_on_music

          ; turn off music
          mov word ptr is_playing_music, FALSE
          in  al, 61h
          and al, 11111100b
          out 61h, al
          jmp .new09_exit

.turn_on_music:
          ;prepare to play music
          mov word ptr next_note_addr, offset for_elise
          mov ax, 40h
          mov es, ax
          mov bx, 6ch
          mov cx, es:[bx]
          add cx, SOUND_LENGTH
          mov word ptr next_note_time, cx

          mov word ptr is_playing_music, TRUE
          jmp .new09_exit
          

;------------------------------------------------------------
;------------------------------------------------------------

send_EOIs:

          ;send EOI to keyboard
          in al, 61h
          mov ah, al
          or al, 80h
          out 61h, al
          mov al, ah
          out 61h, al

          ;send EOI to master PIC
          mov al, 20h
          out 20h, al

          ret

;------------------------------------------------------------
;------------------------------------------------------------
print_dec:
; Print a number in decimal notation to screen
; es -- screen sector
; di -- begin position (where to print)
; dh -- color
; ax -- number to print
;
; After this function work
; -- di becomes the end position
; -- es and dh stay the same
; Other registers:
; -- dl changes
; ax, bx, cx -- caller-saved registers

          cmp ax, 0
          jne .print_dec_notzero_number
          mov dl, '0'
          mov es:[di], dx
          inc di
          inc di
          ret

.print_dec_notzero_number:
          mov cx, 10
          mov bh, dh
          mov dx, 0deadh
          push dx

.bin_to_dec_loop:
          mov dx, 0
          div cx
          push dx
          cmp ax, 0
          jne .bin_to_dec_loop

.print_dec_loop:
          pop dx
          cmp dx, 0deadh
          je .end_print_dec
          add dl, '0'
          mov dh, bh
          mov es:[di], dx
          inc di
          inc di
          jmp .print_dec_loop

.end_print_dec:
          mov dh, bh
          ret

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
          mov dl, ds:[bp]
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

;------------------------------------------------------------
;------------------------------------------------------------

new_1c:
          push ds
          push es
          push dx
          push cx
          push bx
          push ax

          mov ax, cs
          mov ds, ax

          cmp word ptr is_playing_music, TRUE
          jne .end_new_1c

          mov ax, 40h
          mov es, ax
          mov bx, 6ch
          mov cx, es:[bx]

          cmp word ptr next_note_time, cx
          jne .end_new_1c

          ; get note and update next_note_addr
          mov bx, word ptr next_note_addr 
          mov cx, [bx]  ; save note in cx
          inc bx 
          inc bx 
          cmp bx, offset for_elise_end
          jne .set_next_note_addr
          ; start from the begining
          mov bx, offset for_elise
.set_next_note_addr:
          mov word ptr next_note_addr, bx
          
          ; update next_note_time
          add word ptr next_note_time, SOUND_LENGTH
          
          ; play note (cx)
          cmp cx, PAUSE
          je .play_pause_

          ; turn music on
          in  al, 61h
          or  al, 00000011b
          out 61h, al
          mov al, 10110110b
          out 43h, al

          ; play note in cx
          mov al, cl
          out 42h, al
          mov al, ch
          out 42h, al

          jmp .end_new_1c

.play_pause_:
          ; turn music off
          in  al, 61h
          and al, 11111100b
          out 61h, al

.end_new_1c:
          pop ax
          pop bx
          pop cx
          pop dx
          pop es
          pop ds

          db 0eah   ; jmp far command code
old_1c    dd 0      ; address where to jump

;------------------------------------------------------------
;------------------------------------------------------------

save_and_rewrite_handler:
; Arguments of the function:
;   ax -- new handler 
;   bx -- interrupt number
;   di -- address to save old handler
; ax, bx, cx, dx -- caller-saved registers
; Other registers callee-saved
          push es

          xor dx, dx
          mov es, dx
          shl bx, 2

          cli
          mov dx, es:[bx]
          mov [di], dx
          mov dx, es:[bx+2]
          mov [di+2], dx
          
          mov es:[bx], ax
          mov ax, cs
          mov es:[bx+2], ax
          sti

          pop es
          ret


;------------------------------------------------------------
;------------------------------------------------------------

digits db '0123456789ABCDEF'

;------------------------------------------------------------
;------------------------------------------------------------

frame_pattern dw 5, 16, 20, 4
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

;------------------------------------------------------------
;------------------------------------------------------------

for_elise dw E_2, DS2
          dw E_2, DS2, E_2, B_1, D_2, C_2
          dw A_1, A_1, PAUSE, C_1, E_1, A_1
          dw B_1, B_1, PAUSE, C_1, GS1, B_1         
          dw C_2, C_2, PAUSE, E_1, E_2, DS2
          dw E_2, DS2, E_2, B_1, D_2, C_2
          dw A_1, A_1, PAUSE, C_1, E_1, A_1
          dw B_1, B_1, PAUSE, E_1, C_2, B_1
          dw A_1, A_1, A_1, A_1
for_elise_end:

;------------------------------------------------------------
;------------------------------------------------------------

next_note_time dw 0
next_note_addr dw 0
is_playing_music dw FALSE

;------------------------------------------------------------
;------------------------------------------------------------

end_of_program:
end start