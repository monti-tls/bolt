;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Recursive factorial toy ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.global fac-r
fac-r:
    push [%ab]
    push #2u
    ucmp              ; compare n with 2
    jg rec
    mov %rv, [%ab]    ; if less of equal, return n
    ret
rec:                  ; else
    push [%ab]
    push #1u
    usub              ; there is n-1 on stack
    call fac-r        ; recursively call fac(n-1)
    pop               ; pop off the argument
    push %rv          ; push return value
    push [%ab]        ; push n
    umul              ; push n*%rv
    pop %rv           ; pop in %rv
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Iterative factorial toy ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
.global fac-it
fac-it:
    ; create room for temp var
    push #1u
    
while:
    push [%ab]
    push #1u
    ucmp
    jle end    ; if n <= 1, finish
    
    push [%ab] ; temp = temp*n
    umul
    
    push [%ab]
    push #1u
    usub
    pop [%ab]  ; n = n-1
    
    jmp while
    
end:
    pop %rv
    ret
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Get size of null-terminated string ;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; int i = 0;
; while (*(str + i) != 0)
;     i = i + 1;
; return i;
    
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
;;;; int strcmp(char* a, char* b) ;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; int i = 0;
; while (*(str1 + i) == *(str2 + i))
;     i = i + 1;
; return *(str1 + i) - *(str2 + i);

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Print a string ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
; int i = 0;
; int c = 0;
; do
; {
;     c = *(str + i);
;     putch(c);
;     i = i+1;
; } while (c != 0);

.global print
print:
    push #0      ; i ~ [%r0-2]
    push #0      ; c ~ [%r0-1]
    mov %r0, %sp
    
print-loop:
    push [%ab]   ; c = *(str + i)
    push [%r0-2]
    uadd
    load
    pop [%r0-1]
    
    push [%r0-1] ; putch(c)
    dive putch
    pop
    
    push [%r0-2] ; i = i+1
    push #1
    uadd
    pop [%r0-2]
    
    push [%r0-1] ; c != 0 ?
    push #0
    ucmp
    jne print-loop ; do-while
    
    pop          ; clean up locals
    pop
    ret          ; return


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Print a string + new line ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

println-endl:
    .data "\n"

.global println
println:
    push [%ab]        ; print the string
    call print
    pop
    cst println-endl  ; print new line
    dive putch
    pop
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Print an integer ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; int x = ?; n = 0;
; if (x < 0)
; {
;     print '-'
;     x = -x;
; }
; do
; {
;     push(x - (x / 10) * 10);
;     n = n + 1;
;     x = x / 10;
; } while (x != 0);
; while (n != 0)
; {
;     pop(d);
;     putch(d);
;     n = n - 1;
; }

printi-digits:
    .data "0123456789-"
    
.global printi
printi:
    push [%ab]         ; x ~ [%r0-2]
    push #0            ; n ~ [%r0-1]
    mov %r0, %sp

    push [%r0-2]
    push #0
    icmp
    jge printi-loop
    push printi-digits
    push #10
    uadd
    cst
    dive putch
    pop
    push [%r0-2]
    push #-1
    imul
    pop [%r0-2]
    
printi-loop:
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
    jne printi-loop

printi-loop2:
    push printi-digits ; putch(digits + d), implicit pop(d)
    uadd
    cst
    dive putch
    pop

    push [%r0-1]       ; n = n-1
    push #1
    usub
    pop [%r0-1]
    
    push [%r0-1]       ; while (n != 0)
    push #0
    ucmp
    jne printi-loop2
    
printi-exit:
    pop               ; clean up locals
    pop
    ret
    
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; float fabs(float x) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; return x > 0 ? x : -x; 
  
.global fabs
fabs:
    push [%ab]
    dup
    push #f0
    fcmp
    jg fabs-end
    push #f-1
    fmul
fabs-end:
    pop %rv
    ret
  
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; float fsqrt(float x) ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
  
; if (x <= 0)
;     return x;
; float y = x;
; float yn = 0;
; float e = 0;
; do
; {
;     yn = 0.5 * (y + x/y);
;     e = fabs(yn - y);
;     y = yn;
; } while (e < epsilon);
; return y;

.global fsqrt
fsqrt:
    push #f0 ; y ~ [%r0-3]
    push #f0 ; yn ~ [%r0-2]
    push #f0 ; e ~ [%r0-1]
    mov %r0, %sp
    
    push [%ab]           ; if (x <= 0)
    push #f0
    fcmp
    jg fsqrt-work
    mov %rv, [%ab]       ; return x
    jmp fsqrt-exit
    
fsqrt-work:
    mov [%r0-3], [%ab]   ; y = estimator = x
    
fsqrt-loop:
    push #f0.5           ; yn = 0.5 * (y + x/y)
    push [%r0-3]
    push [%ab]
    push [%r0-3]
    fdiv
    fadd
    fmul
    pop [%r0-2]
    
    push [%r0-2]         ; e = fabs(yn - y)
    push [%r0-3]
    fsub
    call fabs
    pop
    mov [%r0-1], %rv
    
    mov [%r0-3], [%r0-2] ; y = yn
    
    push [%r0-1]         ; do {} while (e < epsilon)
    push #f0.001
    fcmp
    jge fsqrt-loop
    
    mov %rv, [%r0-3]     ; return y
    
fsqrt-exit:
    pop                  ; clean up locals
    pop
    pop
    ret
