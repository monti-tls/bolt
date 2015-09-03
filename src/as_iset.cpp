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

#include "as_iset.h"
#include "vm_bytes.h"

namespace as
{
    /*************************************/
    /*** Private module implementation ***/
    /*************************************/
    
    //! A small flag macro helper, to avoid
    //!   typing ISET_OP_FLAG_* everytime.
    #define F(flag) ISET_OP_FLAG_ ## flag
    //! This macro defines the behavior of the declarations in iset.inc,
    //!   here we use it to extract mnemonic, icode and operand
    //!   flags data to our iset_entry array.
    #define DECL_INSTR(group, name, code, a, b) \
        { \
            .mnemonic = #name, \
            .icode = vm::I_CODE_ ## name, \
            .aflags = (a), \
            .bflags = (b) \
        },
    
    static iset_entry iset[] =
    {
        #include "iset.inc"
    };
    
    #undef DECL_INSTR
    #undef F
    
    //! The length of the above array, used for loops.
    static unsigned int iset_size = sizeof(iset) / sizeof(iset_entry);
    
    //! Compare two string without sensitivity to case.
    //! We use this as the mnemonics in iset are probably in
    //!   uppercase, while I like to program in lowercase.
    static bool case_compare(std::string const& a, std::string const& b)
    {
        if (a.size() != b.size())
            return false;
        
        for (unsigned int i = 0; i < a.size(); ++i)
            if (std::tolower(a[i]) != std::tolower(b[i]))
                return false;
            
        return true;
    }
    
    /*************************/
    /*** Public module API ***/
    /*************************/
    
    bool iset_exists(std::string const& mnemonic)
    { return iset_find(mnemonic); }
    
    iset_entry* iset_find(std::string const& mnemonic)
    {
        for (unsigned int i = 0; i < iset_size; ++i)
            if (case_compare(mnemonic, iset[i].mnemonic))
                return iset + i;
        
        return 0;
    }
}
