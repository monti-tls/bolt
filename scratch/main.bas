;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Load a string from program memory to heap ;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
strload:
    push #0 ; i = 0
    mov %r0, %sp

again$2:
    push [%ab-1]     ; push s[base+i]
    push [%r0-1]
    uadd
    load
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

    jne again$2    ; loop if not at end

    pop            ; pop off i
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.entry init
.extern println
.extern strlen
.extern strcmp
.extern fsqrt
.extern fabs
.extern printi

nl:
    .data ""

init:
    ; init "nl" field
    push nl
    push %hb
    call strload
    pop
    pop
    
    ; go to user main
    jmp main
    
main:
    push #123
    call printi
    pop
    
    push %hb
    call println
    pop
    
    halt
