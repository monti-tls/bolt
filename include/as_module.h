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

#ifndef BOLT_AS_MODULE_H
#define BOLT_AS_MODULE_H

#include <string>
#include <vector>

//!
//! as_module
//!

//! This file defines the as_module structure, which represents
//!   a single assembled file.
//! It introduces the exported symbols / external dependencies via the symbol
//!   and relocation tables.
//! It is the ouput of as_assembler, and input of as_linker.
//! It mainly defines helper functions to manipulate the tables.

namespace as
{
    //! An exported symbol, a name mapped to
    //!   a location in this module's own segment.
    //! These come from .global directives.
    struct symbol
    {
        std::string name;
        uint32_t location;
    };
    
    //! A relocation entry, that correspond
    //!   to long calls in the program.
    //! Those come from .extern directives.
    //! The segments and locations arrays (of length count)
    //!   points to the locations that must be
    //!   fixed when linking this module, i.e.
    //!   CALL's seg and pc arguments.
    struct relocation
    {
        std::string name;
        uint32_t count;
        uint32_t* segments;
        uint32_t* locations;
    };
    
    //! An assembled module, that comes from a single Bolt assembly file.
    //! It eventually exports symbols through the symbol table,
    //!   and probably also rely on other modules through the
    //!   relocation table.
    //! A set of modules must be linked together to form
    //!   the final vm::core.
    //! This basically consist in fixin' the relocations based
    //!   on other module's export information (see as_linker.h).
    struct module
    {
        uint32_t symbols_size;
        symbol* symbols;
        
        uint32_t relocations_size;
        relocation* relocations;
        
        uint32_t segment_size;
        uint32_t* segment;
        
        uint32_t entry;
    };
    
    //! Create an empty relocation.
    relocation relocation_create();
    
    //! Delete a relocation.
    void relocation_free(relocation& reloc);
    
    //! Add an entry in a relocation.
    void relocation_append(relocation& reloc, uint32_t seg, uint32_t loc);
    
    //! Create an empty module.
    module module_create();
    
    //! Delete a module.
    void module_free(module& mod);
    
    //! Add a symbol to a module, returning a reference to it.
    symbol& module_add_symbol(module& mod, symbol const& sym);
    
    //! Find a module's symbol by name.
    //! Returns 0 if not found.
    symbol* module_find_symbol(module& mod, std::string const& name);
    
    //! Add a relocation to a module, returning a reference to it.
    relocation& module_add_relocation(module& mod, relocation const& reloc);
    
    //! Find a relocation by name.
    //! Returns 0 if not found.
    relocation* module_find_relocation(module& mod, std::string const& name);
    
    //! Add a relocation entry by name.
    //! Create the relocation if it does not exists.
    void module_append_relocation(module& mod, std::string const& name, uint32_t seg, uint32_t loc);
    
    //! Add a word to the module's segment buffer, returning its offset in the buffer.
    uint32_t module_add_word(module& mod, uint32_t word);
}

#endif // BOLT_AS_MODULE_H
