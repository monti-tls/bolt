; This file is part of bolt.
;    
; Copyright (c) 2015 Alexandre Monti
;
; bolt is free software: you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation, either version 3 of the License, or
; (at your option) any later version.
; 
; bolt is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU General Public License for more details.
; 
; You should have received a copy of the GNU General Public License
; along with bolt.  If not, see <http://www.gnu.org/licenses/>.

;;
;; string.bas
;;

;; This file is a simple demonstration of the Bolt assembly.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; void cststr(int* str, int* addr)
; {
;     int c = 0;
;     int i = 0;
;     do
;     {
;         c = *(str + i);
;         *(addr + i) = c;
;         i = i + 1;
;     } while (c != 0);
; }
   
cststr:
    push #0       ; int c = 0
    push #0       ; int i = 0
    mov %r0, %sp  ; (locals setup)
    
cststr-l1:
    push [%ab-1]  ; str + i
    push [%r0-1]
    uadd
    cst           ; *()
    pop [%r0-2]   ; c =
    
    push [%r0-2]  ; c
    push [%ab]    ; addr + i
    push [%r0-1]
    uadd
    stor          ; *()
    
    push [%r0-1]  ; i + 1
    push #1u
    uadd
    pop [%r0-1]   ; i =
    
    push [%r0-2]  ; c
    push #0
    ucmp
    jne cststr-l1 ; do {} while()
    
cststr-cleanup:
    pop           ; (locals cleanup)
    pop
    ret
   
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
   
.extern myputs

 double:
   push #2
   push [%ab]
   push #1
   iadd
   imul
   push #2
   isub
   pop %rv
   ret
    
.entry init
str:
    .data "Hello, World !\n"
    
init:
    push str
    push %hb
    dmr
    call cststr
    pop
    pop

    push #8
    call double
    dmo %rv
    
    ; go to user main
    jmp main
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

factorial:
factorial-prelude:
    push #1 ; r ~ [%r0-1]
    mov %r0, %sp
factorial-while0-cond:
    push [%ab] ; n > 1
    push #1
    icmp
    jg factorial-while0-body
    jmp factorial-while0-end
factorial-while0-body:
    push [%r0-1] ; r = r * n
    push [%ab]
    imul
    pop [%r0-1]
    push [%ab] ; n = n - 1
    push #1
    isub
    pop [%ab]
    jmp factorial-while0-cond
factorial-while0-end:

    mov %rv, [%r0-1]
    jmp factorial-cleanup

factorial-cleanup:
    pop
    ret

main:
    push %hb
    dive puts
    pop
    
    push #f10
    push #f50
    dive atan2
    pop
    pop
    
    push %rv
    dive putf
    pop

    push #6
    call factorial
    dmo %rv
    
    halt
