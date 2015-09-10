/* This file is part of bolt.
 * 
 * Copyright (c) 2015, Alexandre Monti
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

#include "bolt/as_lexer.h"
#include "bolt/as_module.h"

//!
//! as_assembler
//!

//! This module defines the text assembly program assembler for Bolt.

//! A text Bolt assembly file (called a module) contain
//!   a mix of directives and instructions (in any order).
//!
//! Directives begins with a dot, immediately followed by an identifier,
//!   then by optional arguments.
//! The available directives are :
//!   .entry  (label): specify the entry point of this module
//!   .extern (label): specifies that the given label is to be found in another object
//!   .global (label): exports the given label so it can be found in another object
//!
//! Instructions are of the form :
//!   mnemonic [operandA [operandB]]
//! Where an operand is :
//!   a register %reg
//!   an immediate value #val
//!   an indirection [%reg] or [#val]
//!   an offset indirection [%reg+val] or [#val+val] (useless ;) )

namespace bolt { namespace as
{
    //! A pending label entry.
    //! Each referenced label creates a pending entry, that
    //!   contains words to be fixed when the label is encountered.
    //! A pending label comprises two lists :
    //!   - a list of pointers to fix, used to fix module
    //!     parameters such as module's entry point.
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
    
    //! A label entry, each time a label is defined
    //!   in the source assembly file, a such entry is
    //!   added in the table.
    //! Later on, pending label references (pointers and locations)
    //!   are fixed using this table.
    struct label
    {
        std::string name;
        uint32_t location;
    };
    
    //! The assembler structure.
    struct assembler
    {
        assembler(lexer& lex) : lex(lex) {}
        
        lexer& lex;
        
        uint32_t pending_labels_size;
        pending_label* pending_labels;
        
        uint32_t externs_size;
        std::string* externs;
        
        uint32_t labels_size;
        label* labels;
        
        module mod;
    };
    
    //! Create an assembler from a lexer.
    assembler assembler_create(lexer& lex);
    
    //! Delete an assembler object.
    void assembler_free(assembler& ass);
    
    //! Assemble the stream into a module.
    //! Note that you must free it yourself !
    module assembler_assemble(assembler& ass);
} }

#endif // BOLT_AS_ASSEMBLER_H
