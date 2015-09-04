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
 
//! This file defines the Bolt virtual core's register set.
//! It is included  by :
//!   vm_module.h:  defines the REG_CODE_* constants
//!   as_layer.cpp: defines the assembler's register names

//! IR:  instruction register
//! PC:  program counter
//! SEG: segment register
//! SP:  stack pointer
//! PSR: program status register
//! RV:  return value register
//! AB:  arguments base register

DECL_REGISTER(IR,  0x00)
DECL_REGISTER(SEG, 0x01)
DECL_REGISTER(PC,  0x02)
DECL_REGISTER(SP,  0x03)
DECL_REGISTER(PSR, 0x04)
DECL_REGISTER(RV,  0x05)
DECL_REGISTER(AB,  0x06)
