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

//!
//! vm_registers
//!
 
//! This file defines the Bolt virtual core's register set.
//! It is included  by :
//!   vm_module.h:  defines the REG_CODE_* constants
//!   as_layer.cpp: defines the assembler's register names

//! R0-R9: general purpose registers
//! IR:    instruction register
//! PC:    program counter
//! SEG:   segment register
//! SP:    stack pointer
//! PSR:   program status register
//! RV:    return value register
//! AB:    arguments base register, contains the base address of the arguments when in a called function
//! HB:    heap base register, contains the heap's base address (i.e. the end of the stack)
//!        you can modify this but this is tricky

DECL_REGISTER(R0,  0x00)
DECL_REGISTER(R1,  0x01)
DECL_REGISTER(R2,  0x02)
DECL_REGISTER(R3,  0x03)
DECL_REGISTER(R4,  0x04)
DECL_REGISTER(R5,  0x05)
DECL_REGISTER(R6,  0x06)
DECL_REGISTER(R7,  0x07)
DECL_REGISTER(R8,  0x08)
DECL_REGISTER(R9,  0x09)

DECL_REGISTER(IR,  0x0A)
DECL_REGISTER(SEG, 0x0B)
DECL_REGISTER(PC,  0x0C)
DECL_REGISTER(SP,  0x0D)
DECL_REGISTER(PSR, 0x0E)
DECL_REGISTER(RV,  0x0F)
DECL_REGISTER(AB,  0x10)
DECL_REGISTER(HB,  0x11)
