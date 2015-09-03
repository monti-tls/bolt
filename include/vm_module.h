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
    //! SEG: segment register
    //! SP:  stack pointer
    //! PSR: program status register
    //! RV:  return value register
    //! AB:  arguments base register
    enum : uint32_t
    {
        REG_CODE_IR   = 0x00,
        REG_CODE_SEG  = 0x01,
        REG_CODE_PC   = 0x02,
        REG_CODE_SP   = 0x03,
        REG_CODE_PSR  = 0x04,
        REG_CODE_RV   = 0x05,
        REG_CODE_AB   = 0x06,
        
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
    
    //! This structure represents a program to be run on a virtual core.
    //! It holds a buffer containing the instructions (buffer),
    //!   plus its size (in uint32_t increments).
    //! The entry point specifies the initial PC value (if applicable).
    struct segment
    {
        uint32_t* buffer;
        uint32_t size;
        uint32_t entry;
    };
    
    //! A hatch is a C wrapper function which links the
    //!   virtual code to host C functions.
    struct module;
    struct hatch
    {
        void(*entry)(module& mod);
    };
    
    //! Each VM module emulates a physical CPU core.
    //! It has its own register bank and stack memory.
    //! It is possible to execute code from another program
    //! (that may be dynamically loaded for example) by using the segments
    //! table.
    //! The SEG register contains the segment from within the current program
    //!   originates.
    //! It is saved and restored upon CALL and RET instructions and therefore
    //!   one can easily call a function residing in another segment.
    //! The base field is the index of the initial segment (it inits the SEG
    //!   register upon reset).
    struct module
    {
        uint32_t stack_size;
        uint32_t segments_size;
        uint32_t hatches_size;
        uint32_t base;
        
        uint32_t registers[REG_COUNT];
        
        uint32_t* stack;
        segment** segments;
        hatch** hatches;
    };
    
    //! Create a virtual core.
    //! Note that you are responsible of setting the prgm
    //!   field manually.
    module module_create(uint32_t stack_size, uint32_t segments_size, uint32_t hatchs_size);
    
    //! Delete a virtual core.
    void module_free(module& mod);
    
    //! Reset a virtual core.
    void module_reset(module& mod);
    
    //! Run until the end of the program (or the core halted).
    void module_run(module& mod);
}

#endif // BOLT_VM_MODULE_H
