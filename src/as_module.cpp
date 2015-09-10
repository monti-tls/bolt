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

#include "bolt/as_module.h"
#include <algorithm>

namespace bolt { namespace as
{
    /**************************************/
    /*** Private implementation section ***/
    /**************************************/
    
    //FIXME: merge this with the one in as_assembler.cpp and as_linker.cpp
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
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    relocation relocation_create()
    {
        relocation reloc;
        reloc.name = "";
        
        reloc.count = 0;
        reloc.segments = 0;
        reloc.locations = 0;
        
        return reloc;
    }
    
    void relocation_free(relocation& reloc)
    {
        if (reloc.count)
        {
            delete[] reloc.segments;
            delete[] reloc.locations;
        }
        
        reloc.count = 0;
        reloc.segments = 0;
        reloc.locations = 0;
    }
    
    void relocation_append(relocation& reloc, uint32_t seg, uint32_t loc)
    {
        grow_array(reloc.count, reloc.segments) = seg;
        grow_array(reloc.count, reloc.locations) = loc;
        ++reloc.count;
    }
    
    hatch_reference hatch_reference_create()
    {
        hatch_reference ref;
        ref.name = "";
        
        ref.count = 0;
        ref.locations = 0;
        
        return ref;
    }
    
    void hatch_reference_free(hatch_reference& ref)
    {
        if (ref.count)
            delete[] ref.locations;
        
        ref.count = 0,
        ref.locations = 0;
    }
    
    void hatch_reference_append(hatch_reference& ref, uint32_t loc)
    {
        grow_array(ref.count++, ref.locations) = loc;
    }
    
    module module_create()
    {
        module mod;
        
        mod.symbols_size = 0;
        mod.symbols = 0;
        
        mod.hatch_references_size = 0;
        mod.hatch_references = 0;
        
        mod.relocations_size = 0;
        mod.relocations = 0;
        
        mod.segment_size = 0;
        mod.segment = 0;
        
        mod.has_entry = false;
        mod.entry = 0;
        
        return mod;
    }
    
    void module_free(module& mod)
    {
        if (mod.segment)
            delete[] mod.segment;
        mod.segment = 0;
        mod.segment_size = 0;
        
        if (mod.hatch_references)
        {
            for (uint32_t i = 0; i < mod.hatch_references_size; ++i)
                hatch_reference_free(mod.hatch_references[i]);
            delete[] mod.hatch_references;
        }
        mod.hatch_references = 0;
        mod.hatch_references_size = 0;
        
        if (mod.relocations)
        {
            for (uint32_t i = 0; i < mod.relocations_size; ++i)
                relocation_free(mod.relocations[i]);
            delete[] mod.relocations;
        }
        mod.relocations = 0;
        mod.relocations_size = 0;
        
        if (mod.symbols)
            delete[] mod.symbols;
        mod.symbols = 0;
        mod.symbols_size = 0;
    }
    
    symbol& module_add_symbol(module& mod, symbol const& sym)
    {
        return (grow_array(mod.symbols_size++, mod.symbols) = sym);
    }
    
    symbol* module_find_symbol(module& mod, std::string const& name)
    {
        for (uint32_t i = 0; i < mod.symbols_size; ++i)
            if (mod.symbols[i].name == name)
                return mod.symbols + i;
    
        return 0;
    }
    
    relocation& module_add_relocation(module& mod, std::string const& name)
    {
        relocation& reloc = grow_array(mod.relocations_size++, mod.relocations);
        reloc = relocation_create();
        reloc.name = name;
        return reloc;
    }
    
    relocation* module_find_relocation(module& mod, std::string const& name)
    {
        for (uint32_t i = 0; i < mod.relocations_size; ++i)
            if (mod.relocations[i].name == name)
                return mod.relocations + i;
    
        return 0;
    }
    
    void module_append_relocation(module& mod, std::string const& name, uint32_t seg, uint32_t loc)
    {
        relocation* reloc = module_find_relocation(mod, name);
        if (!reloc)
            reloc = &module_add_relocation(mod, name);
        
        relocation_append(*reloc, seg, loc);
    }
    
    hatch_reference& module_add_hatch_reference(module& mod, std::string const& name)
    {
        hatch_reference& ref = grow_array(mod.hatch_references_size++, mod.hatch_references);
        ref = hatch_reference_create();
        ref.name = name;
        return ref;
    }
    
    hatch_reference* module_find_hatch_reference(module& mod, std::string const& name)
    {
        for (uint32_t i = 0; i < mod.hatch_references_size; ++i)
            if (mod.hatch_references[i].name == name)
                return mod.hatch_references + i;
    
        return 0;
    }
    
    void module_append_hatch_reference(module& mod, std::string const& name, uint32_t loc)
    {
        hatch_reference* ref = module_find_hatch_reference(mod, name);
        if (!ref)
            ref = &module_add_hatch_reference(mod, name);
        
        hatch_reference_append(*ref, loc);
    }
    
    uint32_t module_add_word(module& mod, uint32_t word)
    {
        grow_array(mod.segment_size++, mod.segment) = word;
        return mod.segment_size - 1;
    }
} }
