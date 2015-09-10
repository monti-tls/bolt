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

#ifndef BOLT_RUN_RUNTIME_H
#define BOLT_RUN_RUNTIME_H

#include "bolt/vm_core.h"
#include "bolt/as_linker.h"
#include "bolt/run_details.h"
#include <string>

//!
//! run_runtime
//!

//! This module defines the Bolt standard library, as well as an
//!   automatic hatch generator.

namespace bolt { namespace run
{
    //! Generate a hatch from a function's signature and pointer.
    //! For example, to bind "int foo(int)" with name "foo", do :
    //!   runtime_generate_hatch<int(*)(int), &foo>("foo");
    //! Note that S is a function *pointer* type, so
    //!   using S = int(int) will *not* work.
    template <typename S, S function_ptr>
    vm::hatch runtime_generate_hatch(std::string const& name)
    {
        vm::hatch htc;
        htc.name = name;
        htc.entry = &details::bound_exposer<S, function_ptr>::work;
        return htc;
    }
    
    //! Expose the Bolt's runtime library to a linker.
    void runtime_expose(as::linker& ln);
} }

#endif // BOLT_RUN_RUNTIME_H
