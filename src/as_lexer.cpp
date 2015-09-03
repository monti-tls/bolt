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

#include "as_lexer.h"

using namespace as;

Lexer::Lexer(std::istream& in) :
    m_in(in)
{
    M_init();
}

Token Lexer::get()
{
    Token tok = m_nextToken;
    m_nextToken = M_getToken();
    return tok;
}

Token const& Lexer::seek() const
{ return m_nextToken; }

//! Init the lexer (called from constructors).
void Lexer::M_init()
{
    m_currentInfo.line = 1;
    m_currentInfo.column = 0;
    
    // Get first char (m_nextChar is now valid)
    m_nextChar = 0;
    M_getChar();
    // Get first token (m_nextToken is now valid)
    m_nextToken = M_getToken();
}

//! Exctract a character from the input stream.
//! This method manages the position in the input
//!   stream for debug and error messages purposes.
int Lexer::M_getChar()
{
    int ch = m_nextChar;
    m_nextChar = m_in.get();
    
    // Update stream information
    if (ch == '\n')
    {
        ++m_currentInfo.line;
        m_currentInfo.column = 0;
    }
    ++m_currentInfo.column;
    
    return ch;
}

//! Skip whitespaces (and new lines).
void Lexer::M_skipWs()
{
    while (std::isspace(m_nextChar) || m_nextChar == '\n')
    {
        M_getChar();
        if (m_nextChar < 0) return;
    }
}

//! Skip comments, starting with a semicolon.
void Lexer::M_skipComments()
{
    while (m_nextChar == ';')
    {
        while (m_nextChar != '\n')
        {
            M_getChar();
            if (m_nextChar < 0) return;
        }
        
        M_skipWs();
    }
}

//! Skip whitespaces and comments.
void Lexer::M_skip()
{
    M_skipWs();
    M_skipComments();
    M_skipWs();
}

Token Lexer::M_getToken()
{
    M_skip();
    
    Token token = Token::Bad;
    Token::Info info = m_currentInfo;
    
    if (m_nextChar < 0)
        token = Token::Eof;
    else
    {
        // Single-char tokens
        if (m_nextChar == '[')
        {
            M_getChar();
            token = Token::LeftBracket;
        }
        else if (m_nextChar == ']')
        {
            M_getChar();
            token = Token::RightBracket;
        }
        else if (m_nextChar == ',')
        {
            M_getChar();
            token = Token::Comma;
        }
        // Directive, in the form .<alpha alnum_*>
        else if (m_nextChar == '.')
        {
            M_getChar();
            
            std::string value = "";
            bool ok = true;
            
            if (!std::isalpha(m_nextChar))
                ok = false;
            
            while (ok && (std::isalnum(m_nextChar) || m_nextChar == '_'))
                value += M_getChar();
            
            if (ok)
                token = Token(Token::Directive, value);
        }
        // Register name, %<alpha alnum*>
        else if (m_nextChar == '%')
        {
            M_getChar();
            
            std::string name = "";
            bool ok = true;
            
            if (!std::isalpha(m_nextChar))
                ok = false;
            
            while (ok && std::isalnum(m_nextChar))
                name += M_getChar();
            
            if (ok)
                token = Token(Token::Register, name);
        }
        // Offset
        else if (m_nextChar == '+')
        {
            M_getChar();
            
            std::string value = "";
            if (M_getNumeric(value, false))
                token = Token(Token::Offset, value);
        }
        // Immediate value
        else if (m_nextChar == '#')
        {
            M_getChar();
            
            std::string value = "";
            if (M_getNumeric(value, true))
                token = Token(Token::Immediate, value);
        }
        // Labels and identifiers
        else if (std::isalpha(m_nextChar) || m_nextChar == '_')
        {
            std::string value = "";
            
            value += M_getChar();
            while (std::isalnum(m_nextChar) || m_nextChar == '_')
                value += M_getChar();
            
            // No space is allowed between label name and semicolon
            if (m_nextChar == ':')
            {
                M_getChar();
                token = Token(Token::Label, value);
            }
            else
            {
                token = Token(Token::Identifier, value);
            }
        }
    }
    
    token.setInfo(info);
    return token;
}

//! Get a numeric value.
//! allowF specifies if floating-point constants are allowed (for
//!   offsets, it is not).
bool Lexer::M_getNumeric(std::string& value, bool allowF)
{
    // Decimal base number
    if (std::isdigit(m_nextChar) || m_nextChar == '-')
    {
        // Get sign if needed
        if (m_nextChar == '-')
            value += M_getChar();
        
        // Get value
        while (std::isdigit(m_nextChar))
            value += M_getChar();
        
        // Eventual unsigned qualifier
        if (m_nextChar == 'u' || m_nextChar == 'U')
            value += M_getChar();
    }
    // Hexadecimal base number
    else if (m_nextChar == 'x' || m_nextChar == 'X')
    {
        // Get the explicit X qualifier
        value += M_getChar();
        
        // Get sign if needed
        if (m_nextChar == '-')
            value += M_getChar();
        
        // Get value
        if (!std::isxdigit(m_nextChar))
            return false;
        
        while (std::isxdigit(m_nextChar))
            value += m_nextChar;
        
        // Eventual unsigned qualifier
        if (m_nextChar == 'u' || m_nextChar == 'U')
            value += M_getChar();
    }
    // 
    else if (allowF && (m_nextChar == 'f' || m_nextChar == 'F'))
    {
        // Get explicit F qualifier
        value += M_getChar();
        
        // Get sign if needed
        if (m_nextChar == '-')
            value += M_getChar();
        
        // Get value
        if (!std::isdigit(m_nextChar) || m_nextChar != '.')
            return false;
        
        while (std::isdigit(m_nextChar))
            value += m_nextChar;
        
        if (m_nextChar == '.')
        {
            value += M_getChar();
            
            while (std::isdigit(m_nextChar))
                value += M_getChar();
        }
    }
    else
        return false;
    
    return true;
}
