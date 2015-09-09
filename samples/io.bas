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
;; io.bas
;;

;; This file defines some input/output routines in Bolt assembly.


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; puts - Print a string ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
; void puts(int* str)
; {
;     int i = 0;
;     int c = 0;
;     do
;     {
;         c = *(str + i);
;         putc(c);
;         i = i+1;
;     } while (c != 0);
; }

.global myputs
myputs:
    push #0       ; i ~ [%r0-2]
    push #0       ; c ~ [%r0-1]
    mov %r0, %sp
    
puts-loop:
    push [%ab]    ; c = *(str + i)
    push [%r0-2]
    uadd
    load
    pop [%r0-1]
    
    push [%r0-1]  ; putc(c)
    dive putc
    pop
    
    push [%r0-2]  ; i = i+1
    push #1
    uadd
    pop [%r0-2]
    
    push [%r0-1]  ; c != 0 ?
    push #0
    ucmp
    jne puts-loop ; do-while
    
    pop           ; clean up locals
    pop
    ret           ; return

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; puti - Print an integer ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; void puti(int x)
; {
;     int x = ?; n = 0;
;     if (x < 0)
;     {
;         putc('-')
;         x = -x;
;     }
;     do
;     {
;         push(x - (x / 10) * 10);
;         n = n + 1;
;         x = x / 10;
;     } while (x != 0);
;     while (n != 0)
;     {
;         pop(d);
;         putc(d);
;         n = n - 1;
;     }
; }

puti-digits:
    .data "0123456789-"
    
.global puti
puti:
    push [%ab]         ; x ~ [%r0-2]
    push #0            ; n ~ [%r0-1]
    mov %r0, %sp

    push [%r0-2]
    push #0
    icmp
    jge puti-loop
    push puti-digits
    push #10
    uadd
    cst
    dive putc
    pop
    push [%r0-2]
    push #-1
    imul
    pop [%r0-2]
    
puti-loop:
    push [%r0-2]      ; push(d = x - (x / 10) * 10)
    push [%r0-2]
    push #10
    idiv
    push #10
    imul
    isub
    
    push [%r0-1]      ; n = n+1
    push #1
    uadd
    pop [%r0-1]
    
    push [%r0-2]      ; x = x / 10
    push #10
    idiv
    pop [%r0-2]
    
    push [%r0-2]      ; do {} while (x != 0)
    push #0
    icmp
    jne puti-loop

puti-loop2:
    push puti-digits  ; putch(digits + d), implicit pop(d)
    uadd
    cst
    dive putc
    pop

    push [%r0-1]      ; n = n-1
    push #1
    usub
    pop [%r0-1]
    
    push [%r0-1]      ; while (n != 0)
    push #0
    ucmp
    jne puti-loop2
    
puti-exit:
    pop               ; clean up locals
    pop
    ret
