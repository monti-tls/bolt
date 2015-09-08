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
.extern fsqrt
.extern fabs
.extern printi

.entry init
nl:
    .data "Hello, World !"
    
nl2:
    .data "Hello, World !"

init:
    ; init "nl" field
    push nl
    push %hb
    call strload
    pop
    pop
    
    ; init "nl2" field
    push nl2
    push %hb
    push #xF
    uadd
    call strload
    pop
    pop
    
    ; go to user main
    jmp main
    
main:
    push %hb
    call println
    pop
    
    push #123546897
    call printi
    pop
    
    halt