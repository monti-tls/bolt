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

#ifndef BOLT_AS_PARSER_H
#define BOLT_AS_PARSER_H

#include "as_lexer.h"

namespace as
{
    //! This class defines a parser (built on top
    //!   of as::Lexer) to parse text Bolt assembly files.
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
    //!
    //! Labels can be placed at any point in the program and represent
    //!   the addresses in the program memory immediately after where they are placed.
    //!
    //! This parser simply translates the token stream to a
    //!   more comfortable representation (as::Element, as::Instruction,
    //!   as::Directive, as::Label, as::Operand).
    //! It also does some basic error checking:
    //!   - invalid token sequences
    //!   - invalid registers, directives and instruction mnemonics
    //!   - invalid instruction operands
    class Parser
    {
    public:
        Parser(Lexer& lex);
        
    private:
        void M_error(Token const& at, std::string const& msg);
        
    private:
        Lexer& m_lex;
    };
}

#endif // BOLT_AS_PARSER_H
