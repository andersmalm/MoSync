/* Copyright (C) 2009 Mobile Sorcery AB

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2, as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the Free
Software Foundation, 59 Temple Place - Suite 330, Boston, MA
02111-1307, USA.
*/

#ifndef PHONE_RELEASE
#ifdef __SYMBIAN32__
#include <e32base.h>
#else
#include <stdio.h>
#include <stdarg.h>
#endif

#include "helpers/attribute.h"

typedef unsigned char byte;
typedef unsigned int uint;

#ifdef __SYMBIAN32__
#define WRITE(argv...) if(buf) buf += write(buf, argv)
__inline int write(char* buf, const char* fmt, ...) {
	//LOG("write 0x%08x \"%s\"\n", buf, fmt);
	VA_LIST argptr;
	VA_START(argptr, fmt);
	TPtr8 ptr((byte*) buf, 1024);
	TPtrC8 fmtPtr((byte*) fmt);
	ptr.FormatList(fmtPtr, argptr);
	//LOG("writeResult: %x \"%s\"\n", ptr.Length(), buf);
	buf[ptr.Length()] = 0;
	return ptr.Length();
}
#else
static char* sPtr;
__inline void WRITE(const char* fmt, ...) GCCATTRIB(format(printf, 1, 2));
__inline void WRITE(const char* fmt, ...) {
	if(!sPtr) return;
	va_list argptr;
	va_start(argptr, fmt);
	sPtr += vsprintf(sPtr, fmt, argptr);
}
#endif	//__SYMBIAN32__


#define IB ((int)(*ip++))

#define OPC(opcode)	case OP_##opcode: WRITE("%x: %i %s", (int)(ip - mem_cs - 1), OP_##opcode, #opcode);
#define EOP	WRITE("\n"); break;

#define LOGC WRITE

#include "core_helpers.h"
#include "CoreCommon.h"
#include "gen-opcodes.h"
#include "disassembler.h"

#define FETCH_RD	rd = IB; WRITE(" rd%i", rd);
#define FETCH_RS	rs = IB; WRITE(" rs%i", rs);
#define FETCH_FRD	rd = IB; WRITE(" frd%i", rd);
#define FETCH_FRS	rs = IB; WRITE(" frs%i", rs);
#define FETCH_CONST	FETCH_INT

#define ARITH(a, oper, b)
#define DIVIDE(a, b)
#define CALL_IMM
#define CALL_RD
#define JMP_IMM
#define JMP_GENERIC(a)
#define REG(i) (i)
#define MEM(type, addr) (addr)

int disassemble_one ( const byte* ip,
                      const byte* mem_cs,
                      char* buf,
                      byte& op,
                      byte &rd,
                      byte &rs,
                      int &imm32,
                      int &imm2,
                      int &imm3,
                      int &imm4)
{
	const byte* startIp = ip;
#ifndef __SYMBIAN32__
	sPtr = buf;
#endif
	op = *ip++;

	int RD=0, RS=0, RDU=0, RSU=0;

	switch (op)
	{
		OPC(ADD)	FETCH_RD_RS	ARITH(RD, +, RS);	EOP;
		OPC(ADDI)	FETCH_RD_CONST	ARITH(RD, +, IMM);	EOP;
		OPC(SUB)	FETCH_RD_RS	ARITH(RD, -, RS);	EOP;
		OPC(SUBI)	FETCH_RD_CONST	ARITH(RD, -, IMM);	EOP;
		OPC(MUL)	FETCH_RD_RS	ARITH(RD, *, RS);	EOP;
		OPC(MULI)	FETCH_RD_CONST	ARITH(RD, *, IMM);	EOP;
		OPC(AND)	FETCH_RD_RS	ARITH(RD, &, RS);	EOP;
		OPC(ANDI)	FETCH_RD_CONST	ARITH(RD, &, IMM);	EOP;
		OPC(OR)	FETCH_RD_RS	ARITH(RD, |, RS);	EOP;
		OPC(ORI)	FETCH_RD_CONST	ARITH(RD, |, IMM);	EOP;
		OPC(XOR)	FETCH_RD_RS	ARITH(RD, ^, RS);	EOP;
		OPC(XORI)	FETCH_RD_CONST	ARITH(RD, ^, IMM);	EOP;
		OPC(DIVU)	FETCH_RD_RS	DIVIDE(RDU, RSU);	EOP;
		OPC(DIVUI)	FETCH_RD_CONST	DIVIDE(RDU, IMMU);	EOP;
		OPC(DIV)	FETCH_RD_RS	DIVIDE(RD, RS);		EOP;
		OPC(DIVI)	FETCH_RD_CONST	DIVIDE(RD, IMM);	EOP;
		OPC(SLL)	FETCH_RD_RS	ARITH(RDU, <<, RSU);	EOP;
		OPC(SLLI)	FETCH_RD_IMM8	ARITH(RDU, <<, IMMU);	EOP;
		OPC(SRA)	FETCH_RD_RS	ARITH(RD, >>, RS);	EOP;
		OPC(SRAI)	FETCH_RD_IMM8	ARITH(RD, >>, IMM);	EOP;
		OPC(SRL)	FETCH_RD_RS	ARITH(RDU, >>, RSU);	EOP;
		OPC(SRLI)	FETCH_RD_IMM8	ARITH(RDU, >>, IMMU);	EOP;

		OPC(NOT)	FETCH_RD_RS	RD = ~RS;	EOP;
		OPC(NEG)	FETCH_RD_RS	RD = -RS;	EOP;

		OPC(PUSH) FETCH_RD_RS EOP;
		OPC(POP) FETCH_RD_RS EOP;

		OPC(FPUSH) FETCH_FRD_FRS EOP;
		OPC(FPOP) FETCH_FRD_FRS EOP;

		OPC(LDDR) FETCH_RD_RS EOP;
		OPC(LDDI) FETCH_RD_CONST EOP;

		OPC(FLOATS) FETCH_FRD_RS EOP;
		OPC(FLOATUNS) FETCH_FRD_RS EOP;

		OPC(FLOATD) FETCH_FRD_RS; EOP;

		OPC(FLOATUND) FETCH_FRD_RS; EOP;

		OPC(FSTRS) FETCH_RD_RS EOP;
		OPC(FSTRD) FETCH_RD_RS EOP;

		OPC(FLDRS) FETCH_FRD_RS EOP;
		OPC(FLDRD) FETCH_FRD_RS EOP;

		OPC(FLDR) FETCH_FRD_FRS EOP;

		OPC(FLDIS) FETCH_FRD_CONST EOP;
		OPC(FLDID) FETCH_FRD_CONST EOP;

		OPC(FIX_TRUNCS) FETCH_RD_FRS EOP;
		OPC(FIX_TRUNCD) FETCH_RD_FRS EOP;

		OPC(FIXUN_TRUNCS) FETCH_RD_FRS EOP;
		OPC(FIXUN_TRUNCD) FETCH_RD_FRS EOP;

		OPC(FSTS) FETCH_RD_FRS_CONST EOP;
		OPC(FSTD) FETCH_RD_FRS_CONST EOP;

		OPC(FLDS) FETCH_RD_RS_CONST EOP;
		OPC(FLDD) FETCH_RD_RS_CONST EOP;

		OPC(FADD) FETCH_FRD_FRS EOP;
		OPC(FSUB) FETCH_FRD_FRS EOP;
		OPC(FMUL) FETCH_FRD_FRS EOP;
		OPC(FDIV) FETCH_FRD_FRS EOP;

		OPC(FSQRT) FETCH_FRD_FRS EOP;
		OPC(FSIN) FETCH_FRD_FRS EOP;
		OPC(FCOS) FETCH_FRD_FRS EOP;
		OPC(FEXP) FETCH_FRD_FRS EOP;
		OPC(FLOG) FETCH_FRD_FRS EOP;
		OPC(FPOW) FETCH_FRD_FRS EOP;
		OPC(FATAN2) FETCH_FRD_FRS EOP;

		OPC(LDD) FETCH_RD_RS_CONST EOP;

		OPC(STD) FETCH_RD_RS_CONST EOP;

		OPC(LDB) FETCH_RD_RS_CONST EOP;

		OPC(LDH) FETCH_RD_RS_CONST EOP;

		OPC(LDW) FETCH_RD_RS_CONST EOP;

		OPC(STB) FETCH_RD_RS_CONST EOP;

		OPC(STH) FETCH_RD_RS_CONST EOP;

		OPC(STW) FETCH_RD_RS_CONST EOP;

		OPC(LDI)	FETCH_RD_CONST	EOP;
		OPC(LDR)	FETCH_RD_RS	EOP;

		OPC(RET)
		JMP_GENERIC(REG(REG_rt));
		EOP;

		OPC(CALLR)
			FETCH_RD
			CALL_RD
		EOP;
		OPC(CALLI)
			FETCH_INT
			CALL_IMM
		EOP;

		OPC(JC_EQ) 	FETCH_RD_RS_CONST	if (RD == RS)	{ JMP_IMM; } 	EOP;
		OPC(JC_NE)	FETCH_RD_RS_CONST	if (RD != RS)	{ JMP_IMM; }	EOP;
		OPC(JC_GE)	FETCH_RD_RS_CONST	if (RD >= RS)	{ JMP_IMM; }	EOP;
		OPC(JC_GT)	FETCH_RD_RS_CONST	if (RD >  RS)	{ JMP_IMM; }	EOP;
		OPC(JC_LE)	FETCH_RD_RS_CONST	if (RD <= RS)	{ JMP_IMM; }	EOP;
		OPC(JC_LT)	FETCH_RD_RS_CONST	if (RD <  RS)	{ JMP_IMM; }	EOP;

		OPC(JC_LTU)	FETCH_RD_RS_CONST	if (RDU <  RSU)	{ JMP_IMM; }	EOP;
		OPC(JC_GEU)	FETCH_RD_RS_CONST	if (RDU >= RSU)	{ JMP_IMM; }	EOP;
		OPC(JC_GTU)	FETCH_RD_RS_CONST	if (RDU >  RSU)	{ JMP_IMM; }	EOP;
		OPC(JC_LEU)	FETCH_RD_RS_CONST	if (RDU <= RSU)	{ JMP_IMM; }	EOP;

		OPC(JPI)		FETCH_INT		JMP_IMM		EOP;

		OPC(XB)		FETCH_RD_RS		RD = (int)((char) RS);	EOP;
		OPC(XH)		FETCH_RD_RS		RD = (int)((short) RS);	EOP;

		OPC(SYSCALL)
		{
			int syscallNumber = imm32 = IB;
			WRITE("%i", syscallNumber);
		}
		EOP;

		OPC(CASE) FETCH_RD; FETCH_CONST; {
			uint CaseStart = imm32;
			FETCH_CONST; uint CaseLength = imm32;
			FETCH_CONST; uint tableAddress = imm32;
			FETCH_CONST; uint defaultLabel = imm32;
			WRITE("cs 0x%x, cl 0x%x, ta 0x%x, dl 0x%x",
				CaseStart, CaseLength, tableAddress, defaultLabel);
			// store decoded fields
			imm32 = CaseStart;
			imm2 = CaseLength;
			imm3 = tableAddress;
			imm4 = defaultLabel;
		} EOP;

	default:
		WRITE("Illegal instruction 0x%02x @ 0x%x\n", op, (int)((ip - mem_cs) - 1));
		//BIG_PHAT_ERROR(ERR_ILLEGAL_INSTRUCTION);
	}

	return (int)(ip-startIp);
}

int disassemble_one ( const byte* ip,
                      const byte* mem_cs,
                      char* buf )
{
	byte rd=0, rs=0;
	byte op;
	int imm32, imm2, imm3, imm4;
	return disassemble_one(ip, mem_cs, buf, op, rd, rs, imm32,
		imm2, imm3, imm4);
}

#endif	//PHONE_RELEASE
