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

#ifndef BOLT_VM_CORE_H
#define BOLT_VM_CORE_H

#include "vm_bytes.h"

namespace vm
{
    //! This macro defines the behavior of the declarations
    //!   in vm_registers.inc, here they define the enumeration
    //!   constants REG_CODE_*.
    #define DECL_REGISTER(name, value) \
        REG_CODE_ ## name = value,
    
    enum : uint32_t
    {
        #include "vm_registers.inc"
        
        REG_COUNT
    };
    
    #undef DECL_REGISTER
    
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
    
    //! A hatch is a structure which links the
    //!   virtual code to a host C function.
    struct core;
    struct hatch
    {
        void(*entry)(core& vco);
    };
    
    //! Each VM core emulates a physical CPU core.
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
    struct core
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
    //! Note that you are responsible of setting the segments
    //!   and hatches fields manually.
    core core_create(uint32_t stack_size, uint32_t segments_size, uint32_t hatchs_size);
    
    //! Free all segments in the virtual core.
    void core_free_segments(core& vco);
    
    //! Delete a virtual core (does *not* frees segments).
    void core_free(core& vco);
    
    //! Reset a virtual core.
    void core_reset(core& vco);
    
    //! Run until the end of the program (or the core halted).
    void core_run(core& vco);
}

#endif // BOLT_VM_CORE_H
