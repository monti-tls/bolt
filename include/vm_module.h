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
    enum rcode
    {
        RC_IR   = 0x00,
        RC_PC   = 0x01,
        RC_SP   = 0x02,
        RC_PSR  = 0x03,
        
        RC_SIZE
    };
    
    //! PSR register flags.
    //!
    //! Z: zero flag
    //! N: negative (or less) flag
    enum psrflag
    {
        PSRF_NONE = 0x00000000,
        
        PSRF_Z    = 0x00000001,
        PSRF_N    = 0x00000002
    };
    
    struct module
    {
        uint32_t registers[RC_SIZE];
        
        uint32_t* stack;
        uint32_t stackSize;
        
        uint32_t* program;
        uint32_t programSize;
    };
    
    //! Run until a HALT instruction is reached.
    void run(module& mod);
}

#endif // BOLT_VM_MODULE_H
