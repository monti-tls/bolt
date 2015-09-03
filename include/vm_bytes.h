/* This file is part of bolt.
 * 
 * bolt is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * bolt is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with bolt.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef BOLT_VM_BYTES_H
#define BOLT_VM_BYTES_H

#include <common.h>

namespace vm
{
    //!FIXME: We must use "enum : uint32_t" that is, a C++11's feature :(
    //!         because some constants do not fit in a signed int, and GCC messes
    //!         them up when using instruction crafting macros.
    //!       I hope this is a temporary issue !
    
    //! The bolt instruction set is rather simple.
    //! An instruction is made up of an operation code,
    //!   and zero, one or two operands.
    //! The bolt VM is 32-bits in nature and so words (and instructions) are 32-bits wide.
    //! The operands can have the following form :
    //!   register (reg)
    //!   immediate value (#imm)
    //! Each immediate value is encoded as a supplementary word after the instruction code,
    //!   and register numbers are encoded in the instruction code itself.
    //! One can use indirections in the operand, meaning that the value is interpreted as
    //!   a memory address, we note :
    //!   [reg] or [#imm]
    //! An immediate offset can be added (this sets the off bit) :
    //!   [reg+#imm]
    //!
    //! Here are the bit fields in the 32-bit instruction word :
    //!
    //! +32           +24             +16              +8              +0
    //!   7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0 7 6 5 4 3 2 1 0
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!  |     |             |I|O| C |             |I|O| C |             |
    //!  |GROUP|    CODE     |N|F| O |    VALUE    |N|F| O |    VALUE    |
    //!  |     |             |D|F| D |             |D|F| D |             |
    //!  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    //!
    //! Instructions are splitted into groups :
    //!   SYS:   the system group:
    //!            HALT: halt the virtual core
    //!            RST:  reset the virtual core
    //!            DMS:  dump the current stack contents
    //!            DMR:  dump the current register contents
    //!
    //!   MEM:   the memory group:
    //!            PUSH <A>:      push A to the stack
    //!            POP  <A>:      pop A from the stack
    //!            MOV  <A>, <B>: move B's value to A
    //!
    //!   FLOW:  the program flow control group:
    //!            CALL <A>:      call the function starting at A's address
    //!            CALL <A>, <B>: call the function at B (within segment A)
    //!            DIVE <A>:      dive in the hatch A (that is, call a host function)
    //!            RET:           return from the current function
    //!            JMP  <A>:      go to A's address
    //!            JZ   <A>:      jump to A if the Z flag is set in PSR
    //!            JNZ  <A>:      jump to A if the Z flag is not set in PSR
    //!
    //!            JE, JNE, JL, JLE, JG, JGE behaves similarly
    //!
    //!   ARITH: the arithmetics group:
    //!            UADD: pops off two unsigned integers from the stacks and pushes their addition
    //!            USUB: " " subtraction
    //!            UMUL: " " multiplication
    //!            UDIV: " " division
    //!            UCMP: compared the two popped values (as unsigneds) and updates PSR accordingly
    //!
    //!            IADD, ISUB, ... behaves similarly for signed integers
    //!            FADD, FSUB, ...    "        "      "  single-precision floatings
    
    //! To get the operand code from an encoded instruction, do :
    //!   opcode = (instr & OP_x_CODE) >> OP_x_CODE_SHIFT
    //!   Then compare with OP_CODE_*.
    //!
    //! To read the indirection bit, do :
    //!   bit = instr & OP_x_IND.
    //!   Then compare with 0.
    //!
    //! To read the offset bit, do :
    //!   bit = instr & OP_x_OFF.
    //!   Then compare with 0.
    //!
    //! To read the value, do :
    //!   val = (instr & OP_x_VAL) >> OP_x_SHIFT
    enum : uint32_t
    {
        //! Operand code values.
        OP_CODE_NONE    = 0x0,
        OP_CODE_REG     = 0x1,
        OP_CODE_IMM     = 0x2,
        
        //! Operand B code & indirection and offset bit masks and shifts.
        OP_A_IND        = 0x00200000, // bit 21
        OP_A_OFF        = 0x00100000, // bit 20
        OP_A_CODE       = 0x000C0000, // bits 18-19
        OP_A_CODE_SHIFT = 0x12, // 18 bits
        OP_A_VAL        = 0x0003F800, // bits 11-17
        OP_A_VAL_SHIFT  = 0x0B, // 11 bits
        
        //! Operand B code & indirection and offset bit masks and shifts.
        OP_B_IND        = 0x00000400, // bit 10
        OP_B_OFF        = 0x00000200, // bit 9
        OP_B_CODE       = 0x00000180, // bits 7-8
        OP_B_CODE_SHIFT = 0x07, // 7 bits
        OP_B_VAL        = 0x0000007F, // bits 0-6
        OP_B_VAL_SHIFT  = 0x00 // 0 bits
    };
    
    //! To retrieve the instruction code from an encoded instruction, do :
    //!   icode = (instr & I_CODE_MASK) >> I_CODE_SHIFT
    //! Then compare with I_CODE_*.
    enum : uint32_t
    {
        //! Instruction code mask.
        I_CODE_MASK = 0xFFC00000, // bits 22-31
        
        //! Instruction code shift.
        I_CODE_SHIFT = 0x16 // 22
    };
    
    //! To get the group for an instruction, take the instruction code (see above), then do :
    //!   igroup = (icode & I_GROUP_MASK) >> I_GROUP_SHIFT;
    //!   Then compare with I_GROUP_*.
    enum : uint32_t
    {
        //! Instruction group values.
        I_GROUP_SYS   = 0x01,
        I_GROUP_MEM   = 0x02,
        I_GROUP_FLOW  = 0x03,
        I_GROUP_ARITH = 0x04,
        
        //! Instruction group mask.
        I_GROUP_MASK  = 0x0380,
        //! Instruction group shift.
        I_GROUP_SHIFT = 0x07 // 7 bits
    };
    
    //! This macro defines the behavior of the definitions in
    //!   iset.inc, here we discard the last two arguments,
    //!   keeping the first three to cratf our handy icode defines.
    #define DECL_INSTR(group, name, offset, a, b) \
        I_CODE_ ## name = ((I_GROUP_ ## group << I_GROUP_SHIFT) + (offset)),
    
    enum : uint32_t
    {
        #include "iset.inc"
    };
    
    #undef DECL_INSTR
}

#endif // BOLT_VM_BYTES_H
