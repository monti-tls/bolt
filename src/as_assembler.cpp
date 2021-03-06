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

#include "bolt/as_assembler.h"
#include "bolt/as_layer.h"
#include "bolt/vm_bytes.h"
#include "bolt/vm_core.h"
#include <stdexcept>
#include <sstream>
#include <algorithm>

namespace bolt { namespace as
{
    // Forward declarations.
    static uint32_t assembler_parse_immediate(std::string const& value);
    
    //FIXME: merge this with the one in as_module.cpp and as_linker.cpp
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
    
    //! A handy operand intermediate structure, to pass from
    //!   assembler_parse_operand to assembler_parse_instruction for
    //!   encoding in the latter.
    struct assembler_operand
    {
        //! Value from OP_CODE_*
        uint32_t code;
        //! Operand value (if not immediate).
        uint32_t value;
        //! Indirection and offset bits.
        bool ind, off;
    };
    
    //! Output an error, printing debug information about its location.
    static void assembler_parse_error(token const& where, std::string const& what)
    {
        std::ostringstream ss;
        ss << "as::assembler_parse_error: line " << where.info.line
           << ", col " << where.info.column << ": "
           << what;
        throw std::logic_error(ss.str());
    }
    
    //! Just output an error.
    static void assembler_error(std::string const& what)
    {
        std::ostringstream ss;
        ss << "as::assembler_error: " << what;
        throw std::logic_error(ss.str());
    }
    
    //! Check the next token against the given type, and output an error
    //!   if mismatch.
    static void assembler_expect(assembler& ass, uint32_t type, std::string const& what)
    {
        if (lexer_peekt(ass.lex) != type)
            assembler_parse_error(lexer_peek(ass.lex), what);
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
        
        ass.labels_size = 0;
        ass.labels = 0;
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
            
            delete[] ass.pending_labels;
        }
        ass.pending_labels = 0;
        ass.pending_labels_size = 0;
        
        if (ass.externs)
            delete[] ass.externs;
        ass.externs = 0;
        ass.externs_size = 0;
        
        if (ass.labels)
            delete[] ass.labels;
        ass.labels = 0;
        ass.labels_size = 0;
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
    
    //! Add an empty pending label.
    static pending_label* assembler_add_pending(assembler& ass, std::string const& name)
    {
        pending_label* pending = &grow_array(ass.pending_labels_size++, ass.pending_labels);
        
        pending->name = name;
        pending->pointers_size = 0;
        pending->pointers = 0;
        pending->locations_size = 0;
        pending->locations = 0;
        
        return pending;
    }
    
    //! Add a pending label pointer by name.
    //! This searchs the pending label table, and create a new entry if needed.
    static void assembler_add_pending_pointer(assembler& ass, std::string const& name, uint32_t* pointer)
    {
        pending_label* pending = assembler_find_pending(ass, name);
        
        if (!pending)
            pending = assembler_add_pending(ass, name);
        
        grow_array(pending->pointers_size++, pending->pointers) = pointer;
    }
    
    //! Add a pending label location by name.
    //! This searchs the pending label table, and create a new entry if needed.
    static void assembler_add_pending_location(assembler& ass, std::string const& name, uint32_t location)
    {
        pending_label* pending = assembler_find_pending(ass, name);
        
        if (!pending)
            pending = assembler_add_pending(ass, name);
        
        grow_array(pending->locations_size++, pending->locations) = location;
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
    
    //! Find a label in the label table.
    //! Return 0 if not found.
    static label* assembler_find_label(assembler& ass, std::string const& name)
    {
        for (uint32_t i = 0; i < ass.labels_size; ++i)
            if (ass.labels[i].name == name)
                return ass.labels + i;
        
        return 0;
    }
    
    //! Add a label to the label table.
    static void assembler_add_label(assembler& ass, std::string const& name, uint32_t location)
    {
        label& l = grow_array(ass.labels_size++, ass.labels);   
        l.name = name;
        l.location = location;
    }
    
    //! Parse an assembler directive.
    static void assembler_parse_directive(assembler& ass)
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
            ass.mod.has_entry = true;
            assembler_add_pending_pointer(ass, identifier, &ass.mod.entry);
        }
        else if (directive == "global")
        {
            // Find the target label
            assembler_expect(ass, TOKEN_IDENTIFIER, ".global directive expects an identifier");
            tok = lexer_get(ass.lex);
            std::string symname = tok.value;
            
            if (module_find_symbol(ass.mod, symname))
                assembler_parse_error(tok, "symbol is already exported");
            
            // Create the new symbol, its location will be fixed later
            symbol sym;
            sym.name = symname;
            module_add_symbol(ass.mod, sym);
        }
        else if (directive == "extern")
        {
            // Find the target label
            assembler_expect(ass, TOKEN_IDENTIFIER, ".extern directive expects an identifier");
            tok = lexer_get(ass.lex);
            std::string name = tok.value;
            
            if (module_find_symbol(ass.mod, name))
                assembler_parse_error(tok, "symbol \"" + name + "\" was declared global earlier");
            
            if (assembler_find_extern(ass, name))
                assembler_parse_error(tok, "symbol \"" + name + "\" is already declared extern");
            
            assembler_add_extern(ass, name);
        }
        else if (directive == "data")
        {
            while (lexer_peekt(ass.lex) != TOKEN_NEWLINE)
            {
                token tok = lexer_get(ass.lex);
                
                if (tok.type == TOKEN_IMMEDIATE)
                {
                    uint32_t imm = assembler_parse_immediate(tok.value);
                    module_add_word(ass.mod, imm);
                }
                else if (tok.type == TOKEN_STRING)
                {
                    // Chars are 32-bits
                    for (unsigned int i = 0; i < tok.value.size(); ++i)
                        module_add_word(ass.mod, tok.value[i]);
                    // String are NULL-terminated
                    module_add_word(ass.mod, 0);
                }
                else
                    assembler_parse_error(tok, ".data directive expect either immediate operands or strings");
                
                if (lexer_peekt(ass.lex) != TOKEN_NEWLINE)
                {
                    assembler_expect(ass, TOKEN_COMMA, "`,' expected");
                    lexer_get(ass.lex);
                }
            }
        }
        else
            assembler_parse_error(tok, "unknown directive \"" + directive + "\"");
    }
    
    //! Parse an assembler label, adding it to the label table.
    static void assembler_parse_label(assembler& ass)
    {
        assembler_expect(ass, TOKEN_LABEL, "label expected");
        token tok = lexer_get(ass.lex);
        std::string name = tok.value;
        
        if (assembler_find_label(ass, name))
            assembler_parse_error(tok, "label \"" + name + "\" was already defined");
        
        assembler_add_label(ass, name, ass.mod.segment_size);
    }
    
    //! Parse an immediate string as a floating-point value.
    static uint32_t assembler_parse_immediate_f(std::string const& value)
    {
        union {
            float imm;
            uint32_t imm_as_uint32;
        };
        
        imm = std::stof(value);
        return imm_as_uint32;
    }
    
    //! Parse an immediate string as a hexadecimal value.
    static uint32_t assembler_parse_immediate_x(std::string const& value)
    {
        union {
            int imm;
            uint32_t imm_as_uint32;
        };
        
        // std::stoi will ignore the eventual unsigned qualifier 'U'
        imm = std::stoi(value, 0, 16);
        
        if (value.back() == 'u' || value.back() == 'U')
            return (uint32_t) imm;
        
        return imm_as_uint32;
    }
    
    //! Parse an immediate string as a decimal value.
    static uint32_t assembler_parse_immediate_d(std::string const& value)
    {
        union {
            int imm;
            uint32_t imm_as_uint32;
        };
        
        // std::stoi will ignore the eventual unsigned qualifier 'U'
        imm = std::stoi(value, 0, 10);
        
        if (value.back() == 'u' || value.back() == 'U')
            return (uint32_t) imm;
        
        return imm_as_uint32;
    }
    
    //! Parse an immediate value string and get its uint32_t representation.
    static uint32_t assembler_parse_immediate(std::string const& value)
    {
        switch (value[0])
        {
            case 'f':
            case 'F':
                return assembler_parse_immediate_f(value.substr(1));
                
            case 'x':
            case 'X':
                return assembler_parse_immediate_x(value.substr(1));
                
            default:
                return assembler_parse_immediate_d(value);
        }
    }
    
    //! Parse an operand, adding if needed immediate words to the output module.
    //! This is why the instruction code must be written *before* calling this function.
    static assembler_operand assembler_parse_operand(assembler& ass, uint32_t allowed_flags)
    {
        assembler_operand op;
        op.value = 0;
        op.ind = false;
        op.off = false;
        
        // Check for indirection modifier
        if (lexer_peekt(ass.lex) == TOKEN_LEFT_BRACKET)
        {
            lexer_get(ass.lex);
            op.ind = true;
        }
        
        // Check for operand type
        token tok = lexer_get(ass.lex);
        switch (tok.type)
        {
            //! The operand is a register.
            case TOKEN_REGISTER:
            {
                // Check if allowed
                if (!(allowed_flags & OP_FLAG_REG))
                    assembler_parse_error(tok, "register operand is not allowed for this instruction");
                
                // Find it in the hard layer
                layer_register* reg = layer_find_register(tok.value);
                if (!reg)
                    assembler_parse_error(tok, "invalid register name \"" + tok.value + "\"");
                
                // Set up the operand's code and value.
                op.code = vm::OP_CODE_REG;
                op.value = reg->code;
                break;
            }
                
            //! The operand is an immediate value.
            case TOKEN_IMMEDIATE:
            {
                // Check if allowed
                if (!(allowed_flags & OP_FLAG_IMM))
                    assembler_parse_error(tok, "immediate operand is not allowed for this instruction");
                
                // Set up the operand code
                op.code = vm::OP_CODE_IMM;
                
                // Parse the immediate value and add it to the segment code
                uint32_t value = assembler_parse_immediate(tok.value);
                module_add_word(ass.mod, value);
                break;
            }
            
            //! The operand is an identifier (i.e. a label reference).
            case TOKEN_IDENTIFIER:
            {
                // Check if allowed
                if (!(allowed_flags & OP_FLAG_IMM))
                    assembler_parse_error(tok, "immediate operand is not allowed for this instruction");
                
                // Set up the operand code
                op.code = vm::OP_CODE_IMM;
                
                // Add a pending label entry to fix this value later on
                uint32_t location = module_add_word(ass.mod, 0);
                assembler_add_pending_location(ass, tok.value, location);
                break;
            }
                
            default:
                assembler_parse_error(tok, "bad operand token");
        }
        
        // Check for immediate offset modifier
        uint32_t offset;
        if (lexer_peekt(ass.lex) == TOKEN_OFFSET)
        {
            tok = lexer_get(ass.lex);
            
            if (!op.ind)
                assembler_parse_error(tok, "offset is allowed only within indirection");
            
            op.off = true;
            
            // Read in the offset value
            offset = assembler_parse_immediate(tok.value);
            
            // Add the offset as an immediate word
            module_add_word(ass.mod, offset);
        }
        
        // Match indirection modifier
        if (op.ind)
        {
            assembler_expect(ass, TOKEN_RIGHT_BRACKET, "`]' expected");
            lexer_get(ass.lex);
        }
        
        return op;
    }
    
    static void assembler_parse_instruction(assembler& ass)
    {
        // Parse the mnemonic.
        assembler_expect(ass, TOKEN_IDENTIFIER, "mnemonic expected");
        token tok = lexer_get(ass.lex);
        std::string mnemonic = tok.value;
        
        // Find the mapped layer entry.
        layer_instruction* instr = layer_find_instruction(mnemonic);
        if (!instr)
            assembler_parse_error(tok, "invalid mnemonic \"" + mnemonic + "\"");
        
        // Add the icode word now, as assembler_parse_operand will add
        //   its owns.
        uint32_t instr_location = module_add_word(ass.mod, instr->icode << vm::I_CODE_SHIFT);
        
        // Take special care for instruction that accept long jumps
        if (instr->iflags & I_FLAG_LONG)
        {
            // We need to worry only if the operand is an extern symbol
            if (lexer_peekt(ass.lex) == TOKEN_IDENTIFIER &&
                assembler_find_extern(ass, lexer_peek(ass.lex).value))
            {
                token tok = lexer_get(ass.lex);
                
                // Add the segment and location words, to the
                //   program segment, and get their offsets
                uint32_t seg = module_add_word(ass.mod, 0);
                uint32_t loc = module_add_word(ass.mod, 0);
                // Add the corresponding relocation
                module_append_relocation(ass.mod, tok.value, seg, loc);
                
                // Fix the instruction word and set two immediate operands
                uint32_t& instr_word = ass.mod.segment[instr_location];
                instr_word |= vm::OP_CODE_IMM << vm::OP_A_CODE_SHIFT;
                instr_word |= vm::OP_CODE_IMM << vm::OP_B_CODE_SHIFT;
                
                // Get the new line
                assembler_expect(ass, TOKEN_NEWLINE, "no more operands expected after long jump");
                lexer_get(ass.lex);
                
                return;
            }
        }
        // Take special care for instructions that accept hatch names
        if (instr->iflags & I_FLAG_HATCH)
        {
            // Continue only if the instructio operand is an identifier
            if (lexer_peekt(ass.lex) == TOKEN_IDENTIFIER)
            {
                token tok = lexer_get(ass.lex);
                
                // Add the hatch id. operand word to the program segment
                uint32_t loc = module_add_word(ass.mod, 0);
                // Add the corresponding reference
                module_append_hatch_reference(ass.mod, tok.value, loc);
                
                // Fix the instruction word, setting an immediate operand
                uint32_t& instr_word = ass.mod.segment[instr_location];
                instr_word |= vm::OP_CODE_IMM << vm::OP_A_CODE_SHIFT;
                
                // Get the new line
                assembler_expect(ass, TOKEN_NEWLINE, "no more operands expected after hatch reference");
                lexer_get(ass.lex);
                
                return;
            }
        }
        
        // Get the operands, if any
        bool hasA = false;
        bool hasB = false;
        assembler_operand a, b;
        
        if (lexer_peekt(ass.lex) != TOKEN_NEWLINE)
        {
            hasA = true;
            a = assembler_parse_operand(ass, instr->aflags);
            
            if (lexer_peekt(ass.lex) != TOKEN_NEWLINE)
            {
                assembler_expect(ass, TOKEN_COMMA, "`,' expected");
                lexer_get(ass.lex);
                
                hasB = true;
                b = assembler_parse_operand(ass, instr->bflags);
            }
        }
        
        // Check for operand presence vs. optional flags
        if ((!hasA && !(instr->aflags & OP_FLAG_OPT) && instr->aflags != OP_FLAG_NONE) ||
            (!hasB && !(instr->bflags & OP_FLAG_OPT) && instr->bflags != OP_FLAG_NONE))
            assembler_parse_error(tok, "this instruction expects an operand");
        
        // Encode the operands in the icode
        uint32_t& instr_word = ass.mod.segment[instr_location];
        if (hasA)
        {
            instr_word |= a.code << vm::OP_A_CODE_SHIFT;
            instr_word |= a.value << vm::OP_A_VAL_SHIFT;
            if (a.ind)
                instr_word |= vm::OP_A_IND;
            if (a.off)
                instr_word |= vm::OP_A_OFF;
        }
        if (hasB)
        {
            instr_word |= b.code << vm::OP_B_CODE_SHIFT;
            instr_word |= b.value << vm::OP_B_VAL_SHIFT;
            if (b.ind)
                instr_word |= vm::OP_B_IND;
            if (b.off)
                instr_word |= vm::OP_B_OFF;
        }
    }
    
    //! Skip new lines.
    static void assembler_skip(assembler& ass)
    {
        while (lexer_peekt(ass.lex) == TOKEN_NEWLINE)
            lexer_get(ass.lex);
    }
    
    //! Parse the test assembly file.
    static void assembler_parse(assembler& ass)
    {
        while (lexer_peekt(ass.lex) != TOKEN_EOF)
        {
            assembler_skip(ass);
            
            switch (lexer_peekt(ass.lex))
            {
                case TOKEN_DIRECTIVE:
                    assembler_parse_directive(ass);
                    break;
                    
                case TOKEN_LABEL:
                    assembler_parse_label(ass);
                    break;
                    
                case TOKEN_IDENTIFIER:
                    assembler_parse_instruction(ass);
                    break;
                
                default:
                    assembler_parse_error(lexer_peek(ass.lex), "bad token");
            }
            
            assembler_skip(ass);
        }
    }
    
    //! Fix pending label references using the label table.
    static void assembler_fix_pending_labels(assembler& ass)
    {
        // Fix pending locations
        for (uint32_t i = 0; i < ass.pending_labels_size; ++i)
        {
            pending_label* pending = ass.pending_labels + i;
            
            // Find the associated label
            label* l = assembler_find_label(ass, pending->name);
            if (!l)
                assembler_error("unresolved label \"" + pending->name + "\"");
            
            // Fix pending pointers
            for (uint32_t j = 0; j < pending->pointers_size; ++j)
                *pending->pointers[j] = l->location;
            
            // Fix pending words in the program
            for (uint32_t j = 0; j < pending->locations_size; ++j)
                ass.mod.segment[pending->locations[j]] = l->location;
        }
        
        // Fix exported (.global) symbol locations
        for (uint32_t i = 0; i < ass.mod.symbols_size; ++i)
        {
            symbol* sym = ass.mod.symbols + i;
            
            // Find the associated label
            label* l = assembler_find_label(ass, sym->name);
            if (!l)
                assembler_error("unresolved symbol \"" + sym->name + "\"");
            
            // Fix the location
            sym->location = l->location;
        }
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
    
    module assembler_assemble(assembler& ass)
    {
        // Init temp resources (various tables...)
        assembler_temps_create(ass);
        // Parse the assembly file
        assembler_parse(ass);
        // Fix label locations
        assembler_fix_pending_labels(ass);
        // Free temp resources (this does *not* free mod)
        assembler_temps_free(ass);
        
        return ass.mod;
    }
} }
