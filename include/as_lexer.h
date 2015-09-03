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
    //! A class to split a character stream into tokens,
    //!   for parsing Bolt assembly text files.
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
    class Lexer
    {
    public:
        Lexer(std::istream& in);
        
        Token get();
        Token const& seek() const;
        
    private:
        void M_init();
        int M_getChar();
        void M_skipWs();
        void M_skipComments();
        void M_skip();
        Token M_getToken();
        
        bool M_getNumeric(std::string& value, bool allowF);
        
    private:
        std::istream& m_in;
        
        int m_nextChar;
        Token m_nextToken;
        Token::Info m_currentInfo;
    };
}

#endif // BOLT_AS_LEXER_H
