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

#ifndef BOLT_VM_MODULE_H
#define BOLT_VM_MODULE_H

#include "vm_bytes.h"

namespace vm
{
    //! Register names definitions.
    //!
    //! IR:  instruction register
    //! PC:  program counter
    //! SP:  stack pointer
    //! PSR: program status register
    //! RV:  return value register
    //! AB:  arguments base register
    enum : uint32_t
    {
        REG_CODE_IR   = 0x00,
        REG_CODE_PC   = 0x01,
        REG_CODE_SP   = 0x02,
        REG_CODE_PSR  = 0x03,
        REG_CODE_RV   = 0x04,
        REG_CODE_AB   = 0x05,
        
        REG_COUNT
    };
    
    //! PSR register flags.
    //!
    //! HALT: set to 1 if VM halted
    //! Z:    zero flag
    //! N:    negative (or less) flag
    enum : uint32_t
    {
        PSR_FLAG_NONE = 0x00000000,
        
        PSR_FLAG_HALT = 0x80000000,
        PSR_FLAG_Z    = 0x00000001,
        PSR_FLAG_N    = 0x00000002
    };
    
    struct module
    {
        struct
        {
            uint32_t stack_size;
            uint32_t program_size;
            
            uint32_t entry;
        } header;
        
        uint32_t registers[REG_COUNT];
        
        uint32_t* stack;
        uint32_t* program;
    };
    
    //! Create a virtual core.
    //! Note that you are responsible of initializing (and freeing) the
    //!   header.program_size, header.entry and program fields.
    module module_create(uint32_t stack_size);
    
    //! Delete a virtual core.
    void module_free(module& mod);
    
    //! Reset a virtual core.
    void module_reset(module& mod);
    
    //! Run until the end of the program (or the core halted).
    void module_run(module& mod);
}

#endif // BOLT_VM_MODULE_H
