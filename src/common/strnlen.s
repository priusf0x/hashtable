; =========================================================
; |                 Printf implementation                 |
; | Filename:................................. my_print.s |
; | Architect:.....................................x86_16 |
; | Assembler:...................................... nasm |
; | Made for:.................................. Linux x86 |
; | Author:..................................... pr1usf0x |
; =========================================================

; _________________________________________________________
; |                     Function name                     |
; _________________________________________________________

global          ssestrncmp

; _________________________________________________________
; |                     avxstrncmp                        |
; | optimized srtnlen function                            |
; | Args: rdi - 1st str                                   |
; |       rsi - 2st str                                   |
; |       rdx - string length                             |
; | Returns: 0 - if strings are equal                     |
; |          1 - if not                                   |
; | Delete: rcx, xmm0                                     |
; _________________________________________________________

default rel
section .text

ssestrncmp:
            cmp edx, 16
            jg .if_greater_16

            mov eax, edx                    
            vmovdqu xmm0, [rdi] ; load from memory
            vpcmpestrm xmm0, [rsi], 24  ; compare string 
            setc al
            movzx eax, al
            ret

.if_greater_16:
            xor ecx, ecx
.loop:                                                          
            mov eax, edx
            vmovdqu xmm0, [rdi+rcx]
            vpcmpestrm xmm0, [rsi+rcx], 24
            setc al
            sub edx, 16
            add rcx, 16
            movzx eax, al
            test edx, edx
            jle .leave
            test eax, eax
            je .loop
.leave:

            ret
