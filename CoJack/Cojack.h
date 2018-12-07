/* OeyEnc - Outlook Express plugin for decoding messages encoded with
 * the yEnc format.
 *
 * Copyright (C) 2004-2008 Jeremy Boschen. All rights reserved.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 *  Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 * claim that you wrote the original software. If you use this software in
 * a product, an acknowledgment in the product documentation would be
 * appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not
 * be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 */

/*  Cojack.h
 *    General purpose function hooks for x86 and x64
 */

#pragma once

#if !defined(_M_IX86) && !defined(_M_X64)
   #error Cojack requires X86 or AMD64
#endif

#include <limits.h>
#include <stddef.h>

C_ASSERT(1 == sizeof(CHAR));
C_ASSERT(2 == sizeof(SHORT));
C_ASSERT(4 == sizeof(LONG));
C_ASSERT(8 == sizeof(LONGLONG));

#pragma warning( push )
#pragma warning( disable : 4201 4127 )
#pragma strict_gs_check(on)

/* 
   Experimental: Include support in the disassembler for SSE/3DNow!  
 */
#define _COJACK_TARGET_MEDIATABLES

/**********************************************************************
    
    Cojack : Instruction Tables

 **********************************************************************/
/* Things must be 1 byte aligned */
#pragma pack( push )
#pragma pack( 1 )

/**
 * OPCODE_INFO
 *    1-byte bitfield of opcode encoding information
 *
 * Remarks
 *    There are a few implied rules for each instruction table.
 *
 *    1) This is not a listing dissassembler. Its only purpose is
 *       to move a section of machine code to a different location
 *       in memory while preserving 2 rules. A) Entire instructions
 *       are copied, including immediate bytes, addresses, etc. B) The
 *       functionality of each instruction is identical in the new
 *       location, which primarily deals with relative displacements.
 *    2) The instruction's opcode is its index within the table, not
 *       including any mandatory prefix.
 *    3) The instruction size is the smallest possible size, which
 *       is always the 16bit size. 
 *    4) The instruction size is the total size of the instruction
 *       and any immediate bytes encoded with it, excluding any
 *       ModRM and/or SIB bytes.
 *    5) An instruction's size does not change between operating modes
 *       unless explicitly identified as such. If it does, it does so 
 *       by either 2 bytes for 32bit and 6 bytes for 64bit mode.
 *    6) The base size of all instructions is 1 byte, so that the offset to
 *       a relative target value is always 1 byte, excluding any mandatory 
 *       and/or optional prefixes.
 *    7) Either X86 or AMD64 instruction sets are used, not both as the x64
 *       versions of Windows do not support both 32bit & 64bit code in the 
 *       same process.
 *    8) Table names are "__InstructionTable+3-byte-madatory-prefix". So
 *       instructions which use no prefix are in __InstructionTable000000,
 *       those that use 66h 0Fh 0Fh are in __InstructionTable660F0F, etc.
 *    9) In most cases, instructions that use ModRM.reg encoding share 
 *       records with instructions that use the same opcode byte. None of
 *       the instructions receive special processing unless one of them
 *       uses different operands than the rest. See opcode FFh in table
 *       __InstructionTable000000 for an example.
 *
 *    Some instructions may be missing and/or represented incorrectly. In
 *    general, if an instruction is commonly used, its matching record in
 *    these tables is correct.
 */
typedef struct _OPCODE_INFO
{
   BYTE Size       : 3;/* Size in bytes of opcode+imm             */
   BYTE ModRM      : 1;/* The opcode uses ModRM encoding          */

   BYTE Extend32   : 1;/* Immediate bytes are extended to 32bits  */
   BYTE Extend64   : 1;/* Immediate bytes are extended to 64bits  */

   BYTE IsAddress  : 1;/* Operands are address-sized              */
   BYTE IsRelative : 1;/* Operands are rIP-relative displacements */
}OPCODE_INFO, *POPCODE_INFO;

#pragma region InstructionTables
/**
 * __InstructionTable000000
 *    1 byte instructions, 00h - FFh
 */
static const OPCODE_INFO __InstructionTable000000[] =
{
/*00h*/{1, 1, 0, 0, 0, 0},    // ADD/r reg/mem8,reg8
/*01h*/{1, 1, 0, 0, 0, 0},    // ADD/r reg/mem16,reg16
                              // ADD/r reg/mem32,reg32
                              // ADD/r reg/mem64,reg64
/*02h*/{1, 1, 0, 0, 0, 0},    // ADD/r reg8,reg/mem8
/*03h*/{1, 1, 0, 0, 0, 0},    // ADD/r reg16,reg/mem16
                              // ADD/r reg32,reg/mem32
                              // ADD/r reg64,reg/mem64
/*04h*/{2, 0, 0, 0, 0, 0},    // ADD/ib AL,imm8
/*05h*/{3, 0, 1, 0, 0, 0},    // ADD/iw AX,imm16
                              // ADD/id EAX,imm32
                              // ADD/id RAX,imm32
#ifdef _M_X64
/*06h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*07h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*06h*/{1, 0, 0, 0, 0, 0},    // PUSH ES
/*07h*/{1, 0, 0, 0, 0, 0},    // POP ES
#endif /* _M_X64 */
/*08h*/{1, 1, 0, 0, 0, 0},    // OR/r reg/mem8,reg8
/*09h*/{1, 1, 0, 0, 0, 0},    // OR/r reg/mem16,reg16
                              // OR/r reg/mem32,reg32
                              // OR/r reg/mem64,reg64
/*0Ah*/{1, 1, 0, 0, 0, 0},    // OR/r reg8,reg/mem8
/*0Bh*/{1, 1, 0, 0, 0, 0},    // OR/r reg16,reg/mem16
                              // OR/r reg32,reg/mem32
                              // OR/r reg64,reg/mem64
/*0Ch*/{2, 0, 0, 0, 0, 0},    // OR/ib AL,imm8
/*0Dh*/{3, 0, 1, 0, 0, 0},    // OR/iw AX,imm16
                              // OR/id EAX,imm32
                              // OR/id RAX,imm32
#ifdef _M_X64
/*0Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*0Eh*/{1, 0, 0, 0, 0, 0},    // PUSH CS
#endif /* _M_X64 */
/*0Fh*/{0, 0, 0, 0, 0, 0},    // TWO-BYTE-OPCODE-BRANCH
/*10h*/{1, 1, 0, 0, 0, 0},    // ADC/r reg/mem8,reg8
/*11h*/{1, 1, 0, 0, 0, 0},    // ADC/r reg/mem16,reg16
                              // ADC/r reg/mem32,reg32
                              // ADC/r reg/mem64,reg64
/*12h*/{1, 1, 0, 0, 0, 0},    // ADC/r reg8,reg/mem8
/*13h*/{1, 1, 0, 0, 0, 0},    // ADC/r reg16,reg/mem16
                              // ADC/r reg32,reg/mem32
                              // ADC/r reg64,reg/mem64
/*14h*/{2, 0, 0, 0, 0, 0},    // ADC/ib AL,imm8
/*15h*/{3, 0, 1, 0, 0, 0},    // ADC/iw AX,imm16
                              // ADC/id EAX,imm32
                              // ADC/id RAX,imm32
#ifdef _M_X64
/*16h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*17h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*16h*/{1, 0, 0, 0, 0, 0},    // PUSH SS
/*17h*/{1, 0, 0, 0, 0, 0},    // POP SS
#endif /* _M_X64 */
/*18h*/{1, 1, 0, 0, 0, 0},    // SBB/r reg/mem8,reg8
/*19h*/{1, 1, 0, 0, 0, 0},    // SBB/r reg/mem16,reg16
                              // SBB/r reg/mem32,reg32
                              // SBB/r reg/mem64,reg64
/*1Ah*/{1, 1, 0, 0, 0, 0},    // SBB/r reg8,reg/mem8
/*1Bh*/{1, 1, 0, 0, 0, 0},    // SBB/r reg16,reg/mem16
                              // SBB/r reg32,reg/mem32
                              // SBB/r reg64,reg/mem64
/*1Ch*/{2, 0, 0, 0, 0, 0},    // SBB/ib AL,imm8
/*1Dh*/{3, 0, 1, 0, 0, 0},    // SBB/iw AX,imm16
                              // SBB/id EAX,imm32
                              // SBB/id RAX,imm32
#ifdef _M_X64
/*1Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*1Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*1Eh*/{1, 0, 0, 0, 0, 0},    // PUSH DS
/*1Fh*/{1, 0, 0, 0, 0, 0},    // POP DS
#endif /* _M_X64 */
/*20h*/{1, 1, 0, 0, 0, 0},    // AND/r reg/mem8,reg8
/*21h*/{1, 1, 0, 0, 0, 0},    // AND/r reg/mem16,reg16
                              // AND/r reg/mem32,reg32
                              // AND/r reg/mem64,reg64
/*22h*/{1, 1, 0, 0, 0, 0},    // AND/r reg8,reg/mem8
/*23h*/{1, 1, 0, 0, 0, 0},    // AND/r reg16,reg/mem16
                              // AND/r reg32,reg/mem32
                              // AND/r reg64,reg/mem64
/*24h*/{2, 0, 0, 0, 0, 0},    // AND/ib AL,imm8
/*25h*/{3, 0, 1, 0, 0, 0},    // AND/iw AX,imm16
                              // AND/id EAX,imm32
                              // AND/id RAX,imm32
#ifdef _M_X64
/*26h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*27h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*26h*/{1, 0, 0, 0, 0, 0},    // ES:
/*27h*/{1, 0, 0, 0, 0, 0},    // DAA
#endif /* _M_X64 */
/*28h*/{1, 1, 0, 0, 0, 0},    // SUB/r reg/mem8,reg8
/*29h*/{1, 1, 0, 0, 0, 0},    // SUB/r reg/mem16,reg16
                              // SUB/r reg/mem32,reg32
                              // SUB/r reg/mem64,reg64
/*2Ah*/{1, 1, 0, 0, 0, 0},    // SUB/r reg8,reg/mem8
/*2Bh*/{1, 1, 0, 0, 0, 0},    // SUB/r reg16,reg/mem16
                              // SUB/r reg32,reg/mem32
                              // SUB/r reg64,reg/mem64
/*2Ch*/{2, 0, 0, 0, 0, 0},    // SUB/ib AL,imm8
/*2Dh*/{3, 0, 1, 0, 0, 0},    // SUB/iw AX,imm16
                              // SUB/id EAX,imm32
                              // SUB/id RAX,imm32
#ifdef _M_X64
/*2Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*2Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*2Eh*/{1, 0, 0, 0, 0, 0},    // CS:
/*2Fh*/{1, 0, 0, 0, 0, 0},    // DAS
#endif /* _M_X64 */
/*30h*/{1, 1, 0, 0, 0, 0},    // XOR/r reg/mem8,reg8
/*31h*/{1, 1, 0, 0, 0, 0},    // XOR/r reg/mem16,reg16
                              // XOR/r reg/mem32,reg32
                              // XOR/r reg/mem64,reg64
/*32h*/{1, 1, 0, 0, 0, 0},    // XOR/r reg8,reg/mem8
/*33h*/{1, 1, 0, 0, 0, 0},    // XOR/r reg16,reg/mem16
                              // XOR/r reg32,reg/mem32
                              // XOR/r reg64,reg/mem64
/*34h*/{2, 0, 0, 0, 0, 0},    // XOR/ib AL,imm8
/*35h*/{3, 0, 1, 0, 0, 0},    // XOR/iw AX,imm16
                              // XOR/id EAX,imm32
                              // XOR/id RAX,imm32
#ifdef _M_X64
/*36h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*37h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*36h*/{1, 0, 0, 0, 0, 0},    // SS:
/*37h*/{1, 0, 0, 0, 0, 0},    // AAA
#endif /* _M_X64 */
/*38h*/{1, 1, 0, 0, 0, 0},    // CMP/r reg/mem8,reg8
/*39h*/{1, 1, 0, 0, 0, 0},    // CMP/r reg/mem16,reg16
                              // CMP/r reg/mem32,reg32
                              // CMP/r reg/mem64,reg64
/*3Ah*/{1, 1, 0, 0, 0, 0},    // CMP/r reg8,reg/mem8
/*3Bh*/{1, 1, 0, 0, 0, 0},    // CMP/r reg16,reg/mem16
                              // CMP/r reg32,reg/mem32
                              // CMP/r reg64,reg/mem64
/*3Ch*/{2, 0, 0, 0, 0, 0},    // CMP/ib AL,imm8
/*3Dh*/{3, 0, 1, 0, 0, 0},    // CMP/iw AX,imm16
                              // CMP/id EAX,imm32
                              // CMP/id RAX,imm32
#ifdef _M_X64
/*3Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*3Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*40h*/{1, 0, 0, 0, 0, 0},    // REX
/*41h*/{1, 0, 0, 0, 0, 0},    // REX
/*42h*/{1, 0, 0, 0, 0, 0},    // REX
/*43h*/{1, 0, 0, 0, 0, 0},    // REX
/*44h*/{1, 0, 0, 0, 0, 0},    // REX
/*45h*/{1, 0, 0, 0, 0, 0},    // REX
/*46h*/{1, 0, 0, 0, 0, 0},    // REX
/*47h*/{1, 0, 0, 0, 0, 0},    // REX
/*48h*/{1, 0, 0, 0, 0, 0},    // REX
/*49h*/{1, 0, 0, 0, 0, 0},    // REX
/*4Ah*/{1, 0, 0, 0, 0, 0},    // REX
/*4Bh*/{1, 0, 0, 0, 0, 0},    // REX
/*4Ch*/{1, 0, 0, 0, 0, 0},    // REX
/*4Dh*/{1, 0, 0, 0, 0, 0},    // REX
/*4Eh*/{1, 0, 0, 0, 0, 0},    // REX
/*4Fh*/{1, 0, 0, 0, 0, 0},    // REX
#else /* _M_X64 */
/*3Eh*/{1, 0, 0, 0, 0, 0},    // DS:
/*3Fh*/{1, 0, 0, 0, 0, 0},    // AAS
/*40h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (AX)
                              // INC+rd reg32 (EAX)
/*41h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (CX)
                              // INC+rd reg32 (ECX)
/*42h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (DX)
                              // INC+rd reg32 (ECX)
/*43h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (BX)
                              // INC+rd reg32 (EBX)
/*44h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (SP)
                              // INC+rd reg32 (ESP)
/*45h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (BP)
                              // INC+rd reg32 (EBP)
/*46h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (SI)
                              // INC+rd reg32 (ESI)
/*47h*/{1, 0, 0, 0, 0, 0},    // INC+rw reg16 (DI)
                              // INC+rd reg32 (EDI)
/*48h*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (AX)
                              // DEC+rd reg32 (EAX)
/*49h*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (CX)
                              // DEC+rd reg32 (ECX)
/*4Ah*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (DX)
                              // DEC+rd reg32 (EDX)
/*4Bh*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (BX)
                              // DEC+rd reg32 (EBX)
/*4Ch*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (SP)
                              // DEC+rd reg32 (ESP)
/*4Dh*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (BP)
                              // DEC+rd reg32 (EBP)
/*4Eh*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (SI)
                              // DEC+rd reg32 (ESI)
/*4Fh*/{1, 0, 0, 0, 0, 0},    // DEC+rw reg16 (DI)
                              // DEC+rd reg32 (EDI)
#endif /* _M_X64 */
/*50h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (AX)
                              // PUSH+rd reg32 (EAX)
                              // PUSH+rq reg64 (RAX)
/*51h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (CX)
                              // PUSH+rd reg32 (ECX)
                              // PUSH+rq reg64 (RCX)
/*52h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (DX)
                              // PUSH+rd reg32 (EDX)
                              // PUSH+rq reg64 (RDX)
/*53h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (BX)
                              // PUSH+rd reg32 (EBX)
                              // PUSH+rq reg64 (RBX)
/*54h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (SP)
                              // PUSH+rd reg32 (ESP)
                              // PUSH+rq reg64 (RSP)
/*55h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (BP)
                              // PUSH+rd reg32 (EBP)
                              // PUSH+rq reg64 (RBP)
/*56h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (SI)
                              // PUSH+rd reg32 (ESI)
                              // PUSH+rq reg64 (RSI)
/*57h*/{1, 0, 0, 0, 0, 0},    // PUSH+rw reg16 (DI)
                              // PUSH+rd reg32 (EDI)
                              // PUSH+rq reg64 (RDI)
/*58h*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (AX)
                              // POP+rd reg32 (EAX)
                              // POP+rq reg64 (RAX)
/*59h*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (CX)
                              // POP+rd reg32 (ECX)
                              // POP+rq reg64 (RCX)
/*5Ah*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (DX)
                              // POP+rd reg32 (EDX)
                              // POP+rq reg64 (RDX)
/*5Bh*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (BX)
                              // POP+rd reg32 (EBX)
                              // POP+rq reg64 (RBX)
/*5Ch*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (SP)
                              // POP+rd reg32 (ESP)
                              // POP+rq reg64 (RSP)
/*5Dh*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (BP)
                              // POP+rd reg32 (EBP)
                              // POP+rq reg64 (RBP)
/*5Eh*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (SI)
                              // POP+rd reg32 (ESI)
                              // POP+rq reg64 (RSI)
/*5Fh*/{1, 0, 0, 0, 0, 0},    // POP+rw reg16 (DI)
                              // POP+rd reg32 (EDI)
                              // POP+rq reg64 (RDI)
#ifdef _M_X64
/*60h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*61h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*62h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*63h*/{1, 1, 0, 0, 0, 0},    // MOVSXD/r reg64,reg/mem32
#else /* _M_X64 */
/*60h*/{1, 0, 0, 0, 0, 0},    // PUSHA
                              // PUSHAD
/*61h*/{1, 0, 0, 0, 0, 0},    // POPA
/*62h*/{1, 1, 0, 0, 0, 0},    // BOUND/r reg16,mem16&mem16
/*63h*/{1, 1, 0, 0, 0, 0},    // ARPL/r reg/mem16,reg16
#endif /* _M_X64 */
/*64h*/{1, 0, 0, 0, 0, 0},    // FS:
/*65h*/{1, 0, 0, 0, 0, 0},    // GS:
/*66h*/{1, 0, 0, 0, 0, 0},    // OPERAND-SIZE-PREFIX / MMX-PREFIX
/*67h*/{1, 0, 0, 0, 0, 0},    // ADDRESS-SIZE-PREFIX
/*68h*/{3, 0, 1, 0, 0, 0},    // PUSH imm16
                              // PUSH imm32
                              // PUSH imm64 (SIGN-EXTENDED imm32)
/*69h*/{3, 1, 1, 0, 0, 0},    // IMUL/r/iw reg16,reg/mem16,imm16
                              // IMUL/r/id reg32,reg/mem32,imm32
                              // IMUL/r/id reg64,reg/mem64,imm32
/*6Ah*/{2, 0, 0, 0, 0, 0},    // PUSH imm8
/*6Bh*/{2, 1, 0, 0, 0, 0},    // IMUL/r/ib reg16,reg/mem16,imm8
                              // IMUL/r/ib reg32,reg/mem32,imm8
                              // IMUL/r/ib reg64,reg/mem64,imm8
/*6Ch*/{1, 0, 0, 0, 0, 0},    // INS mem8,DX
                              // INSB
/*6Dh*/{1, 0, 0, 0, 0, 0},    // INS mem16,DX
                              // INS mem32,DX
                              // INSD
                              // INSW
/*6Eh*/{1, 0, 0, 0, 0, 0},    // OUTS DX,mem8
                              // OUTSB
/*6Fh*/{1, 0, 0, 0, 0, 0},    // OUTS DX,mem16
                              // OUTS DX,mem32
                              // OUTSD
                              // OUTSW
/*70h*/{2, 0, 0, 0, 0, 1},    // JO/cb rel8off
/*71h*/{2, 0, 0, 0, 0, 1},    // JNO/cb rel8off
/*72h*/{2, 0, 0, 0, 0, 1},    // JB/cb rel8off
                              // JC/cb rel8off
                              // JNAE/cb rel8off
/*73h*/{2, 0, 0, 0, 0, 1},    // JAE/cb rel8off
                              // JNB/cb rel8off
                              // JNC/cb rel8off
/*74h*/{2, 0, 0, 0, 0, 1},    // JE/cb rel8off
                              // JZ/cb rel8off
/*75h*/{2, 0, 0, 0, 0, 1},    // JNE/cb rel8off
                              // JNZ/cb rel8off
/*76h*/{2, 0, 0, 0, 0, 1},    // JBE/cb rel8off
                              // JNA/cb rel8off
/*77h*/{2, 0, 0, 0, 0, 1},    // JA/cb rel8off
                              // JNBE/cb rel8off
/*78h*/{2, 0, 0, 0, 0, 1},    // JS/cb rel8off
/*79h*/{2, 0, 0, 0, 0, 1},    // JNS/cb rel8off
/*7Ah*/{2, 0, 0, 0, 0, 1},    // JP/cb rel8off
                              // JPE/cb rel8off
/*7Bh*/{2, 0, 0, 0, 0, 1},    // JNP/cb rel8off
                              // JPO/cb rel8off
/*7Ch*/{2, 0, 0, 0, 0, 1},    // JL/cb rel8off
                              // JNGE/cb rel8off
/*7Dh*/{2, 0, 0, 0, 0, 1},    // JGE/cb rel8off
                              // JNL/cb rel8off
/*7Eh*/{2, 0, 0, 0, 0, 1},    // JLE/cb rel8off
                              // JNG/cb rel8off
/*7Fh*/{2, 0, 0, 0, 0, 1},    // JG/cb rel8off
                              // JNLE/cb rel8off
/*80h*/{2, 1, 0, 0, 0, 0},    // ADD/0/ib reg/mem8,imm8
                              // OR/1/ib reg/mem8,imm8
                              // ADC/2/ib reg/mem8,imm8
                              // SBB/3/ib reg/mem8,imm8
                              // AND/4/ib reg/mem8,imm8
                              // SUB/5/ib reg/mem8,imm8
                              // XOR/6/ib reg/mem8,imm8
                              // CMP/7/ib reg/mem8,imm8
/*81h*/{3, 1, 1, 0, 0, 0},    // ADD/0/iw reg/mem16,imm16
                              // ADD/0/id reg/mem32,imm32
                              // ADD/0/id reg/mem64,imm32
                              // OR/1/iw reg/mem16,imm16
                              // OR/1/id reg/mem32,imm32
                              // OR/1/id reg/mem64,imm32
                              // ADC/2/iw reg/mem16,imm16
                              // ADC/2/id reg/mem32,imm32
                              // ADC/2/id reg/mem64,imm32
                              // SBB/3/iw reg/mem16,imm16
                              // SBB/3/id reg/mem32,imm32
                              // SBB/3/id reg/mem64,imm32
                              // AND/4/iw reg/mem16,imm16
                              // AND/4/id reg/mem32,imm32
                              // AND/4/id reg/mem64,imm32
                              // SUB/5/iw reg/mem16,imm16
                              // SUB/5/id reg/mem32,imm32
                              // SUB/5/id reg/mem64,imm32
                              // XOR/6/iw reg/mem16,imm16
                              // XOR/6/id reg/mem32,imm32
                              // XOR/6/id reg/mem64,imm32
                              // CMP/7/iw reg/mem16,imm16
                              // CMP/7/id reg/mem32,imm32
                              // CMP/7/id reg/mem64,imm32
/*82h*/{3, 1, 1, 0, 0, 0},    // ADD/0/iw reg/mem16,imm16
                              // ADD/0/id reg/mem32,imm32
                              // ADD/0/id reg/mem64,imm32
                              // OR/1/iw reg/mem16,imm16
                              // OR/1/id reg/mem32,imm32
                              // OR/1/id reg/mem64,imm32
                              // ADC/2/iw reg/mem16,imm16
                              // ADC/2/id reg/mem32,imm32
                              // ADC/2/id reg/mem64,imm32
                              // SBB/3/iw reg/mem16,imm16
                              // SBB/3/id reg/mem32,imm32
                              // SBB/3/id reg/mem64,imm32
                              // AND/4/iw reg/mem16,imm16
                              // AND/4/id reg/mem32,imm32
                              // AND/4/id reg/mem64,imm32
                              // SUB/5/iw reg/mem16,imm16
                              // SUB/5/id reg/mem32,imm32
                              // SUB/5/id reg/mem64,imm32
                              // XOR/6/iw reg/mem16,imm16
                              // XOR/6/id reg/mem32,imm32
                              // XOR/6/id reg/mem64,imm32
                              // CMP/7/iw reg/mem16,imm16
                              // CMP/7/id reg/mem32,imm32
                              // CMP/7/id reg/mem64,imm32
/*83h*/{2, 1, 0, 0, 0, 0},    // ADD/0/ib reg/mem16,imm8
                              // ADD/0/ib reg/mem32,imm8
                              // ADD/0/ib reg/mem64,imm8
                              // OR/1/ib reg/mem16,imm8
                              // OR/1/ib reg/mem32,imm8
                              // OR/1/ib reg/mem64,imm8
                              // ADC/2/ib reg/mem16,imm8
                              // ADC/2/ib reg/mem32,imm8
                              // ADC/2/ib reg/mem64,imm8
                              // SBB/3/ib reg/mem16,imm8
                              // SBB/3/ib reg/mem32,imm8
                              // SBB/3/ib reg/mem64,imm8
                              // AND/4/ib reg/mem16,imm8
                              // AND/4/ib reg/mem32,imm8
                              // AND/4/ib reg/mem64,imm8
                              // SUB/5/ib reg/mem16,imm8
                              // SUB/5/ib reg/mem32,imm8
                              // SUB/5/ib reg/mem64,imm8
                              // XOR/6/ib reg/mem16,imm8
                              // XOR/6/ib reg/mem32,imm8
                              // XOR/6/ib reg/mem64,imm8
                              // CMP/7/ib reg/mem16,imm8
                              // CMP/7/ib reg/mem32,imm8
                              // CMP/7/ib reg/mem64,imm8
/*84h*/{1, 1, 0, 0, 0, 0},    // TEST/r reg/mem8,reg8
/*85h*/{1, 1, 0, 0, 0, 0},    // TEST/r reg/mem16,reg16
                              // TEST/r reg/mem32,reg32
                              // TEST/r reg/mem64,reg64
/*86h*/{1, 1, 0, 0, 0, 0},    // XCHG/r reg/mem8,reg8
                              // XCHG/r reg8,reg/mem8
/*87h*/{1, 1, 0, 0, 0, 0},    // XCHG/r reg/mem16,reg16
                              // XCHG/r reg/mem32,reg32
                              // XCHG/r reg/mem64,reg64
                              // XCHG/r reg16,reg/mem16
                              // XCHG/r reg32,reg/mem32
                              // XCHG/r reg64,reg/mem64
/*88h*/{1, 1, 0, 0, 0, 0},    // MOV/r reg/mem8,reg8
/*89h*/{1, 1, 0, 0, 0, 0},    // MOV/r reg/mem16,reg16
                              // MOV/r reg/mem32,reg32
                              // MOV/r reg/mem64,reg64
/*8Ah*/{1, 1, 0, 0, 0, 0},    // MOV/r reg8,reg/mem8
/*8Bh*/{1, 1, 0, 0, 0, 0},    // MOV/r reg16,reg/mem16
                              // MOV/r reg32,reg/mem32
                              // MOV/r reg64,reg/mem64
/*8Ch*/{1, 1, 0, 0, 0, 0},    // MOV/r reg16/32/64/mem16,segReg
/*8Dh*/{1, 1, 0, 0, 0, 0},    // LEA/r reg16,mem
                              // LEA/r reg32,mem
                              // LEA/r reg64,mem
/*8Eh*/{1, 1, 0, 0, 0, 0},    // MOV/r segReg,reg/mem16
/*8Fh*/{1, 1, 0, 0, 0, 0},    // POP/0 reg/mem16
                              // POP/0 reg/mem32
                              // POP/0 reg/mem64
/*90h*/{1, 0, 0, 0, 0, 0},    // NOP
                              // XCHG+rw AX,reg16 (AX)
                              // XCHG+rw reg16,AX (AX)
                              // XCHG+rd EAX,reg32 (EAX)
                              // XCHG+rd reg32,EAX (EAX)
                              // XCHG+rq RAX,reg64 (RAX)
                              // XCHG+rq reg64,RAX (RAX)
/*91h*/{1, 0, 0, 0, 0, 0},    // XCHG+rw AX,reg16 (CX)
                              // XCHG+rw reg16,AX (CX)
                              // XCHG+rd EAX,reg32 (ECX)
                              // XCHG+rd reg32,EAX (ECX)
                              // XCHG+rq RAX,reg64 (RCX)
                              // XCHG+rq reg64,RAX (RCX)
/*92h*/{1, 0, 0, 0, 0, 0},    // XCHG+rw AX,reg16 (DX)
                              // XCHG+rw reg16,AX (DX)
                              // XCHG+rd EAX,reg32 (EDX)
                              // XCHG+rd reg32,EAX (EDX)
                              // XCHG+rq RAX,reg64 (RDX)
                              // XCHG+rq reg64,RAX (RDX)
/*93h*/{1, 0, 0, 0, 0, 0},    // XCHG+rw AX,reg16 (BX)
                              // XCHG+rw reg16,AX (BX)
                              // XCHG+rd EAX,reg32 (EBX)
                              // XCHG+rd reg32,EAX (EBX)
                              // XCHG+rq RAX,reg64 (RBX)
                              // XCHG+rq reg64,RAX (RBX)
/*94h*/{1, 0, 0, 0, 0, 0},    // XCHG+rw AX,reg16 (SP)
                              // XCHG+rw reg16,AX (SP)
                              // XCHG+rd EAX,reg32 (ESP)
                              // XCHG+rd reg32,EAX (ESP)
                              // XCHG+rq RAX,reg64 (RSP)
                              // XCHG+rq reg64,RAX (RSP)
/*95h*/{1, 0, 0, 0, 0, 0},    // XCHG+rw AX,reg16 (BP)
                              // XCHG+rw reg16,AX  (BP)
                              // XCHG+rd EAX,reg32 (EBP)
                              // XCHG+rd reg32,EAX (EBP)
                              // XCHG+rq RAX,reg64 (RBP)
                              // XCHG+rq reg64,RAX (RBP)
/*96h*/{1, 0, 0, 0, 0, 0},    // XCHG+rw AX,reg16 (SI)
                              // XCHG+rw reg16,AX (SI)
                              // XCHG+rd EAX,reg32 (ESI)
                              // XCHG+rd reg32,EAX (ESI)
                              // XCHG+rq RAX,reg64 (RSI)
                              // XCHG+rq reg64,RAX (RSI)
/*97h*/{1, 0, 0, 0, 0, 0},    // XCHG+rw AX,reg16 (DI)
                              // XCHG+rw reg16,AX (DI)
                              // XCHG+rd EAX,reg32 (EDI)
                              // XCHG+rd reg32,EAX (EDI)
                              // XCHG+rq RAX,reg64 (RDI)
                              // XCHG+rq reg64,RAX (RDI)
/*98h*/{1, 0, 0, 0, 0, 0},    // CBW
                              // CDQE
                              // CWDE
/*99h*/{1, 0, 0, 0, 0, 0},    // CDQ
                              // CQO
                              // CWD
#ifdef _M_X64
/*9Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*9Ah*/{5, 0, 1, 0, 0, 0},    // CALL/cd FAR pntr16:16
                              // CALL/cp FAR pntr16:32
#endif /* _M_X64 */
/*9Bh*/{1, 0, 0, 0, 0, 0},    // WAIT
                              // FWAIT
/*9Ch*/{1, 0, 0, 0, 0, 0},    // PUSHF
                              // PUSHFD
                              // PUSHFQ
/*9Dh*/{1, 0, 0, 0, 0, 0},    // POPF
                              // POPFD
                              // POPFQ
/*9Eh*/{1, 0, 0, 0, 0, 0},    // SAHF
/*9Fh*/{1, 0, 0, 0, 0, 0},    // LAHF
/*A0h*/{3, 0, 0, 0, 1, 0},    // MOV AL,moffset8
/*A1h*/{3, 0, 0, 0, 1, 0},    // MOV AX,moffset16
                              // MOV EAX,moffset32
                              // MOV RAX,moffset64
/*A2h*/{3, 0, 0, 0, 1, 0},    // MOV moffset8,AL
/*A3h*/{3, 0, 0, 0, 1, 0},    // MOV moffset16,AX
                              // MOV moffset32,EAX
                              // MOV moffset64,RAX
/*A4h*/{1, 0, 0, 0, 0, 0},    // MOVS mem8,mem8
                              // MOVSB
/*A5h*/{1, 0, 0, 0, 0, 0},    // MOVS mem16,mem16
                              // MOVS mem32,mem32
                              // MOVS mem64,mem64
                              // MOVSD
                              // MOVSQ
                              // MOVSW
/*A6h*/{1, 0, 0, 0, 0, 0},    // CMPS mem8,mem8
                              // CMPSB
/*A7h*/{1, 0, 0, 0, 0, 0},    // CMPS mem16,mem16
                              // CMPS mem32,mem32
                              // CMPS mem64,mem64
                              // CMPSW
                              // CMPSD
                              // CMPSQ
/*A8h*/{2, 0, 0, 0, 0, 0},    // TEST/ib AL,imm8
/*A9h*/{3, 0, 1, 0, 0, 0},    // TEST/iw AX,imm16
                              // TEST/id EAX,imm32
                              // TEST/id RAX,imm32
/*AAh*/{1, 0, 0, 0, 0, 0},    // STOS mem8
                              // STOSB
/*ABh*/{1, 0, 0, 0, 0, 0},    // STOS mem16
                              // STOS mem32
                              // STOS mem64
                              // STOSD
                              // STOSQ
                              // STOSW
/*ACh*/{1, 0, 0, 0, 0, 0},    // LODS mem8
                              // LODSB
/*ADh*/{1, 0, 0, 0, 0, 0},    // LODS mem16
                              // LODS mem32
                              // LODS mem64
                              // LODSD
                              // LODSQ
                              // LODSW
/*AEh*/{1, 0, 0, 0, 0, 0},    // SCAS mem8
                              // SCASB
/*AFh*/{1, 0, 0, 0, 0, 0},    // SCAS mem16
                              // SCAS mem32
                              // SCAS mem64
                              // SCASD
                              // SCASQ
                              // SCASW
/*B0h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (AX)
/*B1h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (CX)
/*B2h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (DX)
/*B3h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (BX)
/*B4h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (SP)
/*B5h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (BP)
/*B6h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (SI)
/*B7h*/{2, 0, 0, 0, 0, 0},    // MOV+rb reg8,imm8 (DI)
/*B8h*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (AX)
                              // MOV+rd reg32,imm32 (EAX)
                              // MOV+rq reg64,imm64 (RAX) - requires REX.W
/*B9h*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (CX)
                              // MOV+rd reg32,imm32 (ECX)
                              // MOV+rq reg64,imm64 (RCX) - requires REX.W
/*BAh*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (DX)
                              // MOV+rd reg32,imm32 (EDX)
                              // MOV+rq reg64,imm64 (RDX) - requires REX.W
/*BBh*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (BX)
                              // MOV+rd reg32,imm32 (EBX)
                              // MOV+rq reg64,imm64 (RBX) - requires REX.W
/*BCh*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (SP)
                              // MOV+rd reg32,imm32 (ESP)
                              // MOV+rq reg64,imm64 (RSP) - requires REX.W
/*BDh*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (BP)
                              // MOV+rd reg32,imm32 (EBP)
                              // MOV+rq reg64,imm64 (RBP) - requires REX.W
/*BEh*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (SI)
                              // MOV+rd reg32,imm32 (ESI)
                              // MOV+rq reg64,imm64 (RSI) - requires REX.W
/*BFh*/{3, 0, 1, 1, 0, 0},    // MOV+rw reg16,imm16 (DI)
                              // MOV+rd reg32,imm32 (EDI)
                              // MOV+rq reg64,imm64 (RDI) - requires REX.W
/*C0h*/{2, 1, 0, 0, 0, 0},    // ROL/0/ib reg/mem8,imm8
                              // ROR/1/ib reg/mem8,imm8
                              // RCL/2/ib reg/mem8,imm8
                              // RCR/3/ib reg/mem8,imm8
                              // SAL/4/ib reg/mem8,imm8
                              // SHL/4/ib reg/mem8,imm8
                              // SHR/5/ib reg/mem8,imm8
                              // SAR/7/ib reg/mem8,imm8
/*C1h*/{2, 1, 0, 0, 0, 0},    // ROL/0/ib reg/mem16,imm8
                              // ROL/0/ib reg/mem32,imm8
                              // ROL/0/ib reg/mem64,imm8
                              // ROR/1/ib reg/mem16,imm8
                              // ROR/1/ib reg/mem32,imm8
                              // ROR/1/ib reg/mem64,imm8
                              // RCL/2/ib reg/mem16,imm8
                              // RCL/2/ib reg/mem32,imm8
                              // RCL/2/ib reg/mem64,imm8
                              // RCR/3/ib reg/mem16,imm8
                              // RCR/3/ib reg/mem32,imm8
                              // RCR/3/ib reg/mem64,imm8
                              // SAL/4/ib reg/mem16,imm8
                              // SAL/4/ib reg/mem32,imm8
                              // SAL/4/ib reg/mem64,imm8
                              // SHL/4/ib reg/mem16,imm8
                              // SHL/4/ib reg/mem32,imm8
                              // SHL/4/ib reg/mem64,imm8
                              // SHR/5/ib reg/mem16,imm8
                              // SHR/5/ib reg/mem32,imm8
                              // SHR/5/ib reg/mem64,imm8
                              // SAR/7/ib reg/mem16,imm8
                              // SAR/7/ib reg/mem32,imm8
                              // SAR/7/ib reg/mem64,imm8
/*C2h*/{3, 0, 0, 0, 0, 0},    // RET/iw imm16
/*C3h*/{1, 0, 0, 0, 0, 0},    // RET
#ifdef _M_X64
/*C4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*C5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*C4h*/{1, 1, 0, 0, 0, 0},    // LES/r reg16,mem16:16
                              // LES/r reg32,mem16:32
/*C5h*/{1, 1, 0, 0, 0, 0},    // LDS/r reg16,mem16:16
                              // LDS/r reg32,mem16:32
#endif /* _M_X64 */
/*C6h*/{2, 1, 0, 0, 0, 0},    // MOV/0 reg/mem8,imm8
/*C7h*/{3, 1, 1, 0, 0, 0},    // MOV/0 reg/mem16,imm16
                              // MOV/0 reg/mem32,imm32
                              // MOV/0 reg/mem64,imm32
/*C8h*/{4, 0, 0, 0, 0, 0},    // ENTER/iw/00 imm16,0
                              // ENTER/iw/01 imm16,1
                              // ENTER/ib/iw imm16,imm8
/*C9h*/{1, 0, 0, 0, 0, 0},    // LEAVE (SP/BP)
                              // LEAVE (ESP/EBP)
                              // LEAVE (RSP/RBP)
/*CAh*/{3, 0, 0, 0, 0, 0},    // RETF/iw imm16
/*CBh*/{1, 0, 0, 0, 0, 0},    // RETF
/*CCh*/{1, 0, 0, 0, 0, 0},    // INT 3
/*CDh*/{2, 0, 0, 0, 0, 0},    // INT/ib imm8
#ifdef _M_X64
/*CEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*CEh*/{1, 0, 0, 0, 0, 0},    // INTO
#endif /* _M_X64 */
/*CFh*/{1, 0, 0, 0, 0, 0},    // IRET
                              // IRETD
                              // IRETQ
/*D0h*/{1, 1, 0, 0, 0, 0},    // ROL/0 reg/mem8,1
                              // ROR/1 reg/mem8,1
                              // RCL/2 reg/mem8,1
                              // RCR/3 reg/mem8,1
                              // SAL/4 reg/mem8,1
                              // SHL/4 reg/mem8,1
                              // SHR/5 reg/mem8,1
                              // SAR/7 reg/mem8,1
/*D1h*/{1, 1, 0, 0, 0, 0},    // ROL/0 reg/mem16,1
                              // ROL/0 reg/mem32,1
                              // ROL/0 reg/mem64,1
                              // ROR/1 reg/mem16,1
                              // ROR/1 reg/mem32,1
                              // ROR/1 reg/mem64,1
                              // RCL/2 reg/mem16,1
                              // RCL/2 reg/mem32,1
                              // RCL/2 reg/mem64,1
                              // RCR/3 reg/mem16,1
                              // RCR/3 reg/mem32,1
                              // RCR/3 reg/mem64,1
                              // SAL/4 reg/mem16,1
                              // SAL/4 reg/mem32,1
                              // SAL/4 reg/mem64,1
                              // SHL/4 reg/mem16,1
                              // SHL/4 reg/mem32,1
                              // SHL/4 reg/mem64,1
                              // SHR/5 reg/mem16,1
                              // SHR/5 reg/mem32,1
                              // SHR/5 reg/mem64,1
                              // SAR/7 reg/mem16,1
                              // SAR/7 reg/mem32,1
                              // SAR/7 reg/mem64,1
/*D2h*/{1, 1, 0, 0, 0, 0},    // ROL/0 reg/mem8,CL
                              // ROR/1 reg/mem8,CL
                              // RCL/2 reg/mem8,CL
                              // RCR/3 reg/mem8,CL
                              // SAL/4 reg/mem8,CL
                              // SHL/4 reg/mem8,CL
                              // SHR/5 reg/mem8,CL
                              // SAR/7 reg/mem8,CL
/*D3h*/{1, 1, 0, 0, 0, 0},    // ROL/0 reg/mem16,CL
                              // ROL/0 reg/mem32,CL
                              // ROL/0 reg/mem64,CL
                              // ROR/1 reg/mem16,CL
                              // ROR/1 reg/mem32,CL
                              // ROR/1 reg/mem64,CL
                              // RCL/2 reg/mem16,CL
                              // RCL/2 reg/mem32,CL
                              // RCL/2 reg/mem64,CL
                              // RCR/3 reg/mem16,CL
                              // RCR/3 reg/mem32,CL
                              // RCR/3 reg/mem64,CL
                              // SAL/4 reg/mem16,CL
                              // SAL/4 reg/mem32,CL
                              // SAL/4 reg/mem64,CL
                              // SHL/4 reg/mem16,CL
                              // SHL/4 reg/mem32,CL
                              // SHL/4 reg/mem64,CL
                              // SHR/5 reg/mem16,CL
                              // SHR/5 reg/mem32,CL
                              // SHR/5 reg/mem64,CL
                              // SAR/7 reg/mem16,CL
                              // SAR/7 reg/mem32,CL
                              // SAR/7 reg/mem64,CL
#ifdef _M_X64
/*D4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*D5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*D6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*D4h*/{2, 0, 0, 0, 0, 0},    // AAM/ib 0Ah/imm8
/*D5h*/{2, 0, 0, 0, 0, 0},    // AAD/ib 0Ah/imm8
/*D6h*/{1, 0, 0, 0, 0, 0},    // SALC
#endif /* _M_X64 */
/*D7h*/{1, 0, 0, 0, 0, 0},    // XLAT mem8
                              // XLATB
/*D8h*/{1, 1, 0, 0, 0, 0},    // FADD/0 mem32real
                              // FMUL/1 mem32real
                              // FCOM/2 mem32real
                              // FCOMP/3 mem32real
                              // FSUB/4 mem32real
                              // FSUBR/5 mem32real
                              // FDIV/6 mem32real
                              // FDIVR/7 mem32real
                              // FADD+i ST(0),ST(0)
                              // FADD+i ST(0),ST(1)
                              // FADD+i ST(0),ST(2)
                              // FADD+i ST(0),ST(3)
                              // FADD+i ST(0),ST(4)
                              // FADD+i ST(0),ST(5)
                              // FADD+i ST(0),ST(6)
                              // FADD+i ST(0),ST(7)
                              // FMUL+i ST(0),ST(0)
                              // FMUL+i ST(0),ST(1)
                              // FMUL+i ST(0),ST(2)
                              // FMUL+i ST(0),ST(3)
                              // FMUL+i ST(0),ST(4)
                              // FMUL+i ST(0),ST(5)
                              // FMUL+i ST(0),ST(6)
                              // FMUL+i ST(0),ST(7)
                              // FCOM+i ST(0)
                              // FCOM+i ST(1)
                              // FCOM+i ST(2)
                              // FCOM+i ST(3)
                              // FCOM+i ST(4)
                              // FCOM+i ST(5)
                              // FCOM+i ST(7)
                              // FCOM+i ST(8)
                              // FCOMP+i ST(0)
                              // FSUB+i ST(0),ST(0)
                              // FSUB+i ST(0),ST(1)
                              // FSUB+i ST(0),ST(2)
                              // FSUB+i ST(0),ST(3)
                              // FSUB+i ST(0),ST(4)
                              // FSUB+i ST(0),ST(5)
                              // FSUB+i ST(0),ST(6)
                              // FSUB+i ST(0),ST(7)
                              // FCOMP+i ST(0)
                              // FCOMP+i ST(1)
                              // FCOMP+i ST(2)
                              // FCOMP+i ST(3)
                              // FCOMP+i ST(4)
                              // FCOMP+i ST(5)
                              // FCOMP+i ST(6)
                              // FCOMP+i ST(7)
                              // FSUBR+i ST(0),ST(0)
                              // FSUBR+i ST(0),ST(1)
                              // FSUBR+i ST(0),ST(2)
                              // FSUBR+i ST(0),ST(3)
                              // FSUBR+i ST(0),ST(4)
                              // FSUBR+i ST(0),ST(5)
                              // FSUBR+i ST(0),ST(6)
                              // FSUBR+i ST(0),ST(7)
                              // FDIV+i ST(0),ST(0)
                              // FDIV+i ST(0),ST(1)
                              // FDIV+i ST(0),ST(2)
                              // FDIV+i ST(0),ST(3)
                              // FDIV+i ST(0),ST(4)
                              // FDIV+i ST(0),ST(5)
                              // FDIV+i ST(0),ST(6)
                              // FDIV+i ST(0),ST(7)
                              // FDIVR+i ST(0),ST(0)
                              // FDIVR+i ST(0),ST(1)
                              // FDIVR+i ST(0),ST(2)
                              // FDIVR+i ST(0),ST(3)
                              // FDIVR+i ST(0),ST(4)
                              // FDIVR+i ST(0),ST(5)
                              // FDIVR+i ST(0),ST(6)
                              // FDIVR+i ST(0),ST(7)
/*D9h*/{1, 1, 0, 0, 0, 0},    // FLD/0 mem32real
                              // FST/2 mem32real
                              // FSTP/3 mem32real
                              // FLDENV/4 mem14/28env
                              // FLDCW/5 mem2env
                              // FNSTENV/6 mem14/28env
                              // FNSTCW/7 mem2env
                              // FLD+i ST(0)
                              // FLD+i ST(1)
                              // FLD+i ST(2)
                              // FLD+i ST(3)
                              // FLD+i ST(4)
                              // FLD+i ST(5)
                              // FLD+i ST(6)
                              // FLD+i ST(7)
                              // FXCH+i ST(0)
                              // FXCH+i ST(1)
                              // FXCH+i ST(2)
                              // FXCH+i ST(3)
                              // FXCH+i ST(4)
                              // FXCH+i ST(5)
                              // FXCH+i ST(6)
                              // FXCH+i ST(7)
                              // FNOP
                              // FCHS
                              // FABS
                              // FTST
                              // FXAM
                              // FLD1
                              // FLDL2T
                              // FLDL2E
                              // FLDPI
                              // FLDLG2
                              // FLDLN2
                              // FLDZ
                              // F2XM1
                              // FYL2X
                              // FPTAN
                              // FPATAN
                              // FXTRACT
                              // FPREM1
                              // FDECSTP
                              // FINCSTP
                              // FPREM
                              // FYL2XP1
                              // FSQRT
                              // FSINCOS
                              // FSCALE
                              // FSIN
                              // FCOS
/*DAh*/{1, 1, 0, 0, 0, 0},    // FIADD/0 mem32int
                              // FIMUL/1 mem32int
                              // FICOM/2 mem32int
                              // FICOMP/3 mem32int
                              // FISUB/4 mem32int
                              // FISUBR/5 mem32int
                              // FIDIV/6 mem32int
                              // FIDIVR/7 mem32int
                              // FCMOVB+i ST(0),ST(0)
                              // FCMOVB+i ST(0),ST(1)
                              // FCMOVB+i ST(0),ST(2)
                              // FCMOVB+i ST(0),ST(3)
                              // FCMOVB+i ST(0),ST(4)
                              // FCMOVB+i ST(0),ST(5)
                              // FCMOVB+i ST(0),ST(6)
                              // FCMOVB+i ST(0),ST(7)
                              // FCMOVE+i ST(0),ST(0)
                              // FCMOVE+i ST(0),ST(1)
                              // FCMOVE+i ST(0),ST(2)
                              // FCMOVE+i ST(0),ST(3)
                              // FCMOVE+i ST(0),ST(4)
                              // FCMOVE+i ST(0),ST(5)
                              // FCMOVE+i ST(0),ST(6)
                              // FCMOVE+i ST(0),ST(7)
                              // FCMOVBE+i ST(0),ST(0)
                              // FCMOVBE+i ST(0),ST(1)
                              // FCMOVBE+i ST(0),ST(2)
                              // FCMOVBE+i ST(0),ST(3)
                              // FCMOVBE+i ST(0),ST(4)
                              // FCMOVBE+i ST(0),ST(5)
                              // FCMOVBE+i ST(0),ST(6)
                              // FCMOVBE+i ST(0),ST(7)
                              // FCMOVU+i ST(0),ST(0)
                              // FCMOVU+i ST(0),ST(1)
                              // FCMOVU+i ST(0),ST(2)
                              // FCMOVU+i ST(0),ST(3)
                              // FCMOVU+i ST(0),ST(4)
                              // FCMOVU+i ST(0),ST(5)
                              // FCMOVU+i ST(0),ST(6)
                              // FCMOVU+i ST(0),ST(7)
                              // FUCOMPP
/*DBh*/{1, 1, 0, 0, 0, 0},    // FILD/0 mem32int
                              // FIST/2 mem32int
                              // FISTP/3 mem32int
                              // FLD/5 mem80real
                              // FSTP/7 mem80real
                              // FCMOVNB+i ST(0),ST(0)
                              // FCMOVNB+i ST(0),ST(1)
                              // FCMOVNB+i ST(0),ST(2)
                              // FCMOVNB+i ST(0),ST(3)
                              // FCMOVNB+i ST(0),ST(4)
                              // FCMOVNB+i ST(0),ST(5)
                              // FCMOVNB+i ST(0),ST(6)
                              // FCMOVNB+i ST(0),ST(7)
                              // FCMOVNE+i ST(0),ST(0)
                              // FCMOVNE+i ST(0),ST(1)
                              // FCMOVNE+i ST(0),ST(2)
                              // FCMOVNE+i ST(0),ST(3)
                              // FCMOVNE+i ST(0),ST(4)
                              // FCMOVNE+i ST(0),ST(5)
                              // FCMOVNE+i ST(0),ST(6)
                              // FCMOVNE+i ST(0),ST(7)
                              // FCMOVNBE+i ST(0),ST(0)
                              // FCMOVNBE+i ST(0),ST(1)
                              // FCMOVNBE+i ST(0),ST(2)
                              // FCMOVNBE+i ST(0),ST(3)
                              // FCMOVNBE+i ST(0),ST(4)
                              // FCMOVNBE+i ST(0),ST(5)
                              // FCMOVNBE+i ST(0),ST(6)
                              // FCMOVNBE+i ST(0),ST(7)
                              // FCMOVNU+i ST(0),ST(0)
                              // FCMOVNU+i ST(0),ST(1)
                              // FCMOVNU+i ST(0),ST(2)
                              // FCMOVNU+i ST(0),ST(3)
                              // FCMOVNU+i ST(0),ST(4)
                              // FCMOVNU+i ST(0),ST(5)
                              // FCMOVNU+i ST(0),ST(6)
                              // FCMOVNU+i ST(0),ST(7)
                              // FCLEX
                              // FNINIT
                              // FUCOMI+i ST(0),ST(0)
                              // FUCOMI+i ST(0),ST(1)
                              // FUCOMI+i ST(0),ST(2)
                              // FUCOMI+i ST(0),ST(3)
                              // FUCOMI+i ST(0),ST(4)
                              // FUCOMI+i ST(0),ST(5)
                              // FUCOMI+i ST(0),ST(6)
                              // FUCOMI+i ST(0),ST(7)
                              // FCOMI+i ST(0),ST(0)
                              // FCOMI+i ST(0),ST(1)
                              // FCOMI+i ST(0),ST(2)
                              // FCOMI+i ST(0),ST(3)
                              // FCOMI+i ST(0),ST(4)
                              // FCOMI+i ST(0),ST(5)
                              // FCOMI+i ST(0),ST(6)
                              // FCOMI+i ST(0),ST(7)
/*DCh*/{1, 1, 0, 0, 0, 0},    // FADD/0 mem64real
                              // FMUL/1 mem64real
                              // FCOM/2 mem64real
                              // FCOMP/3 mem64real
                              // FSUB/4 mem64real
                              // FSUBR/5 mem64real
                              // FDIV/6 mem64real
                              // FDIVR/7 mem64real
                              // FADD+i ST(0),ST(0)
                              // FADD+i ST(1),ST(0)
                              // FADD+i ST(2),ST(0)
                              // FADD+i ST(3),ST(0)
                              // FADD+i ST(4),ST(0)
                              // FADD+i ST(5),ST(0)
                              // FADD+i ST(6),ST(0)
                              // FADD+i ST(7),ST(0)
                              // FMUL+i ST(0),ST(0)
                              // FMUL+i ST(1),ST(0)
                              // FMUL+i ST(2),ST(0)
                              // FMUL+i ST(3),ST(0)
                              // FMUL+i ST(4),ST(0)
                              // FMUL+i ST(5),ST(0)
                              // FMUL+i ST(6),ST(0)
                              // FMUL+i ST(7),ST(0)
                              // FSUBR+i ST(0),ST(0)
                              // FSUBR+i ST(1),ST(0)
                              // FSUBR+i ST(2),ST(0)
                              // FSUBR+i ST(3),ST(0)
                              // FSUBR+i ST(4),ST(0)
                              // FSUBR+i ST(5),ST(0)
                              // FSUBR+i ST(6),ST(0)
                              // FSUBR+i ST(7),ST(0)
                              // FSUB+i ST(0),ST(0)
                              // FSUB+i ST(1),ST(0)
                              // FSUB+i ST(2),ST(0)
                              // FSUB+i ST(3),ST(0)
                              // FSUB+i ST(4),ST(0)
                              // FSUB+i ST(5),ST(0)
                              // FSUB+i ST(6),ST(0)
                              // FSUB+i ST(7),ST(0)
                              // FDIVR+i ST(0),ST(0)
                              // FDIVR+i ST(1),ST(0)
                              // FDIVR+i ST(2),ST(0)
                              // FDIVR+i ST(3),ST(0)
                              // FDIVR+i ST(4),ST(0)
                              // FDIVR+i ST(5),ST(0)
                              // FDIVR+i ST(6),ST(0)
                              // FDIVR+i ST(7),ST(0)
                              // FDIV+i ST(0),ST(0)
                              // FDIV+i ST(1),ST(0)
                              // FDIV+i ST(2),ST(0)
                              // FDIV+i ST(3),ST(0)
                              // FDIV+i ST(4),ST(0)
                              // FDIV+i ST(5),ST(0)
                              // FDIV+i ST(6),ST(0)
                              // FDIV+i ST(7),ST(0)
/*DDh*/{1, 1, 0, 0, 0, 0},    // FLD/0 mem64real
                              // FST/2 mem64real
                              // FSTP/3 mem64real
                              // FRSTOR/4 mem94/108env
                              // FNSAVE/6 mem94/108env
                              // FNSTSW/7 mem16
                              // FFREE+i ST(0)
                              // FFREE+i ST(1)
                              // FFREE+i ST(2)
                              // FFREE+i ST(3)
                              // FFREE+i ST(4)
                              // FFREE+i ST(5)
                              // FFREE+i ST(6)
                              // FFREE+i ST(7)
                              // FST+i ST(0)
                              // FST+i ST(1)
                              // FST+i ST(2)
                              // FST+i ST(3)
                              // FST+i ST(4)
                              // FST+i ST(5)
                              // FST+i ST(6)
                              // FST+i ST(7)
                              // FSTP+i ST(0)
                              // FSTP+i ST(1)
                              // FSTP+i ST(2)
                              // FSTP+i ST(3)
                              // FSTP+i ST(4)
                              // FSTP+i ST(5)
                              // FSTP+i ST(6)
                              // FSTP+i ST(7)
                              // FUCOM+i ST(0)
                              // FUCOM+i ST(1)
                              // FUCOM+i ST(2)
                              // FUCOM+i ST(3)
                              // FUCOM+i ST(4)
                              // FUCOM+i ST(5)
                              // FUCOM+i ST(6)
                              // FUCOM+i ST(7)
                              // FUCOMP+i ST(0)
                              // FUCOMP+i ST(1)
                              // FUCOMP+i ST(2)
                              // FUCOMP+i ST(3)
                              // FUCOMP+i ST(4)
                              // FUCOMP+i ST(5)
                              // FUCOMP+i ST(6)
                              // FUCOMP+i ST(7)
/*DEh*/{1, 1, 0, 0, 0, 0},    // FIADD/0 mem16int
                              // FIMUL/1 mem16int
                              // FICOM/2 mem16int
                              // FICOMP/3 mem16int
                              // FISUB/4 mem16int
                              // FISUBR/5 mem16int
                              // FIDIV/6 mem16int
                              // FIDIVR/7 mem16int
                              // FADDP+i ST(0),ST(0)
                              // FADDP+i ST(1),ST(0)
                              // FADDP+i ST(2),ST(0)
                              // FADDP+i ST(3),ST(0)
                              // FADDP+i ST(4),ST(0)
                              // FADDP+i ST(5),ST(0)
                              // FADDP+i ST(6),ST(0)
                              // FADDP+i ST(7),ST(0)
                              // FMULP+i ST(0),ST(0)
                              // FMULP+i ST(1),ST(0)
                              // FMULP+i ST(2),ST(0)
                              // FMULP+i ST(3),ST(0)
                              // FMULP+i ST(4),ST(0)
                              // FMULP+i ST(5),ST(0)
                              // FMULP+i ST(6),ST(0)
                              // FMULP+i ST(7),ST(0)
                              // FCOMPP
                              // FSUBRP+i ST(0),ST(0)
                              // FSUBRP+i ST(1),ST(0)
                              // FSUBRP+i ST(2),ST(0)
                              // FSUBRP+i ST(3),ST(0)
                              // FSUBRP+i ST(4),ST(0)
                              // FSUBRP+i ST(5),ST(0)
                              // FSUBRP+i ST(6),ST(0)
                              // FSUBRP+i ST(7),ST(0)
                              // FSUBP+i ST(0),ST(0)
                              // FSUBP+i ST(1),ST(0)
                              // FSUBP+i ST(2),ST(0)
                              // FSUBP+i ST(3),ST(0)
                              // FSUBP+i ST(4),ST(0)
                              // FSUBP+i ST(5),ST(0)
                              // FSUBP+i ST(6),ST(0)
                              // FSUBP+i ST(7),ST(0)
                              // FDIVRP+i ST(0),ST(0)
                              // FDIVRP+i ST(1),ST(0)
                              // FDIVRP+i ST(2),ST(0)
                              // FDIVRP+i ST(3),ST(0)
                              // FDIVRP+i ST(4),ST(0)
                              // FDIVRP+i ST(5),ST(0)
                              // FDIVRP+i ST(6),ST(0)
                              // FDIVRP+i ST(7),ST(0)
                              // FDIVP+i ST(0),ST(0)
                              // FDIVP+i ST(1),ST(0)
                              // FDIVP+i ST(2),ST(0)
                              // FDIVP+i ST(3),ST(0)
                              // FDIVP+i ST(4),ST(0)
                              // FDIVP+i ST(5),ST(0)
                              // FDIVP+i ST(6),ST(0)
                              // FDIVP+i ST(7),ST(0)
/*DFh*/{1, 1, 0, 0, 0, 0},    // FILD/0 mem16int
                              // FISTTP/1 mem16int
                              // FIST/2 mem16int
                              // FISTP/3 mem16int
                              // FBLD/4 mem80dec
                              // FILD/5 mem64int
                              // FBSTP/6 mem80dec
                              // FISTP/7 mem64int
                              // FSTSW AX
                              // FUCOMIP+i ST(0),ST(0)
                              // FUCOMIP+i ST(0),ST(1)
                              // FUCOMIP+i ST(0),ST(2)
                              // FUCOMIP+i ST(0),ST(3)
                              // FUCOMIP+i ST(0),ST(4)
                              // FUCOMIP+i ST(0),ST(5)
                              // FUCOMIP+i ST(0),ST(6)
                              // FUCOMIP+i ST(0),ST(7)
                              // FCOMIP+i ST(0),ST(0)
                              // FCOMIP+i ST(0),ST(1)
                              // FCOMIP+i ST(0),ST(2)
                              // FCOMIP+i ST(0),ST(3)
                              // FCOMIP+i ST(0),ST(4)
                              // FCOMIP+i ST(0),ST(5)
                              // FCOMIP+i ST(0),ST(6)
                              // FCOMIP+i ST(0),ST(7)
/*E0h*/{2, 0, 0, 0, 0, 1},    // LOOPNE/cb rel8off
                              // LOOPNZ/cb rel8off
/*E1h*/{2, 0, 0, 0, 0, 1},    // LOOPE/cb rel8off
                              // LOOPZ/cb rel8off
/*E2h*/{2, 0, 0, 0, 0, 1},    // LOOP/cb rel8off
/*E3h*/{2, 0, 0, 0, 0, 1},    // JCXZ/cb rel8off
                              // JECXZ/cb rel8off
                              // JRCXZ/cb rel8off
/*E4h*/{2, 0, 0, 0, 0, 0},    // IN/ib AL,imm8
/*E5h*/{2, 0, 0, 0, 0, 0},    // IN/ib AX,imm8
                              // IN/ib EAX,imm8
/*E6h*/{2, 0, 0, 0, 0, 0},    // OUT/ib imm8,AL
/*E7h*/{2, 0, 0, 0, 0, 0},    // OUT/ib imm8,AX
                              // OUT/ib imm8,EAX
/*E8h*/{3, 0, 1, 0, 0, 1},    // CALL/iw rel16off
                              // CALL/id rel32off
/*E9h*/{3, 0, 1, 0, 0, 1},    // JMP/cw rel16off
                              // JMP/cd rel32off
#ifdef _M_X64
/*EAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*EAh*/{5, 0, 1, 0, 0, 0},    // JMP/cd FAR pntr16:16
                              // JMP/cp FAR pntr16:32
#endif /* _M_X64 */
/*EBh*/{2, 0, 0, 0, 0, 1},    // JMP/cb rel8off
/*ECh*/{1, 0, 0, 0, 0, 0},    // IN AL,DX
/*EDh*/{1, 0, 0, 0, 0, 0},    // IN AX,DX
                              // IN EAX,DX
/*EEh*/{1, 0, 0, 0, 0, 0},    // OUT DX,AL
/*EFh*/{1, 0, 0, 0, 0, 0},    // OUT DX,AX
                              // OUT DX,EAX
/*F0h*/{1, 0, 0, 0, 0, 0},    // LOCK:
/*F1h*/{1, 0, 0, 0, 0, 0},    // INT1
/*F2h*/{1, 0, 0, 0, 0, 0},    // REPNE:
/*F3h*/{1, 0, 0, 0, 0, 0},    // REP:
                              // REPE:
/*F4h*/{1, 0, 0, 0, 0, 0},    // HLT
/*F5h*/{1, 0, 0, 0, 0, 0},    // CMC

/* TEST/0 is handled specially and the imm8 is _NOT_ included in
 * the table description */
/*F6h*/{1, 1, 0, 0, 0, 0},    // TEST/0/ib reg/mem8,imm8 (2 bytes+modRM)
                              // NOT/2 reg/mem8
                              // NEG/3 reg/mem8
                              // MUL/4 reg/mem8
                              // IMUL/5 reg/mem8
                              // DIV/6 reg/mem8
                              // IDIV/7 reg/mem8

/* TEST/0 is handled specially and the imm16/imm32 is _NOT_ included in
 * the table description */
/*F7h*/{1, 1, 0, 0, 0, 0},    // TEST/0/iw reg/mem16,imm16 (3 bytes+modRM)
                              // TEST/0/id reg/mem32,imm32 (4 bytes+modRM)
                              // TEST/0/id reg/mem64,imm32 (4 bytes+modRM)
                              // NOT/2 reg/mem16
                              // NOT/2 reg/mem32
                              // NOT/2 reg/mem64
                              // NEG/3 reg/mem16
                              // NEG/3 reg/mem32
                              // NEG/3 reg/mem64
                              // MUL/4 reg/mem16
                              // MUL/4 reg/mem32
                              // MUL/4 reg/mem64
                              // IMUL/5 reg/mem16
                              // IMUL/5 reg/mem32
                              // IMUL/5 reg/mem64
                              // DIV/6 reg/mem16
                              // DIV/6 reg/mem32
                              // DIV/6 reg/mem64
                              // IDIV/7 reg/mem16
                              // IDIV/7 reg/mem32
                              // IDIV/7 reg/mem64
/*F8h*/{1, 0, 0, 0, 0, 0},    // CLC
/*F9h*/{1, 0, 0, 0, 0, 0},    // STC
/*FAh*/{1, 0, 0, 0, 0, 0},    // CLI
/*FBh*/{1, 0, 0, 0, 0, 0},    // STI
/*FCh*/{1, 0, 0, 0, 0, 0},    // CLD
/*FDh*/{1, 0, 0, 0, 0, 0},    // STD
/*FEh*/{1, 1, 0, 0, 0, 0},    // INC/0 reg/mem8
                              // DEC/1 reg/mem8

/* CALL/3 & JMP/5 are handled specially and the mem16:16/mem16:32 is _NOT_ included
 * in the table description */
/*FFh*/{1, 1, 0, 0, 0, 0}     // CALL/3 FAR mem16:16
                              // CALL/3 FAR mem16:32
                              // JMP/5 FAR mem16:16
                              // JMP/5 FAR mem16:32
                              // INC/0 reg/mem16
                              // INC/0 reg/mem32
                              // INC/0 reg/mem64
                              // DEC/1 reg/mem16
                              // DEC/1 reg/mem32
                              // DEC/1 reg/mem64
                              // CALL/2 reg/mem16
                              // JMP/4 reg/mem16
                              // JMP/4 reg/mem32
                              // JMP/4 reg/mem64
                              // PUSH/6 reg/mem16
                              // PUSH/6 reg/mem32
                              // PUSH/6 reg/mem64
};

/**
 * __InstructionTable00000F
 *    2 byte opcodes, 0F00h-0FFFh
 */
static const OPCODE_INFO __InstructionTable00000F[] =
{
/*00h*/{1, 1, 0, 0, 0, 0},    // LLDT/2 reg/mem16
                              // LTR/3 reg/mem16
                              // SLDT/0 mem16
                              // SLDT/0 reg16
                              // SLDT/0 reg32
                              // SLDT/0 reg64
                              // STR/1 mem16
                              // STR/1 reg16
                              // STR/1 reg32
                              // STR/1 reg64
                              // VERR/4 reg/mem16
                              // VERW/5 reg/mem16
//TODO:SWAPGS & INVLPG need to be handled specially
/*01h*/{1, 1, 0, 0, 0, 0},    // INVLPG/7 mem8
                              // SWAPGS/7 0xF8
                              // LGDT/2 mem16:32
                              // LGDT/2 mem16:64
                              // LIDT/3 mem16:32
                              // LIDT/3 mem16:64
                              // LMSW/6 reg/mem16
                              // SGDT/0 mem16:32
                              // SGDT/0 mem16:64
                              // SIDT/1 mem16:32
                              // SIDT/1 mem16:64
                              // SMSW/4 mem16
                              // SMSW/4 reg16
                              // SMSW/4 reg32
                              // SMSW/4 reg64
/*02h*/{1, 1, 0, 0, 0, 0},    // LAR/r reg16,reg/mem16
                              // LAR/r reg32,reg/mem16
                              // LAR/r reg64,reg/mem16
/*03h*/{1, 1, 0, 0, 0, 0},    // LSL/r reg16,reg/mem16
                              // LSL/r reg32,reg/mem16
                              // LSL/r reg64,reg/mem16
/*04h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*05h*/{1, 0, 0, 0, 0, 0},    // SYSCALL
/*06h*/{1, 0, 0, 0, 0, 0},    // CLTS
/*07h*/{1, 0, 0, 0, 0, 0},    // SYSRET
/*08h*/{1, 0, 0, 0, 0, 0},    // INVD
/*09h*/{1, 0, 0, 0, 0, 0},    // WBINVD
/*0Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Bh*/{1, 0, 0, 0, 0, 0},    // UD2
/*0Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Dh*/{1, 1, 0, 0, 0, 0},    // PREFETCH/0 mem8
                              // PREFETCHW/1 mem8
/*0Eh*/{1, 0, 0, 0, 0, 0},    // FEMMS
/*0Fh*/{0, 0, 0, 0, 0, 0},    // 3DNow! OPCODE-BRANCH
/*10h*/{1, 1, 0, 0, 0, 0},    // MOVUPS/r xmm1,xmm2/mem128
/*11h*/{1, 1, 0, 0, 0, 0},    // MOVUPS/r xmm1/mem128,xmm2
/*12h*/{1, 1, 0, 0, 0, 0},    // MOVHLPS/r xmm1,xmm2
                              // MOVLPS/r xmm,mem64
/*13h*/{1, 1, 0, 0, 0, 0},    // MOVLPS/r mem64,xmm
/*14h*/{1, 1, 0, 0, 0, 0},    // UNPCKLPS/r xmm1,xmm2/mem128
/*15h*/{1, 1, 0, 0, 0, 0},    // UNPCKHPS/r xmm1,xmm2/mem128
/*16h*/{1, 1, 0, 0, 0, 0},    // MOVHPS/r xmm,mem64
                              // MOVLHPS/r xmm1,xmm2
/*17h*/{1, 1, 0, 0, 0, 0},    // MOVHPS/r mem64,xmm
/*18h*/{1, 1, 0, 0, 0, 0},    // PREFETCH NTA/0 mem8
                              // PREFETCH T1/0 mem8
                              // PREFETCH T2/1 mem8
                              // PREFETCH T3/2 mem8
                              // NOP/4
                              // NOP/5
                              // NOP/6
                              // NOP/7
/*19h*/{1, 1, 0, 0, 0, 0},    // NOP
/*1Ah*/{1, 1, 0, 0, 0, 0},    // NOP
/*1Bh*/{1, 1, 0, 0, 0, 0},    // NOP
/*1Ch*/{1, 1, 0, 0, 0, 0},    // NOP
/*1Dh*/{1, 1, 0, 0, 0, 0},    // NOP
/*1Eh*/{1, 1, 0, 0, 0, 0},    // NOP
/*1Fh*/{1, 1, 0, 0, 0, 0},    // NOP
/*20h*/{1, 1, 0, 0, 0, 0},    // MOV/r reg32,CRn
                              // MOV/r reg64,CRn
/*21h*/{1, 1, 0, 0, 0, 0},    // MOV/r reg32,DRn
                              // MOV/r reg64,DRn
/*22h*/{1, 1, 0, 0, 0, 0},    // MOV/r CRn,reg32
                              // MOV/r CRn,reg64
/*23h*/{1, 1, 0, 0, 0, 0},    // MOV/r DRn,reg32
                              // MOV/r DRn,reg64
/*24h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*25h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*26h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*27h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*28h*/{1, 1, 0, 0, 0, 0},    // MOVAPS/r xmm1,xmm2/mem128
/*29h*/{1, 1, 0, 0, 0, 0},    // MOVAPS/r xmm1/mem128,xmm2
/*2Ah*/{1, 1, 0, 0, 0, 0},    // CVTPI2PS/r xmm,mmx/mem64
/*2Bh*/{1, 1, 0, 0, 0, 0},    // MOVNTPS/r mem128,xmm
/*2Ch*/{1, 1, 0, 0, 0, 0},    // CVTTPS2PI/r mmx,xmm/mem64
/*2Dh*/{1, 1, 0, 0, 0, 0},    // CVTPS2PI/r mmx,xmm/mem64
/*2Eh*/{1, 1, 0, 0, 0, 0},    // UCOMISS/r xmm1,xmm2/mem32
/*2Fh*/{1, 1, 0, 0, 0, 0},    // COMISS/r xmm1,xmm2/mem32
/*30h*/{1, 0, 0, 0, 0, 0},    // WRMSR
/*31h*/{1, 0, 0, 0, 0, 0},    // RDTSC
/*32h*/{1, 0, 0, 0, 0, 0},    // RDMSR
/*33h*/{1, 0, 0, 0, 0, 0},    // RDPMC
#ifdef _M_X64
/*34h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
/*35h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE (64bit)
#else /* _M_X64 */
/*34h*/{1, 0, 0, 0, 0, 0},    // SYSENTER
/*35h*/{1, 0, 0, 0, 0, 0},    // SYSEXIT
#endif /* _M_X64 */
/*36h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*37h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*38h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*39h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*40h*/{1, 1, 0, 0, 0, 0},    // CMOVO/r reg16,reg/mem16
                              // CMOVO/r reg32,reg/mem32
                              // CMOVO/r reg64,reg/mem64
/*41h*/{1, 1, 0, 0, 0, 0},    // CMOVNO/r reg16,reg/mem16
                              // CMOVNO/r reg32,reg/mem32
                              // CMOVNO/r reg64,reg/mem64
/*42h*/{1, 1, 0, 0, 0, 0},    // CMOVB/r reg16,reg/mem16
                              // CMOVB/r reg32,reg/mem32
                              // CMOVB/r reg64,reg/mem64
                              // CMOVC/r reg16,reg/mem16
                              // CMOVC/r reg32,reg/mem32
                              // CMOVC/r reg64,reg/mem64
                              // CMOVNAE/r reg16,reg/mem16
                              // CMOVNAE/r reg32,reg/mem32
                              // CMOVNAE/r reg64,reg/mem64
/*43h*/{1, 1, 0, 0, 0, 0},    // CMOVAE/r reg16,reg/mem16
                              // CMOVAE/r reg32,reg/mem32
                              // CMOVAE/r reg64,reg/mem64
                              // CMOVNB/r reg16,reg/mem16
                              // CMOVNB/r reg32,reg/mem32
                              // CMOVNB/r reg64,reg/mem64
                              // CMOVNC/r reg16,reg/mem16
                              // CMOVNC/r reg32,reg/mem32
                              // CMOVNC/r reg64,reg/mem64
/*44h*/{1, 1, 0, 0, 0, 0},    // CMOVE/r reg16,reg/mem16
                              // CMOVE/r reg32,reg/mem32
                              // CMOVE/r reg64,reg/mem64
                              // CMOVZ/r reg16,reg/mem16
                              // CMOVZ/r reg32,reg/mem32
                              // CMOVZ/r reg64,reg/mem64
/*45h*/{1, 1, 0, 0, 0, 0},    // CMOVNE/r reg16,reg/mem16
                              // CMOVNE/r reg32,reg/mem32
                              // CMOVNE/r reg64,reg/mem64
                              // CMOVNZ/r reg16,reg/mem16
                              // CMOVNZ/r reg32,reg/mem32
                              // CMOVNZ/r reg64,reg/mem64
/*46h*/{1, 1, 0, 0, 0, 0},    // CMOVBE/r reg16,reg/mem16
                              // CMOVBE/r reg32,reg/mem32
                              // CMOVBE/r reg64,reg/mem64
                              // CMOVNA/r reg16,reg/mem16
                              // CMOVNA/r reg32,reg/mem32
                              // CMOVNA/r reg64,reg/mem64
/*47h*/{1, 1, 0, 0, 0, 0},    // CMOVA/r reg16,reg/mem16
                              // CMOVA/r reg32,reg/mem32
                              // CMOVA/r reg64,reg/mem64
                              // CMOVNBE/r reg 16,reg/mem16
                              // CMOVNBE/r reg32,reg/mem32
                              // CMOVNBE/r reg64,reg/mem64
/*48h*/{1, 1, 0, 0, 0, 0},    // CMOVS/r reg16,reg/mem16
                              // CMOVS/r reg32,reg/mem32
                              // CMOVS/r reg64,reg/mem64
/*49h*/{1, 1, 0, 0, 0, 0},    // CMOVNS/r reg16,reg/mem16
                              // CMOVNS/r reg32,reg/mem32
                              // CMOVNS/r reg64,reg/mem64
/*4Ah*/{1, 1, 0, 0, 0, 0},    // CMOVP/r reg16,reg/mem16
                              // CMOVP/r reg32,reg/mem32
                              // CMOVP/r reg64,reg/mem64
                              // CMOVPE/r reg16,reg/mem16
                              // CMOVPE/r reg32,reg/mem32
                              // CMOVPE/r reg64,reg/mem64
/*4Bh*/{1, 1, 0, 0, 0, 0},    // CMOVNP/r reg16,reg/mem16
                              // CMOVNP/r reg32,reg/mem32
                              // CMOVNP/r reg64,reg/mem64
                              // CMOVPO/r reg16,reg/mem16
                              // CMOVPO/r reg32,reg/mem32
                              // CMOVPO/r reg64,reg/mem64
/*4Ch*/{1, 1, 0, 0, 0, 0},    // CMOVL/r reg16,reg/mem16
                              // CMOVL/r reg32,reg/mem32
                              // CMOVL/r reg64,reg/mem64
                              // CMOVNGE/r reg16,reg/mem16
                              // CMOVNGE/r reg32,reg/mem32
                              // CMOVNGE/r reg64,reg/mem64
/*4Dh*/{1, 1, 0, 0, 0, 0},    // CMOVGE/r reg16,reg/mem16
                              // CMOVGE/r reg32,reg/mem32
                              // CMOVGE/r reg64,reg/mem64
                              // CMOVNL/r reg16,reg/mem16
                              // CMOVNL/r reg32,reg/mem32
                              // CMOVNL/r reg64,reg/mem64
/*4Eh*/{1, 1, 0, 0, 0, 0},    // CMOVLE/r reg16,reg/mem16
                              // CMOVLE/r reg32,reg/mem32
                              // CMOVLE/r reg64,reg/mem64
                              // CMOVNG/r reg16,reg/mem16
                              // CMOVNG/r reg32,reg/mem32
                              // CMOVNG/r reg64,reg/mem64
/*4Fh*/{1, 1, 0, 0, 0, 0},    // CMOVG/r reg16,reg/mem16
                              // CMOVG/r reg32,reg/mem32
                              // CMOVG/r reg64,reg/mem64
                              // CMOVNLE/r reg16,reg/mem16
                              // CMOVNLE/r reg32,reg/mem32
                              // CMOVNLE/r reg64,reg/mem64
/*50h*/{1, 1, 0, 0, 0, 0},    // MOVMSKPS/r reg32,xmm
                              // MOVMSKPS/r reg32,xmm
/*51h*/{1, 1, 0, 0, 0, 0},    // SQRTPS/r xmm1,xmm2/mem128
/*52h*/{1, 1, 0, 0, 0, 0},    // RSQRTPS/r xmm1,xmm2/mem128
/*53h*/{1, 1, 0, 0, 0, 0},    // RCPPS/r xmm1,xmm2/mem128
/*54h*/{1, 1, 0, 0, 0, 0},    // ANDPS/r xmm1,xmm2/mem128
/*55h*/{1, 1, 0, 0, 0, 0},    // ANDNPS/r xmm1,xmm2/mem128
/*56h*/{1, 1, 0, 0, 0, 0},    // ORPS/r xmm1,xmm2/mem128
/*57h*/{1, 1, 0, 0, 0, 0},    // XORPS/r xmm1,xmm2/mem128
/*58h*/{1, 1, 0, 0, 0, 0},    // ADDPS/r xmm1,xmm2/mem128
/*59h*/{1, 1, 0, 0, 0, 0},    // MULPS/r xmm1,xmm2/mem128
/*5Ah*/{1, 1, 0, 0, 0, 0},    // CVTPS2PD/r xmm1,xmm2/mem64
/*5Bh*/{1, 1, 0, 0, 0, 0},    // CVTDQ2PS/r xmm1,xmm2/mem128
/*5Ch*/{1, 1, 0, 0, 0, 0},    // SUBPS/r xmm1,xmm2/mem128
/*5Dh*/{1, 1, 0, 0, 0, 0},    // MINPS/r xmm1,xmm2/mem128
/*5Eh*/{1, 1, 0, 0, 0, 0},    // DIVPS/r xmm1,xmm/2mem128
/*5Fh*/{1, 1, 0, 0, 0, 0},    // MAXPS/r xmm1,xmm2/mem128
/*60h*/{1, 1, 0, 0, 0, 0},    // PUNPCKLBW/r xmm1,xmm2/mem128
/*61h*/{1, 1, 0, 0, 0, 0},    // PUNPCKLWD/r xmm1,xmm2/mem128
/*62h*/{1, 1, 0, 0, 0, 0},    // PUNPCKLDQ/r xmm1,xmm2/mem128
/*63h*/{1, 1, 0, 0, 0, 0},    // PACKSSWB/r xmm1,xmm2/mem128
/*64h*/{1, 1, 0, 0, 0, 0},    // PCMPGTB/r xmm1,xmm2/mem128
/*65h*/{1, 1, 0, 0, 0, 0},    // PCMPGTW/r xmm1,xmm2/mem128
/*66h*/{1, 1, 0, 0, 0, 0},    // PCMPGTD/r xmm1,xmm2/mem128
/*67h*/{1, 1, 0, 0, 0, 0},    // PACKUSWB/r xmm1,xmm2/mem128
/*68h*/{1, 1, 0, 0, 0, 0},    // PUNPCKHBW/r xmm1,xmm2/mem128
/*69h*/{1, 1, 0, 0, 0, 0},    // PUNPCKHWD/r xmm1,xmm2/mem128
/*6Ah*/{1, 1, 0, 0, 0, 0},    // PUNPCKHDQ/r xmm1,xmm2/mem128
/*6Bh*/{1, 1, 0, 0, 0, 0},    // PACKSSDW/r xmm1,xmm2/mem128
/*6Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Eh*/{1, 1, 0, 0, 0, 0},    // MOVD/r mmx,reg/mem32
                              // MOVD/r mmx,reg/mem64
/*6Fh*/{1, 1, 0, 0, 0, 0},    // MOVDQA/r xmm1,xmm2/mem128
/*70h*/{1, 1, 0, 0, 0, 0},    // PSHUFD/r/ib xmm1,xmm2/mem128,imm8
/*71h*/{2, 1, 0, 0, 0, 0},    // PSLLW/6/ib xmm,imm8
                              // PSRAW/4/ib xmm,imm8
                              // PSRLW/2/ib xmm,imm8
/*72h*/{2, 1, 0, 0, 0, 0},    // PSLLD/6/ib xmm,imm8
                              // PSRAD/4/ib xmm,imm8
                              // PSRLD/2/ib xmm,imm8
/*73h*/{2, 1, 0, 0, 0, 0},    // PSLLDQ/7/ib xmm,imm8
                              // PSLLQ/6/ib xmm,imm8
                              // PSRLDQ/3/ib xmm,imm8
                              // PSRLQ/2/ib xmm,imm8
/*74h*/{1, 1, 0, 0, 0, 0},    // PCMPEQB/r xmm1,xmm2/mem128
/*75h*/{1, 1, 0, 0, 0, 0},    // PCMPEQW/r xmm1,xmm2/mem128
/*76h*/{1, 1, 0, 0, 0, 0},    // PCMPEQD/r xmm1,xmm2/mem128
/*77h*/{1, 0, 0, 0, 0, 0},    // EMMS
/*78h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*79h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Eh*/{1, 1, 0, 0, 0, 0},    // MOVD/r reg/mem32,mmx
                              // MOVD/r reg/mem64,mmx
/*7Fh*/{1, 1, 0, 0, 0, 0},    // MOVQ/r xmm1,xmm2/mem64
/*80h*/{2, 0, 1, 0, 0, 1},    // JO/cw rel16off
                              // JO/cd rel32off
/*81h*/{3, 0, 1, 0, 0, 1},    // JNO/cw rel16off
                              // JNO/cd rel32off
/*82h*/{3, 0, 1, 0, 0, 1},    // JB/cw rel16off
                              // JB/cd rel32off
                              // JC/cw rel16off
                              // JC/cd rel32off
                              // JNAE/cw rel16off
                              // JNAE/cd rel32off
/*83h*/{3, 0, 1, 0, 0, 1},    // JAE/cw rel16off
                              // JAE/cd rel32off
                              // JNB/cw rel16off
                              // JNB/cd rel32off
                              // JNC/cw rel16off
                              // JNC/cd rel32off
/*84h*/{3, 0, 1, 0, 0, 1},    // JE/cw rel16off
                              // JE/cd rel32off
                              // JZ/cw rel16off
                              // JZ/cd rel32off
/*85h*/{3, 0, 1, 0, 0, 1},    // JNE/cw rel16off
                              // JNE/cd rel32off
                              // JNZ/cw rel16off
                              // JNZ/cd rel32off
/*86h*/{3, 0, 1, 0, 0, 1},    // JBE/cw rel16off
                              // JBE/cd rel32off
                              // JNA/cw rel16off
                              // JNA/cd rel32off
/*87h*/{3, 0, 1, 0, 0, 1},    // JA/cw rel16off
                              // JA/cd rel32off
                              // JNBE/cw rel16off
                              // JNBE/cd rel32off
/*88h*/{3, 0, 1, 0, 0, 1},    // JS/cw rel16off
                              // JS/cd rel32off
/*89h*/{3, 0, 1, 0, 0, 1},    // JNS/cw rel16off
                              // JNS/cd rel32off
/*8Ah*/{3, 0, 1, 0, 0, 1},    // JP/cw rel16off
                              // JP/cd rel32off
                              // JPE/cw rel16off
                              // JPE/cd rel32off
/*8Bh*/{3, 0, 1, 0, 0, 1},    // JNP/cw rel16off
                              // JNP/cd rel32off
                              // JPO/cw rel16off
                              // JPO/cd rel32off
/*8Ch*/{3, 0, 1, 0, 0, 1},    // JL/cw rel16off
                              // JL/cd rel32off
                              // JNGE/cw rel16off
                              // JNGE/cd rel32off
/*8Dh*/{3, 0, 1, 0, 0, 1},    // JGE/cw rel16off
                              // JGE/cd rel32off
                              // JNL/cw rel16off
                              // JNL/cd rel32off
/*8Eh*/{3, 0, 1, 0, 0, 1},    // JLE/cw rel16off
                              // JLE/cd rel32off
                              // JNG/cw rel16off
                              // JNG/cd rel32off
/*8Fh*/{3, 0, 1, 0, 0, 1},    // JG/cw rel16off
                              // JG/cd rel32off
                              // JNLE/cw rel16off
                              // JNLE/cd rel32off
/*90h*/{1, 1, 0, 0, 0, 0},    // SETO/0 reg/mem8
/*91h*/{1, 1, 0, 0, 0, 0},    // SETNO/0 reg/mem8
/*92h*/{1, 1, 0, 0, 0, 0},    // SETB/0 reg/mem8
                              // SETC/0 reg/mem8
                              // SETNAE/0 reg/mem8
/*93h*/{1, 1, 0, 0, 0, 0},    // SETAE/0 reg/mem8
                              // SETNB/0 reg/mem8
                              // SETNC/0 reg/mem8
/*94h*/{1, 1, 0, 0, 0, 0},    // SETE/0 reg/mem8
                              // SETZ/0 reg/mem8
/*95h*/{1, 1, 0, 0, 0, 0},    // SETNE/0 reg/mem8
                              // SETNZ/0 reg/mem8
/*96h*/{1, 1, 0, 0, 0, 0},    // SETBE/0 reg/mem8
                              // SETNA/0 reg/mem8
/*97h*/{1, 1, 0, 0, 0, 0},    // SETA/0 reg/mem8
                              // SETNBE/0 reg/mem8
/*98h*/{1, 1, 0, 0, 0, 0},    // SETS/0 reg/mem8
/*99h*/{1, 1, 0, 0, 0, 0},    // SETNS/0 reg/mem8
/*9Ah*/{1, 1, 0, 0, 0, 0},    // SETP/0 reg/mem8
                              // SETPE/0 reg/mem8
/*9Bh*/{1, 1, 0, 0, 0, 0},    // SETNP/0 reg/mem8
                              // SETPO/0 reg/mem8
/*9Ch*/{1, 1, 0, 0, 0, 0},    // SETL/0 reg/mem8
                              // SETNGE/0 reg/mem8
/*9Dh*/{1, 1, 0, 0, 0, 0},    // SETGE/0 reg/mem8
                              // SETNL/0 reg/mem8
/*9Eh*/{1, 1, 0, 0, 0, 0},    // SETLE/0 reg/mem8
                              // SETNG/0 reg/mem8
/*9Fh*/{1, 1, 0, 0, 0, 0},    // SETG/0 reg/mem8
                              // SETNLE/0 reg/mem8
/*A0h*/{1, 0, 0, 0, 0, 0},    // PUSH FS
/*A1h*/{1, 0, 0, 0, 0, 0},    // POP FS
/*A2h*/{1, 0, 0, 0, 0, 0},    // CPUID
/*A3h*/{1, 1, 0, 0, 0, 0},    // BT/r reg/mem16,reg16
                              // BT/r reg/mem32,reg32
                              // BT/r reg/mem64,reg64
/*A4h*/{2, 1, 0, 0, 0, 0},    // SHLD/r/ib reg/mem16,reg16,imm8
                              // SHLD/r/ib reg/mem32,reg32,imm8
                              // SHLD/r/ib reg/mem64,reg64,imm8
/*A5h*/{1, 1, 0, 0, 0, 0},    // SHLD/r reg/mem16,reg16,CL
                              // SHLD/r reg/mem32,reg32,CL
                              // SHLD/r reg/mem64,reg64,CL
/*A6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A8h*/{1, 0, 0, 0, 0, 0},    // PUSH GS
/*A9h*/{1, 0, 0, 0, 0, 0},    // POP GS
/*AAh*/{1, 0, 0, 0, 0, 0},    // RSM
/*ABh*/{1, 1, 0, 0, 0, 0},    // BTS/r reg/mem16,reg16
                              // BTS/r reg/mem32,reg32
                              // BTS/r reg/mem64,reg64
/*ACh*/{2, 1, 0, 0, 0, 0},    // SHRD/r/ib reg/mem16,reg16,imm8
                              // SHRD/r/ib reg/mem32,reg32,imm8
                              // SHRD/r/ib reg/mem64,reg64,imm8
/*ADh*/{1, 1, 0, 0, 0, 0},    // SHRD/r reg/mem16,reg16,CL
                              // SHRD/r reg/mem32,reg32,CL
                              // SHRD/r reg/mem64,reg64,CL
/*AEh*/{1, 1, 0, 0, 0, 0},    // LFENCE/E8h (as ModRM byte, will consume 0 extra bytes)
                              // MFENCE/F0h (as ModRM byte, will consume 0 extra bytes)
                              // SFENCE/F8h (as ModRM byte, will consume 0 extra bytes)
                              // CFLUSH/7 mem8
                              // FXRSTOR/1 mem512env
                              // FXSAVE/0 mem512env
                              // LDMXCSR/2 mem32
                              // STMXCSR/3 mem32
/*AFh*/{1, 1, 0, 0, 0, 0},    // IMUL/r reg16,reg/mem16
                              // IMUL/r reg32,reg/mem32
                              // IMUL/r reg64,reg/mem64
/*B0h*/{1, 1, 0, 0, 0, 0},    // CMPXCHG/r reg/mem8,reg8
/*B1h*/{1, 1, 0, 0, 0, 0},    // CMPXCHG/r reg/mem16,reg16
                              // CMPXCHG/r reg/mem32,reg32
                              // CMPXCHG/r reg/mem64,reg64
/*B2h*/{1, 1, 0, 0, 0, 0},    // LSS/r reg16,mem16:16
                              // LSS/r reg32,mem16:32
/*B3h*/{1, 1, 0, 0, 0, 0},    // BTR/r reg/mem16,reg16
                              // BTR/r reg/mem32,reg32
                              // BTR/r reg/mem64,reg64
/*B4h*/{1, 1, 0, 0, 0, 0},    // LFS/r reg16,mem16:16
                              // LFS/r reg32,mem16:32
/*B5h*/{1, 1, 0, 0, 0, 0},    // LGS/r reg16,mem16:16
                              // LGS/r reg32,mem16:32
/*B6h*/{1, 1, 0, 0, 0, 0},    // MOVZX/r reg16,reg/mem8
                              // MOVZX/r reg32,reg/mem8
                              // MOVZX/r reg64,reg/mem8
/*B7h*/{1, 1, 0, 0, 0, 0},    // MOVZX/r reg32,reg/mem16
                              // MOVZX/r reg64,reg/mem16
/*B8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BAh*/{2, 1, 0, 0, 0, 0},    // BT/4/ib reg/mem16,imm8
                              // BT/4/ib reg/mem32,imm8
                              // BT/4/ib reg/mem64,imm8
                              // BTC/7/ib reg/mem16,imm8
                              // BTC/7/ib reg/mem32,imm8
                              // BTC/7/ib reg/mem64,imm8
                              // BTR/6/ib reg/mem16,imm8
                              // BTR/6/ib reg/mem32,imm8
                              // BTR/6/ib reg/mem64,imm8
                              // BTS/5/ib reg/mem16,imm8
                              // BTS/5/ib reg/mem32,imm8
                              // BTS/5/ib reg/mem64,imm8
/*BBh*/{1, 1, 0, 0, 0, 0},    // BTC/r reg/mem16,reg16
                              // BTC/r reg/mem32,reg32
                              // BTC/r reg/mem64,reg64
/*BCh*/{1, 1, 0, 0, 0, 0},    // BSF/r reg16,reg/mem16
                              // BSF/r reg32,reg/mem32
                              // BSF/r reg64,reg/mem64
/*BDh*/{1, 1, 0, 0, 0, 0},    // BSR/r reg16,reg/mem16
                              // BSR/r reg32,reg/mem32
                              // BSR/r reg64,reg/mem64
/*BEh*/{1, 1, 0, 0, 0, 0},    // MOVSX/r reg16,reg/mem8
                              // MOVSX/r reg32,reg/mem8
                              // MOVSX/r reg64,reg/mem8
/*BFh*/{1, 1, 0, 0, 0, 0},    // MOVSX/r reg32,reg/mem16
                              // MOVSX/r reg64,reg/mem16
/*C0h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem8,reg8
/*C1h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem16,reg16
                              // XADD/r reg/mem32,reg32
                              // XADD/r reg/mem64,reg64
/*C2h*/{2, 1, 0, 0, 0, 0},    // CMPPS/r/ib xmm1,xmm2/mem128,imm8
/*C3h*/{1, 1, 0, 0, 0, 0},    // MOVNTI/r mem32,reg32
                              // MOVNTI/r mem64,reg64
/*C4h*/{2, 1, 0, 0, 0, 0},    // PINSRW/r/ib xmm,reg32/mem16,imm8
/*C5h*/{2, 1, 0, 0, 0, 0},    // PEXTRW/r/ib reg32,xmm,imm8
/*C6h*/{2, 1, 0, 0, 0, 0},    // SHUFPS/r/ib xmm1,xmm2/mem128,imm8
/*C7h*/{1, 1, 0, 0, 0, 0},    // CMPXCHG8B/1/m64 mem64
/*C8h*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (EAX)
                              // BSWAP+rd reg64 (RAX)
/*C9h*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (ECX)
                              // BSWAP+rd reg64 (RCX)
/*CAh*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (EDX)
                              // BSWAP+rd reg64 (RDX)
/*CBh*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (EBX)
                              // BSWAP+rd reg64 (RBX)
/*CCh*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (ESP)
                              // BSWAP+rd reg64 (RSP)
/*CDh*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (EBP)
                              // BSWAP+rd reg64 (RBP)
/*CEh*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (ESI)
                              // BSWAP+rd reg64 (RSI)
/*CFh*/{1, 0, 0, 0, 0, 0},    // BSWAP+rd reg32 (EDI)
                              // BSWAP+rd reg32 (RDI)
/*D0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D1h*/{1, 1, 0, 0, 0, 0},    // PSRLW/r xmm1,xmm2/mem128
/*D2h*/{1, 1, 0, 0, 0, 0},    // PSRLD/r xmm1,xmm2/mem128
/*D3h*/{1, 1, 0, 0, 0, 0},    // PSRLQ/r xmm1,xmm2/mem128
/*D4h*/{1, 1, 0, 0, 0, 0},    // PADDQ/r xmm1,xmm2/mem128
/*D5h*/{1, 1, 0, 0, 0, 0},    // PMULLW/r xmm1,xmm2/mem128
/*D6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D7h*/{1, 1, 0, 0, 0, 0},    // PMOVMSKB/r reg32,xmm
/*D8h*/{1, 1, 0, 0, 0, 0},    // PSUBUSB/r xmm1,xmm2/mem128
/*D9h*/{1, 1, 0, 0, 0, 0},    // PSUBUSW/r xmm1,xmm2/mem128
/*DAh*/{1, 1, 0, 0, 0, 0},    // PMINUB/r xmm1,xmm2/mem128
/*DBh*/{1, 1, 0, 0, 0, 0},    // PAND/r xmm1,xmm2/mem128
/*DCh*/{1, 1, 0, 0, 0, 0},    // PADDUSB/r xmm1,xmm2/mem128
/*DDh*/{1, 1, 0, 0, 0, 0},    // PADDUSW/r xmm1,xmm2/mem128
/*DEh*/{1, 1, 0, 0, 0, 0},    // PMAXUB/r xmm1,xmm2/mem128
/*DFh*/{1, 1, 0, 0, 0, 0},    // PANDN/r xmm1,xmm2/mem128
/*E0h*/{1, 1, 0, 0, 0, 0},    // PAVGB/r xmm1,xmm2/mem128
/*E1h*/{1, 1, 0, 0, 0, 0},    // PSRAW/r xmm1,xmm2/mem128
/*E2h*/{1, 1, 0, 0, 0, 0},    // PSRAD/r xmm1,xmm2/mem128
/*E3h*/{1, 1, 0, 0, 0, 0},    // PAVGW/r xmm1,xmm2/mem128
/*E4h*/{1, 1, 0, 0, 0, 0},    // PMULHUW/r xmm1,xmm2/mem128
/*E5h*/{1, 1, 0, 0, 0, 0},    // PMULHW/r xmm1,xmm2/mem128
/*E6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E7h*/{1, 1, 0, 0, 0, 0},    // MOVNTQ/r mem128,xmm
/*E8h*/{1, 1, 0, 0, 0, 0},    // PSUBSB/r xmm1,xmm2/mem128
/*E9h*/{1, 1, 0, 0, 0, 0},    // PSUBSW/r xmm1,xmm2/mem128
/*EAh*/{1, 1, 0, 0, 0, 0},    // PMINSW/r xmm1,xmm2/mem128
/*EBh*/{1, 1, 0, 0, 0, 0},    // POR/r xmm1,xmm2/mem128
/*ECh*/{1, 1, 0, 0, 0, 0},    // PADDSB/r xmm1,xmm2/mem128
/*EDh*/{1, 1, 0, 0, 0, 0},    // PADDSW/r xmm1,xmm2/mem128
/*EEh*/{1, 1, 0, 0, 0, 0},    // PMAXSW/r xmm1,xmm2/mem128
/*EFh*/{1, 1, 0, 0, 0, 0},    // PXOR/r xmm1,xmm2/mem128
/*F0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F1h*/{1, 1, 0, 0, 0, 0},    // PSLLW/r xmm1,xmm2/mem128
/*F2h*/{1, 1, 0, 0, 0, 0},    // PSLLD/r xmm1,xmm2/mem128
/*F3h*/{1, 1, 0, 0, 0, 0},    // PSLLQ/r xmm1,xmm2/mem128
/*F4h*/{1, 1, 0, 0, 0, 0},    // PMULUDQ/r xmm1,xmm2/mem128
/*F5h*/{1, 1, 0, 0, 0, 0},    // PMADDWD/r xmm1,xmm2/mem128
/*F6h*/{1, 1, 0, 0, 0, 0},    // PSADBW/r xmm1,xmm2/mem128
/*F7h*/{1, 1, 0, 0, 0, 0},    // MASKMOVQ/r xmm1,xmm2
/*F8h*/{1, 1, 0, 0, 0, 0},    // PSUBB/r xmm1,xmm2/mem128
/*F9h*/{1, 1, 0, 0, 0, 0},    // PSUBW/r xmm1,xmm2/mem128
/*FAh*/{1, 1, 0, 0, 0, 0},    // PSUBD/r xmm1,xmm2/mem128
/*FBh*/{1, 1, 0, 0, 0, 0},    // PSUBQ/r xmm1,xmm2/mem128
/*FCh*/{1, 1, 0, 0, 0, 0},    // PADDB/r xmm1,xmm2/mem128
/*FDh*/{1, 1, 0, 0, 0, 0},    // PADDW/r xmm1,xmm2/mem128
/*FEh*/{1, 1, 0, 0, 0, 0},    // PADDD/r xmm1,xmm2/mem128
/*FFh*/{0, 0, 0, 0, 0, 0}     // INVALID-OPCODE
};

#ifdef _COJACK_TARGET_MEDIATABLES
/**
 * __InstructionTable000F0F
 *    3 byte 3DNow! opcodes, 0F0F00h-0F0FFFh
 */
static const OPCODE_INFO __InstructionTable000F0F[] =
{
/*00h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*01h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*02h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*03h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*04h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*05h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*06h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*07h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*08h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*09h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ch*/{1, 1, 0, 0, 0, 0},    // PI2FW/r mmx1,mmx2/mem64
/*0Dh*/{1, 1, 0, 0, 0, 0},    // PI2FD/r mmx1,mmx2/mem64
/*0Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*10h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*11h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*12h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*13h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*14h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*15h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*16h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*17h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*18h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*19h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ch*/{1, 1, 0, 0, 0, 0},    // PF2IW/r mmx1,mmx2/mem64
/*1Dh*/{1, 1, 0, 0, 0, 0},    // PF2ID/r mmx1,mmx2/mem64
/*1Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*20h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*21h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*22h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*23h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*24h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*25h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*26h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*27h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*28h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*29h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*30h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*31h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*32h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*33h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*34h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*35h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*36h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*37h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*38h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*39h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*40h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*41h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*42h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*43h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*44h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*45h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*46h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*47h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*48h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*49h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*50h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*51h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*52h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*53h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*54h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*55h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*56h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*57h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*58h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*59h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*5Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*5Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*5Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*5Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*5Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*5Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*60h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*61h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*62h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*63h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*64h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*65h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*66h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*67h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*68h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*69h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*70h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*71h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*72h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*73h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*74h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*75h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*76h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*77h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*78h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*79h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*80h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*81h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*82h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*83h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*84h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*85h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*86h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*87h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*88h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*89h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ah*/{1, 1, 0, 0, 0, 0},    // PFNACC/r mmx1,mmx2/mem64
/*8Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Eh*/{1, 1, 0, 0, 0, 0},    // PFPNACC/r mmx1,mmx2/mem64
/*8Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*90h*/{1, 1, 0, 0, 0, 0},    // PFCMPGE/r mmx1,mmx2/mem64
/*91h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*92h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*93h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*94h*/{1, 1, 0, 0, 0, 0},    // PFMIN/r mmx1,mmx2/mem64
/*95h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*96h*/{1, 1, 0, 0, 0, 0},    // PFRCP/r mmx1,mmx2/mem64
/*97h*/{1, 1, 0, 0, 0, 0},    // PFRSQRT/r mmx1,mmx2/mem64
/*98h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*99h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ah*/{1, 1, 0, 0, 0, 0},    // PFSUB/r mmx1,mmx2/mem64
/*9Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Eh*/{1, 1, 0, 0, 0, 0},    // PFADD/r mmx1,mmx2/mem64
/*9Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A0h*/{1, 1, 0, 0, 0, 0},    // PFCMPGT/r mmx1,mmx2/mem64
/*A1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A4h*/{1, 1, 0, 0, 0, 0},    // PFMAX/r mmx1,mmx2/mem64
/*A5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A6h*/{1, 1, 0, 0, 0, 0},    // PFRCPIT1/r mmx1,mmx2/mem64
/*A7h*/{1, 1, 0, 0, 0, 0},    // PFRSQIT1/r mmx1,mmx2/mem64
/*A8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AAh*/{1, 1, 0, 0, 0, 0},    // PFSUBR/r mmx1,mmx2/mem64
/*ABh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ACh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ADh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AEh*/{1, 1, 0, 0, 0, 0},    // PFACC/r mmx1,mmx2/mem64
/*AFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B0h*/{1, 1, 0, 0, 0, 0},    // PFCMPEQ/r mmx1,mmx2/mem64
/*B1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B4h*/{1, 1, 0, 0, 0, 0},    // PFMUL/r mmx1,mmx2/mem64
/*B5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B6h*/{1, 1, 0, 0, 0, 0},    // PFRCPIT2/r mmx1,mmx2/mem64
/*B7h*/{1, 1, 0, 0, 0, 0},    // PMULHRW/r mmx1,mmx2/mem64
/*B8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BBh*/{1, 1, 0, 0, 0, 0},    // PSWAPD/r mmx1,mmx2/mem64
/*BCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BFh*/{1, 1, 0, 0, 0, 0},    // PAVGUSB/r mmx1,mmx2/mem64
/*C0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ECh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FFh*/{0, 0, 0, 0, 0, 0}     // INVALID-OPCODE
};

/**
 * __InstructionTableF30F0F
 *    3 byte media opcodes, F30F00h-F30FFFh
 */
static const OPCODE_INFO __InstructionTableF30F0F[] =
{
/*00h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*01h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*02h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*03h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*04h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*05h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*06h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*07h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*08h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*09h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*10h*/{1, 1, 0, 0, 0, 0},    // MOVSS xmm1,xmm2/mem32
/*11h*/{1, 1, 0, 0, 0, 0},    // MOVSS xmm1/mem32,xmm2
/*12h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*13h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*14h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*15h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*16h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*17h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*18h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*19h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*20h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*21h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*22h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*23h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*24h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*25h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*26h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*27h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*28h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*29h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Ah*/{1, 1, 0, 0, 0, 0},    // CVTSI2SS xmm,reg/mem32
/*2Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Dh*/{1, 1, 0, 0, 0, 0},    // CVTSS2SI reg32,xmm2/mem32
/*2Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*30h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*31h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*32h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*33h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*34h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*35h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*36h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*37h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*38h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*39h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*40h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*41h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*42h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*43h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*44h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*45h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*46h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*47h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*48h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*49h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*50h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*51h*/{1, 1, 0, 0, 0, 0},    // SQRTSS xmm1,xmm2/mem32
/*52h*/{1, 1, 0, 0, 0, 0},    // RSQRTSS xmm1,xmm2/mem32
/*53h*/{1, 1, 0, 0, 0, 0},    // RCPSS xmm1,xmm2/mem32
/*54h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*55h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*56h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*57h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*58h*/{1, 1, 0, 0, 0, 0},    // ADDSS xmm1,xmm2/mem32
/*59h*/{1, 1, 0, 0, 0, 0},    // MULSS xmm1,xmm2/mem32
/*5Ah*/{1, 1, 0, 0, 0, 0},    // CVTSS2SD xmm1,xmm2/mem32
/*5Bh*/{1, 1, 0, 0, 0, 0},    // CVTTPS2DQ xmm1,xmm2/mem128
/*5Ch*/{1, 1, 0, 0, 0, 0},    // SUBSS xmm1,xmm2/mem32
/*5Dh*/{1, 1, 0, 0, 0, 0},    // MINSS xmm1,xmm2/mem32
/*5Eh*/{1, 1, 0, 0, 0, 0},    // DIVSS xmm1,xmm2/mem32
/*5Fh*/{1, 1, 0, 0, 0, 0},    // MAXSS xmm1,xmm2/mem32
/*60h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*61h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*62h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*63h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*64h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*65h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*66h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*67h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*68h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*69h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Fh*/{1, 1, 0, 0, 0, 0},    // MOVDQU xmm1,xmm2/mem128
/*70h*/{2, 1, 0, 0, 0, 0},    // PSHUFHW xmm1,xmm2/mem128,imm8
/*71h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*72h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*73h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*74h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*75h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*76h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*77h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*78h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*79h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Eh*/{1, 1, 0, 0, 0, 0},    // MOVQ xmm1,xmm2/mem64
/*7Fh*/{1, 1, 0, 0, 0, 0},    // MOVDQU xmm1/mem128,xmm2
/*80h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*81h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*82h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*83h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*84h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*85h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*86h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*87h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*88h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*89h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*90h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*91h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*92h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*93h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*94h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*95h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*96h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*97h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*98h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*99h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ABh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ACh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ADh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C0h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem8,reg8
/*C1h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem16,reg16
                              // XADD/r reg/mem32,reg32
                              // XADD/r reg/mem64,reg64
/*C2h*/{2, 1, 0, 0, 0, 0},    // CMPSS xmm1,xmm2/mem32,imm8
/*C3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D6h*/{1, 1, 0, 0, 0, 0},    // MOVQ2DQ xmm,mmx
/*D7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E6h*/{1, 1, 0, 0, 0, 0},    // CVTDQ2PD xmm1,xmm2/mem64
/*E7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ECh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FFh*/{0, 0, 0, 0, 0, 0}     // INVALID-OPCODE
};

/**
 * __InstructionTable660F0F
 *    3 byte media opcodes, 660F00h-660FFFh
 */
static const OPCODE_INFO __InstructionTable660F0F[] =
{
/*00h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*01h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*02h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*03h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*04h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*05h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*06h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*07h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*08h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*09h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*10h*/{1, 1, 0, 0, 0, 0},    // MOVUPD/r xmm1,xmm2/mem128
/*11h*/{1, 1, 0, 0, 0, 0},    // MOVUPD/r xmm1/mem128,xmm2
/*12h*/{1, 1, 0, 0, 0, 0},    // MOVLPD/r xmm,mem64
/*13h*/{1, 1, 0, 0, 0, 0},    // MOVLPD/r mem64,xmm
/*14h*/{1, 1, 0, 0, 0, 0},    // UNPCKLPD/r xmm1,xmm2/mem128
/*15h*/{1, 1, 0, 0, 0, 0},    // UNPCKHPD/r xmm1,xmm2/mem128
/*16h*/{1, 1, 0, 0, 0, 0},    // MOVHPD/r xmm,mem64
/*17h*/{1, 1, 0, 0, 0, 0},    // MOVHPD/r mem64,xmm
/*18h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*19h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*20h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*21h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*22h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*23h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*24h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*25h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*26h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*27h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*28h*/{1, 1, 0, 0, 0, 0},    // MOVAPD/r xmm1,xmm2/mem128
/*29h*/{1, 1, 0, 0, 0, 0},    // MOVAPD/r xmm1/mem128,xmm2
/*2Ah*/{1, 1, 0, 0, 0, 0},    // CVTPI2PD/r xmm,mmx/mem64
/*2Bh*/{1, 1, 0, 0, 0, 0},    // MOVNTPD/r mem128,xmm
/*2Ch*/{1, 1, 0, 0, 0, 0},    // CVTTPD2PI/r mmx,xmm/mem128
/*2Dh*/{1, 1, 0, 0, 0, 0},    // CVTPD2PI/r mmx,xmm2/mem128
/*2Eh*/{1, 1, 0, 0, 0, 0},    // UCOMISD/r xmm1,xmm2/mem64
/*2Fh*/{1, 1, 0, 0, 0, 0},    // COMISD/r xmm1,xmm2/mem64
/*30h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*31h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*32h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*33h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*34h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*35h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*36h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*37h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*38h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*39h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*40h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*41h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*42h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*43h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*44h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*45h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*46h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*47h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*48h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*49h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*50h*/{1, 1, 0, 0, 0, 0},    // MOVMSKPD/r reg32,xmm
/*51h*/{1, 1, 0, 0, 0, 0},    // SQRTPD/r xmm1,xmm2/mem128
/*52h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*53h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*54h*/{1, 1, 0, 0, 0, 0},    // ANDPD/r xmm1,xmm2/mem128
/*55h*/{1, 1, 0, 0, 0, 0},    // ANDNPD/r xmm1,xmm2/mem128
/*56h*/{1, 1, 0, 0, 0, 0},    // ORPD/r xmm1,xmm2/mem128
/*57h*/{1, 1, 0, 0, 0, 0},    // XORPD/r xmm1,xmm2/mem128
/*58h*/{1, 1, 0, 0, 0, 0},    // ADDPD/r xmm1,xmm2/mem128
/*59h*/{1, 1, 0, 0, 0, 0},    // MULPD/r xmm1,xmm2/mem128
/*5Ah*/{1, 1, 0, 0, 0, 0},    // CVTPD2PS/r xmm1,xmm2/mem128
/*5Bh*/{1, 1, 0, 0, 0, 0},    // CVTPS2DQ/r xmm1,xmm2/mem128
/*5Ch*/{1, 1, 0, 0, 0, 0},    // SUBPD/r xmm1,xmm2/mem128
/*5Dh*/{1, 1, 0, 0, 0, 0},    // MINPD/r xmm1,xmm2/mem128
/*5Eh*/{1, 1, 0, 0, 0, 0},    // DIVPD/r xmm1,xmm2/mem128
/*5Fh*/{1, 1, 0, 0, 0, 0},    // MAXPD/r xmm1,xmm2/mem128
/*60h*/{1, 1, 0, 0, 0, 0},    // PUNPCKLBW/r xmm1,xmm2/mem128
/*61h*/{1, 1, 0, 0, 0, 0},    // PUNPCKLWD/r xmm1,xmm2/mem128
/*62h*/{1, 1, 0, 0, 0, 0},    // PUNPCKLDQ/r xmm1,xmm2/mem128
/*63h*/{1, 1, 0, 0, 0, 0},    // PACKSSWB/r xmm1,xmm2/mem128
/*64h*/{1, 1, 0, 0, 0, 0},    // PCMPGTB/r xmm1,xmm2/mem128
/*65h*/{1, 1, 0, 0, 0, 0},    // PCMPGTW/r xmm1,xmm2/mem128
/*66h*/{1, 1, 0, 0, 0, 0},    // PCMPGTD/r xmm1,xmm2/mem128
/*67h*/{1, 1, 0, 0, 0, 0},    // PACKUSWB/r xmm1,xmm2/mem128
/*68h*/{1, 1, 0, 0, 0, 0},    // PUNPCKHBW/r xmm1,xmm2/mem128
/*69h*/{1, 1, 0, 0, 0, 0},    // PUNPCKHWD/r xmm1,xmm2/mem128
/*6Ah*/{1, 1, 0, 0, 0, 0},    // PUNPCKHDQ/r xmm1,xmm2/mem128
/*6Bh*/{1, 1, 0, 0, 0, 0},    // PACKSSDW/r xmm1,xmm2/mem128
/*6Ch*/{1, 1, 0, 0, 0, 0},    // PUNPCKLQDQ/r xmm1,xmm2/mem128
/*6Dh*/{1, 1, 0, 0, 0, 0},    // PUNPCKHQDQ/r xmm1,xmm2/mem128
/*6Eh*/{1, 1, 0, 0, 0, 0},    // MOVD/r xmm,reg/mem32
/*6Fh*/{1, 1, 0, 0, 0, 0},    // MOVDQA/r xmm1,xmm2/mem128
/*70h*/{2, 1, 0, 0, 0, 0},    // PSHUFD/r/ib xmm1,xmm2/mem128,imm8
/*71h*/{2, 1, 0, 0, 0, 0},    // PSRLW/2/ib xmm,imm8
                              // PSRAW/4/ib xmm,imm8
                              // PSLLW/6/ib xmm,imm8
/*72h*/{2, 1, 0, 0, 0, 0},    // PSRLD/2/ib xmm,imm8
                              // PSRAD/4/ib xmm,imm8
                              // PSLLD/6/ib xmm,imm8
/*73h*/{2, 1, 0, 0, 0, 0},    // PSRLQ/2/ib xmm,imm8
                              // PSRLDQ/3/ib xmm,imm8
                              // PSLLQ/6/ib xmm,imm8
                              // PSLLDQ/7/ib xmm,imm8
/*74h*/{1, 1, 0, 0, 0, 0},    // PCMPEQB/r xmm1,xmm2/mem128
/*75h*/{1, 1, 0, 0, 0, 0},    // PCMPEQW/r xmm1,xmm2/mem128
/*76h*/{1, 1, 0, 0, 0, 0},    // PCMPEQD/r xmm1,xmm2/mem128
/*77h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*78h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*79h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ch*/{1, 1, 0, 0, 0, 0},    // HADDPD/r xmm1,xmm2/mem128
                              // HADDPS/r xmm1,xmm2/mem128
/*7Dh*/{1, 1, 0, 0, 0, 0},    // HSUBPD/r xmm1,xmm2/mem128
/*7Eh*/{1, 1, 0, 0, 0, 0},    // MOVD/r reg/mem32,xmm
/*7Fh*/{1, 1, 0, 0, 0, 0},    // MOVDQA/r xmm1/mem128,xmm2
/*80h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*81h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*82h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*83h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*84h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*85h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*86h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*87h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*88h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*89h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*90h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*91h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*92h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*93h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*94h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*95h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*96h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*97h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*98h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*99h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ABh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ACh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ADh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C0h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem8,reg8
/*C1h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem16,reg16
                              // XADD/r reg/mem32,reg32
                              // XADD/r reg/mem64,reg64
/*C2h*/{1, 1, 0, 0, 0, 0},    // CMPPD/r/ib xmm1,xmm2/mem128,imm8
/*C3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C4h*/{2, 1, 0, 0, 0, 0},    // PINSRW/r/ib xmm,reg32/mem16,imm8
/*C5h*/{2, 1, 0, 0, 0, 0},    // PEXTRW/r/ib reg32,xmm,imm8
/*C6h*/{2, 1, 0, 0, 0, 0},    // SHUFPD/r/ib xmm1,xmm2/mem128,imm8
/*C7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D0h*/{1, 1, 0, 0, 0, 0},    // ADDSUBPD/r xmm1,xmm2/mem128
/*D1h*/{1, 1, 0, 0, 0, 0},    // PSRLW/r xmm1,xmm2/mem128
/*D2h*/{1, 1, 0, 0, 0, 0},    // PSRLD/r xmm1,xmm2/mem128
/*D3h*/{1, 1, 0, 0, 0, 0},    // PSRLQ/r xmm1,xmm2/mem128
/*D4h*/{1, 1, 0, 0, 0, 0},    // PADDQ/r xmm1,xmm2/mem128
/*D5h*/{1, 1, 0, 0, 0, 0},    // PMULLW/r xmm1,xmm2/mem128
/*D6h*/{1, 1, 0, 0, 0, 0},    // MOVQ/r xmm1/mem64,xmm2
/*D7h*/{1, 1, 0, 0, 0, 0},    // PMOVMSKB/r reg32,xmm
/*D8h*/{1, 1, 0, 0, 0, 0},    // PSUBUSB/r xmm1,xmm2/mem128
/*D9h*/{1, 1, 0, 0, 0, 0},    // PSUBUSW/r xmm1,xmm2/mem128
/*DAh*/{1, 1, 0, 0, 0, 0},    // PMINUB/r xmm1,xmm2/mem128
/*DBh*/{1, 1, 0, 0, 0, 0},    // PAND/r xmm1,xmm2/mem128
/*DCh*/{1, 1, 0, 0, 0, 0},    // PADDUSB/r xmm1,xmm2/mem128
/*DDh*/{1, 1, 0, 0, 0, 0},    // PADDUSW/r xmm1,xmm2/mem128
/*DEh*/{1, 1, 0, 0, 0, 0},    // PMAXUB/r xmm1,xmm2/mem128
/*DFh*/{1, 1, 0, 0, 0, 0},    // PANDN/r xmm1,xmm2/mem128
/*E0h*/{1, 1, 0, 0, 0, 0},    // PAVGB/r xmm1,xmm2/mem128
/*E1h*/{1, 1, 0, 0, 0, 0},    // PSRAW/r xmm1,xmm2/mem128
/*E2h*/{1, 1, 0, 0, 0, 0},    // PSRAD/r xmm1,xmm2/mem128
/*E3h*/{1, 1, 0, 0, 0, 0},    // PAVGW/r xmm1,xmm2/mem128
/*E4h*/{1, 1, 0, 0, 0, 0},    // PMULHUW/r xmm1,xmm2/mem128
/*E5h*/{1, 1, 0, 0, 0, 0},    // PMULHW/r xmm1,xmm2/mem128
/*E6h*/{1, 1, 0, 0, 0, 0},    // CVTTPD2DQ/r xmm1,xmm2/mem128
/*E7h*/{1, 1, 0, 0, 0, 0},    // MOVNTDQ/r mem128,xmm
/*E8h*/{1, 1, 0, 0, 0, 0},    // PSUBSB/r xmm1,xmm2/mem128
/*E9h*/{1, 1, 0, 0, 0, 0},    // PSUBSW/r xmm1,xmm2/mem128
/*EAh*/{1, 1, 0, 0, 0, 0},    // PMINSW/r xmm1,xmm2/mem128
/*EBh*/{1, 1, 0, 0, 0, 0},    // POR/r xmm1,xmm2/mem128
/*ECh*/{1, 1, 0, 0, 0, 0},    // PADDSB/r xmm1,xmm2/mem128
/*EDh*/{1, 1, 0, 0, 0, 0},    // PADDSW/r xmm1,xmm2/mem128
/*EEh*/{1, 1, 0, 0, 0, 0},    // PMAXSW/r xmm1,xmm2/mem128
/*EFh*/{1, 1, 0, 0, 0, 0},    // PXOR/r xmm1,xmm2/mem128
/*F0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F1h*/{1, 1, 0, 0, 0, 0},    // PSLLW/r xmm1,xmm2/mem128
/*F2h*/{1, 1, 0, 0, 0, 0},    // PSLLD/r xmm1,xmm2/mem128
/*F3h*/{1, 1, 0, 0, 0, 0},    // PSLLQ/r xmm1,xmm2/mem128
/*F4h*/{1, 1, 0, 0, 0, 0},    // PMULUDQ/r xmm1,xmm2/mem128
/*F5h*/{1, 1, 0, 0, 0, 0},    // PMADDWD/r xmm1,xmm2/mem128
/*F6h*/{1, 1, 0, 0, 0, 0},    // PSADBW/r xmm1,xmm2/mem128
/*F7h*/{1, 1, 0, 0, 0, 0},    // MASKMOVDQU/r xmm1,xmm2
/*F8h*/{1, 1, 0, 0, 0, 0},    // PSUBB/r xmm1,xmm2/mem128
/*F9h*/{1, 1, 0, 0, 0, 0},    // PSUBW/r xmm1,xmm2/mem128
/*FAh*/{1, 1, 0, 0, 0, 0},    // PSUBD/r xmm1,xmm2/mem128
/*FBh*/{1, 1, 0, 0, 0, 0},    // PSUBQ/r xmm1,xmm2/mem128
/*FCh*/{1, 1, 0, 0, 0, 0},    // PADDB/r xmm1,xmm2/mem128
/*FDh*/{1, 1, 0, 0, 0, 0},    // PADDW/r xmm1,xmm2/mem128
/*FEh*/{1, 1, 0, 0, 0, 0},    // PADDD/r xmm1,xmm2/mem128
/*FFh*/{0, 0, 0, 0, 0, 0}     // INVALID-OPCODE
};

/**
 * __InstructionTableF20F0F
 *    3 byte media opcodes, F20F00h-F20FFFh
 */
static const OPCODE_INFO __InstructionTableF20F0F[] =
{
/*00h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*01h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*02h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*03h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*04h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*05h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*06h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*07h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*08h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*09h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*0Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*10h*/{1, 1, 0, 0, 0, 0},    // MOVSD xmm1,xmm2/mem64
/*11h*/{1, 1, 0, 0, 0, 0},    // MOVSD xmm1/mem64,xmm2
/*12h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*13h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*14h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*15h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*16h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*17h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*18h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*19h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*1Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*20h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*21h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*22h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*23h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*24h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*25h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*26h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*27h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*28h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*29h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Ch*/{1, 1, 0, 0, 0, 0},    // CVTTSD2SI reg32,xmm/mem64
                              // CVTTSD2SI reg64,xmm/mem64
/*2Dh*/{1, 1, 0, 0, 0, 0},    // CVTSD2SI reg32,xmm/mem64
                              // CVTSD2SI reg64,xmm/mem64
/*2Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*2Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*30h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*31h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*32h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*33h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*34h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*35h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*36h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*37h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*38h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*39h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*3Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*40h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*41h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*42h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*43h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*44h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*45h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*46h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*47h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*48h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*49h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*4Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*50h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*51h*/{1, 1, 0, 0, 0, 0},    // SQRTSD/r xmm1,xmm2/mem64
/*52h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*53h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*54h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*55h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*56h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*57h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*58h*/{1, 1, 0, 0, 0, 0},    // ADDSD xmm1,xmm2/mem64
/*59h*/{1, 1, 0, 0, 0, 0},    // MULSD xmm1,xmm2/mem64
/*5Ah*/{1, 1, 0, 0, 0, 0},    // CVTSD2SS xmm1,xmm2/mem64
/*5Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*5Ch*/{1, 1, 0, 0, 0, 0},    // SUBSD xmm1,xmm2/mem64
/*5Dh*/{1, 1, 0, 0, 0, 0},    // MINSD xmm1,xmm2/mem64
/*5Eh*/{1, 1, 0, 0, 0, 0},    // DIVSD xmm1,xmm2/mem64
/*5Fh*/{1, 1, 0, 0, 0, 0},    // MAXSD xmm1,xmm2/mem64
/*60h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*61h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*62h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*63h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*64h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*65h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*66h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*67h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*68h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*69h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*6Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*70h*/{2, 1, 0, 0, 0, 0},    // PSHUFLW xmm1,xmm2/mem128,imm8
/*71h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*72h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*73h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*74h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*75h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*76h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*77h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*78h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*79h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*7Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*80h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*81h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*82h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*83h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*84h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*85h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*86h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*87h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*88h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*89h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*8Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*90h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*91h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*92h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*93h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*94h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*95h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*96h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*97h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*98h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*99h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ah*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Bh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Ch*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Dh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Eh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*9Fh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*A9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ABh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ACh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ADh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*AFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*B9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*BFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C0h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem8,reg8
/*C1h*/{1, 1, 0, 0, 0, 0},    // XADD/r reg/mem16,reg16
                              // XADD/r reg/mem32,reg32
                              // XADD/r reg/mem64,reg64
/*C2h*/{2, 1, 0, 0, 0, 0},    // CMPSD xmm1,xmm2/mem64,imm8
/*C3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*C9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*CFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D6h*/{1, 1, 0, 0, 0, 0},    // MOVDQ2Q/r mmx,xmm
/*D7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*D9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*DFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E6h*/{1, 1, 0, 0, 0, 0},    // CVTPD2DQ xmm1,xmm2/mem128
/*E7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*E9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*ECh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*EFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F0h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F1h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F2h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F3h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F4h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F5h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F6h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F7h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F8h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*F9h*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FAh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FBh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FCh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FDh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FEh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
/*FFh*/{0, 0, 0, 0, 0, 0},    // INVALID-OPCODE
};
#endif /* _COJACK_TARGET_MEDIATABLES */
#pragma endregion InstructionTables
#pragma pack( pop )

/**********************************************************************
    
    Cojack : Exports

 **********************************************************************/

/**
 * DECLARE_CALLGATE
 *    Macro to build a "callgate" function
 * 
 * Parameters
 *    _Prototype
 *       [in] C-style function prototype which matches the
 *       function being hooked
 *    _DummyReturn
 *       [in] A placeholder return value used to satisfy the
 *       compiler. For no return value, specifiy CALLGATE_NORETURN.

 * Usage:
 *    DECLARE_CALLGATE(DWORD WINAPI _CgGetTickCount( ), 0);
 *    DECLARE_CALLGATE(VOID WINAPI _CgSomeFunction( int i ), CALLGATE_NORETURN);
 *
 * Remarks
 *    Callgates are used by this library to hold instructions
 *    copied from a hooked routine, and for transfering control
 *    back to that routine from an unrelated body of code. A
 *    callgate built with this macro guarrantees at least 64
 *    bytes of storage by the use of the 64 0xCC opcodes. The
 *    __debugbreak() intrinsic is used for its support in both
 *    the x86 and x64 compilers.
 */
#define DECLARE_CALLGATE( _Prototype, _DummyReturn ) \
   __pragma(optimize("",off)) \
   static __declspec(noinline) _Prototype \
   { \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      __debugbreak();__debugbreak();__debugbreak();__debugbreak(); \
      return _DummyReturn; \
   } \
   __pragma(optimize("",on))

#define CALLGATE_NORETURN  ;
#define CALLGATE_SIZEOF    64
   
/* Utility stuff */
#pragma pack( push )
#pragma pack( 1 )
typedef enum _MODRM_FLAGS
{
   MODRM_MOD_NODISP = 0,
   MODRM_MOD_DISP8  = 1,
   MODRM_MOD_DISP32 = 2,
   MODRM_MOD_GPREG  = 3,

   MODRM_RM_SIB     = 4,
   MODRM_RM_RIP     = 5,
   MODRM_RM_DISP32  = 5,
   MODRM_RM_DISP16  = 6,
}MODRM_FLAGS;

typedef enum _SIB_FLAGS
{
   SIB_BASE_DISP    = 5
}SIB_FLAGS;

typedef union _PREFIX_GROUP
{
   struct
   {
      ULONG Operand : 1;
      ULONG Address : 1;
      ULONG Segment : 1;
      ULONG Lock    : 1;
      ULONG Repeat  : 1;
      ULONG Rex     : 1;
   };
   ULONG    zBitField;
}PREFIX_GROUP, *PPREFIX_GROUP;

typedef union _PREFIXES
{
   struct
   {
      ULONG Operand   : 1;
      ULONG Address   : 1;
      ULONG SegmentCS : 1;
      ULONG SegmentDS : 1;
      ULONG SegmentES : 1;
      ULONG SegmentFS : 1;
      ULONG SegmentGS : 1;
      ULONG SegmentSS : 1;
      ULONG Lock      : 1;
      ULONG Rep       : 1;
      ULONG Repz      : 1;
      ULONG RepNz     : 1;
      ULONG Rex       : 1;
   };
   ULONG    zBitField;
}PREFIXES, *PPREFIXES;

typedef union _MODRM
{
   struct
   {
      BYTE  RM  : 3;
      BYTE  Reg : 3;
      BYTE  Mod : 2;
   };
   BYTE     zBitField;
}MODRM, *PMODRM;

typedef union _SIB
{
   struct
   {
      BYTE Base  : 3;
      BYTE Index : 3;
      BYTE Scale : 2;
   };
   BYTE    zBitField;
}SIB, *PSIB;

typedef union _REX
{
   struct
   {
      BYTE B : 1;
      BYTE X : 1;
      BYTE R : 1;
      BYTE W : 1;
      BYTE U : 4;
   };
   BYTE    zBitField;
}REX, *PREX;

typedef struct _FAR_PTR
{
   LONG  Offset;
   SHORT Selector;
}FAR_PTR, *PFAR_PTR;

typedef struct _DISPIMM
{
   union
   {
      CHAR      Disp8;
      SHORT     Disp16;
      LONG      Disp32;
      FAR_PTR   Disp48;
      LONGLONG  Disp64;

      CHAR      Imm8;
      SHORT     Imm16;
      LONG      Imm32;
      LONGLONG  Imm64;
   };
}DISPIMM, *PDISPIMM;

typedef struct _DISPLACEMENT
{
   BYTE         Size;
   PUCHAR       Base;
   union
   {
      CHAR      Disp8;
      SHORT     Disp16;
      LONG      Disp32;
      FAR_PTR   Disp48;
#ifdef _M_X64
      LONGLONG  Disp64;
#endif /* _M_X64 */
      LONG_PTR  DispPtr;      
   };
}DISPLACEMENT, *PDISPLACEMENT;

typedef struct _IMMEDIATE
{
   BYTE        Size;
   union
   {
      CHAR     Imm8;
      SHORT    Imm16;
      LONG     Imm32;
#ifdef _M_X64
      LONGLONG Imm64;
#endif /* _M_X64 */
      LONG_PTR ImmPtr;
   };
}IMMEDIATE, *PIMMEDIATE;

typedef union _JMPSEQ
{
   struct
   {
      BYTE     OpCode[2];
      SHORT    Offset;
   }Jmp16;

   struct
   {
      BYTE     OpCode[1];
      LONG     Offset;
   }Jmp32;

   struct
   {
      BYTE     OpCode[1];
      FAR_PTR  Offset;
   }Jmp48;

   struct
   {
      BYTE     OpCode[7];
      LONGLONG Offset;
   }Jmp64;
}JMPSEQ, *PJMPSEQ;

static const BYTE bJmp16[] = {0x66, 0xE9, 0x00, 0x00};
static const BYTE bJmp32[] = {0xE9, 0x00, 0x00, 0x00, 0x00};
static const BYTE bJmp48[] = {0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const BYTE bJmp64[] = {0x40, 0xFF, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#pragma pack( pop )

/* This is a utility record passed to decoding functions and does
 * not require 1-byte alignment */
typedef struct _DECODE_INFO
{
   OPCODE_INFO  OpInfo;
   PREFIXES     Prefix;
   REX          Rex;
   ULONG        OpCode;
   MODRM        ModRM;
   SIB          Sib;
   IMMEDIATE    Imm[2];
   DISPLACEMENT Disp;
   DISPIMM      DispImm[3];
}DECODE_INFO, *PDECODE_INFO;

#ifndef _countof
   #define _countof( rg ) (sizeof(rg) / sizeof(rg[0]))
#endif /* _countof */

__inline void ResetDecodeInfo( DECODE_INFO* pdi ) throw()
{
   ZeroMemory(pdi, 
              sizeof(DECODE_INFO));
}

__inline bool IsEncodedRIP( const DECODE_INFO* pdi ) throw()
{
   return ( !pdi->Prefix.Address && (MODRM_MOD_NODISP == pdi->ModRM.Mod) && (MODRM_RM_RIP == pdi->ModRM.RM) );
}

__inline bool IsEncodedSIB( const DECODE_INFO* pdi ) throw() 
{
   return ( !pdi->Prefix.Address && (MODRM_MOD_GPREG != pdi->ModRM.Mod) && (MODRM_RM_SIB == pdi->ModRM.RM) );
}

#define MOV_AL_MOFFSET8 0xA0
#define MOV_RAX_MOFFSET 0xA1
#define MOV_MOFFSET8_AL 0xA2
#define MOV_MOFFSET_RAX 0xA3

__inline BYTE GetSizeOfEffectiveAddress( const DECODE_INFO* pdi ) throw()
{
   if ( (MOV_AL_MOFFSET8 == pdi->OpCode) || (MOV_MOFFSET8_AL == pdi->OpCode) )
   {
      /* 
       * MOV AL,moffset8 
       * MOV moffset8,AL 
       */
      return ( pdi->Prefix.Address ? 0 : 1 );
   }
   else if ( (MOV_RAX_MOFFSET == pdi->OpCode) || (MOV_MOFFSET_RAX == pdi->OpCode) )
   {
      /* 
       * MOV rAX,moffset(16|32|64)
       * MOV moffset(16|32|64),rAX 
       */
#ifdef _M_X64
      return ( pdi->Prefix.Address ? 4 : 8 );
#else /* _M_X64 */
      return ( pdi->Prefix.Address ? 2 : 4 );
#endif /* _M_X64 */
   }

   /* Default address size for X86 & X64 is 32bits. Address size override
    * prefix changes it to 16bits */
   return ( pdi->Prefix.Address ? 2 : 4 );
}

__inline BYTE GetSizeOfModRM( const DECODE_INFO* pdi ) throw() 
{
#ifdef _M_X64
   if ( pdi->Prefix.Address && !pdi->Prefix.Rex )
#else /* _M_X64 */
   if ( pdi->Prefix.Address )
#endif /* _M_X64 */
   {
      /* Use the 16bit ModRM table */
      switch ( pdi->ModRM.Mod ) {
         case 0:
            return ( MODRM_RM_DISP16 == pdi->ModRM.RM ? 2 : 0 );
         case 1:
            return ( 1 );
         case 2:
            return ( 2 );
         case 3:
            return ( 0 );
      }

      /* Should never get here */
      _ASSERTE(FALSE);
   }

   /* Use the 32bit/64bit ModRM table */
   switch ( pdi->ModRM.Mod ) {
      case 0:
         return ( MODRM_RM_DISP32 == pdi->ModRM.RM ? 4 : 0 );
      case 1:
         return ( 1 );
      case 2:
         return ( 4 );
      case 3:
         return ( 0 );
   }

   _ASSERTE(FALSE);
   /* Failure */
   return ( 0 );
}

__inline BYTE GetSizeOfSIB( const DECODE_INFO* pdi ) throw() 
{
   if ( SIB_BASE_DISP != pdi->Sib.Base )
   {
      return ( 0 );
   }

   switch ( pdi->ModRM.Mod ) {
      case 0:
         return ( 4 );
      case 1:
         return ( 1 );
      case 2:
         return ( 4 );
   }

   /* Should never be reading an SIB byte when ModRM.mod=3 */
   _ASSERTE(FALSE);
   /* Failure */
   return ( 0 );
}

__inline BYTE GetSizeOfOpCode( const DECODE_INFO* pdi ) throw()
{
   /* Start with the base size of the instruction */
   BYTE cb;      
   cb = pdi->OpInfo.Size;

   /* Include any address bytes. This accounts for only a few instructions (A0h-A3h) */
   if ( pdi->OpInfo.IsAddress ) 
   {
      /* Addresses are always extended to the default size of the decoding mode. eg.

         64BIT: 48 A1 E2 B0 00 00 00 00 00 00   mov rax,qword ptr [000000000000B0E2h]
         64BIT: A1 E2 B0 00 00 00 00 00 00      mov eax,dword ptr [000000000000B0E2h]
         64BIT: 66 A1 E2 B0 00 00 00 00 00 00   mov ax,word ptr [000000000000B0E2h]
         64BIT: 67 A1 E2 B0 00 00               mov eax,dword ptr [0000B0E2h]
         
         32BIT: A1 E2 B0 00 00                  mov eax,dword ptr [0000B0E2h]
         32BIT: 67 A1 E2 B0                     mov eax,dword ptr [B0E2h]
      */
   #ifdef _M_X64
      /* REX has no effect on address-size */

      /* 67h for 64bits reduces the size to 32bits, otherwise this is 16->64 bits */
      cb += (pdi->Prefix.Address ? 2 : 6);
   #else /* _M_X64 */
      /* 66h reduces the address size to 16bits in this mode, but the tables
       * are already in that format so nothing additional is added if the 
       * prefix was specified. Otherwise, this is 16->32 bits */
      cb += (pdi->Prefix.Address ? 0 : 2);
   #endif /* _M_X64 */
   }

   /* Include any additional immediate bytes */
#ifdef _M_X64
   /* Mode:64, OperandSize:32bits, AddressSize:64bits */
   if ( pdi->OpInfo.Extend64 && pdi->Rex.W ) 
   {
      /* If the REX.W bit is set then the operand size is extended
       * to 64bits, regardless of a 66h prefix */
      cb += 6;
   }
   else if ( pdi->OpInfo.Extend32 ) 
   {
      /* In 64bit mode, instructions that use relative addressing
       * ignore the 66h prefix and promote to the required size */
      cb += (pdi->Prefix.Operand && !pdi->OpInfo.IsRelative ? 0 : 2);
   }
#else /* _M_X64 */
   /* MODE:32bits, OperandSize:32bits, AddressSize:32bits */
   if ( pdi->OpInfo.Extend32 ) 
   {
      cb += (pdi->Prefix.Operand ? 0 : 2);
   }
#endif /* _M_X64 */

   return ( cb );
}

__inline void __stdcall ExtractDisplacement( PDISPLACEMENT pDisp, PUCHAR pbData, BYTE cbData ) throw()
{
   pDisp->Size    = cbData;
   pDisp->Base    = pbData + cbData;
   pDisp->DispPtr = 0;

   switch ( cbData )
   {
      case sizeof(CHAR):
         pDisp->Disp8 = ((PDISPLACEMENT)pbData)->Disp8;
         break;
      case sizeof(SHORT):
         pDisp->Disp16 = ((PDISPLACEMENT)pbData)->Disp16;
         break;
      case sizeof(LONG):
         pDisp->Disp32 = ((PDISPLACEMENT)pbData)->Disp32;
         break;
      case sizeof(FAR_PTR):
         pDisp->Disp48 = ((PDISPLACEMENT)pbData)->Disp48;
         break;
   #ifdef _M_X64
      case sizeof(LONGLONG):
         //TODO:Does RIP relative address effect this?
         pDisp->Base   = 0;
         pDisp->Disp64 = ((PDISPLACEMENT)pbData)->Disp64;
         break;
   #endif /* _M_X64 */
   }
}

__inline PUCHAR __stdcall GetTargetOfDisplacement( PDISPLACEMENT pDisp ) throw()
{
   if ( sizeof(FAR_PTR) == pDisp->Size )
   {
      /* Ignore the selector */
      return ( pDisp->Base + pDisp->Disp48.Offset );
   }

   return ( pDisp->Base + pDisp->DispPtr );
}

__inline bool __stdcall RecalculateDisplacement( PDISPLACEMENT pDisp, PUCHAR pbBase ) throw()
{
   bool     bRet;
   PUCHAR   pbTarget;
   LONGLONG iDisp;

   /* Calculate the intended target of the displacement */
   pbTarget = GetTargetOfDisplacement(pDisp);

   /* Calculate the displacement for the target from the new base */
   iDisp = (LONG_PTR)(pbTarget - pbBase);
 
   bRet        = false;
   pDisp->Base = pbBase;

   switch ( pDisp->Size )
   {
      case sizeof(CHAR):
         if ( (iDisp >= CHAR_MIN) && (iDisp <= CHAR_MAX) )
         {
            bRet = true;
            pDisp->Disp8 = (CHAR)iDisp;
         }
         break;
      case sizeof(SHORT):
         if ( (iDisp >= SHRT_MIN) && (iDisp <= SHRT_MAX) )
         {
            bRet = true;
            pDisp->Disp16 = (SHORT)iDisp;
         }
         break;
      case sizeof(LONG):
         if ( (iDisp >= LONG_MIN) && (iDisp <= LONG_MIN) )
         {
            bRet = true;
            pDisp->Disp32 = (LONG)iDisp;
         }
         break;

      case sizeof(FAR_PTR):
         if ( (iDisp >= LONG_MIN) && (iDisp <= LONG_MAX) )
         {  
            /* This assumes that pbTarget is in the same segement as pbBase */
            bRet = true;
            pDisp->Disp48.Offset = (LONG)iDisp;
         }
         break;
#ifdef _M_X64
      case sizeof(LONGLONG):
         bRet = true;
         //TODO:Does RIP relative address effect this?
         /* 64bit displacement is always a direct address, so ignore the
          * calculated displacement and use the target */
         pDisp->Disp64 = (LONGLONG)pbTarget;
         /* Clear the base so 64bit displacement is flat */
         pDisp->Base = 0;
#endif /* _M_X64 */
   }
   
   /* Success / Failure */
   return ( bRet );
}

/**
 * _RelocateInstructionStream
 *    Copies complete instructions up to a specified size to a callgate,
 *    modifying any necessary displacements in the process.
 *
 * Parameters
 *    pbStream
 *       [in] Base address of the instruction stream to relocate
 *    pbCallgate
 *       [in] Address of a block of memory to receive the relocated
 *       instructions
 *    cbCallgate
 *       [in]
 *          Size of the callgate in bytes
 *    cbMinimum
 *       [in] Minimum number of bytes to copy from the instruction stream
 *       to the callgate
 *    pcbActual
 *       [out] Number of bytes copied to the callgate
 */
static DWORD __stdcall _RelocateInstructionStream( PUCHAR pbStream, BYTE* pbCallgate, size_t cbCallgate, size_t cbMinimum, size_t* pcbActual ) throw()
{
   DWORD          dwRet;
   BYTE           cbAdvance;
   PUCHAR        pbBase;
   size_t         cbCopy;
   size_t         cbCopied;
   DISPLACEMENT   disp;
   DECODE_INFO    dInfo;

   /* Initialize output params */
   (*pcbActual) = 0;

   /* Initialize locals */
   dwRet      = ERROR_GEN_FAILURE;   
   cbAdvance  = 0;
   pbBase     = pbStream;
   cbCopied   = 0;
   ResetDecodeInfo(&dInfo);
   
   __try
   {
      while ( cbCallgate )
      {
         /* Consume any prefixes... */
         if ( 0x66 == (*pbStream) ) 
         {
            /* OPERAND-SIZE-OVERRIDE 
             *    Valid in any mode on general-purpose instructions and 
             *    FLDENV, FNSTENV, FNSAVE, and FRSTOR */
            dInfo.Prefix.Operand = 1;

            pbStream += 1;
            /* Loop to read the next byte */
            continue;
         }
         else if ( 0x67 == (*pbStream) ) 
         {
            /* ADDRESS-SIZE-OVERRIDE 
             *    Valid in any mode on any instruction */
            dInfo.Prefix.Address = 1;

            pbStream += 1;
            /* Loop to read the next byte */
            continue;
         }
   #ifdef _M_X64
         else if ( ((*pbStream) >= 0x40) && ((*pbStream) <= 0x4F) ) 
         {
            /* REX 
             *    Only valid in 64bit mode. For any other mode, 
             *    it is an opcode byte */            
            dInfo.Rex.zBitField = (*pbStream);
            dInfo.Prefix.Rex    = 1;

            pbStream += 1;
            /* Loop to read the next byte */
            continue;
         }
   #endif /* _M_X64 */
   
         /* All other prefixes are ignored */

         if ( 0x0F == (*pbStream) ) 
         {
            /* MULTI-BYTE OPCODE */

            /* The next byte is needed to determine the opcode, so advance the stream */
            pbStream += 1;

            dInfo.OpCode = 0x00000F00|(*pbStream);
         #ifdef _COJACK_TARGET_MEDIATABLES
            /* If the stream has moved over 2 bytes, then the current byte could be part of
             * a 3-byte media instruction. To be sure, the 2nd byte back, the value 0Fh and
             * the current byte are used to construct a 3-byte prefix. If the prefix matches 
             * a known one then its associated table is used to find the instruction, otherwise
             * it is assumed that the previous bytes are legacy prefixes and the general purpose 
             * 2-byte instruction table is used */            
            if ( (pbStream - pbBase) > 2 ) 
            {
               dInfo.OpCode |= (pbStream[-2] << 16);
            }

            //TODO: what happens when a prefix is found but the instruction in that table
            // is an invalid entry? should that mean the prefixes are legacy ones?
            if ( 0x00F20F0F == dInfo.OpCode ) 
            {
               /* 3-BYTE OPCODE, PREFIX F2h 0Fh 0Fh */
               dInfo.OpInfo = __InstructionTableF20F0F[(*pbStream)];
            }
            else if ( 0x00F30F0F == dInfo.OpCode ) 
            {
               /* 3-BYTE OPCODE, PREFIX F3h 0Fh 0Fh */
               dInfo.OpInfo = __InstructionTableF30F0F[(*pbStream)];
            }
            else if ( 0x00660F0F == dInfo.OpCode ) 
            {
               /* 3-BYTE OPCODE, PREFIX 66h 0Fh 0Fh */
               dInfo.OpInfo = __InstructionTable660F0F[(*pbStream)];
            }
            else 
         #endif /* _COJACK_TARGET_MEDIATABLES */
            {
               /* 2-BYTE OPCODE, PREFIX 0Fh */
               dInfo.OpInfo = __InstructionTable00000F[(*pbStream)];
            }
         }
         else 
         {
            /* Check the stream for RET instructions, which are used to terminate
             * parsing prior to cbRelocate or cbCallgate zeroing out */
            if ( (0xC2 == (*pbStream)) || (0xC3 == (*pbStream)) || (0xCA == (*pbStream)) || (0xCB == (*pbStream)) || (0xCF == (*pbStream)) )
            {
               dwRet = ERROR_NOT_ENOUGH_MEMORY;
               /* Failure */
               __leave;
            }

            /* 1-BYTE OPCODE, NO PREFIX */
            dInfo.OpCode = (*pbStream);
            dInfo.OpInfo = __InstructionTable000000[(*pbStream)];
         }

         /* If the opcode is an invalid one, consume it and move on */
         if ( 0 == dInfo.OpInfo.Size ) 
         {
            pbStream += 1;
            goto __COPYINSTRUCTION;
         }

         /* If the instruction uses ModRM[SIB] encoding it will be next, so
          * decode that part now */
         if ( dInfo.OpInfo.ModRM ) 
         {
            /* Advance the stream by 1 byte */
            pbStream += 1;

            /* Extract the ModRM byte */
            dInfo.ModRM.zBitField = (*pbStream);

            /* Do special processing for instructions which require it... */

            if ( (0xF6 == pbStream[-1]) && (0 == dInfo.ModRM.Reg) )
            {
               /* TEST/0 (F6/0)
                *    Add 1 byte to account for the imm8 in the TEST/0 instruction  */
               dInfo.OpInfo.Size += 1;
            }
            else if ( (0xF7 == pbStream[-1]) && (0 == dInfo.ModRM.Reg) )
            {
               /* TEST/0 (F7/0)
                *    Add 2 bytes to account for the imm16 and signify that the size can be
                *    extended to 32bits */
               dInfo.OpInfo.Size    += 2;
               dInfo.OpInfo.Extend32 = 1;
            }
            else if ( 0xFF == pbStream[-1] ) 
            {
               /* CALL/3 (FF/3) and JMP/5 (FF/5) */
               if ( (3 == dInfo.ModRM.Reg) || (5 == dInfo.ModRM.Reg) ) 
               {
                  dInfo.OpInfo.Size        = 3;
                  dInfo.OpInfo.Extend32    = 1;
                  dInfo.OpInfo.IsRelative  = 1;
               }
               else if ( 4 == dInfo.ModRM.Reg )
               {
                  //TODO: Not sure if JMP/4 should be handled this way
                  dInfo.OpInfo.IsRelative = 1;
               }
            }

            /* Check if this instruction has an SIB byte */
            if ( IsEncodedSIB(&dInfo) ) 
            {
               /* Advance the stream by 1 byte */
               pbStream += 1;

               /* Extract the SIB byte */
               dInfo.Sib.zBitField = (*pbStream);

               /* Advance the number of bytes specified by the SIB, if any */
               cbAdvance = GetSizeOfSIB(&dInfo);
               pbStream += cbAdvance;
            }

            /* Advance the stream the number of bytes specified by the mod displacement, if any */
            cbAdvance = GetSizeOfModRM(&dInfo);
            pbStream += cbAdvance;
            //todo:any fixups need to be done for ModRM[SIB] instruction forms?
         }

         /* Advance the stream by the size specified in the instruction's info record */
         cbAdvance = GetSizeOfOpCode(&dInfo);
         pbStream += cbAdvance;

         /* Calculate displacement values, if any. This only accounts for displacements
          * encoded into the instruction, not ModRM displacements */
#ifdef _M_X64
         if ( IsEncodedRIP(&dInfo) )
         {
            dInfo.OpInfo.IsRelative = 1;
         }
#endif /* _M_X64 */
            
         if ( dInfo.OpInfo.IsRelative )
         {         
            ExtractDisplacement(&disp, 
                                pbStream - cbAdvance - 1, 
                                cbAdvance - 1);
         }

__COPYINSTRUCTION:
         cbCopy = (size_t)(pbStream - pbBase);
         if ( cbCopy > cbCallgate ) 
         {
            dwRet = ERROR_INSUFFICIENT_BUFFER;
            /* Failure */
            __leave;
         }

         CopyMemory(pbCallgate, 
                    pbBase, 
                    cbCopy);

         pbCallgate += cbCopy;
         cbCallgate -= cbCopy;

         /* If the instruction uses a relative displacment, make sure it will be valid after
          * adjustment in the callgate */
         if ( dInfo.OpInfo.IsRelative )
         {
            if ( !RecalculateDisplacement(&disp, 
                                          pbCallgate) )
            {
               dwRet = ERROR_INSUFFICIENT_BUFFER;
               /* Failure */
               __leave;
            }

            CopyMemory(pbCallgate - disp.Size, 
                       &disp, 
                       disp.Size);
         }

         ResetDecodeInfo(&dInfo);
         cbCopied += cbCopy;         
         pbBase    = pbStream;

         if ( cbCopied >= cbMinimum )
         {
            /* Return the number of bytes written to the callgate */
            (*pcbActual) = cbCopied;

            dwRet = NO_ERROR;
            /* Success */
            __leave;
         }

      }
   }
   #pragma warning( suppress : 6320 )
   __except( EXCEPTION_EXECUTE_HANDLER )
   {
      dwRet = GetExceptionCode();
   }

   return ( dwRet );
}

static void __stdcall InitializeJmpSequence( LONGLONG cbDisplacement, PJMPSEQ pJmpSeq, size_t cbJmpSeq ) throw()
{
   FillMemory(pJmpSeq, 
              sizeof(JMPSEQ),
              0xcc);

#ifdef _M_X64
   if ( sizeof(bJmp32) == cbJmpSeq )
   {
      CopyMemory(pJmpSeq, 
                 bJmp32, 
                 sizeof(bJmp32));

      pJmpSeq->Jmp32.Offset = (LONG)cbDisplacement;
   }
   else
   {
      CopyMemory(pJmpSeq, 
                 bJmp64, 
                 sizeof(bJmp64));

      pJmpSeq->Jmp64.Offset = cbDisplacement;
   }
#else /* _M_X64 */   
   UNREFERENCED_PARAMETER(cbJmpSeq);
   CopyMemory(pJmpSeq, 
              bJmp32, 
              sizeof(bJmp32));

   pJmpSeq->Jmp32.Offset = (LONG)cbDisplacement;
#endif /* _M_X64 */
}

static size_t __stdcall GetSizeOfJmpSequence( LONGLONG cbDisplacement ) throw()
{   
#ifdef _M_X64
   if ( (cbDisplacement >= LONG_MIN) && (cbDisplacement <= LONG_MAX) )
   {
      return ( sizeof(bJmp32) );
   }
   else
   {
      return ( sizeof(bJmp64) );
   }
#else  /* _M_X64 */
   UNREFERENCED_PARAMETER(cbDisplacement);
   return ( sizeof(bJmp32) );
#endif /* _M_X64 */
}

static LONGLONG __stdcall GetDisplacement( PUCHAR pbSource, PUCHAR pbTarget, BYTE* pcbJmpSequence ) throw()
{
   LONGLONG cbDisplacement;
   cbDisplacement = pbTarget - pbSource;

#ifdef _M_X64 
   if ( (cbDisplacement >= LONG_MIN) && (cbDisplacement <= LONG_MAX) )
   {
      //16bit not supported, so always JMP rel32
      pbSource += 5;
   }
   else
   {
      (*pcbJmpSequence) = sizeof(bJmp64);
      return ( (LONGLONG)pbTarget );
   }
#else /* _M_X64 */
   pbSource += 5;
#endif /* _M_X64 */
   
   (*pcbJmpSequence) = sizeof(bJmp32);

   return ( pbTarget - pbSource );
}

static LPVOID __stdcall FindFinalTarget( LPVOID pSourceProc ) throw()
{

   PUCHAR pbCode = (PUCHAR)pSourceProc;

   /* Follow any JMP instructions until there are no more */
   while ( true )
   {  
      if ( 0xEB == pbCode[0] )
      {
         pbCode += pbCode[1] + 2;
      }
      else if ( (0x66 == pbCode[0]) && (0xE9 == pbCode[1]) )
      {
         pbCode += *((SHORT*)&pbCode[2]) + 4;
      }
      else if ( 0xE9 == pbCode[0] )
      {
         pbCode += *((LONG*)&pbCode[1]) + 5;
      }
      else if ( (0xFF == pbCode[0]) && (0x25 == pbCode[1]) )
      {
         pbCode = *((BYTE**)&pbCode[2]);
         pbCode = *((BYTE**)pbCode);
      }
      else
      {
         break;
      }
   }

   return ( pbCode );
}

static DWORD __stdcall InstallProcedureHook( LPVOID pSourceProc, LPVOID pCallgate, size_t cbCallgate, LPVOID pHookProc ) throw()
{  
   DWORD    dwRet;
   BOOL     bRet;
   DWORD    flProtect;
   LONGLONG cbOffsetToSource;
   LONGLONG cbOffsetToTarget;
   BYTE     cbJmpToTarget;
   BYTE     cbJmpToSource;
   JMPSEQ   jmpToSource;
   JMPSEQ   jmpToTarget;
   
#ifdef _M_X64
   if ( cbCallgate <= sizeof(bJmp64) )
#else /* _M_X64 */
   if ( cbCallgate <= sizeof(bJmp32) )
#endif /* _M_X64 */
   {
      dwRet = ERROR_INSUFFICIENT_BUFFER;
      /* Failure */
      return ( dwRet );
   }

   /* Initialize locals */
   dwRet = ERROR_GEN_FAILURE;

   /* Pass over any intermediate JMP instructions and get the final
    * address of the 3 procedures */   
   pSourceProc = FindFinalTarget(pSourceProc);
   pCallgate   = FindFinalTarget(pCallgate);
   pHookProc   = FindFinalTarget(pHookProc);

   /* Calculate the displacement for the JMP instruction which transfers control
    * from the source procedure to the hook procedure */
   cbOffsetToTarget = GetDisplacement((PUCHAR)pSourceProc, 
                                      (PUCHAR)pHookProc, 
                                      &cbJmpToTarget);

   /* Enable write access to the callgate */
   bRet = VirtualProtect(pCallgate, 
                        cbCallgate, 
                        PAGE_EXECUTE_READWRITE, 
                        &flProtect);

   if ( !bRet )
   {
      dwRet = GetLastError();
      /* Failure */
      return ( dwRet );
   }
   
   __try
   {
      /* Save room for the size and JMP instruction in the callgate */
      dwRet = _RelocateInstructionStream((PUCHAR)pSourceProc, 
                                         (BYTE*)pCallgate, 
                                         cbCallgate - sizeof(bJmp64), 
                                         cbJmpToTarget, 
                                         &cbCallgate);

      if ( NO_ERROR != dwRet )
      {
         /* Failure */
         __leave;
      }

      /* Recalculate the displacement for the JMP instruction which transfers
       * control from the callgate to the source procedure */
      cbOffsetToSource = GetDisplacement((PUCHAR)pCallgate + cbCallgate, 
                                         (PUCHAR)pSourceProc + cbCallgate, 
                                         &cbJmpToSource);

      /* Build the JMP instruction for the callgate and write it out */
      InitializeJmpSequence(cbOffsetToSource, 
                            &jmpToSource, 
                            cbJmpToSource);

      CopyMemory((BYTE*)pCallgate + cbCallgate, 
                 &jmpToSource, 
                 cbJmpToSource);

   }
   __finally
   {
      bRet = VirtualProtect(pCallgate, 
                            cbCallgate + sizeof(bJmp64), 
                            flProtect, 
                            &flProtect);
   }

   if ( !bRet )
   {
      dwRet = GetLastError();
   }

   if ( NO_ERROR != dwRet )
   {
      /* Failure */
      return ( dwRet );
   }

   /* Now build the JMP instruction which transfers control from the source function
    * to the target function */
   InitializeJmpSequence(cbOffsetToTarget, 
                         &jmpToTarget, 
                         cbJmpToTarget);
   
   /* Enable write access to the source procedure */
   bRet = VirtualProtect(pSourceProc, 
                         cbJmpToTarget, 
                         PAGE_EXECUTE_READWRITE, 
                         &flProtect);

   if ( !bRet )
   {
      dwRet = GetLastError();
      /* Failure */
      return ( dwRet );
   }

   __try
   {
      /* Write the JMP instruction into the source procedure */
      CopyMemory(pSourceProc, 
                 &jmpToTarget, 
                 cbJmpToTarget);

      /* Fill the remaining bytes which were copied to the callgate with INT3 */
      FillMemory((BYTE*)pSourceProc + cbJmpToTarget, 
                 cbCallgate - cbJmpToTarget,
                 0xcc);
   }
   __finally
   {
      bRet = VirtualProtect(pSourceProc, 
                            cbJmpToTarget, 
                            flProtect, 
                            &flProtect);
   }

   if ( !bRet )
   {
      dwRet = GetLastError();
   }

   /* Success / Failure */
   return ( dwRet );
}

static DWORD __stdcall UninstallProcedureHook( LPVOID /*pSourceProc*/, LPVOID /*pCallgate*/, LPVOID /*pHookProc*/ ) throw()
{
   /* This hasn't been implemented */
   return ( ERROR_INVALID_FUNCTION );
}

#pragma pack( push )
#pragma pack( 1 )
/**
 * __bCjStartup
 *    Machine code file generated by ObjEx for CjStartup.asm. 
 *
 * These dat files are generated by the ObjEx.exe tool which simply dumps
 * all the .text sections in the object file as C style array of BYTEs.
 */
static const BYTE __bCjStartup[] = 
{
#ifdef _WIN64
   #include "amd64-CjStartup4.dat"
#else /* _WIN64 */
   #include "x86-CjStartup4.dat"
#endif /* _WIN64 */
};

/* SxDATA
 *    Parameter structure passed to CjStartProc().
 *
 * This structure must remain in sync with that in
 * the CoJack.inc file
 */
typedef struct _SxDATA
{
   DECLSPEC_ALIGN(16) FARPROC  pLoadLibraryW;
   DECLSPEC_ALIGN(16) FARPROC  pSetEvent;
   DECLSPEC_ALIGN(16) FARPROC  pCloseHandle;
   DECLSPEC_ALIGN(16) FARPROC  pGetLastError;
   DECLSPEC_ALIGN(16) FARPROC  pExitProcess;   
   DECLSPEC_ALIGN(16) FARPROC  pCreateThread;
   DECLSPEC_ALIGN(16) FARPROC  pExitThread;
   DECLSPEC_ALIGN(16) FARPROC  pVirtualFree;
   DECLSPEC_ALIGN(16) FARPROC  pSleep;
#ifdef _DEBUG
   DECLSPEC_ALIGN(16) FARPROC  pIsDebuggerPresent;
#endif /* _DEBUG */

   DECLSPEC_ALIGN(16) HANDLE   hEvent;
   
   DECLSPEC_ALIGN(16) WCHAR    chModuleName[MAX_PATH];
}SxDATA;
 
/* SxPUSHEAXSEQ / SxPUSHRAXSEQ
   These are assembly instruction sequences used to push the
   address of an SxDATA structure onto the stack, then transfer
   to the CjStartProc() function from CjStartup.asm.

x32:
   mov   eax, dword ptr imm32
   push  eax
   jmp   dword ptr imm32

x64:
   mov   rax, qword ptr imm64
   push  rax
   jmp   qword ptr imm64

   It is assumed that only the rAX register is available for use. However,
   the parameter cannot be passed directly in rAX because during debugging 
   functions will likely be called that will reset rAX. For this reason,
   the parameter must be passed on the stack. The _RemoteProc() function 
   will handle cleaning up the parameter.
 */
typedef struct _SxPUSHEAXSEQ
{
   BYTE     Mov[1];
   LONG_PTR Imm;
   BYTE     PushReg;
}SxPUSHEAXSEQ;

typedef struct _SxPUSHRAXSEQ
{
   BYTE     Mov[2];
   LONG_PTR Imm;
   BYTE     PushReg;
}SxPUSHRAXSEQ;

#ifdef _WIN64
   typedef SxPUSHRAXSEQ SxPUSHSEQ;
#else /* _WIN64 */
   typedef SxPUSHEAXSEQ SxPUSHSEQ;
#endif /* _WIN64 */

typedef struct _SxJMPSEQ
{
   SxPUSHSEQ MovSeq;
   JMPSEQ    JmpSeq;
}SxJMPSEQ;

typedef struct _SxPKG
{
   /* This must come first */
   DECLSPEC_ALIGN(16) SxDATA  SxData;
   DECLSPEC_ALIGN(16) BYTE    StartupCode[sizeof(SxJMPSEQ)];
   DECLSPEC_ALIGN(16) BYTE    RemoteProc[sizeof(__bCjStartup)];
}SxPKG;

/**
 * __bCjThread
 *    Machine code file generated by ObjEx for CjThread.asm
 */
static const BYTE __bCjThread[] = 
{   
#ifdef _WIN64
   #include "amd64-CjThread.dat"
#else /* _WIN64 */
   #include "x86-CjThread.dat"
#endif /* _WIN64 */
};

/* TxDATA
 *    Parameter structure passed to CjThreadProc()
 *
 * This structure must remain in sync with that in
 * the CoJack.inc file
 */
 typedef struct _TxDATA
{
   DECLSPEC_ALIGN(16) FARPROC pLoadLibraryW;
   DECLSPEC_ALIGN(16) FARPROC pGetLastError;
   DECLSPEC_ALIGN(16) WCHAR   chModuleName[MAX_PATH];
}TxDATA, *PTxDATA;
 
typedef struct _TxPKG
{
   /* This must come first */
   DECLSPEC_ALIGN(16) TxDATA  TxData;
   DECLSPEC_ALIGN(16) BYTE    RemoteProc[sizeof(__bCjThread)];
}TxPKG; 
#pragma pack( pop )

__inline void InitializeSxPushSequence( SxPUSHSEQ* pSeq, LONG_PTR iImm ) throw()
{
#ifdef _WIN64
   pSeq->Mov[0]  = 0x48;
   pSeq->Mov[1]  = 0xb8;
#else /* _WIN64 */
   pSeq->Mov[0] = 0xb8;
#endif /* _WIN64 */
   pSeq->Imm     = iImm;
   pSeq->PushReg = 0x50;
}

static LPVOID __stdcall FindFinalRemoteTarget( HANDLE hProcess, LPVOID pSourceProc ) throw()
{
   BYTE   bBuf[48];
   SIZE_T cbRead;
   PUCHAR pbCode;

   pbCode = bBuf;

   /* Follow any JMP instructions until there are no more */
   while ( true )
   {  
      if ( !ReadProcessMemory(hProcess,
                              pSourceProc,
                              bBuf,
                              sizeof(bBuf),
                              &cbRead) )
      {
         break;
      }

      if ( 0xEB == pbCode[0] )
      {
         if ( cbRead < 3 )
         {
            break;
         }

         pbCode += pbCode[1] + 2;
      }
      else if ( (0x66 == pbCode[0]) && (0xE9 == pbCode[1]) )
      {
         if ( cbRead < 6 )
         {
            break;
         }

         pbCode += *((SHORT*)&pbCode[2]) + 4;
      }
      else if ( 0xE9 == pbCode[0] )
      {
         if ( cbRead < 6 )
         {
            break;
         }

         pbCode += *((LONG*)&pbCode[1]) + 5;
      }
      else if ( (0xFF == pbCode[0]) && (0x25 == pbCode[1]) )
      {
         if ( cbRead < 6 )
         {
            break;
         }

         pbCode = *((BYTE**)&pbCode[2]);

         if ( !ReadProcessMemory(hProcess,
                                 pbCode,
                                 bBuf,
                                 sizeof(BYTE*),
                                 &cbRead) )
         {
            break;
         }

         pbCode = *((BYTE**)pbCode);
      }
      else
      {
         break;
      }

      pSourceProc = pbCode;
   }

   return ( pSourceProc );
}

static ULONG_PTR GetModuleEntryAddress( LPCWSTR pwszFileName ) throw()
{
   HMODULE           hModule;
   PIMAGE_DOS_HEADER pDosHeader;
   PIMAGE_NT_HEADERS pNtHeaders;
   ULONG_PTR         dwAddress;
   
   /* Initialize locals */
   dwAddress = 0;

   hModule = LoadLibraryEx(pwszFileName,
                           NULL,
                           DONT_RESOLVE_DLL_REFERENCES);

   if ( !hModule )
   {
      /* Failure */
      goto __CLEANUP;
   }

   pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(hModule);
   if ( IMAGE_DOS_SIGNATURE != pDosHeader->e_magic )
   {
      /* Failure */
      goto __CLEANUP;
   }

   pNtHeaders = reinterpret_cast<PIMAGE_NT_HEADERS>(reinterpret_cast<ULONG_PTR>(hModule) + pDosHeader->e_lfanew);
   if ( IMAGE_NT_SIGNATURE != pNtHeaders->Signature )
   {
      /* Failure */
      goto __CLEANUP;
   }

#ifdef _WIN64
   if ( IMAGE_NT_OPTIONAL_HDR64_MAGIC != pNtHeaders->OptionalHeader.Magic )
#else
   if ( IMAGE_NT_OPTIONAL_HDR32_MAGIC != pNtHeaders->OptionalHeader.Magic )
#endif
   {
      /* Failure */
      goto __CLEANUP;
   }

   dwAddress = pNtHeaders->OptionalHeader.AddressOfEntryPoint + pNtHeaders->OptionalHeader.ImageBase;
   
__CLEANUP:
   if ( NULL != hModule )
   {
      FreeLibrary(hModule);
   }

   /* Success / Failure */
   return ( dwAddress );
}

/**
 * GetKernelProcedureAddress
 *    Sorta like GetProcAddress(). Using GetProcAddress() can be hooked by things 
 *    like App Verifier which then return an address that may not be valid in the 
 *    target process. Looking up the export manually avoids it.
 */
static FARPROC GetProcedureAddress( HMODULE hModule, LPCSTR lpProcName )
{
   PUCHAR                  pImageBase;
   PIMAGE_DOS_HEADER       pDosHeader;
   PIMAGE_NT_HEADERS       pNtHeader;
   
   PIMAGE_EXPORT_DIRECTORY pExportDir;
   PUCHAR                  pExportDirTail;

   PDWORD                  pNamesTable;
   DWORD                   dwNumberOfNames;
   DWORD                   idx;
   LPCSTR                  pszExportName;
   PWORD                   pOrdinalTable;
   DWORD                   iOrdinal;
   PDWORD                  pFunctionTable;
   FARPROC                 pProcedure;

   LPCSTR                  pszForwardProcName;
   CHAR                    chModule[MAX_PATH];

   pImageBase = (PUCHAR)hModule;
   pDosHeader = (PIMAGE_DOS_HEADER)pImageBase;
   if ( !pDosHeader || (IMAGE_DOS_SIGNATURE != pDosHeader->e_magic) )
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      /* Failure */
      return ( NULL );
   }

   pNtHeader = (PIMAGE_NT_HEADERS)((LONG_PTR)pDosHeader + pDosHeader->e_lfanew);
   if ( IMAGE_NT_SIGNATURE != pNtHeader->Signature )
   {
      SetLastError(ERROR_INVALID_PARAMETER);
      /* Failure */
      return ( NULL );
   }

   pExportDir      = (PIMAGE_EXPORT_DIRECTORY)(pImageBase +  pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
   pExportDirTail  = (PUCHAR)pExportDir + pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].Size;
   pNamesTable     = (PDWORD)(pImageBase + pExportDir->AddressOfNames);
   dwNumberOfNames = pExportDir->NumberOfNames;

   for ( idx = 0; idx < dwNumberOfNames; idx++ )
   {
      pszExportName = (LPCSTR)(pImageBase + pNamesTable[idx]);

      if ( 0 == strcmp(pszExportName,
                       lpProcName) )
      {
         pOrdinalTable  = (PWORD)(pImageBase + pExportDir->AddressOfNameOrdinals);
         iOrdinal       = (DWORD)(pOrdinalTable[idx]) + pExportDir->Base;
         
         if ( (iOrdinal < pExportDir->Base) || ((iOrdinal - pExportDir->Base) > pExportDir->NumberOfFunctions) )
         {
            SetLastError(ERROR_PROC_NOT_FOUND);
            /* Failure - Invalid ordinal */
            return ( NULL );
         }

         pFunctionTable = (PDWORD)(pImageBase + pExportDir->AddressOfFunctions);
         pProcedure     = (FARPROC)(pImageBase + pFunctionTable[iOrdinal - pExportDir->Base]);

         /* Check if the function is forwarded */         
         if ( ((PUCHAR)pProcedure >= (PUCHAR)pExportDir) && ((PUCHAR)pProcedure <= pExportDirTail) )
         {
            pszForwardProcName = strchr((char*)pProcedure,
                                        '.');

            if ( !pszForwardProcName )
            {
               SetLastError(ERROR_BAD_EXE_FORMAT);
               /* Failure */
               return ( NULL );
            }

            if ( (size_t)(pszForwardProcName - (char*)pProcedure) > (_countof(chModule) - 1) )
            {
               SetLastError(ERROR_BAD_EXE_FORMAT);
               /* Failure */
               return ( NULL );
            }

            ZeroMemory(chModule,
                       sizeof(chModule));

            strncpy(chModule,
                    (char*)pProcedure,
                    pszForwardProcName - (char*)pProcedure);

            pProcedure = GetProcedureAddress(GetModuleHandleA(chModule),
                                             ++pszForwardProcName);
         }

#ifdef _DEBUG
         {
            FARPROC pProcedureDbg = GetProcAddress((HMODULE)pImageBase, lpProcName);
            _ASSERTE(pProcedureDbg == pProcedure);
         }
#endif /* _DEBUG */
         
         /* Success / Failure */
         return ( pProcedure );
      }
   }

   SetLastError(ERROR_PROC_NOT_FOUND);
   /* Failure */
   return ( NULL );
}

/**
 * LoadLibraryInProcess
 *    Loads a module in another process
 *
 * Parameters
 *    hProcess
 *       [in] Handle to the process the module should be loaded into. The handle
 *       must have been granted PROCESS_CREATE_THREAD, PROCESS_VM_OPERATION, 
 *       PROCESS_VM_READ, PROCESS_VM_WRITE, PROCESS_QUERY_INFORMATION and SYNCHRONIZE 
 *       privleges.
 *    pwszModuleName
 *       [in] Path of the module to be loaded
 * 
 * Return Value
 *    If the function succeeds, the return value is a handle to the module. If
 *    the function fails, the return value is NULL.
 */
static DWORD __stdcall LoadLibraryInProcess( HANDLE hProcess, LPCWSTR pwszModuleName ) throw()
{
   DWORD                   dwErr;
   LPVOID                  pRemote;
   SYSTEM_INFO             SysInfo;
   HMODULE                 hModule;
   TxPKG                   TxPkg;
   LPTHREAD_START_ROUTINE  pThreadProc;
   HANDLE                  hThread;
   DWORD                   dwThreadId;
   DWORD                   dwWait;
   
   pRemote = NULL;
   hThread = NULL;
   dwErr   = NO_ERROR;

   /* Initialize the TxDATA structure which will be copied
    * to the target process */
   hModule = GetModuleHandleA("KERNEL32");
   TxPkg.TxData.pLoadLibraryW = GetProcedureAddress(hModule, "LoadLibraryW");
   TxPkg.TxData.pGetLastError = GetProcedureAddress(hModule, "GetLastError");
      
   if ( FAILED(StringCchCopyW(TxPkg.TxData.chModuleName, 
                              _countof(TxPkg.TxData.chModuleName), 
                              pwszModuleName)) )
   {
      SetLastError(ERROR_BUFFER_OVERFLOW);
      /* Failure */
      goto __CLEANUP;
   }

   CopyMemory(TxPkg.RemoteProc,
              __bCjThread,
              sizeof(TxPkg.RemoteProc));

   /* Allocate a page-size memory block for both a code and data segment */
   GetSystemInfo(&SysInfo);

   pRemote = VirtualAllocEx(hProcess, 
                            NULL, 
                            max(sizeof(TxPkg), SysInfo.dwPageSize), 
                            MEM_COMMIT, 
                            PAGE_EXECUTE_READWRITE);
      
   if ( !pRemote )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   /* Copy the thread package to the target process */
   if ( !WriteProcessMemory(hProcess, 
                            pRemote, 
                            &TxPkg, 
                            sizeof(TxPkg), 
                            NULL) )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }
   
   /* Create a thread in the target process which will run the thread proc
    * code in the TxPKG. The address of the REMOTETHREADDATA structure
    * is passed as the single thread proc parameter */
   pThreadProc = (LPTHREAD_START_ROUTINE)((ULONG_PTR)pRemote + offsetof(TxPKG, RemoteProc));
   hThread = CreateRemoteThread(hProcess, 
                                NULL, 
                                0, 
                                pThreadProc, 
                                pRemote, 
                                0, 
                                &dwThreadId);
      
   if ( !hThread )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   /* Wait for the thread to terminate so we can safely retrieve its exit code */
   dwWait = WaitForSingleObject(hThread, 
                                INFINITE);

   if ( WAIT_OBJECT_0 != dwWait )
   {
      /* Failure */
      dwErr = GetLastError();
   }
   else
   {
      if ( !GetExitCodeThread(hThread, 
                              &dwErr) )
      {
         /* Failure */
         dwErr = GetLastError();
      }
   }

__CLEANUP:
   if ( NULL != hThread )
   {
      CloseHandle(hThread);
   }

   if ( NULL != pRemote )
   {
      VirtualFreeEx(hProcess, 
                    pRemote, 
                    0, 
                    MEM_RELEASE);
   }

   /* Success / Failure */
   return ( dwErr );
}

/**
 * ResumeProcessWithModule
 *    Resumes a newly created process with the specified module loaded.
 *
 * Parameters:
 *    pProcessInfo
 *       [in] PROCESS_INFORMATION structure received from CreateProcessXxx()
 *
 *    pwszModuleName
 *       [in] Path of the module to be loaded
 *
 * Return Value:
 *    Returns NO_ERROR on success; otherwise an error value.
 *
 * Remarks:
 *    The process identified by pProcessInfo->hProcess must have been created
 *    with the CREATE_SUSPENDED flag. This function will resume the process
 *    during normal execution.
 *
 *    If an error occurs the state of the target process is indeterminate. Therefore
 *    it is recommended that the target process be terminated if an error occurs.
 */
static DWORD __stdcall ResumeProcessWithModule( const LPPROCESS_INFORMATION pProcessInfo, LPCWSTR pwszModuleName ) throw()
{
   DWORD       dwErr;
   SYSTEM_INFO SysInfo;
   LPVOID      pRemoteMem;
   SxPKG       StartupPkg;
   HMODULE     hModule;
   SxDATA*     pSxData;
   HANDLE      hEvent;
   DWORD       dwWait;
   HANDLE      rghWait[2];

   /* Initialize locals */
   pRemoteMem = NULL;
   hEvent     = NULL;

   FillMemory(&StartupPkg,
              sizeof(StartupPkg),
              0xcc);

   ZeroMemory(&StartupPkg.SxData,
              sizeof(StartupPkg.SxData));

   GetSystemInfo(&SysInfo);

   /* Allocate a bunch of memory in the target process for the remote code and all the data */
   pRemoteMem = VirtualAllocEx(pProcessInfo->hProcess,
                               NULL,
                               max(SysInfo.dwPageSize, sizeof(StartupPkg)),
                               MEM_COMMIT,
                               PAGE_EXECUTE_READWRITE);

   if ( !pRemoteMem )
   {
      dwErr = ERROR_OUTOFMEMORY;
      /* Failure */
      goto __CLEANUP;
   }

   /* Copy the binary code generated by CjStartup.asm into the RemoteProc member */
   CopyMemory(StartupPkg.RemoteProc,
              __bCjStartup,
              sizeof(StartupPkg.RemoteProc));
   
   /* Initialize the startup package data */
   pSxData = &StartupPkg.SxData;
   hEvent  = CreateEvent(NULL,
                         TRUE,
                         FALSE,
                         NULL);
   
   if ( !hEvent )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   hModule                 = GetModuleHandleA("KERNEL32");
   pSxData->pLoadLibraryW  = GetProcedureAddress(hModule, "LoadLibraryW");
   pSxData->pSetEvent      = GetProcedureAddress(hModule, "SetEvent");
   pSxData->pCloseHandle   = GetProcedureAddress(hModule, "CloseHandle");
   pSxData->pGetLastError  = GetProcedureAddress(hModule, "GetLastError");
   pSxData->pExitProcess   = GetProcedureAddress(hModule, "ExitProcess");
   pSxData->pCreateThread  = GetProcedureAddress(hModule, "CreateThread");
   pSxData->pExitThread    = GetProcedureAddress(hModule, "ExitThread");
   pSxData->pVirtualFree   = GetProcedureAddress(hModule, "VirtualFree");
   pSxData->pSleep         = GetProcedureAddress(hModule, "Sleep");
#ifdef _DEBUG
   pSxData->pIsDebuggerPresent = GetProcedureAddress(hModule, "IsDebuggerPresent");
#endif /* _DEBUG */
   
   if ( !pSxData->pLoadLibraryW  || 
        !pSxData->pSetEvent      || 
        !pSxData->pCloseHandle   || 
        !pSxData->pGetLastError  ||
        !pSxData->pExitProcess   ||
        !pSxData->pCreateThread  ||
        !pSxData->pExitThread    ||        
        !pSxData->pVirtualFree   ||
        !pSxData->pSleep         
#ifdef _DEBUG
                                 ||
         !pSxData->pIsDebuggerPresent
#endif /* _DEBUG */
      )
   {
      dwErr = ERROR_INVALID_FUNCTION;
      /* Failure */
      goto __CLEANUP;
   }

   /* Copy the module name to be loaded */   
   StringCchCopy(pSxData->chModuleName,
                 _countof(pSxData->chModuleName),
                 pwszModuleName);
   
   /* The target process is responsible for cleaning up this handle. If something goes wrong after
    * this then the handle is leaked */
   if ( !DuplicateHandle(GetCurrentProcess(),
                         hEvent,
                         pProcessInfo->hProcess,
                         &pSxData->hEvent,
                         0,
                         FALSE,
                         DUPLICATE_SAME_ACCESS) )
   {
      dwErr = ERROR_INVALID_FUNCTION;
      /* Failure */
      goto __CLEANUP;
   }
   
   /* Copy the entire package to the process */
   if ( !WriteProcessMemory(pProcessInfo->hProcess,
                            pRemoteMem,
                            &StartupPkg,
                            sizeof(StartupPkg),
                            NULL) )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   /* Queue an APC on the primary thread of the process. When the thread is resumed, it will first
    * run all APC's before running the startup code */
   dwErr = QueueUserAPC((PAPCFUNC)((PUCHAR)pRemoteMem + offsetof(SxPKG, RemoteProc)),
                        pProcessInfo->hThread,
                        (ULONG_PTR)((PUCHAR)pRemoteMem + offsetof(SxPKG, SxData)));

   if ( 0 == dwErr )
   {
      dwErr = GetLastError();
      /* Failure */
      goto __CLEANUP;
   }

   /* The remote process will now be responsible for freeing the virtual memory. This is necessary
    * because freeing the memory while the process could be executing it will cause it to AV */
   pRemoteMem = NULL;

   /* Resume the thread and wait for the event passed to be signaled by the remote proc */   
   ResumeThread(pProcessInfo->hThread);

   /* Wait for either the event to be set or the process to exit */
   rghWait[0] = pProcessInfo->hProcess;
   rghWait[1] = hEvent;
   dwWait     = WaitForMultipleObjects(_countof(rghWait),
                                       rghWait,
                                       FALSE,
                                       INFINITE);

   if ( WAIT_OBJECT_0 == dwWait )
   {
      /* The process has been signaled and so has exited probably due to failure in 
       * the remote proc */

      if ( !GetExitCodeProcess(pProcessInfo->hProcess,
                               &dwErr) )
      {
         dwErr = ERROR_PROCESS_ABORTED;
      }

      if ( (STILL_ACTIVE == dwErr) || (NO_ERROR == dwErr) )
      {
         dwErr = ERROR_PROCESS_ABORTED;
      }

      /* Failure */
      goto __CLEANUP;
   }
   else if ( (WAIT_OBJECT_0 + 1) == dwWait )
   {
      /* Success */
      dwErr = NO_ERROR;
   }
   else
   {
      /* Failure */
      dwErr = ERROR_ACCESS_DENIED;
   }

__CLEANUP:   
   if ( NULL != hEvent )
   {
      CloseHandle(hEvent);
   }

   if ( NULL != pRemoteMem )
   {
      VirtualFreeEx(pProcessInfo->hProcess,
                    pRemoteMem,
                    0,
                    MEM_RELEASE);
   }

   /* Success / Failure */
   return ( dwErr );
}

#pragma warning( pop )
