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

void print(unsigned int n, unsigned int fac)
{
    std::cout << n << "! = " << fac << std::endl;
}

void print_hatch(vm::core& vco)
{
    using namespace vm;
    
    uint32_t* arg0 = vco.stack + vco.registers[REG_CODE_SP]-2;
    uint32_t* arg1 = vco.stack + vco.registers[REG_CODE_SP]-1;
    print(*((unsigned int*) arg0), *((unsigned int*) arg1));
}

int main()
{
    using namespace as;
    using namespace vm;
    
    try
    {
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
        
        uint32_t lib_id = linker_add_module(ln, mod_lib);
        uint32_t main_id = linker_add_module(ln, mod_main);
        
        core vco = linker_link(ln, main_id);
        
        linker_free(ln);
        
        module_free(mod_lib);
        module_free(mod_main);
    }
    catch(std::exception const& exc)
    {
        std::cerr << exc.what() << std::endl;
    }
    
    return 0;
}
