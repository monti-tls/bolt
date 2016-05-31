## bolt

Bolt is a simple virtual machine mostly written in functional C++ (in a C-style fashion).
It emulated a hardware CPU core with a 32-bit instruction set, and is based on a Harvard architecture
(program and data memories are separated and uses different addressing schemes).
It is a stack-oriented machine (i.e. most operations pushes / pops data from the stack).

## Architecture

Bolt can run program units called *cores*, that contain both code, stack, heap and registers.
As said earlier, program and stack/heap memories are separated, and to allow linking of multiple programs
in a single core, the program memory is segmented. For more informations, see vm_core.h.

Assembly for the Bolt vm can be written textually and assembled in a module.
Multiple modules can then be linked together to form the final core, that can be run.

The instruction set is described in vm_bytes.h.
The processor architecture is inspired with Cortex-M cores, and uses registers for program status, stack pointer, ...

## Sample code

Here is a sample code to compute the absolute value of a number :

```asm
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
```

Here is another example comuting the suare root of a number using the Babylonian method :

```asm
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
```

For more information about the assembly syntax, see as_assembler.h.

## License

Bolt is licensed under the GNU GPL license :

```
bolt is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

bolt is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with bolt.  If not, see <http://www.gnu.org/licenses/>.
```