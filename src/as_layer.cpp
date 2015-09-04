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

#include "as_layer.h"
#include "vm_bytes.h"
#include "vm_core.h"

namespace as
{
    /**************************************/
    /*** Private implementation section ***/
    /**************************************/
    
    //! A small flag macro helper, to avoid
    //!   typing OP_FLAG_* everytime.
    #define F(flag) OP_FLAG_ ## flag
    //! Same for I_FLAG_* constants.
    #define I(flag) I_FLAG_ ## flag
    //! This macro defines the behavior of the declarations in vm_instructions.inc,
    //!   here we use it to extract mnemonic, icode and operand
    //!   flags data to our iset_entry array.
    #define DECL_INSTR(group, name, code, flag, a, b) \
        { \
            .mnemonic = #name, \
            .icode = vm::I_CODE_ ## name, \
            .iflags = (flag), \
            .aflags = (a), \
            .bflags = (b) \
        },
    
    static layer_instruction layer_instructions[] =
    {
        #include "vm_instructions.inc"
    };
    
    #undef DECL_INSTR
    #undef F
    #undef I
    
    //! The length of the above array, used for loops.
    static uint32_t layer_instructions_size = sizeof(layer_instructions)
                                            / sizeof(layer_instruction);
    
    //! This macro defines the behavior of the declarations in vm_registers.inc,
    //!   here we use them to associate register names and codes.
    #define DECL_REGISTER(codename, value) \
        { \
            .name = #codename, \
            .code = vm::REG_CODE_ ## codename \
        },
    
    static layer_register layer_registers[] =
    {
        #include "vm_registers.inc"
    };
    
    #undef DECL_REGISTER
    
    static uint32_t layer_registers_size = sizeof(layer_registers)
                                         / sizeof(layer_register);
    
    //! Compare two string without sensitivity to case.
    //! We use this as the mnemonics in layer_instructions are probably in
    //!   uppercase, while I like to program in lowercase.
    static bool case_compare(std::string const& a, std::string const& b)
    {
        if (a.size() != b.size())
            return false;
        
        for (unsigned int i = 0; i < a.size(); ++i)
            if (std::tolower(a[i]) != std::tolower(b[i]))
                return false;
            
        return true;
    }
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    layer_instruction* layer_find_instruction(std::string const& mnemonic)
    {
        for (uint32_t i = 0; i < layer_instructions_size; ++i)
            if (case_compare(mnemonic, layer_instructions[i].mnemonic))
                return layer_instructions + i;
        
        return 0;
    }
    
    layer_register* layer_find_register(std::string const& name)
    {
        for (uint32_t i = 0; i < layer_registers_size; ++i)
            if (case_compare(name, layer_registers[i].name))
                return layer_registers + i;
        
        return 0;
    }
}
