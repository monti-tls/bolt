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

#include "bolt/as_assembler.h"
#include "bolt/as_linker.h"
#include "bolt/run_runtime.h"
#include "bolt/vm_core.h"

#include <lconf/cli.h>
#include <fstream>

int main(int argc, char** argv)
{
    using namespace as;
    using namespace vm;
    using namespace run;
    
    /****************************************/
    /*** Command-line options definitions ***/
    /****************************************/
    
    lconf::cli::Parser options(argc, argv);
    
    options.setProgramDescription("The Bolt assembler, linker & interpreter.");
    
    options.addSwitch('h', "help")
           .setStop()
           .setDescription("Print this help");
           
    options.addSwitch('x', "no-std-lib")
           .setDescription("Do not use the Bolt standard library");
           
    options.addSwitch('a', "assemble-only")
           .setDescription("Only assemble the input modules, do not link nor run them");
           
    options.addSwitch('l', "link-only")
           .setDescription("Only assemble and link the input modules, do not run them");
    
    /************************************/
    /*** Options parsing and checking ***/
    /************************************/
           
    try
    {
        options.parse();
    }
    catch(std::exception const& exc)
    {
        std::cerr << "Error: " << exc.what() << std::endl;
        return -1;
    }
    
    if (options.has("help"))
    {
        options.showHelp();
        return 0;
    }
    
    if (!options.arguments().size())
    {
        std::cerr << "Error: Must specify at least a module to assemble & link." << std::endl;
        std::cerr << "       Try " << argv[0] << " --help." << std::endl;
        return -1;
    }
    
    if (options.has("link-only") && options.has("assemble-only"))
    {
        std::cerr << "Warning: Both --link-only and --assemble-only were specified." << std::endl;
        std::cerr << "         I will stop right after assembling." << std::endl;
    }
    
    if (options.has("no-std-lib") && options.has("assemble-only"))
    {
        std::cerr << "Warning: --no-std-lib has no effect while assembling only" << std::endl;
    }
    
    /******************/
    /*** Assembling ***/
    /******************/
    
    std::vector<module> modules;
    
    for (unsigned int i = 0; i < options.arguments().size(); ++i)
    {
        std::string const& fn = options.arguments()[i];
        
        try
        {
            std::ifstream fs(fn, std::ios::in);
            if (!fs)
                throw std::logic_error("Unable to open file");
            
            lexer lex = lexer_create(fs);
            assembler ass = assembler_create(lex);
            module mod = assembler_assemble(ass);
            assembler_free(ass);
            lexer_free(lex);
            
            modules.push_back(mod);
        }
        catch (std::exception const& exc)
        {
            std::cerr << "Error in \"" << fn << "\": " << exc.what() << std::endl;
            return -1;
        }
    }
    
    if (options.has("assemble-only"))
    {
        for (unsigned int i = 0; i < modules.size(); ++i)
                module_free(modules[i]);
        
        return 0;
    }
    
    /***************/
    /*** Linking ***/
    /***************/
    
    core vco;
    
    try
    {
        linker ln = linker_create();
        
        //! Add all modules to the linker.
        for (unsigned int i = 0; i < modules.size(); ++i)
            linker_add_module(ln, modules[i]);
        
        //! Expose the runtime, unless asked otherwise.
        if (!options.has("no-std-lib"))
            runtime_expose(ln);
        
        //! Link the virtual core.
        vco = linker_link(ln);
        
        //! Release all linking stuff.
        linker_free_modules(ln);
        linker_free(ln);
    }
    catch (std::exception const& exc)
    {
        std::cerr << "Error: " << exc.what() << std::endl;
        return -1;
    }
    
    if (options.has("link-only"))
    {
        core_free_hatches(vco);
        core_free_segments(vco);
        core_free(vco);
        
        return 0;
    }
    
    /*****************/
    /*** Execution ***/
    /*****************/
    
    try
    {
        core_reset(vco);
        core_run(vco);
    }
    catch (std::exception const& exc)
    {
        std::cerr << "Error: " << exc.what() << std::endl;
        return -1;
    }
    
    core_free_hatches(vco);
    core_free_segments(vco);
    core_free(vco);
    
    return 0;
}
