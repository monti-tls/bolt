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

#ifndef BOLT_AS_LAYER_H
#define BOLT_AS_LAYER_H

#include "bolt/common.h"
#include <string>

//!
//! as_layer
//!

//! This file defines the compatibility layer between the assembler module (as_*)
//!   and the virtual machine module (vm_*).
//! It does so by defining an interface to fetch instruction mnemonics & operand
//!   constraints, automatically defined from the vm module, this is achieved
//!   by using the vm_instructions.inc file.
//! It also provides a way to fetch register ids from their names, this uses
//!   the vm_registers.inc file.

namespace as
{
    //! Instruction set allowed operand flags.
    //!
    //! NONE: don't allow any operand
    //! REG:  allow registers (possibly indirected)
    //! IMM:  allow immediate values (possibly indirected)
    //! ALL:  REG or IMM
    //! OPT:  set the operand to be optional
    enum : uint32_t
    {
        OP_FLAG_NONE = 0x00000000,
        OP_FLAG_REG  = 0x00000001,
        OP_FLAG_IMM  = 0x00000002,
        
        OP_FLAG_ALL  = OP_FLAG_REG
                     | OP_FLAG_IMM,
                          
        OP_FLAG_OPT  = 0x80000000
    };
    
    //! Per instruction flags.
    //!
    //! NONE:  normal instruction
    //! LONG:  instruction can handle a long jump
    //! HATCH: instruction can accept a hatch id
    enum : uint32_t
    {
        I_FLAG_NONE  = 0x00000000,
        
        I_FLAG_LONG  = 0x00000001,
        I_FLAG_HATCH = 0x00000002
    };
    
    //! This file holds a static table of layer_instruction (created
    //!   with vm_instructions.inc) describing the instruction set,
    //!   including instruction mnemonics, codes and allowed operands.
    //! Note that mnemonics are case-insensitive.
    struct layer_instruction
    {
        std::string mnemonic;
        uint32_t icode;
        uint32_t iflags;
        uint32_t aflags;
        uint32_t bflags;
    };
    
    //! A register entry, with name and code.
    struct layer_register
    {
        std::string name;
        uint32_t code;
    };
    
    //! Get the layer_instruction mapped to a mnemonic.
    //! Returns 0 if not found.
    layer_instruction* layer_find_instruction(std::string const& mnemonic);
    
    //! Get a register  by name.
    //! Returns 0 if not found.
    layer_register* layer_find_register(std::string const& name);
}

#endif // BOLT_AS_LAYER_H
