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

#include "vm_module.h"
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace vm
{
    /**************************************/
    /*** Private implementation section ***/
    /**************************************/
    
    //! Fetch a word from the module's program memory.
    static uint32_t* fetch_word(module& mod)
    {
        if (mod.registers[REG_CODE_PC] >= mod.segments[mod.registers[REG_CODE_SEG]]->size)
            throw std::runtime_error("vm::fetch_word: PC out of bounds");
        
        return mod.segments[mod.registers[REG_CODE_SEG]]->buffer + mod.registers[REG_CODE_PC]++;
    }
    
    //! Access the module's memory at the given address.
    static uint32_t* mem_access(module& mod, uint32_t addr)
    {
        if (addr > mod.stack_size)
            throw std::runtime_error("vm::mem_access: address out of bounds");
        
        return mod.stack + addr;
    }
    
    //! Push a value onto the module's stack.
    static void stack_push(module& mod, uint32_t value)
    {
        if (mod.registers[REG_CODE_SP] >= mod.stack_size)
            throw std::runtime_error("vm::stack_push: stack overflow :(");
        
        mod.stack[mod.registers[REG_CODE_SP]++] = value;
    }
    
    //! Pop a value from the module's stack.
    static uint32_t stack_pop(module& mod)
    {
        if (mod.registers[REG_CODE_SP] == 0)
            throw std::runtime_error("vm::stack_pop: stack underflow :(");
            
        return mod.stack[--mod.registers[REG_CODE_SP]];
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
    static uint32_t* resolve_operand(module& mod, uint32_t code, uint32_t val, bool ind, bool off)
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
                uint32_t* addr = mod.registers + val;
                // Follow indirection if needed
                if (ind)
                {
                    // Add immediate offset if needed
                    if (off)
                        return mem_access(mod, *addr + *((int32_t*) fetch_word(mod)));
                    return mem_access(mod, *addr);
                }
                return addr;
            }
                
            case OP_CODE_IMM:
            {
                // Get the immediate operand address
                uint32_t* addr = fetch_word(mod);
                // Follow indirection if needed
                if (ind)
                {
                    // Add immediate offset if needed
                    if (off)
                        return mem_access(mod, *addr + *((int32_t*) fetch_word(mod)));
                    return mem_access(mod, *addr);
                }
                return addr;
            }
                
            default:
                throw std::logic_error("vm::decode_operand: invalid operand code");
        }
    }
    
    //! Decode the A operand.
    static uint32_t* decode_A(module& mod)
    {
        uint32_t instr = mod.registers[REG_CODE_IR];
        
        // Read in operand code
        uint32_t code = (instr & OP_A_CODE) >> OP_A_CODE_SHIFT;
        // Read operand value
        uint32_t val = (instr & OP_A_VAL) >> OP_A_VAL_SHIFT;
        // Read indirection bit
        bool ind = instr & OP_A_IND;
        // Read offset bit
        bool off = instr & OP_A_OFF;
        
        return resolve_operand(mod, code, val, ind, off);
    }
    
    //! Decode the B operand.
    static uint32_t* decode_B(module& mod)
    {
        uint32_t instr = mod.registers[REG_CODE_IR];
        
        // Read in operand code
        uint32_t code = (instr & OP_B_CODE) >> OP_B_CODE_SHIFT;
        // Read operand value
        uint32_t val = (instr & OP_B_VAL) >> OP_B_VAL_SHIFT;
        // Read indirection bit
        bool ind = instr & OP_B_IND;
        // Read offset bit
        bool off = instr & OP_B_OFF;
        
        return resolve_operand(mod, code, val, ind, off);
    }
    
    //! Fetch the next instruction form the module's program memory.
    static uint32_t fetch(module& mod)
    {
        // Instructions are single-word
        return (mod.registers[REG_CODE_IR] = *fetch_word(mod));
    }
    
    //! Execute an instruction from the SYS group.
    static void execute_sys(module& mod, uint32_t icode)
    {
        switch (icode)
        {
            case I_CODE_HALT:
                mod.registers[REG_CODE_PSR] |= PSR_FLAG_HALT;
                break;
                
            case I_CODE_RST:
                module_reset(mod);
                break;
                
            case I_CODE_DMS:
                std::cout << "Stack dump :" << std::endl;
                if (mod.registers[REG_CODE_SP] >= mod.stack_size)
                {
                    std::cout << "<corrupted SP>" << std::endl;
                    break;
                }
                for (uint32_t i = 0; i < mod.registers[REG_CODE_SP]; ++i)
                {
                    std::cout << "[" << std::hex << std::setw(8) << std::setfill('0') << i << "] ";
                    std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << mod.stack[i];
                    std::cout << " (I " << std::dec << *((int32_t*) &mod.stack[i]) << ")";
                    std::cout << " (F " << *((float*) &mod.stack[i]) << ")" << std::endl;
                }
                std::cout << "------------" << std::endl;
                break;
                
            case I_CODE_DMR:
                std::cout << "Register dump :" << std::endl;
                std::cout << "PC:  0x" << std::hex << std::setw(8) << std::setfill('0') << mod.registers[REG_CODE_PC] << std::endl;
                std::cout << "SEG: 0x" << std::hex << std::setw(8) << std::setfill('0') << mod.registers[REG_CODE_SEG] << std::endl;
                std::cout << "SP:  0x" << std::hex << std::setw(8) << std::setfill('0') << mod.registers[REG_CODE_SP] << std::endl;
                std::cout << "PSR: 0x" << std::hex << std::setw(8) << std::setfill('0') << mod.registers[REG_CODE_PSR];
                if (mod.registers[REG_CODE_PSR] & PSR_FLAG_HALT)
                    std::cout << " HALT";
                if (mod.registers[REG_CODE_PSR] & PSR_FLAG_N)
                    std::cout << " N";
                if (mod.registers[REG_CODE_PSR] & PSR_FLAG_Z)
                    std::cout << " Z";
                std::cout << std::endl;
                std::cout << "RV:  ";
                std::cout << "0x" << std::hex << std::setw(8) << std::setfill('0') << mod.registers[REG_CODE_RV];
                std::cout << " (I " << std::dec << *((int32_t*) &mod.registers[REG_CODE_RV]) << ")";
                std::cout << " (F " << *((float*) &mod.registers[REG_CODE_RV]) << ")" << std::endl;
                std::cout << "AB:  0x" << std::hex << std::setw(8) << std::setfill('0') << mod.registers[REG_CODE_AB] << std::endl;
                std::cout << "---------------" << std::endl;
                break;
                
            default:
                throw std::logic_error("vm::execute_sys: invalid instruction code");
        }
    }
    
    //! Execute an instruction from the MEM group.
    static void execute_mem(module& mod, uint32_t icode)
    {
        uint32_t* a = decode_A(mod);
        uint32_t* b = decode_B(mod);
        
        switch (icode)
        {
            case I_CODE_PUSH:
                if (!a)
                    throw std::logic_error("vm::execute_mem: expected an operand in PUSH");
                stack_push(mod, *a);
                break;
                
            case I_CODE_POP:
            {
                // We allow POP's without operands so check
                //   before dereferencing A !
                uint32_t value = stack_pop(mod);
                if (a)
                    *a = value;
                break;
            }
                
            case I_CODE_MOV:
                if (!a || !b)
                    throw std::logic_error("vm::execute_mem: expected two operands in MOV");
                *a = *b;
                break;
                
            default:
                throw std::logic_error("vm::execute_mem: invalid instruction code");
        }
    }
    
    static void execute_flow(module& mod, uint32_t icode)
    {
        // Be careful to decode the operands right now,
        //   because if A is an immediate value, we will save a bad PC,
        //   as we will save it before reading the additional word.
        // Don't dereference it now because for RET it is null.
        uint32_t* a = decode_A(mod);
        
        switch (icode)
        {
            //! Here is the calling convention's ABI.
            //! The caller is responsible for pushing the arguments
            //!   on the stack before CALLing.
            //! They must be pushed left to right.
            //! They can be accessed by a [AB+#imm] operand.
            //! The caller is responsible for cleaning up the stack after the callee
            //!   has returned.
            //! The return value can be written in the special register RV.
            //! The caller must save the RV register itself if needed.
            //! 
            //! Stack frame when calling :
            //!
            //! +--------+
            //! |  ARG0  |
            //! +--------+
            //! |  ....  |
            //! +--------+
            //! |  ARGn  |
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
                
                uint32_t* b = decode_B(mod);
                                
                // Because we use post-incrementation stack addressing,
                //   SP is actually just over the top, so we must save SP-1
                //   to get the argument base address.
                uint32_t args_base = mod.registers[REG_CODE_SP] - 1;
                
                stack_push(mod, mod.registers[REG_CODE_AB]);
                stack_push(mod, mod.registers[REG_CODE_PSR]);
                stack_push(mod, mod.registers[REG_CODE_PC]);
                stack_push(mod, mod.registers[REG_CODE_SEG]);
                
                mod.registers[REG_CODE_AB] = args_base;
                
                // Long call case (must change segment)
                // Two operands, A (segment) and B (offset)
                if (b)
                {
                    if (*a >= mod.segments_size)
                        throw std::logic_error("vm::execute_flow: invalid segment address in long CALL");
                    
                    mod.registers[REG_CODE_SEG] = *a;
                    mod.registers[REG_CODE_PC] = *b;
                }
                // Normal call, just one operand A
                else
                {
                    mod.registers[REG_CODE_PC] = *a;
                }
                break;
            }
            
            case I_CODE_DIVE:
            {
                if (!a)
                    throw std::logic_error("vm::execute_flow: expected at least one operand in DIVE");
                if (*a > mod.hatches_size)
                    throw std::logic_error("vm::execute_flow: invalid hatch address in DIVE");
                
                mod.hatches[*a]->entry(mod);
                
                break;
            }
                
            case I_CODE_RET:
                mod.registers[REG_CODE_SEG] = stack_pop(mod);
                mod.registers[REG_CODE_PC] = stack_pop(mod);
                mod.registers[REG_CODE_PSR] = stack_pop(mod);
                mod.registers[REG_CODE_AB] = stack_pop(mod);
                break;
                
            if (!a)
                throw std::logic_error("vm::execute_flow: expected an operand in jump instruction");
                
            case I_CODE_JMP:
                mod.registers[REG_CODE_PC] = *a;
                break;
                
            case I_CODE_JZ:
            case I_CODE_JE:
                if (mod.registers[REG_CODE_PSR] & PSR_FLAG_Z)
                    mod.registers[REG_CODE_PC] = *a;
                break;
                
            case I_CODE_JNZ:
            case I_CODE_JNE:
                if (!(mod.registers[REG_CODE_PSR] & PSR_FLAG_Z))
                    mod.registers[REG_CODE_PC] = *a;
                break;
                
            case I_CODE_JL:
                if (mod.registers[REG_CODE_PSR] & PSR_FLAG_N)
                    mod.registers[REG_CODE_PC] = *a;
                break;
                
            case I_CODE_JLE:
                if (mod.registers[REG_CODE_PSR] & PSR_FLAG_N ||
                    mod.registers[REG_CODE_PSR] & PSR_FLAG_Z)
                    mod.registers[REG_CODE_PC] = *a;
                break;
                
            case I_CODE_JG:
                if (!(mod.registers[REG_CODE_PSR] & PSR_FLAG_N))
                    mod.registers[REG_CODE_PC] = *a;
                break;
                
            case I_CODE_JGE:
                if (!(mod.registers[REG_CODE_PSR] & PSR_FLAG_N) ||
                    mod.registers[REG_CODE_PSR] & PSR_FLAG_Z)
                    mod.registers[REG_CODE_PC] = *a;
                break;
                
            default:
                throw std::logic_error("vm::execute_flow: invalid instruction code");
        }
    }
    
    //! Execute an instruction from the ARITH group.
    static void execute_arith(module& mod, uint32_t icode)
    {
        //! Unsigned integer operands.
        uint32_t u_rhs = stack_pop(mod);
        uint32_t u_lhs = stack_pop(mod);
        
        //! Just in case, we cast here operands to signed integers.
        int32_t i_rhs = *((int32_t*) &u_rhs);
        int32_t i_lhs = *((int32_t*) &u_lhs);
        int32_t i_ret;
        
        //! Same for floating-point values.
        float f_rhs = *((float*) &u_rhs);
        float f_lhs = *((float*) &u_lhs);
        float f_ret;
        
        switch (icode)
        {
            case I_CODE_UADD:
                stack_push(mod, u_lhs + u_rhs);
                break;
                
            case I_CODE_USUB:
                stack_push(mod, u_lhs - u_rhs);
                break;
                
            case I_CODE_UMUL:
                stack_push(mod, u_lhs * u_rhs);
                break;
                
            case I_CODE_UDIV:
                stack_push(mod, u_lhs / u_rhs);
                break;
            
            case I_CODE_UCMP:
                if (u_rhs < u_lhs)
                    mod.registers[REG_CODE_PSR] |= PSR_FLAG_N;
                if (u_rhs == u_lhs)
                    mod.registers[REG_CODE_PSR] |= PSR_FLAG_Z;
                break;
                
            case I_CODE_IADD:
                i_ret = i_lhs + i_rhs;
                stack_push(mod, *((uint32_t*) &i_ret));
                break;
                
            case I_CODE_ISUB:
                i_ret = i_lhs - i_rhs;
                stack_push(mod, *((uint32_t*) &i_ret));
                break;
                
            case I_CODE_IMUL:
                i_ret = i_lhs * i_rhs;
                stack_push(mod, *((uint32_t*) &i_ret));
                break;
                
            case I_CODE_IDIV:
                i_ret = i_lhs / i_rhs;
                stack_push(mod, *((uint32_t*) &i_ret));
                break;
                
            case I_CODE_ICMP:
                if (i_rhs < i_lhs)
                    mod.registers[REG_CODE_PSR] |= PSR_FLAG_N;
                if (i_rhs == i_lhs)
                    mod.registers[REG_CODE_PSR] |= PSR_FLAG_Z;
                break;
                
            case I_CODE_FADD:
                f_ret = f_lhs + f_rhs;
                stack_push(mod, *((uint32_t*) &f_ret));
                break;
                
            case I_CODE_FSUB:
                f_ret = f_lhs - f_rhs;
                stack_push(mod, *((uint32_t*) &f_ret));
                break;
                
            case I_CODE_FMUL:
                f_ret = f_lhs * f_rhs;
                stack_push(mod, *((uint32_t*) &f_ret));
                break;
                
            case I_CODE_FDIV:
                f_ret = f_lhs / f_rhs;
                stack_push(mod, *((uint32_t*) &f_ret));
                break;
                
            case I_CODE_FCMP:
                if (f_rhs < f_lhs)
                    mod.registers[REG_CODE_PSR] |= PSR_FLAG_N;
                if (f_rhs == f_lhs)
                    mod.registers[REG_CODE_PSR] |= PSR_FLAG_Z;
                break;
                
            default:
                throw std::logic_error("vm::execute_arith: invalid instruction code");
        }
    }
    
    //! Execute the next instruction.
    static void execute(module& mod)
    {
        uint32_t instr = fetch(mod);
        
        uint32_t icode = (instr & I_CODE_MASK) >> I_CODE_SHIFT;
        uint32_t igroup = (icode & I_GROUP_MASK) >> I_GROUP_SHIFT;
        
        switch (igroup)
        {
            case I_GROUP_SYS:
                execute_sys(mod, icode);
                break;
                
            case I_GROUP_MEM:
                execute_mem(mod, icode);
                break;
                
            case I_GROUP_FLOW:
                execute_flow(mod, icode);
                break;
                
            case I_GROUP_ARITH:
                execute_arith(mod, icode);
                break;
            
            default:
                throw std::logic_error("vm::execute: invalid instruction group");
        }
    }
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    module module_create(uint32_t stack_size, uint32_t segments_size, uint32_t hatches_size)
    {
        module mod;
        mod.stack_size = stack_size;
        mod.stack = new uint32_t[stack_size];
        
        mod.segments_size = segments_size;
        mod.segments = new segment*[segments_size];
        
        mod.hatches_size = hatches_size;
        mod.hatches = new hatch*[hatches_size];
        
        return mod;
    }
    
    void module_free(module& mod)
    {
        if (mod.hatches)
            delete[] mod.hatches;
        mod.hatches = 0;
        mod.hatches_size = 0;
        
        if (mod.segments)
            delete[] mod.segments;
        mod.segments = 0;
        mod.segments_size = 0;
        
        if (mod.stack)
            delete[] mod.stack;
        mod.stack = 0;
        mod.stack_size = 0;
    }
    
    void module_reset(module& mod)
    {
        mod.registers[REG_CODE_SEG] = mod.base;
        mod.registers[REG_CODE_PC] = mod.segments[mod.registers[REG_CODE_SEG]]->entry;
        mod.registers[REG_CODE_SP] = 0;
        mod.registers[REG_CODE_PSR] = PSR_FLAG_NONE;
    }
    
    void module_run(module& mod)
    {
        while (mod.registers[REG_CODE_PC] < mod.segments[mod.registers[REG_CODE_SEG]]->size &&
               !(mod.registers[REG_CODE_PSR] & PSR_FLAG_HALT))
        {
            execute(mod);
        }
        
        mod.registers[REG_CODE_PSR] |= PSR_FLAG_HALT;
    }
}
