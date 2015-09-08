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
    
strlen$loop:
    push [%ab]       ; str + i
    push [%r0-1]
    uadd
    pop %r1
    
    push [%r1]       ; *(str + i) != 0 ?
    push #0
    ucmp
    je strlen$end    ; while
    
    push [%r0-1]     ; i = i + 1
    push #1
    uadd
    pop [%r0-1]
    
    jmp strlen$loop  ; while
    
strlen$end:
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
    
strcmp$loop:
    push [%ab-1]   ; *(str1 + i)
    push [%r0-1]
    uadd
    pop %r1
    push [%r1]
    dup
    
    push [%ab]     ; *(str2 + i)
    push [%r0-1]
    uadd
    pop %r1
    push [%r1]
    
    ucmp           ; while ==
    jne strcmp$end
    push #0        ; test if \0 reached
    ucmp
    je strcmp$end
    
    push [%r0-1]   ; i = i + 1
    push #1
    uadd
    pop [%r0-1]
    
    dmo [%r0-1]
    
    jmp strcmp$loop
    
strcmp$end:
    push [%ab-1]   ; *(str1 + i)
    push [%r0-1]
    uadd
    pop %r1
    push [%r1]
    
    push [%ab]     ; *(str2 + i)
    push [%r0-1]
    uadd
    pop %r1
    push [%r1]
    
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
    
print$loop:
    push [%ab]   ; c = *(str + i)
    push [%r0-2]
    uadd
    pop %r1
    push [%r1]
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
    jne print$loop ; do-while
    
    pop          ; clean up locals
    pop
    ret          ; return
    
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
