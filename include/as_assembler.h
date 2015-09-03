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

#ifndef BOLT_AS_ASSEMBLER_H
#define BOLT_AS_ASSEMBLER_H

#include "as_lexer.h"
#include "as_module.h"

namespace as
{
    //! A pending label entry.
    //! Each referenced label creates a pending entry, that
    //!   contains words to be fixed when the label is encountered.
    //! A pending label comprises two lists :
    //!   - a list of pointers to fix, used to fix module
    //!     parameters such as symbol locations or entry point.
    //!   - a list of locations to fix, relative to the code segment in the module.
    //! Locations are used for the code segment because of the numerous re-allocations
    //!   performed to resize the buffer, and which would invalidate direct pointers.
    struct pending_label
    {
        std::string name;
        
        uint32_t pointers_size;
        uint32_t** pointers;
        
        uint32_t locations_size;
        uint32_t* locations;
        
        bool fixed;
    };
    
    struct assembler
    {
        assembler(lexer& lex) : lex(lex) {}
        
        lexer& lex;
        
        uint32_t pending_labels_size;
        pending_label* pending_labels;
        
        uint32_t externs_size;
        std::string* externs;
        
        module mod;
    };
    
    //! Create an assembler from a lexer.
    assembler assembler_create(lexer& lex);
    
    //! Delete an assembler object.
    void assembler_free(assembler& ass);
}

#endif // BOLT_AS_ASSEMBLER_H
