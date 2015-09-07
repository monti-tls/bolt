;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Recursive factorial toy ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

.global fac$r
fac$r:
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
    call fac$r        ; recursively call fac(n-1)
    pop               ; pop off the argument
    push %rv          ; push return value
    push [%ab]        ; push n
    umul              ; push n*%rv
    pop %rv           ; pop in %rv
    ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Iterative factorial toy ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
.global fac$it
fac$it:
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
;;;; Print a string from the heap ;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    
.global print
print:
    push #0 ; i = 0
    mov %r0, %sp

print$loop:
    push [%ab]    ; push s[base+i]
    push [%r0-1]
    uadd
    pop %r1
    push [%r1]
    push #0
    ucmp
    je print$stop ; stop if null char encountered
    
    push [%r1]    ; output character
    dive putch
    pop
    
    push [%r0-1]  ; i = i+1
    push #1
    uadd
    pop [%r0-1]
    
    jmp print$loop

print$stop:
    pop           ; pop off i
    ret
    
endl:
    .data "\n"

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;; Print a string from the heap with new lines ;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

println$endl:
    .data "\n"

.global println
println:
    push [%ab]        ; print the string
    call print
    pop
    load println$endl ; print endl
    dive putch
    pop
    ret
