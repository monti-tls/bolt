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

#ifndef BOLT_AS_TOKEN_H
#define BOLT_AS_TOKEN_H

#include <string>

//!
//! as_token
//!

//! This header file defines token values for as_lexer.

namespace as
{
    enum : uint32_t
    {
        TOKEN_EOF,
        TOKEN_BAD,
        TOKEN_DIRECTIVE,
        TOKEN_IDENTIFIER,
        TOKEN_LABEL,
        TOKEN_REGISTER,
        TOKEN_IMMEDIATE,
        TOKEN_LEFT_BRACKET,
        TOKEN_RIGHT_BRACKET,
        TOKEN_OFFSET,
        TOKEN_COMMA,
        TOKEN_NEWLINE
    };
    
    //! This is an information structure about where
    //!   the token come from.
    struct token_info
    {
        int line, column;
    };
    
    //! The token structure, with type and value.
    struct token
    {
        uint32_t type;
        std::string value;
        token_info info;
    };
}

#endif // BOLT_AS_TOKEN_H
