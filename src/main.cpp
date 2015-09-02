#include "vm_bytes.h"
#include "vm_module.h"
#include <iostream>
#include <iomanip>

#define I(instr) ((I_CODE_ ## instr) << I_CODE_SHIFT)
#define A(code, val) \
    ( (OP_CODE_ ## code << OP_A_CODE_SHIFT) \
    | ((val) << OP_A_VAL_SHIFT))
#define Ai(code, val) \
    ( (OP_CODE_ ## code << OP_A_CODE_SHIFT) \
    | (OP_A_IND) \
    | ((val) << OP_A_VAL_SHIFT))
#define B(code, val) \
    ( (OP_CODE_ ## code << OP_B_CODE_SHIFT) \
    | ((val) << OP_B_VAL_SHIFT))
#define Bi(code, val) \
    ( (OP_CODE_ ## code << OP_B_CODE_SHIFT) \
    | (OP_B_IND) \
    | ((val) << OP_B_VAL_SHIFT))

int main()
{
    using namespace vm;

    uint32_t prog[] =
    {
        I(PUSH) | A(IMM, 0), 0x12345678,
        I(POP)  | A(REG, RC_SP),
        I(MOV)  | A(REG, RC_PSR) | B(REG, RC_SP),
        I(HALT)
    };
    
    module mod;
    mod.stack = new uint32_t[64];
    mod.stackSize = 64;
    mod.program = prog;
    mod.programSize = sizeof(prog) / sizeof(uint32_t);
    mod.registers[RC_SP] = 0;
    mod.registers[RC_PC] = 0;
    
    run(mod);
    
    return 0;
}
