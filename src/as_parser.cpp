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

#include "as_parser.h"
#include <sstream>
#include <stdexcept>

using namespace as;

Parser::Parser(Lexer& lex) :
    m_lex(lex)
{
}

void Parser::M_error(Token const& at, std::string const& msg)
{
    std::ostringstream ss;
    ss << "json::Parser::M_error: [" << at.info().line
       << ":" << at.info().column << "]"
       << ": " << msg;
    
    throw std::logic_error(ss.str());
}