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

#ifndef BOLT_AS_ISET_H
#define BOLT_AS_ISET_H

#include "common.h"
#include <string>

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
        ISET_OP_FLAG_NONE = 0x00000000,
        ISET_OP_FLAG_REG  = 0x00000001,
        ISET_OP_FLAG_IMM  = 0x00000002,
        
        ISET_OP_FLAG_ALL  = ISET_OP_FLAG_REG
                          | ISET_OP_FLAG_IMM,
                          
        ISET_OP_FLAG_OPT  = 0x80000000
    };
    
    //! This file holds a static table of iset_entry (created
    //!   with vm_iset.inc) describing the instruction set,
    //!   including instruction mnemonics, codes and allowed operands.
    //! Note that mnemonics are case-insensitive.
    struct iset_entry
    {
        const char* mnemonic;
        uint32_t icode;
        uint32_t aflags;
        uint32_t bflags;
    };
    
    //! Check if a given mnemonic exists in the instruction set.
    bool iset_exists(std::string const& mnemonic);
    
    //! Get the iset_entry corresponding to a mnemonic.
    //! Returns 0 if not found.
    iset_entry* iset_find(std::string const& mnemonic);
}

#endif // BOLT_AS_ISET_H
