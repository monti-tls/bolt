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

#include "as_linker.h"
#include <stdexcept>
#include <algorithm>

namespace as
{
    /**************************************/
    /*** Private implementation section ***/
    /**************************************/
    
    //FIXME: merge this with the one in as_module.cpp and as_assembler.cpp
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
    
    //! Attempt to find a solution to the relocation reloc
    //!   asked by the applicant module.
    //! The solution is written to sol, and the function returns false
    //!   if it is not found.
    //! It throws an error if multiple solutions are found.
    bool linker_find_solution(linker& ln, solution& sol, uint32_t applicant, relocation* reloc)
    {
        bool found = false;
        
        for (uint32_t i = 0; i < ln.modules_size; ++i)
        {
            if (i == applicant)
                break;
            
            if (module_find_symbol(ln.modules[i], reloc->name))
            {
                if (found)
                    throw std::logic_error("as::linker_find_solution: symbol `" + reloc->name + "' is multiply defined");
                
                found = true;
                sol.symbol_name = reloc->name;
                sol.applicant = applicant;
                sol.provider = i;
            }
        }
        
        return found;
    }
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    linker linker_create()
    {
        linker ln;
        
        ln.modules_size = 0;
        ln.modules = 0;
        
        return ln;
    }
    
    void linker_free_modules(linker& ln)
    {
        for (uint32_t i = 0; i < ln.modules_size; ++i)
            module_free(ln.modules[i]);
    }
    
    void linker_free(linker& ln)
    {
        if (ln.modules)
            delete[] ln.modules;
        ln.modules = 0;
        ln.modules_size = 0;
    }
    
    uint32_t linker_add_module(linker& ln, module const& mod)
    {
        grow_array(ln.modules_size, ln.modules) = mod;
        return ln.modules_size++;
    }
    
    vm::core linker_link(linker& ln, uint32_t base)
    {
        for (uint32_t i = 0; i < ln.modules_size; ++i)
        {
            module& mod = ln.modules[i];
            
            for (uint32_t j = 0; j < mod.relocations_size; ++j)
            {
                relocation& reloc = mod.relocations[j];
                
                solution sol;
                if (!linker_find_solution(ln, sol, i, &reloc))
                    throw std::logic_error("linker_link: could not resolve symbol `" + reloc.name + "'");
            }
        }
    }
}
