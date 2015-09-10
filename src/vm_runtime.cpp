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

#include "bolt/vm_runtime.h"
#include <iostream>
#include <cmath>

/***************************************/
/*** Bolt standard library functions ***/
/***************************************/

namespace bolt
{
    //!
    //! IO functions
    //!
    
    static void putc(int c)
    { std::cout << (char) c; }

    static void puti(int x)
    { std::cout << x; }

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
}

/*************************/
/*** Public module API ***/
/*************************/

namespace bolt { namespace vm
{
    void runtime_expose(as::linker& ln)
    {
        //! This macro is used here for readability only.
        #define EXPOSE(sig, ns, name) \
            as::linker_add_hatch(ln, runtime_generate_hatch<sig, &ns::name>(#name));
        
        EXPOSE(void(*)(int),           bolt, putc)
        EXPOSE(void(*)(int),           bolt, puti)
        EXPOSE(void(*)(float),         bolt, putf)
        EXPOSE(void(*)(int*),          bolt, puts)
        EXPOSE(int (*)(void),          bolt, getc)
        
        EXPOSE(float(*)(float),        std,  cos)
        EXPOSE(float(*)(float),        std,  sin)
        EXPOSE(float(*)(float),        std,  tan)
        EXPOSE(float(*)(float),        std,  acos)
        EXPOSE(float(*)(float),        std,  asin)
        EXPOSE(float(*)(float),        std,  atan)
        EXPOSE(float(*)(float, float), std,  atan2)
        EXPOSE(float(*)(float),        std,  exp)
        EXPOSE(float(*)(float),        std,  log)
        EXPOSE(float(*)(float),        std,  log2)
        EXPOSE(float(*)(float),        std,  log10)
        EXPOSE(float(*)(float, float), std,  pow)
        EXPOSE(float(*)(float),        std,  sqrt)
        EXPOSE(float(*)(float),        std,  ceil)
        EXPOSE(float(*)(float),        std,  floor)
        EXPOSE(float(*)(float),        std,  abs)
        
        #undef EXPOSE
    }
} }
