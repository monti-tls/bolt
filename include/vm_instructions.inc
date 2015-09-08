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
 
//!
//! vm_instructions
//!
 
//! This file defines the Bolt virtual instruction set.
//! It is included by several files, namely :
//!   vm_bytes.h:   defines I_CODE_* constants.
//!   vm_layer.cpp: instruction set mnemonic maps and
//!                 allowed operands check flags.

//! Here is the format :
//!   GROUP NAME value (instr flags) (allowed op A flags) (allowed op B flags)
//! See as_layer.h for available flags and their meaning.
//! See vm_bytes.h for additional information about instruction groups and encoding.

DECL_INSTR(SYS,   HALT, 0x01, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(SYS,   RST,  0x02, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(SYS,   DMS,  0x03, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(SYS,   DMR,  0x04, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(SYS,   DMO,  0x05, I(NONE),  F(ALL),          F(NONE))

DECL_INSTR(MEM,   PUSH, 0x01, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(MEM,   POP,  0x02, I(NONE),  F(ALL) | F(OPT), F(NONE))
DECL_INSTR(MEM,   DUP,  0x03, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(MEM,   MOV,  0x04, I(NONE),  F(ALL),          F(ALL))
DECL_INSTR(MEM,   LOAD, 0x05, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(MEM,   STOR, 0x06, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(MEM,   CST,  0x07, I(LONG),  F(ALL) | F(OPT), F(ALL) | F(OPT))

DECL_INSTR(FLOW,  CALL, 0x01, I(LONG),  F(ALL),          F(ALL) | F(OPT))
DECL_INSTR(FLOW,  RET,  0x02, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(FLOW,  DIVE, 0x03, I(HATCH), F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JMP,  0x04, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JZ,   0x05, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JNZ,  0x06, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JE,   0x07, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JNE,  0x08, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JL,   0x09, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JLE,  0x0A, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JG,   0x0B, I(NONE),  F(ALL),          F(NONE))
DECL_INSTR(FLOW,  JGE,  0x0C, I(NONE),  F(ALL),          F(NONE))

DECL_INSTR(ARITH, UADD, 0x01, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, USUB, 0x02, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, UMUL, 0x03, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, UDIV, 0x04, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, UCMP, 0x05, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, IADD, 0x06, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, ISUB, 0x07, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, IMUL, 0x08, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, IDIV, 0x09, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, ICMP, 0x0A, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, FADD, 0x0B, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, FSUB, 0x0C, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, FMUL, 0x0D, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, FDIV, 0x0E, I(NONE),  F(NONE),         F(NONE))
DECL_INSTR(ARITH, FCMP, 0x0F, I(NONE),  F(NONE),         F(NONE))
