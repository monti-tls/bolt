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

#include "bolt/as_lexer.h"

namespace as
{
    /**************************************/
    /*** Private implementation section ***/
    /**************************************/
    
    static int lexer_get_char(lexer&);
    static token lexer_get_token(lexer&);
    
    //! Init the lexer.
    static void lexer_init(lexer& lex)
    {
        lex.current_info.line = 1;
        lex.current_info.column = 1;
        
        // Get first char
        lex.next_char = 0;
        lexer_get_char(lex);
        
        // Get first token
        lex.next_token = lexer_get_token(lex);
    }
    
    //! Exctract a character from the input stream.
    //! This function manages the position in the input
    //!   stream for debug and error messages purposes.
    static int lexer_get_char(lexer& lex)
    {
        int ch = lex.next_char;
        lex.next_char = lex.in.get();
        
        // Update stream information
        if (ch == '\n')
        {
            ++lex.current_info.line;
            lex.current_info.column = 0;
        }
        ++lex.current_info.column;
        
        return ch;
    }
    
    //! Skip whitespaces (but not lines).
    static void lexer_skip_ws(lexer& lex)
    {
        while (std::isspace(lex.next_char) &&
               lex.next_char != '\n')
            lexer_get_char(lex);
    }
    
    //! Skip comments (starting with a semicolon).
    static void lexer_skip_comments(lexer& lex)
    {
        while (lex.next_char == ';')
        {
            while (lex.next_char != '\n')
            {
                lexer_get_char(lex);
                // EOF guard
                if (lex.next_char < 0) return;
            }
        }
    }
    
    //! Skip unwanted characters.
    static void lexer_skip(lexer& lex)
    {
        lexer_skip_ws(lex);
        lexer_skip_comments(lex);
    }
    
    //! Get a numeric value.
    //! allowF specifies if floating-point constants are allowed (for
    //!   offsets, it is not).
    static bool lexer_get_numeric(lexer& lex, std::string& value, bool allowF)
    {
        // Decimal base number
        if (std::isdigit(lex.next_char) || lex.next_char == '-')
        {
            // Get sign if needed
            if (lex.next_char == '-')
                value += lexer_get_char(lex);
            
            // Get value
            while (std::isdigit(lex.next_char))
                value += lexer_get_char(lex);
            
            // Eventual unsigned qualifier
            if (lex.next_char == 'u' || lex.next_char == 'U')
                value += lexer_get_char(lex);
        }
        // Hexadecimal base number
        else if (lex.next_char == 'x' || lex.next_char == 'X')
        {
            // Get explicit X qualifier
            value += lexer_get_char(lex);
            
            // Get sign if needed
            if (lex.next_char == '-')
                value += lexer_get_char(lex);
            
            // Get value
            if (!std::isxdigit(lex.next_char))
                return false;
            
            while (std::isxdigit(lex.next_char))
                value += lexer_get_char(lex);
            
            // Eventual unsigned qualifier
            if (lex.next_char == 'u' || lex.next_char == 'U')
                value += lexer_get_char(lex);
        }
        // Floating-point number.
        else if (allowF && (lex.next_char == 'f' || lex.next_char == 'F'))
        {
            // Get explicit F qualifier
            value += lexer_get_char(lex);
            
            // Get sign if needed
            if (lex.next_char == '-')
                value += lexer_get_char(lex);
            
            // Get value
            if (!std::isdigit(lex.next_char) && lex.next_char != '.')
                return false;
            
            while (std::isdigit(lex.next_char))
                value += lexer_get_char(lex);
            
            if (lex.next_char == '.')
            {
                value += lexer_get_char(lex);
                
                while (std::isdigit(lex.next_char))
                    value += lexer_get_char(lex);
            }
        }
        else
            return false;
        
        return true;
    }
    
    //! Get the next token from the input stream.
    //! This function also sets debug information for the token.
    token lexer_get_token(lexer& lex)
    {
        // Ignore uninteresting characters
        lexer_skip(lex);
        
        // Create token
        token tok;
        tok.type = TOKEN_BAD;
        
        // Save debug information
        token_info info = lex.current_info;
        
        if (lex.next_char < 0)
            tok.type = TOKEN_EOF;
        else if (lex.next_char == '\n')
        {
            lexer_get_char(lex);
            tok.type = TOKEN_NEWLINE;
        }
        else
        {
            // Single-char tokens
            if (lex.next_char == '[')
            {
                lexer_get_char(lex);
                tok.type = TOKEN_LEFT_BRACKET;
            }
            else if (lex.next_char == ']')
            {
                lexer_get_char(lex);
                tok.type = TOKEN_RIGHT_BRACKET;
            }
            else if (lex.next_char == ',')
            {
                lexer_get_char(lex);
                tok.type = TOKEN_COMMA;
            }
            // Directive, in the form .<alpha alnum_*>
            else if (lex.next_char == '.')
            {
                lexer_get_char(lex);
                
                std::string value = "";
                bool ok = true;
                
                if (!std::isalpha(lex.next_char))
                    ok = false;
                
                while (ok && (std::isalnum(lex.next_char) || lex.next_char == '_'))
                    value += lexer_get_char(lex);
                
                if (ok)
                {
                    tok.type = TOKEN_DIRECTIVE;
                    tok.value = value;
                }
            }
            // Register name, %<alpha alnum*>
            else if (lex.next_char == '%')
            {
                lexer_get_char(lex);
                
                std::string name = "";
                bool ok = true;
                
                if (!std::isalpha(lex.next_char))
                    ok = false;
                
                while (ok && std::isalnum(lex.next_char))
                    name += lexer_get_char(lex);
                
                if (ok)
                {
                    tok.type = TOKEN_REGISTER;
                    tok.value = name;
                }
            }
            // Offset
            else if (lex.next_char == '+' || lex.next_char == '-')
            {
                std::string value = "";
                value += lexer_get_char(lex);
                if (lexer_get_numeric(lex, value, false))
                {
                    tok.type = TOKEN_OFFSET;
                    tok.value = value;
                }
            }
            // Immediate value
            else if (lex.next_char == '#')
            {
                lexer_get_char(lex);
                
                std::string value = "";
                if (lexer_get_numeric(lex, value, true))
                {
                    tok.type = TOKEN_IMMEDIATE;
                    tok.value = value;
                }
            }
            // Labels and identifiers
            else if (std::isalpha(lex.next_char) || lex.next_char == '_')
            {
                std::string value = "";
                
                value += lexer_get_char(lex);
                while (std::isalnum(lex.next_char) ||
                       lex.next_char == '_' ||
                       lex.next_char == '$' ||
                       lex.next_char == '-')
                    value += lexer_get_char(lex);
                
                // No space is allowed between label name and semicolon
                if (lex.next_char == ':')
                {
                    lexer_get_char(lex);
                    tok.type = TOKEN_LABEL;
                    tok.value = value;
                }
                else
                {
                    tok.type = TOKEN_IDENTIFIER;
                    tok.value= value;
                }
            }
            // Strings
            else if (lex.next_char == '"')
            {
                lexer_get_char(lex);
                
                std::string value = "";
                bool ok = true;
                
                while (ok && lex.next_char != '"')
                {
                    if (lex.next_char == '\n' || lex.next_char < 0)
                        ok = false;
                    else
                    {
                        int ch = lexer_get_char(lex);
                        if (ch == '\\')
                        {
                            ch = lexer_get_char(lex);
                            switch (ch)
                            {
                                case '\\': ch = '\\'; break;
                                case '"':  ch = '"';  break;
                                case 'n':  ch = '\n'; break;
                                case 't':  ch = '\t'; break;
                                case 'r':  ch = '\r'; break;
                                
                                default: ok = false;
                            }
                        }
                        
                        value += ch;
                    }
                }
                
                // Eat last double quote
                if (ok)
                    lexer_get_char(lex);
                
                if (ok)
                {
                    tok.type = TOKEN_STRING;
                    tok.value = value;
                }
            }
        }
        
        tok.info = info;
        return tok;
    }

    /*************************/
    /*** Public module API ***/
    /*************************/
    
    lexer lexer_create(std::istream& in)
    {
        lexer lex(in);
        lexer_init(lex);
        return lex;
    }
    
    void lexer_free(lexer&)
    {}
    
    void lexer_reset(lexer& lex)
    {
        lex.in.clear();
        lex.in.seekg(0, std::ios::beg);
        lexer_init(lex);
    }
    
    token const& lexer_seek(lexer& lex)
    {
        return lex.next_token;
    }
    
    uint32_t lexer_seekt(lexer& lex)
    {
        return lex.next_token.type;
    }
    
    token lexer_get(lexer& lex)
    {
        token tok = lex.next_token;
        lex.next_token = lexer_get_token(lex);
        return tok;
    }
}
