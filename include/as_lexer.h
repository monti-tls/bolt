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

#ifndef BOLT_AS_LEXER_H
#define BOLT_AS_LEXER_H

#include "as_token.h"
#include <iostream>

namespace as
{
    //! The Bolt text assembly file tokenizer.
    //! Here are the allowed tokens :
    //!   comments:   ';' blabla (single line)
    //!
    //!   directives: '.'(alpha_ alnum_*)
    //!   identifier: (alpha_ alnum_*)
    //!   label:      (alpha_ alnum_*)':'
    //!   register:   '%'(alpha_ alnum_*)
    //!   immediate:  '#'( digit+ ('u' | 'U')?
    //!                  | ('x' | 'x') xdigit+ ('u' | 'U')?
    //!                  | ('f' | 'F') digit+ ('.' digit*)?)
    //!   left bra:   '['
    //!   right bra:  ']'
    //!   offset:     '+'( digit+ ('u' | 'U')?
    //!                  | ('x' | 'x') xdigit+ ('u' | 'U')?)
    //!   comma:      ','
    //!   newline:    '\n'
    
    struct lexer
    {
        lexer(std::istream& in) : in(in) {};
        
        std::istream& in;
        int next_char;
        
        token next_token;
        token_info current_info;
    };
    
    //! Create a lexer from an input character stream.
    lexer lexer_create(std::istream& in);
    
    //! Delete a lexer.
    void lexer_free(lexer& lex);
    
    //! Reset a lexer to the beginning of the stream.
    void lexer_reset(lexer& lex);
    
    //! Seek for the next token to be extracted.
    token const& lexer_seek(lexer& lex);
    
    //! Seek for the type of the next token to be extracted.
    uint32_t lexer_seekt(lexer& lex);
    
    //! Extract the next token from the input stream.
    token lexer_get(lexer& lex);
}

#endif // BOLT_AS_LEXER_H
