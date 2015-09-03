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

#include "as_iset.h"
#include <string>
#include <vector>

namespace as
{
    //! An exported symbol, a name mapped to
    //!   a location in this module's own segment.
    //! These come from .global directives.
    struct symbol
    {
        const char* name;
        uint32_t location;
    };
    
    //! A relocation entry, that correspond
    //!   to long calls in the program.
    //! Those come from .extern directives.
    //! The segments and locations arrays (of length count)
    //!   points to the locations that must be
    //!   fixed when linking this module, i.e.
    //!   CALL pc and seg arguments.
    struct relocation
    {
        const char* name;
        uint32_t count;
        uint32_t* segments;
        uint32_t* locations;
    };
    
    //! An assembled module, that come from a single Bolt assembly file.
    //! It eventually exports symbols through the symbol table,
    //!   and probably also rely on other modules through the
    //!   relocation table.
    //! A set of modules must be linked together to form
    //!   the final vm::segments.
    //! This basically consist to fix the relocations based
    //!   on the export information (see as_link.h).
    struct module
    {
        uint32_t symbols_size;
        symbol* symbols;
        
        uint32_t relocations_size;
        relocation* relocations_size;
        
        uint32_t segment_size;
        uint32_t* segment;
        
        uint32_t entry;
    };
}

#endif // BOLT_AS_MODULE_H
