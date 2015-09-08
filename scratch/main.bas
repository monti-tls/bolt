;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Load a string from program memory to heap ;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
strload:
    push #0 ; i = 0
    mov %r0, %sp

again-2:
    push [%ab-1]     ; push s[base+i]
    push [%r0-1]
    uadd
    cst
    dup
    push #0
    ucmp           ; compare with '\0'

    push [%ab]   ; get output address
    push [%r0-1]
    uadd
    pop %r1
    pop %r2        ; write character
    mov [%r1], %r2

    push [%r0-1] ; i = i+1
    push #1
    uadd
    pop [%r0-1]

    jne again-2    ; loop if not at end

    pop            ; pop off i
    ret    

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.extern println
.extern strlen
.extern strcmp
;.extern fsqrt
.extern fabs
.extern printi

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
    
.entry init
nl:
    .data "Hello, World !"
    
nl2:
    .data "Hello, World !"

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
    ret
   
init:
    ; init "nl" field
    push nl
    push %hb
    call cststr
    pop
    pop
    
    ; init "nl2" field
    push nl2
    push %hb
    push #xF
    uadd
    call cststr
    pop
    pop
    
    ; go to user main
    jmp main
    
main:
    push %hb
    call println
    pop
    
    push #f64
    call fsqrt
    pop
    dmo %rv
    
    push #f-64
    call fsqrt
    pop
    dmo %rv
    
    halt
