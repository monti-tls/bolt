#include "vm_bytes.h"
#include "vm_core.h"
#include "as_token.h"
#include "as_lexer.h"
#include "as_assembler.h"
#include "as_linker.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cstdio>
#include <stdexcept>

void putch(int ch)
{ std::cout << (char) ch; }

void h_putch(vm::core& vco)
{ putch(*((int*) vco.stack + vco.registers[vm::REG_CODE_SP]-1)); }

#define HATCH_NAME(nm) \
    h_ ## nm ## _hatch

#define DECL_HATCH(nm) \
    static vm::hatch HATCH_NAME(nm) = { .name = #nm, .entry = h_ ## nm };

DECL_HATCH(putch)

int main()
{
    using namespace as;
    using namespace vm;
    
    try
    {
        /*** Assemble modules ***/
        
        module mod_lib, mod_main;
    
        {
            std::ifstream fs("scratch/lib.bas", std::ios::in);
            lexer lex = lexer_create(fs);
            assembler ass = assembler_create(lex);
            mod_lib = assembler_assemble(ass);
            assembler_free(ass);
            lexer_free(lex);
        }
        
        {
            std::ifstream fs("scratch/main.bas", std::ios::in);
            lexer lex = lexer_create(fs);
            assembler ass = assembler_create(lex);
            mod_main = assembler_assemble(ass);
            assembler_free(ass);
            lexer_free(lex);
        }
        
        linker ln = linker_create();
        
        /*** Add modules to link together ***/
        
        linker_add_module(ln, mod_lib);
        uint32_t main_id = linker_add_module(ln, mod_main);
        
        /*** Expose hatches ***/
        
        linker_add_hatch(ln, HATCH_NAME(putch));
        
        /*** Link modules ***/
        
        core vco = linker_link(ln, main_id);
        linker_free_modules(ln);
        linker_free(ln);
        
        /*** Run program ***/
        
        core_reset(vco);
        core_run(vco);
        
        /*** Release core ***/
        
        core_free_hatches(vco);
        core_free_segments(vco);
        core_free(vco);
    }
    catch(std::exception const& exc)
    {
        std::cerr << exc.what() << std::endl;
    }
    
    return 0;
}
