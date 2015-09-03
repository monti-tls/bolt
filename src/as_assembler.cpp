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

#include "as_assembler.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace as
{
    //FIXME: merge this with the one in as_module.cpp
    //! Grows an array by one element, returning a
    //!   reference to the last value.
    template <typename T>
    static T& grow_array(uint32_t size, T*& array)
    {
        T* old = array;
        array = new T[size + 1];
        
        if (old)
        {
            std::copy_n(old, size, array);
            delete[] old;
        }
        
        return array[size];
    }
    
    //! Output an error, printing debug information about its location.
    static void assembler_error(token const& where, std::string const& what)
    {
        std::ostringstream ss;
        ss << "as::assembler_error: line " << where.info.line
           << ", col " << where.info.column << ": "
           << what;
        throw std::logic_error(ss.str());
    }
    
    //! Check the next token against the given type, and output an error
    //!   if mismatch.
    static void assembler_expect(assembler& ass, uint32_t type, std::string const& what)
    {
        if (lexer_seekt(ass.lex) != type)
            assembler_error(lexer_seek(ass.lex), what);
    }
    
    //! Set up temporary things for the assembler,
    //!   like the output module and the various tables.
    static void assembler_temps_create(assembler& ass)
    {
        ass.mod = module_create();
        
        ass.pending_labels_size = 0;
        ass.pending_labels = 0;
        
        ass.externs_size = 0;
        ass.externs = 0;
    }
    
    //! Free temporary objects from the assembler.
    //! This do not free the assembled module, as this is the output !
    static void assembler_temps_free(assembler& ass)
    {
        if (ass.pending_labels)
        {
            for (uint32_t i = 0; i < ass.pending_labels_size; ++i)
            {
                if (ass.pending_labels[i].pointers)
                    delete[] ass.pending_labels[i].pointers;
                
                if (ass.pending_labels[i].locations)
                    delete[] ass.pending_labels[i].locations;
            }
        }
        ass.pending_labels = 0;
        ass.pending_labels_size = 0;
        
        if (ass.externs)
            delete[] ass.externs;
        ass.externs = 0;
        ass.externs_size = 0;
    }
    
    //! Search the pending label table for the given label name.
    //! Returns 0 if not found.
    static pending_label* assembler_find_pending(assembler& ass, std::string const& name)
    {
        for (uint32_t i = 0; i < ass.pending_labels_size; ++i)
            if (ass.pending_labels[i].name == name)
                return ass.pending_labels + i;
        
        return 0;
    }
    
    //! Add a pending label pointer by name.
    //! This searchs the pending label table, and create a new entry if needed.
    static void assembler_add_pending_pointer(assembler& ass, std::string const& name, uint32_t* pointer)
    {
        pending_label* label = assembler_find_pending(ass, name);
        if (!label)
        {
            label = &grow_array(ass.pending_labels_size++, ass.pending_labels);
            label->pointers_size = 0;
            label->pointers = 0;
            label->locations_size = 0;
            label->locations = 0;
        }
        
        grow_array(label->pointers_size++, label->pointers) = pointer;
    }
    
    //! Add a pending label location by name.
    //! This searchs the pending label table, and create a new entry if needed.
    static void assembler_add_pending_location(assembler& ass, std::string const& name, uint32_t location)
    {
        pending_label* label = assembler_find_pending(ass, name);
        if (!label)
        {
            label = &grow_array(ass.pending_labels_size++, ass.pending_labels);
            label->pointers_size = 0;
            label->pointers = 0;
            label->locations_size = 0;
            label->locations = 0;
        }
        
        grow_array(label->locations_size++, label->locations) = location;
    }
    
    //! Find an externed function by name in the externs table.
    std::string* assembler_find_extern(assembler& ass, std::string const& name)
    {
        for (uint32_t i = 0; i < ass.externs_size; ++i)
            if (ass.externs[i] == name)
                return ass.externs + i;
        
        return 0;
    }
    
    //! Add an extern symbol name to the externs table.
    static void assembler_add_extern(assembler& ass, std::string const& name)
    {
        grow_array(ass.externs_size++, ass.externs) = name;
    }
    
    //! Parse an assembler directive.
    static void assembler_directive(assembler& ass)
    {
        assembler_expect(ass, TOKEN_DIRECTIVE, "directive expected");
        
        token tok = lexer_get(ass.lex);
        std::string directive = tok.value;
        
        if (directive == "entry")
        {
            // Get the target label
            assembler_expect(ass, TOKEN_IDENTIFIER, ".entry directive expects an identifier");
            tok = lexer_get(ass.lex);
            std::string identifier = tok.value;
            
            // Add the module's entry point to the pending list
            assembler_add_pending_pointer(ass, identifier, &ass.mod.entry);
        }
        else if (directive == "global")
        {
            // Find the target label
            assembler_expect(ass, TOKEN_IDENTIFIER, ".global directive expects an identifier");
            tok = lexer_get(ass.lex);
            std::string symname = tok.value;
            
            if (module_find_symbol(ass.mod, symname))
                assembler_error(tok, "symbol is already exported");
            
            // Create the new symbol
            symbol sym;
            sym.name = symname;
            symbol& real = module_add_symbol(ass.mod, sym);
            
            // Bind the pending label to the real symbol location,
            //   as module_ass_symbol does a copy !
            assembler_add_pending_pointer(ass, symname, &real.location);
        }
        else if (directive == "extern")
        {
            // Find the target label
            assembler_expect(ass, TOKEN_IDENTIFIER, ".global directive expects an identifier");
            tok = lexer_get(ass.lex);
            std::string name = tok.value;
            
            if (module_find_symbol(ass.mod, name))
                assembler_error(tok, "symbol was declared global earlier");
            
            if (assembler_find_extern(ass, name))
                assembler_error(tok, "symbol is already declared extern");
            
            assembler_add_extern(ass, name);
        }
        else
            assembler_error(tok, "unknown directive \"" + directive + "\"");
    }
    
    static void assembler_label(assembler& ass)
    {
        //TODO: fix the label from the pending list
    }
    
    static void assembler_parse(assembler& ass)
    {
        
    }
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    assembler assembler_create(lexer& lex)
    {
        assembler ass(lex);
        return ass;
    }
    
    void assembler_free(assembler&)
    {}
}
