#include "vm_bytes.h"
#include "vm_module.h"
#include <iostream>
#include <iomanip>

#define I(instr) ((I_CODE_ ## instr) << I_CODE_SHIFT)
#define R(reg) (REG_CODE_ ## reg)
#define A(code, val) \
    ( (OP_CODE_ ## code << OP_A_CODE_SHIFT) \
    | ((val) << OP_A_VAL_SHIFT))
    #define Ai(code, val) \
    ( (OP_CODE_ ## code << OP_A_CODE_SHIFT) \
    | (OP_A_IND) \
    | ((val) << OP_A_VAL_SHIFT))
#define Aio(code, val) \
    ( (OP_CODE_ ## code << OP_A_CODE_SHIFT) \
    | (OP_A_IND) \
    | ((val) << OP_A_VAL_SHIFT) \
    | (OP_A_OFF))
#define B(code, val) \
    ( (OP_CODE_ ## code << OP_B_CODE_SHIFT) \
    | ((val) << OP_B_VAL_SHIFT))
#define Bi(code, val) \
    ( (OP_CODE_ ## code << OP_B_CODE_SHIFT) \
    | (OP_B_IND) \
    | ((val) << OP_B_VAL_SHIFT))
#define Bio(code, val) \
    ( (OP_CODE_ ## code << OP_B_CODE_SHIFT) \
    | (OP_B_IND) \
    | ((val) << OP_B_VAL_SHIFT) \
    | (OP_B_OFF))

unsigned int inc(unsigned int a)
{
    return a+1;
}

void inc_hatch(vm::module& mod)
{
    using namespace vm;
    
    uint32_t* arg = mod.stack + mod.registers[REG_CODE_SP]-1;
    unsigned int r = inc(*((unsigned int*) arg));
    mod.registers[REG_CODE_RV] = *((uint32_t*) &r);
}
    
int main1()
{
    using namespace vm;

    /* Equivalent program :
     * 
     * uint inc(uint i)
     *   return i+1;
     * 
     * uint i = 1;
     * while (i < 5)
     *   i = inc(i);
     * // dump stack and registers
     */
    
    /* Equivalent assembly :
     * 
     *   mov RV, #1
     * .again:
     *   push RV
     *   call .inc
     *   push 5
     *   ucmp
     *   jne .again
     *   dmr
     *   halt
     * 
     * .inc:
     *   push [AB]
     *   push #0
     *   uadd
     *   pop RV
     *   ret
     */
    
    // Segment 1 code
    uint32_t prog[] =
    {
        I(MOV) | A(REG, R(RV)) | B(IMM, 0),
        1,
        I(PUSH) | A(REG, R(RV)),
        // Long call to another segment
        I(CALL) | A(IMM, 0) | B(IMM, 0),
        0x01, // segment 1
        0x00, // @0
        // Hatch dive (host call)
        /*I(DIVE) | A(IMM, 0),
        0x00,*/
        I(PUSH) | A(IMM, 0),
        5,
        I(UCMP),
        I(JNE) | A(IMM, 0),
        0x02,
        I(DMR),
        I(DMS),
        I(HALT)
    };
    
    // Segment 2 code
    uint32_t prog2[] =
    {
        I(PUSH) | Ai(REG, R(AB)),
        I(PUSH) | A(IMM, 0),
        0x01,
        I(UADD),
        I(POP) | A(REG, R(RV)),
        I(RET)
    };
    
    module mod = module_create(128, 2, 1);
    
    segment prgm;
    prgm.buffer = prog;
    prgm.size = sizeof(prog) / sizeof(uint32_t);
    prgm.entry = 0;
    mod.segments[0] = &prgm;
    
    segment prgm2;
    prgm2.buffer = prog2;
    prgm2.size = sizeof(prog2) / sizeof(uint32_t);
    prgm2.entry = 0;
    mod.segments[1] = &prgm2;
    
    mod.base = 0;
    
    hatch test;
    test.entry = inc_hatch;
    mod.hatches[0] = &test;
    
    module_reset(mod);
    module_run(mod);
    
    module_free(mod);
    
    return 0;
}

#include "as_token.h"
#include "as_lexer.h"
#include "as_assembler.h"
#include <fstream>

int main()
{
    using namespace as;
    
    std::ifstream fs("sample.bas", std::ios::in);
    
    lexer lex = lexer_create(fs);
    
    /*token tok;
    while (lexer_seekt(lex) != TOKEN_EOF)
    {
        tok = lexer_get(lex);
        
        if (tok.type == TOKEN_BAD)
        {
            std::cout << "BAD" << std::endl;
            break;
        }
        else
        {
            std::cout << "[" << tok.type << "] = (" << tok.value << ")" << std::endl;
        }
    }
    
    lexer_free(lex);*/
    
    assembler ass = assembler_create(lex);
    module mod = assembler_assemble(ass);
    assembler_free(ass);
    
    return 0;
}
