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

#include "bolt/run_runtime.h"
#include <iostream>

//! Print a character.
static void putc(int c)
{ std::cout << (char) c; }

//! Print an integer.
static void puti(int x)
{ std::cout << x; }

//! Print a float.
static void putf(float x)
{ std::cout << x; }

//! Print a string.
//! Note that we simply take a pointer here, that is
//!   automatically redirected by the hatch generator
//!   to the appropriate location on the heap.
//! Note also that we can't use a char*, because we would,
//!   when doing ++p, advance by a byte instead of 4 (as sizeof(uint32_t) = 4).
static void puts(int* str)
{
    for (int* p = str; *p; ++p)
        std::cout << (char) *p;
}

static int getc()
{ return std::cin.get(); }

namespace run
{
    void runtime_expose(as::linker& ln)
    {
        //! This macro is used here for readability only.
        #define EXPOSE(sig, name) \
            as::linker_add_hatch(ln, runtime_generate_hatch<sig, &name>(#name));
        
        EXPOSE(void(*)(int),   putc)
        EXPOSE(void(*)(int),   puti)
        EXPOSE(void(*)(float), putf)
        EXPOSE(void(*)(int*),  puts)
        EXPOSE(int (*)(void),  getc)
        
        #undef EXPOSE
    }
}
