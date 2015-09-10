/* This file is part of bolt.
 * 
 * Copyright (c) 2015, Alexandre Monti
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

#include "bolt/vm_core.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace bolt { namespace vm
{
    /**************************************/
    /*** Private implementation section ***/
    /**************************************/
    
    //! Dump an hex value to a text stream.
    //! If print_casts == true, it also dumps the value as decimal signed integer and 
    //!   floating numerics.
    void core_dump_value(std::ostream& os, uint32_t value, bool print_casts)
    {
        os << std::hex << std::setw(8) << std::setfill('0');
        os << value;
        os << std::dec;
        
        if (print_casts)
        {
            union {
                uint32_t as_uint32;
                int32_t as_int32;
                float as_float;
            };
            as_uint32 = value;
            
            os << " (I " << as_int32 << ")";
            os << " (F " << as_float << ")";
        }
    }
    
    //! Fetch a word from the module's program memory.
    static uint32_t* fetch_word(core& vco)
    {
        if (vco.registers[REG_CODE_PC] >= vco.segments[vco.registers[REG_CODE_SEG]]->size)
            throw std::runtime_error("vm::fetch_word: PC out of bounds");
        
        return vco.segments[vco.registers[REG_CODE_SEG]]->buffer + vco.registers[REG_CODE_PC]++;
    }
    
    //! Access the module's memory at the given address.
    static uint32_t* mem_access(core& vco, uint32_t addr)
    {
        if (addr > vco.stack_size + vco.heap_size)
            throw std::runtime_error("vm::mem_access: address out of bounds");
        
        return vco.stack + addr;
    }
    
    //! Push a value onto the module's stack.
    static void stack_push(core& vco, uint32_t value)
    {
        if (vco.registers[REG_CODE_SP] >= vco.stack_size)
            throw std::runtime_error("vm::stack_push: stack overflow :(");
        
        vco.stack[vco.registers[REG_CODE_SP]++] = value;
    }
    
    //! Pop a value from the module's stack.
    static uint32_t stack_pop(core& vco)
    {
        if (vco.registers[REG_CODE_SP] == 0)
            throw std::runtime_error("vm::stack_pop: stack underflow :(");
            
        return vco.stack[--vco.registers[REG_CODE_SP]];
    }
    
    //! Resolve an operand based on its code, value and indirection bit.
    //! Resolving imply following eventual indirections.
    //! An operand can have the following forms :
    //!   reg:          register value
    //!   #imm:         immediate value
    //!   [reg]:        register addressing
    //!   [#imm]:       immediate addressing
    //!   [reg+#off]:  register addressing plus immediate offset
    //!   [#imm+#off]: immediate addressing plus immediate offset (useless but allowed)
    //! Note that immediate offsets are always signed.
    //! All of them (including immediate values) are writable.
    //! Writing to an immediate operand will modify its value in the program memory.
    //! The offset bit is ignored if the indirection bit is not set.
    static uint32_t* resolve_operand(core& vco, uint32_t code, uint32_t val, bool ind, bool off)
    {
        switch (code)
        {
            case OP_CODE_NONE:
                return 0;
                
            case OP_CODE_REG:
            {
                if (val >= REG_COUNT)
                    return 0;
                
                // Get the register address
                uint32_t* addr = vco.registers + val;
                // Follow indirection if needed
                if (ind)
                {
                    // Add immediate offset if needed
                    if (off)
                        return mem_access(vco, *addr + *((int32_t*) fetch_word(vco)));
                    return mem_access(vco, *addr);
                }
                return addr;
            }
                
            case OP_CODE_IMM:
            {
                // Get the immediate operand address
                uint32_t* addr = fetch_word(vco);
                // Follow indirection if needed
                if (ind)
                {
                    // Add immediate offset if needed
                    if (off)
                        return mem_access(vco, *addr + *((int32_t*) fetch_word(vco)));
                    return mem_access(vco, *addr);
                }
                return addr;
            }
                
            default:
                throw std::logic_error("vm::decode_operand: invalid operand code");
        }
    }
    
    //! Decode the A operand.
    static uint32_t* decode_A(core& vco)
    {
        uint32_t instr = vco.registers[REG_CODE_IR];
        
        // Read in operand code
        uint32_t code = (instr & OP_A_CODE) >> OP_A_CODE_SHIFT;
        // Read operand value
        uint32_t val = (instr & OP_A_VAL) >> OP_A_VAL_SHIFT;
        // Read indirection bit
        bool ind = instr & OP_A_IND;
        // Read offset bit
        bool off = instr & OP_A_OFF;
        
        return resolve_operand(vco, code, val, ind, off);
    }
    
    //! Decode the B operand.
    static uint32_t* decode_B(core& vco)
    {
        uint32_t instr = vco.registers[REG_CODE_IR];
        
        // Read in operand code
        uint32_t code = (instr & OP_B_CODE) >> OP_B_CODE_SHIFT;
        // Read operand value
        uint32_t val = (instr & OP_B_VAL) >> OP_B_VAL_SHIFT;
        // Read indirection bit
        bool ind = instr & OP_B_IND;
        // Read offset bit
        bool off = instr & OP_B_OFF;
        
        return resolve_operand(vco, code, val, ind, off);
    }
    
    //! Fetch the next instruction form the module's program memory.
    static uint32_t fetch(core& vco)
    {
        // Instructions are single-word
        return (vco.registers[REG_CODE_IR] = *fetch_word(vco));
    }
    
    //! Execute an instruction from the SYS group.
    static void execute_sys(core& vco, uint32_t icode)
    {
        switch (icode)
        {
            case I_CODE_HALT:
                vco.registers[REG_CODE_PSR] |= PSR_FLAG_HALT;
                break;
                
            case I_CODE_RST:
                core_reset(vco);
                break;
                
            case I_CODE_DMS:
                core_stack_dump(vco);
                break;
                
            case I_CODE_DMR:
                core_register_dump(vco);
                break;
                
            case I_CODE_DMO:
            {
                uint32_t* a = decode_A(vco);
                if (!a)
                    throw std::logic_error("vm::execute_sys: DMO expects an operand");
                core_dump_value(std::cout, *a, true);
                break;
            }
                
            default:
                throw std::logic_error("vm::execute_sys: invalid instruction code");
        }
    }
    
    //! Execute an instruction from the MEM group.
    static void execute_mem(core& vco, uint32_t icode)
    {
        uint32_t* a = decode_A(vco);
        uint32_t* b = decode_B(vco);
        
        switch (icode)
        {
            case I_CODE_PUSH:
                if (!a)
                    throw std::logic_error("vm::execute_mem: expected an operand in PUSH");
                stack_push(vco, *a);
                break;
                
            case I_CODE_POP:
            {
                // We allow POP's without operands so check
                //   before dereferencing A !
                uint32_t value = stack_pop(vco);
                if (a)
                    *a = value;
                break;
            }
            
            case I_CODE_DUP:
            {
                uint32_t top = stack_pop(vco);
                stack_push(vco, top);
                stack_push(vco, top);
                break;
            }
                
            case I_CODE_MOV:
                if (!a || !b)
                    throw std::logic_error("vm::execute_mem: expected two operands in MOV");
                *a = *b;
                break;
                
            case I_CODE_LOAD:
            {
                uint32_t addr = stack_pop(vco);
                stack_push(vco, *mem_access(vco, addr));
                break;
            }
                
            case I_CODE_STOR:
            {
                uint32_t addr = stack_pop(vco);
                *mem_access(vco, addr) = stack_pop(vco);
                break;
            }
                
            case I_CODE_CST:
            {
                uint32_t addr;
                
                if (a)
                    addr = *a;
                else
                    addr = stack_pop(vco);
                
                uint32_t seg = vco.registers[REG_CODE_SEG];
                if (b)
                    seg = *b;
                
                if (seg >= vco.segments_size)
                    throw std::logic_error("vm::execute_mem: bad segment in CST");
                if (addr >= vco.segments[seg]->size)
                    throw std::logic_error("vm::execute_mem: bad program address in CST");
                
                stack_push(vco, vco.segments[seg]->buffer[addr]);
                break;
            }
                
            default:
                throw std::logic_error("vm::execute_mem: invalid instruction code");
        }
    }
    
    static void execute_flow(core& vco, uint32_t icode)
    {
        // Be careful to decode the operands right now,
        //   because if A is an immediate value, we will save a bad PC,
        //   as we will save it before reading the additional word.
        // Don't dereference it now because for RET it is null.
        uint32_t* a = decode_A(vco);
        
        switch (icode)
        {
            //! Here is the calling convention's ABI.
            //! The caller is responsible for pushing the arguments
            //!   on the stack before CALLing.
            //! They must be pushed right to left.
            //! For example (if arguments are 1-word wide), the n-th argument is accessed with
            //!   [%ab-n] (n is zero-based)
            //! The caller is responsible for cleaning up the stack after the callee
            //!   has returned.
            //! The return value can be written in the special register RV.
            //! The caller must save the RV register itself if needed.
            //! 
            //! Stack frame when calling :
            //!
            //! +--------+
            //! |  ARGn  |
            //! +--------+
            //! |  ....  |
            //! +--------+
            //! |  ARG0  |
            //! +--------+ <-- %ab
            //! | R0-R9  |
            //! +--------+
            //! |  AB    |
            //! +--------+
            //! |  PSR   |
            //! +--------+
            //! |  PC    |
            //! +--------+
            //! |  SEG   |
            //! +--------+ <--- top of the stack
            //!
            case I_CODE_CALL:
            {
                if (!a)
                    throw std::logic_error("vm::execute_flow: expected at least one operand in CALL");
                
                uint32_t* b = decode_B(vco);
                                
                // Because we use post-incrementation stack addressing,
                //   SP is actually just over the top, so we must save SP-1
                //   to get the argument base address.
                uint32_t args_base = vco.registers[REG_CODE_SP] - 1;
                
                for (int i = (int) REG_CODE_R0; i <= (int) REG_CODE_R9; ++i)
                    stack_push(vco, vco.registers[i]);
                stack_push(vco, vco.registers[REG_CODE_AB]);
                stack_push(vco, vco.registers[REG_CODE_PSR]);
                stack_push(vco, vco.registers[REG_CODE_PC]);
                stack_push(vco, vco.registers[REG_CODE_SEG]);
                
                vco.registers[REG_CODE_AB] = args_base;
                
                // Long call case (must change segment)
                // Two operands, A (segment) and B (offset)
                if (b)
                {
                    if (*a >= vco.segments_size)
                        throw std::logic_error("vm::execute_flow: invalid segment address in long CALL");
                    
                    vco.registers[REG_CODE_SEG] = *a;
                    vco.registers[REG_CODE_PC] = *b;
                }
                // Normal call, just one operand A
                else
                {
                    vco.registers[REG_CODE_PC] = *a;
                }
                break;
            }
            
            case I_CODE_DIVE:
            {
                if (!a)
                    throw std::logic_error("vm::execute_flow: expected at least one operand in DIVE");
                if (*a > vco.hatches_size)
                    throw std::logic_error("vm::execute_flow: invalid hatch address in DIVE");
                
                vco.hatches[*a]->entry(vco);
                
                break;
            }
                
            case I_CODE_RET:
                vco.registers[REG_CODE_SEG] = stack_pop(vco);
                vco.registers[REG_CODE_PC] = stack_pop(vco);
                vco.registers[REG_CODE_PSR] = stack_pop(vco);
                vco.registers[REG_CODE_AB] = stack_pop(vco);
                for (int i = (int) REG_CODE_R9; i >= (int) REG_CODE_R0; --i)
                    vco.registers[i] = stack_pop(vco);
                break;
                
            if (!a)
                throw std::logic_error("vm::execute_flow: expected an operand in jump instruction");
                
            case I_CODE_JMP:
                vco.registers[REG_CODE_PC] = *a;
                break;
                
            case I_CODE_JZ:
            case I_CODE_JE:
                if (vco.registers[REG_CODE_PSR] & PSR_FLAG_Z)
                    vco.registers[REG_CODE_PC] = *a;
                goto psr_clr;
                
            case I_CODE_JNZ:
            case I_CODE_JNE:
                if (!(vco.registers[REG_CODE_PSR] & PSR_FLAG_Z))
                    vco.registers[REG_CODE_PC] = *a;
                goto psr_clr;
                
            case I_CODE_JL:
                if (vco.registers[REG_CODE_PSR] & PSR_FLAG_N)
                    vco.registers[REG_CODE_PC] = *a;
                goto psr_clr;
                
            case I_CODE_JLE:
                if (vco.registers[REG_CODE_PSR] & PSR_FLAG_N ||
                    vco.registers[REG_CODE_PSR] & PSR_FLAG_Z)
                    vco.registers[REG_CODE_PC] = *a;
                goto psr_clr;
                
            case I_CODE_JG:
                if (!(vco.registers[REG_CODE_PSR] & PSR_FLAG_N))
                    vco.registers[REG_CODE_PC] = *a;
                goto psr_clr;
                
            case I_CODE_JGE:
                if (!(vco.registers[REG_CODE_PSR] & PSR_FLAG_N) ||
                    vco.registers[REG_CODE_PSR] & PSR_FLAG_Z)
                    vco.registers[REG_CODE_PC] = *a;
                goto psr_clr;
                
            psr_clr:
                vco.registers[REG_CODE_PSR] &= PSR_FLAG_CLR;
                break;
                
            default:
                throw std::logic_error("vm::execute_flow: invalid instruction code");
        }
    }
    
    //! Execute an instruction from the ARITH group.
    static void execute_arith(core& vco, uint32_t icode)
    {
        //! We use unions here to avoid warning about type-punned pointers.
        union {
            uint32_t u_rhs;
            int32_t i_rhs;
            float f_rhs;
        };
        
        union {
            uint32_t u_lhs;
            int32_t i_lhs;
            float f_lhs;
        };
        
        union {
            uint32_t u_ret;
            int32_t i_ret;
            float f_ret;
        };
        
        u_rhs = stack_pop(vco);
        u_lhs = stack_pop(vco);
        
        switch (icode)
        {
            case I_CODE_UADD:
                stack_push(vco, u_lhs + u_rhs);
                break;
                
            case I_CODE_USUB:
                stack_push(vco, u_lhs - u_rhs);
                break;
                
            case I_CODE_UMUL:
                stack_push(vco, u_lhs * u_rhs);
                break;
                
            case I_CODE_UDIV:
                stack_push(vco, u_lhs / u_rhs);
                break;
                
            case I_CODE_UAND:
                stack_push(vco, u_lhs & u_rhs);
                break;
                
            case I_CODE_UOR:
                stack_push(vco, u_lhs | u_rhs);
                break;
                
            case I_CODE_UXOR:
                stack_push(vco, u_lhs ^ u_rhs);
                break;
            
            case I_CODE_UCMP:
                if (u_lhs < u_rhs)
                    vco.registers[REG_CODE_PSR] |= PSR_FLAG_N;
                if (u_lhs == u_rhs)
                    vco.registers[REG_CODE_PSR] |= PSR_FLAG_Z;
                break;
                
            case I_CODE_IADD:
                i_ret = i_lhs + i_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_ISUB:
                i_ret = i_lhs - i_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_IMUL:
                i_ret = i_lhs * i_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_IDIV:
                i_ret = i_lhs / i_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_ICMP:
                if (i_lhs < i_rhs)
                    vco.registers[REG_CODE_PSR] |= PSR_FLAG_N;
                if (i_lhs == i_rhs)
                    vco.registers[REG_CODE_PSR] |= PSR_FLAG_Z;
                break;
                
            case I_CODE_FADD:
                f_ret = f_lhs + f_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_FSUB:
                f_ret = f_lhs - f_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_FMUL:
                f_ret = f_lhs * f_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_FDIV:
                f_ret = f_lhs / f_rhs;
                stack_push(vco, u_ret);
                break;
                
            case I_CODE_FCMP:
                if (f_lhs < f_rhs)
                    vco.registers[REG_CODE_PSR] |= PSR_FLAG_N;
                if (f_lhs == f_rhs)
                    vco.registers[REG_CODE_PSR] |= PSR_FLAG_Z;
                break;
                
            default:
                throw std::logic_error("vm::execute_arith: invalid instruction code");
        }
    }
    
    //! Execute the next instruction.
    static void execute(core& vco)
    {   
        uint32_t instr = fetch(vco);
        
        uint32_t icode = (instr & I_CODE_MASK) >> I_CODE_SHIFT;
        uint32_t igroup = (icode & I_GROUP_MASK) >> I_GROUP_SHIFT;
        
        switch (igroup)
        {
            case I_GROUP_SYS:
                execute_sys(vco, icode);
                break;
                
            case I_GROUP_MEM:
                execute_mem(vco, icode);
                break;
                
            case I_GROUP_FLOW:
                execute_flow(vco, icode);
                break;
                
            case I_GROUP_ARITH:
                execute_arith(vco, icode);
                break;
            
            default:
                throw std::logic_error("vm::execute: invalid instruction group");
        }
    }
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    core core_create(uint32_t stack_size, uint32_t heap_size, uint32_t segments_size, uint32_t hatches_size)
    {
        core vco;
        vco.stack_size = stack_size;
        vco.heap_size = heap_size;
        if (vco.stack_size)
            vco.stack = new uint32_t[stack_size + heap_size];
        else
            vco.stack = 0;
        
        vco.segments_size = segments_size;
        if (vco.segments_size)
            vco.segments = new segment*[segments_size];
        else
            vco.segments = 0;
        
        vco.hatches_size = hatches_size;
        if (vco.hatches_size)
            vco.hatches = new hatch*[hatches_size];
        else
            vco.hatches = 0;
        
        return vco;
    }
    
    void core_free_segments(core& vco)
    {
        for (uint32_t i = 0; i < vco.segments_size; ++i)
        {
            if (vco.segments[i] && vco.segments[i]->buffer)
                delete[] vco.segments[i]->buffer;
            delete vco.segments[i];
        }
    }
    
    void core_free_hatches(core& vco)
    {
        for (uint32_t i = 0; i < vco.hatches_size; ++i)
            delete vco.hatches[i];
    }
    
    void core_free(core& vco)
    {
        if (vco.hatches)
            delete[] vco.hatches;
        vco.hatches = 0;
        vco.hatches_size = 0;
        
        if (vco.segments)
            delete[] vco.segments;
        vco.segments = 0;
        vco.segments_size = 0;
        
        if (vco.stack)
            delete[] vco.stack;
        vco.stack = 0;
        vco.stack_size = 0;
        vco.heap_size = 0;
    }
    
    void core_reset(core& vco)
    {
        vco.registers[REG_CODE_SEG] = vco.base;
        vco.registers[REG_CODE_PC] = vco.segments[vco.registers[REG_CODE_SEG]]->entry;
        vco.registers[REG_CODE_SP] = 0;
        vco.registers[REG_CODE_PSR] = PSR_FLAG_NONE;
        vco.registers[REG_CODE_HB] = vco.stack_size;
    }
    
    void core_run(core& vco)
    {
        while (vco.registers[REG_CODE_PC] < vco.segments[vco.registers[REG_CODE_SEG]]->size &&
               !(vco.registers[REG_CODE_PSR] & PSR_FLAG_HALT))
        {
            execute(vco);
        }
        
        vco.registers[REG_CODE_PSR] |= PSR_FLAG_HALT;
    }
    
    void core_register_dump(core& vco, std::ostream& os)
    {
        os << "--- Register dump ---" << std::endl;
        
        for (uint32_t i = REG_CODE_R0; i <= REG_CODE_R9; ++i)
        {
            os << "R" << i - REG_CODE_R0 << ":  ";
            core_dump_value(os, vco.registers[i], true);
            os << std::endl;
        }
        
        os << "PC:  ";
        core_dump_value(os, vco.registers[REG_CODE_PC], false);
        os << std::endl;
        
        os << "SEG: ";
        core_dump_value(os, vco.registers[REG_CODE_SEG], false);
        os << std::endl;
        
        os << "SP:  ";
        core_dump_value(os, vco.registers[REG_CODE_SP], false);
        os << std::endl;
        
        os << "PSR: ";
        core_dump_value(os, vco.registers[REG_CODE_PSR], false);
        if (vco.registers[REG_CODE_PSR] & PSR_FLAG_HALT)
            os << " HALT";
        if (vco.registers[REG_CODE_PSR] & PSR_FLAG_N)
            os << " N";
        if (vco.registers[REG_CODE_PSR] & PSR_FLAG_Z)
            os << " Z";
        os << std::endl;
        
        os << "RV:  ";
        core_dump_value(os, vco.registers[REG_CODE_RV], true);
        os << std::endl;
        
        os << "AB:  ";
        core_dump_value(os, vco.registers[REG_CODE_AB], false);
        os << std::endl;
        
        os << "HB:  ";
        core_dump_value(os, vco.registers[REG_CODE_HB], false);
        os << std::endl;
        
        os << "---------------------" << std::endl;
    }
    
    void core_stack_dump(core& vco, std::ostream& os)
    {
        os << "---- Stack dump ----" << std::endl;
        if (vco.registers[REG_CODE_SP] >= vco.stack_size)
            os << "** Corrupted SP **" << std::endl;
        else
        {
            for (int i = (int) vco.registers[REG_CODE_SP]; i >= 0 ; --i)
            {
                os << "-" << std::hex << std::setw(8) << std::setfill('0') << (vco.registers[REG_CODE_SP] - i) << ": ";
                core_dump_value(os, vco.stack[i], true);
                os << std::endl;
            }
        }
        os << "--------------------" << std::endl;
    }
} }
