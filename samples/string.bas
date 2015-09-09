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

;; This file defines some string functions in Bolt assembly.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; strlen - Get length of string ;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; int strlen(int* str)
; {
;     int i = 0;
;     while (*(str + i) != 0)
;         i = i + 1;
;     return i;
; }
    
.global strlen
strlen:
    push #0          ; i ~ [%r0-1]
    mov %r0, %sp
    
strlen-loop:
    push [%ab]       ; *(str + i)
    push [%r0-1]
    uadd
    load
    
    push #0          ; while
    ucmp
    je strlen-end
    
    push [%r0-1]     ; i = i + 1
    push #1
    uadd
    pop [%r0-1]
    
    jmp strlen-loop  ; while
    
strlen-end:
    mov %rv, [%r0-1] ; return i
    
    pop              ; clean up locals
    ret
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; strcmp - Compare two strings ;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; int strcmp(int* str1, int* str2)
; {
;     int i = 0;
;     while (*(str1 + i) == *(str2 + i))
;         i = i + 1;
;     return *(str1 + i) - *(str2 + i);
; }

.global strcmp
strcmp:
    push #0        ; i ~ [%sp-1]
    mov %r0, %sp
    
strcmp-loop:
    push [%ab-1]   ; *(str1 + i)
    push [%r0-1]
    uadd
    load
    dup
    
    push [%ab]     ; *(str2 + i)
    push [%r0-1]
    uadd
    load
    
    ucmp           ; while ==
    jne strcmp-end-2
    push #0        ; test if \0 reached
    ucmp
    je strcmp-end-1
    
    push [%r0-1]   ; i = i + 1
    push #1
    uadd
    pop [%r0-1]
    
    jmp strcmp-loop
    
strcmp-end-2:
    pop
    
strcmp-end-1:
    push [%ab-1]   ; *(str1 + i)
    push [%r0-1]
    uadd
    load
    
    push [%ab]     ; *(str2 + i)
    push [%r0-1]
    uadd
    load
    
    isub           ; return -
    pop %rv
    
    pop            ; clean up locals
    ret
