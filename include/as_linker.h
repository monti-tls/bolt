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

#ifndef BOLT_AS_LINKER_H
#define BOLT_AS_LINKER_H

#include "as_module.h"
#include "vm_core.h"

//!
//! as_linker
//!

//! This module defines the Bolt program linker.
//! Multiple assembled modules are first added to the linker,
//!   specifying the entry one.
//! They are internally stored as 'objects', that just add data fields to modules.
//! In the first iteration, 'solutions' are extracted for each relocation.
//! Then, objects are mapped to segments (the one in vm::core), possibly
//!   discarding unused segments.
//! The virtual core is created, code is copied to its segment memory.
//! Lastly, all solutions are applied (and so the relocations are fixed), and the core
//!   is ready !
//!
//! All those steps in the assembling of multiple modules into a final core
//!   add a lot of algorithmic complexity to the system, but it ensures
//!   that faster possible operation.
//! Because all function calls are finally mapped to simple offsets, no table
//!   lookup is needed at runtime.

namespace as
{
    //! A relocation solution, that links a relocation from a module (the applicant)
    //!   to a symbol in another (the provider).
    struct solution
    {
        std::string symbol_name;
        uint32_t provider;
    };
    
    //! Modules are wrapped in objects,
    //!   to additional data such as solutions.
    struct object
    {
        module mod;
        
        uint32_t solutions_size;
        solution* solutions;
        
        bool used;
        uint32_t segment_id;
    };
    
    //! The linker structure.
    struct linker
    {
        uint32_t objects_size;
        object* objects;
        
        uint32_t base_object;
        uint32_t segments_count;
        vm::core vco;
    };
    
    //! Create a linker.
    linker linker_create();
    
    //! Free all modules added to this linker.
    void linker_free_modules(linker& ln);
    
    //! Free a linker structure.
    void linker_free(linker& ln);
    
    //! Add a module to a linker, returning its id.
    uint32_t linker_add_module(linker& ln, module const& mod);
    
    //! Link all modules added to the linker and
    //!   create a virtual core containing the linked program.
    //! The base argument specifies the entry module id.
    vm::core linker_link(linker& ln, uint32_t base);
}

#endif // BOLT_AS_LINKER_H
