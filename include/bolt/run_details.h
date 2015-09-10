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

#include "bolt/vm_core.h"
#include <functional>

//!
//! run_details
//!

//! This file contains implementation details for the
//!   host function exposing mechanism.
//! Do NOT include this file, and do NOT use the run::details:: contents.
//! The public API for this module resides in run_runtime.h.

namespace bolt { namespace run
{
    namespace details
    {
        //! Used to get the size of a type in Bolt's word increments.
        //! Uses a divUp to support types smaller than 32-bits (e.g char and short).
        template <typename T>
        struct type_size
        { enum { value = (sizeof(T) + sizeof(uint32_t) - 1U) / sizeof(uint32_t) }; };
        
        //! A core wrapper used to keep track of actual position in the stack.
        struct core_wrapper
        {
            vm::core& vco;
            int position;
        };
        
        //! Used to extract arguments from a vm::core stack.
        template <typename T>
        struct argument_extractor
        {
            static T work(core_wrapper& cwr)
            {
                // We use an union to fool GCC about pointer aliasing
                union
                {
                    uint32_t* raw_ptr;
                    T* arg_ptr;
                };
                
                // Compute the offset for this argument
                unsigned int offset = cwr.vco.registers[vm::REG_CODE_SP] - type_size<T>::value;
                offset -= cwr.position;
                // Get the corresponding argument's location in the stack
                raw_ptr = cwr.vco.stack + offset;
                // Advance the core wrapper's position for the next argument extraction
                cwr.position += type_size<T>::value;
                
                return *arg_ptr;
            }
        };
        
        //! Specialization for pointers, that must be redirected to the core's stack/heap.
        template <typename T>
        struct argument_extractor<T*>
        {
            static T* work(core_wrapper& cwr)
            {
                // Compute the offset for the argument (pointers are always 1 word)
                unsigned int offset = cwr.vco.registers[vm::REG_CODE_SP] - 1;
                offset -= cwr.position;
                // Get its location on the stack
                uint32_t* address = cwr.vco.stack + offset;
                // Advance position
                cwr.position += 1;
                
                // Fool GCC about pointer aliasing
                union
                {
                    uint32_t* raw_ptr;
                    T* arg_ptr;
                };
                
                // Follow the indirection
                raw_ptr = cwr.vco.stack + *address;
                return arg_ptr;
            }
        };
        
        //! This trick is needed as the static_assert will be always triggered
        //!   if it does not relies on a dependent name.
        template <bool B, typename...>
        struct dependent_bool_type : std::integral_constant<bool, B> {};
        template <bool B, typename... T>
        using dependent_bool = typename dependent_bool_type<B, T...>::type;
        
        //! We use the dependent_bool trick to prevent the static_assert to
        //!   be always triggered.
        template <typename S>
        struct exposer
        { static_assert(dependent_bool<false, S>::value, "run::detail::exposer: invalid function signature"); };
        
        //! The invoker structure for non-void returns.
        //! H is decltype from a std::bind.
        template <typename R, typename H>
        struct invoker
        {
            static void work(H handler, vm::core& vco)
            { vco.registers[vm::REG_CODE_RV] = handler(); }
        };
        
        //! The invoker structure for void returns (that does not
        //!   overwrites the %rv register.
        template <typename H>
        struct invoker<void, H>
        {
            static void work(H handler, vm::core&)
            { handler(); }
        };
        
        //! Defines a static function that invokes a generic function pointer
        //!   extracting arguments from the vm::core's stack.
        template <typename R, typename... Args>
        struct exposer<R(*)(Args...)>
        {
            static void work(vm::core& vco, R(*function_ptr)(Args...))
            {
                // The unused attribute silents the warning for functions
                //   with no arguments.
                core_wrapper __attribute__((unused)) cwr = { vco, 0 };
                
                // Bind the extracted arguments to the function pointer
                auto handler = std::bind(*function_ptr, argument_extractor<Args>::work(cwr)...);
                
                // Invoke the bound function, taking care of the return value
                invoker<R, decltype(handler)>::work(handler, vco);
            }
        };
        
        //! An exposer bound to a function pointer.
        template <typename S, S function_ptr>
        struct bound_exposer
        {
            static void work(vm::core& vco)
            { exposer<S>::work(vco, function_ptr); }
        };
    }
} }
