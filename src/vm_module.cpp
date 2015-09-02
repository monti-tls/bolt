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

namespace vm
{
    //! Fetch a word from the module's program memory.
    uint32_t* fetch_word(module& mod)
    {
        // error !
        if (mod.registers[RC_PC] >= mod.programSize)
            return 0;
        
        return mod.program + mod.registers[RC_PC]++;
    }
    
    //! Access the module's memory at the given address.
    uint32_t* mem_access(module& mod, uint32_t addr)
    {
        // error !
        if (addr > mod.stackSize)
            return 0;
        
        return mod.stack + addr;
    }
    
    //! Push a value onto the module's stack.
    void stack_push(module& mod, uint32_t value)
    {
        if (mod.registers[RC_SP] >= mod.stackSize)
        {} // error !
        
        mod.stack[mod.registers[RC_SP]++] = value;
    }
    
    //! Pop a value from the module's stack.
    uint32_t stack_pop(module& mod)
    {
        if (mod.registers[RC_SP] == 0)
        {} // error !
            
        return mod.stack[--mod.registers[RC_SP]];
    }
    
    //! Decode an operand based on its code, value and indirection bit.
    uint32_t* decode_operand(module& mod, uint32_t code, uint32_t val, bool ind)
    {
        switch (code)
        {
            case OP_CODE_NONE:
                return 0;
                
            case OP_CODE_REG:
            {
                if (val >= RC_SIZE)
                    return 0;
                
                uint32_t* addr = mod.registers + val;
                if (addr && ind)
                    return mem_access(mod, *addr);
                return addr;
            }
                
            case OP_CODE_IMM:
            {
                uint32_t* addr = fetch_word(mod);
                if (addr && ind)
                    return mem_access(mod, *addr);
                return addr;
            }
                
            default:
                // error !
                return 0;
        }
    }
    
    //! Decode the A operand.
    uint32_t* decode_A(module& mod)
    {
        uint32_t instr = mod.registers[RC_IR];
        
        // Read in operand code and indirection bit
        uint32_t code = (instr & OP_A_CODE) >> OP_A_CODE_SHIFT;
        uint32_t val = (instr & OP_A_VAL) >> OP_A_VAL_SHIFT;
        bool ind = instr & OP_A_IND;
        
        return decode_operand(mod, code, val, ind);
    }
    
    //! Decode the B operand.
    uint32_t* decode_B(module& mod)
    {
        uint32_t instr = mod.registers[RC_IR];
        
        // Read in operand code and indirection bit
        uint32_t code = (instr & OP_B_CODE) >> OP_B_CODE_SHIFT;
        uint32_t val = (instr & OP_B_VAL) >> OP_B_VAL_SHIFT;
        bool ind = instr & OP_B_IND;
        
        return decode_operand(mod, code, val, ind);
    }
    
    //! Decode the sA operand (A as signed integer).
    int32_t* decode_sA(module& mod)
    {
        return (int32_t*) decode_A(mod);
    }
    
    //! Fetch the next instruction form the module's program memory.
    uint32_t fetch(module& mod)
    {
        uint32_t* addr = fetch_word(mod);
        if (!addr)
            return 0;
        
        return (mod.registers[RC_IR] = *addr);
    }
    
    //! Execute an instruction from the MEM group.
    void execute_mem(module& mod, uint32_t icode)
    {
        switch (icode)
        {
            case I_CODE_PUSH:
                stack_push(mod, *decode_A(mod));
                break;
            
            case I_CODE_POP:
                *decode_A(mod) = stack_pop(mod);
                break;
                
            case I_CODE_MOV:
                *decode_A(mod) = *decode_B(mod);
                break;
            
            default:
                // error !
                break;
        }
    }
    
    //! Execute an instruction from the ARITH group.
    void execute_arith(module& mod, uint32_t icode)
    {
        switch (icode)
        {
            case I_CODE_UADD:
            {
                uint32_t rhs = stack_pop(mod);
                uint32_t lhs = stack_pop(mod);
                stack_push(mod, lhs + rhs);
                break;
            }
                
            case I_CODE_USUB:
            {
                uint32_t rhs = stack_pop(mod);
                uint32_t lhs = stack_pop(mod);
                stack_push(mod, lhs - rhs);
                break;
            }
                
            case I_CODE_UMUL:
            {
                uint32_t rhs = stack_pop(mod);
                uint32_t lhs = stack_pop(mod);
                stack_push(mod, lhs * rhs);
                break;
            }
                
            case I_CODE_UDIV:
            {
                uint32_t rhs = stack_pop(mod);
                uint32_t lhs = stack_pop(mod);
                stack_push(mod, lhs - rhs);
                break;
            }
                
            default:
                // error !
                break;
        }
    }
    
    //! Execute the next instruction.
    bool execute(module& mod)
    {
        uint32_t instr = fetch(mod);
        
        uint32_t icode = (instr & I_CODE_MASK) >> I_CODE_SHIFT;
        uint32_t igroup = (icode & I_GROUP_MASK) >> I_GROUP_SHIFT;
        
        switch (igroup)
        {
            case I_GROUP_SYS:
                if (icode == I_CODE_HALT)
                    return false;
                break;
                
            case I_GROUP_MEM:
                execute_mem(mod, icode);
                break;
                
            case I_GROUP_ARITH:
                execute_arith(mod, icode);
                break;
            
            default:
                // error !
                break;
        }
        
        return true;
    }
    
    //! Run until a HALT instruction is reached.
    void run(module& mod)
    {
        for (; execute(mod); );
    }
}
