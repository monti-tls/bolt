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

#include "bolt/as_linker.h"
#include <stdexcept>
#include <algorithm>

#include <iostream>

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
    
    //! Create and init temporary things in the lexer.
    void linker_temps_create(linker& ln)
    {
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            ln.objects[i].solutions_size = 0;
            ln.objects[i].solutions = 0;
            ln.objects[i].used = false;
        }
        
        for (uint32_t i = 0; i < ln.hatch_entries_size; ++i)
        {
            ln.hatch_entries[i].solutions_size = 0;
            ln.hatch_entries[i].solutions = 0;
            ln.hatch_entries[i].used = false;
        }
    }
    
    //! Free temporary stuff from the linker (tables, ...).
    void linker_temps_free(linker& ln)
    {
        for (uint32_t i = 0; i < ln.hatch_entries_size; ++i)
        {
            if (ln.hatch_entries[i].solutions)
                delete[] ln.hatch_entries[i].solutions;
            
            ln.hatch_entries[i].solutions = 0;
            ln.hatch_entries[i].solutions_size = 0;
            ln.hatch_entries[i].used = false;
        }
        
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            if (ln.objects[i].solutions)
                delete[] ln.objects[i].solutions;
            
            ln.objects[i].solutions = 0;
            ln.objects[i].solutions_size = 0;
            ln.objects[i].used = false;
        }
    }
    
    //! Add a solution to an object.
    void linker_object_add_solution(object& obj, solution const& sol)
    {
        grow_array(obj.solutions_size++, obj.solutions) = sol;
    }
    
    //! Attempt to find a solution to the relocation reloc
    //!   asked by the applicant module.
    //! The solution, if found, is written to sol, and the function returns false
    //!   if it is not found.
    //! It throws an error if multiple solutions are found.
    bool linker_find_solution_for(linker& ln, solution& sol, uint32_t applicant, relocation* reloc)
    {
        bool found = false;
        
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            if (i == applicant)
                break;
            
            if (module_find_symbol(ln.objects[i].mod, reloc->name))
            {
                if (found)
                    throw std::logic_error("as::linker_find_solution_for: symbol `" + reloc->name + "' is multiply defined");
                
                found = true;
                sol.symbol_name = reloc->name;
                sol.provider = i;
            }
        }
        
        return found;
    }
    
    //! Find all solutions to all relocations in all objects.
    //! Oh my ! So much 'all'...
    void linker_find_solutions(linker& ln)
    {
        // For each object, resolve its relocations
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            object& obj = ln.objects[i];
            
            // For each relocation
            for (uint32_t j = 0; j < obj.mod.relocations_size; ++j)
            {
                relocation& reloc = obj.mod.relocations[j];
                
                // Find the solution
                solution sol;
                if (!linker_find_solution_for(ln, sol, i, &reloc))
                    throw std::logic_error("linker_find_solutions: could not resolve symbol `" + reloc.name + "'");
                
                // Add it to the solutions table
                linker_object_add_solution(obj, sol);
            }
        }
    }
    
    //! Assign segments number to objects, skipping those
    //!   who are unused.
    //! This updates ln.objects[i].used and ln.segment_count.
    void linker_assign_segments(linker& ln)
    {
        uint32_t segment = 0;
        
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            object& obj = ln.objects[i];
            obj.used = false;
            
            // Determine if object is used
            // It is if this is our base object !
            if (i == ln.base_object)
                obj.used = true;
            // It is if it has solutions itself
            if (!obj.used && obj.solutions)
                obj.used = true;
            // Look if another module is relying on a symbol from this one
            for (uint32_t j = 0; !obj.used && j < ln.objects_size; ++j)
            {
                if (j == i)
                    continue;
                
                for (uint32_t k = 0; k < ln.objects[j].solutions_size; ++k)
                {
                    if (ln.objects[j].solutions[k].provider == i)
                    {
                        obj.used = true;
                        break;
                    }
                }
            }
            
            // If the object is not used, don't assign him a segment id
            if (!obj.used)
                continue;
            
            // Assign next segment id
            obj.segment_id = segment++;
        }
        
        ln.segments_count = segment;
    }
    
    //! Copy each used object's code into virtual core segment memory.
    void linker_copy_segments(linker& ln)
    {
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            object& obj = ln.objects[i];
            if (!obj.used)
                continue;
            
            // Create the new VCO's segment
            vm::segment* seg = new vm::segment;
            seg->size = obj.mod.segment_size;
            seg->buffer = new uint32_t[seg->size];
            seg->entry = obj.mod.entry;
            
            // Copy program code
            std::copy_n(obj.mod.segment, seg->size, seg->buffer);
            
            // Register it in the VCO
            ln.vco.segments[obj.segment_id] = seg;
        }
    }
    
    //! Apply all solutions.
    void linker_apply_solutions(linker& ln)
    {
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            object& obj = ln.objects[i];
            if (!obj.used)
                continue;
            
            for (uint32_t j = 0; j < obj.solutions_size; ++j)
            {
                // Take the solution
                solution& sol = obj.solutions[j];
                // Take the provider object
                object& provider_obj = ln.objects[sol.provider];
                
                // Fetch the associated relocation
                relocation* reloc = module_find_relocation(obj.mod, sol.symbol_name);
                
                // Get the resolved symbol's location from the provider module
                symbol* sym = module_find_symbol(provider_obj.mod, sol.symbol_name);
                
                // Get this module's associated segment memory
                uint32_t* segment = ln.vco.segments[obj.segment_id]->buffer;
                
                // Fix the relocation, segment operand and target PC operand
                for (uint32_t k = 0; k < reloc->count; ++k)
                {
                    segment[reloc->segments[k]] = provider_obj.segment_id;
                    segment[reloc->locations[k]] = sym->location;
                }
            }
        }
    }
    
    //! Find an exposed hatch entry by name.
    //! Returns 0 if not found.
    hatch_entry* linker_find_hatch(linker& ln, std::string const& name)
    {
        for (uint32_t i = 0; i < ln.hatch_entries_size; ++i)
            if (ln.hatch_entries[i].hatch.name == name)
                return ln.hatch_entries + i;
        
        return 0;
    }
    
    void linker_add_hatch_solution(hatch_entry* hte, uint32_t seg, uint32_t loc)
    {
        hatch_solution& solution = grow_array(hte->solutions_size++, hte->solutions);
        solution.segment_id = seg;
        solution.location = loc;
    }
    
    //! Resolve all hatch references from all modules.
    void linker_resolve_hatch_references(linker& ln)
    {
        uint32_t hatch = 0;
        
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            object& obj = ln.objects[i];
            if (!obj.used)
                continue;
            
            for (uint32_t j = 0; j < obj.mod.hatch_references_size; ++j)
            {
                hatch_reference& ref = obj.mod.hatch_references[j];
                
                // Find the associated entry
                hatch_entry* hte = linker_find_hatch(ln, ref.name);
                if (!hte)
                    throw std::logic_error("as::linker_resolve_hatch_references: could not resolve hatch `" + ref.name + "'");
                
                // If not yet used, give it an id
                if (!hte->used)
                {
                    hte->hatch_id = hatch++;
                    hte->used = true;
                }
                
                // Add the solution to the list
                for (uint32_t k = 0; k < ref.count; ++k)
                    linker_add_hatch_solution(hte, obj.segment_id, ref.locations[k]);
            }
        }
        
        ln.hatches_count = hatch;
    }
    
    void linker_apply_hatch_solutions(linker& ln)
    {
        for (uint32_t i = 0; i < ln.hatch_entries_size; ++i)
        {
            hatch_entry& hte = ln.hatch_entries[i];
            if (!hte.used)
                break;
            
            for (uint32_t j = 0; j < hte.solutions_size; ++j)
            {
                hatch_solution& solution = hte.solutions[j];
                
                // Apply the solution by fixing segment memory
                ln.vco.segments[solution.segment_id]->buffer[solution.location] = hte.hatch_id;
            }
        }
    }
    
    void linker_copy_hatches(linker& ln)
    {
        for (uint32_t i = 0; i < ln.hatch_entries_size; ++i)
        {
            hatch_entry& hte = ln.hatch_entries[i];
            if (!hte.used)
                break;
            
            // Copy the hatch structure into the VCO's hatch table
            ln.vco.hatches[hte.hatch_id] = new vm::hatch;
            ln.vco.hatches[hte.hatch_id]->name = hte.hatch.name;
            ln.vco.hatches[hte.hatch_id]->entry = hte.hatch.entry;
        }
    }
    
    //! Find the default entry module.
    //! This searchs for the only module with a .entry directive.
    //! If multiple modules uses .entry directive, an error will be thrown.
    uint32_t linker_find_default_entry_module(linker& ln)
    {
        bool found = false;
        uint32_t id = 0;
        
        for (uint32_t i = 0; i < ln.objects_size; ++i)
        {
            object& obj = ln.objects[i];
            
            if (obj.mod.has_entry)
            {
                if (found)
                    throw std::logic_error("as::linker_find_default_entry_module: multiple entry points defined");
                
                found = true;
                id = i;
            }
        }
        
        if (!found)
            throw std::logic_error("as::linker_find_default_entry_module: no entry point found");
        
        return id;
    }
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    linker linker_create()
    {
        linker ln;
        
        ln.objects_size = 0;
        ln.objects = 0;
        
        ln.hatch_entries_size = 0;
        ln.hatch_entries = 0;
        
        return ln;
    }
    
    void linker_free_modules(linker& ln)
    {
        for (uint32_t i = 0; i < ln.objects_size; ++i)
            module_free(ln.objects[i].mod);
    }
    
    void linker_free(linker& ln)
    {
        if (ln.objects)
            delete[] ln.objects;
        ln.objects = 0;
        ln.objects_size = 0;
        
        if (ln.hatch_entries)
            delete[] ln.hatch_entries;
        ln.hatch_entries = 0;
        ln.hatch_entries_size = 0;
    }
    
    int linker_add_module(linker& ln, module const& mod)
    {
        object& obj = grow_array(ln.objects_size, ln.objects);
        obj.mod = mod;
        obj.solutions_size = 0;
        obj.solutions = 0;
        
        return ln.objects_size++;
    }
    
    void linker_add_hatch(linker& ln, vm::hatch const& hatch)
    {
        hatch_entry& hte = grow_array(ln.hatch_entries_size++, ln.hatch_entries);
        hte.hatch = hatch;
        hte.solutions_size = 0;
        hte.solutions = 0;
    }
    
    vm::core linker_link(linker& ln, int base)
    {
        // Init temp stuff
        linker_temps_create(ln);
        
        // Set up the base object id so other functions can see it
        //   (notably assign_segments, that could mark it as unused otherwise)
        if (base < 0)
            ln.base_object = linker_find_default_entry_module(ln);
        else
            ln.base_object = base;
        
        // Find all solutions to all relocations
        linker_find_solutions(ln);
        
        // Assign segment numbers, discard unused modules
        linker_assign_segments(ln);
        
        // Resolt hatch references
        linker_resolve_hatch_references(ln);
        
        // Create the output virtual core
        //FIXME: determine stack size automatically !
        //FIXME: determine heap size automatically !
        ln.vco = vm::core_create(1024, 1024, ln.segments_count, ln.hatches_count);
        
        // Copy all code segments in virtual core's segment memory
        linker_copy_segments(ln);
        
        // Apply all solutions
        linker_apply_solutions(ln);
        
        // Copy hatches to VCO's memory
        linker_copy_hatches(ln);
        
        // Apply hatch solutions to segment memory
        linker_apply_hatch_solutions(ln);
        
        // Set VCO's base segment
        ln.vco.base = ln.objects[ln.base_object].segment_id;
        
        // Release temporaries
        linker_temps_free(ln);
        
        return ln.vco;
    }
}
