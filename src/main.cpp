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

int main()
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
    
    uint32_t prog[] =
    {
        I(MOV) | A(REG, R(RV)) | B(IMM, 0),
        1,
        I(PUSH) | A(REG, R(RV)),
        I(CALL) | A(IMM, 0),
        0x0D,
        I(PUSH) | A(IMM, 0),
        5,
        I(UCMP),
        I(JNE) | A(IMM, 0),
        0x02,
        I(DMR),
        I(DMS),
        I(HALT),
        
        I(PUSH) | Ai(REG, R(AB)),
        I(PUSH) | A(IMM, 0),
        0x01,
        I(UADD),
        I(POP) | A(REG, R(RV)),
        I(RET)
    };
    
    module mod = module_create(128);
    
    mod.header.program_size = sizeof(prog) / sizeof(uint32_t);
    mod.header.entry = 0;
    mod.program = prog;
    
    module_reset(mod);
    module_run(mod);
    
    module_free(mod);
    
    return 0;
}
