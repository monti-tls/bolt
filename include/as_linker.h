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

namespace as
{
    //! A relocation solution, that links a relocation from a module (the applicant)
    //!   to a symbol in another (the provider).
    struct solution
    {
        std::string symbol_name;
        uint32_t applicant;
        uint32_t provider;
    };
    
    //! The linker structure.
    struct linker
    {
        uint32_t modules_size;
        module* modules;
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
