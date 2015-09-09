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
;; math.bas
;;

;; This file defines some math functions in Bolt assembly.

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; fabs - compute the absolute value ;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; float fabs(float x)
; {
;     if (x < 0.0f)
;         return -x;
;     return x;
; }

.global fabs
fabs:
    push [%ab]
    push #f0
    fcmp
    jl fabs-l1
    jmp fabs-l2
fabs-l1:
    push [%ab]
    push #f-1
    fmul
    pop %rv
    ret
fabs-l2:
    mov %rv, [%ab]
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; fsqrt - compute the approximate square root ;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; float fsqrt(float x)
; {
;     float yn = x, y = 0.0f;
;     if (x < 0.0f)
;          return 0.0f;
;     do
;     {
;         y = yn;
;         yn = 0.5f * (y + x/y);
;     } while (fabs(yn - y) > EPSILON);
;     return y;
; }

.global fsqrt
fsqrt:
    push [%ab]           ; float yn = x
    push #f0             ; float y = 0.0f
    mov %r0, %sp         ; (locals setup)
    
    push [%ab]           ; x
    push #f0             ; < 0.0f
    fcmp
    jl fsqrt-l1          ; if {}
    jmp fsqrt-l2
fsqrt-l1:
    mov %rv, #f0         ; return 0.0f
    jmp fsqrt-cleanup    ; (return handler)
    
fsqrt-l2:

fsqrt-l3:
    mov [%r0-1], [%r0-2] ; y = yn
    
    push #f0.5           ; yn = 0.05f * (y + x/y)
    push [%r0-1]
    push [%ab]
    push [%r0-1]
    fdiv
    fadd
    fmul
    pop [%r0-2]
    
    push [%r0-2]         ; fabsf(yn - y)
    push [%r0-1]
    fsub
    call fabs
    pop
    push %rv
    
    push #f0.001         ; > EPSILON
    fcmp
    
    jg fsqrt-l3          ; do {} while
    
    mov %rv, [%r0-1]     ; return y
    jmp fsqrt-cleanup    ; (return handler)
    
fsqrt-cleanup:
    pop                  ; (locals clean)
    pop
    ret
