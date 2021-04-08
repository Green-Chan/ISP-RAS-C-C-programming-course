.model tiny
.code
org 100h

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
          ;prepare to play music
          mov word ptr next_note_addr, offset for_elise
          mov ax, 40h
          mov es, ax
          mov bx, 6ch
          mov cx, es:[bx]
          add cx, SOUND_LENGTH
          mov word ptr next_note_time, cx

          ; set new timer handler
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
is_playing_music dw TRUE

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

end_of_program:
end       start