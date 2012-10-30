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

#ifdef COUNT_INSTRUCTION_USE
struct InstructionBucket {
	int id;
	int count;
	const char *opName;
};

std::vector<InstructionBucket> gInstructionUseCount;

void countInstructionUse(const char* opName, byte x) {
	gInstructionUseCount[x].count++;
	gInstructionUseCount[x].opName = opName;
}

void initInstructionUseCount() {
	gInstructionUseCount.resize(OPCODE_COUNT);
	for(int i = 0; i < OPCODE_COUNT; i++) {
		gInstructionUseCount[i].id = i;
		gInstructionUseCount[i].count = 0;
		gInstructionUseCount[i].opName = "UNUSED";
	}
}

static bool UDgreater ( InstructionBucket& elem1, InstructionBucket& elem2 )
{
   return elem1.count > elem2.count;
}


void logInstructionUse() {
	std::sort(gInstructionUseCount.begin(),
		gInstructionUseCount.end(), &Core::VMCoreInt::UDgreater);

	char temp[1024];
	Base::WriteFileStream use("instruction_use.txt", false);
	for(int i = 0; i < OPCODE_COUNT; i++) {
		int len = sprintf(temp, "inst %s: %d\n", gInstructionUseCount[i].opName, gInstructionUseCount[i].count);
		use.write(temp, len);
	}
}
#else
#define countInstruction(x)
#endif

byte* RUN_NAME(byte* ip) {
	byte op,rd,rs;
	uint32_t imm32;

	VM_Yield = 0;

#ifdef _WIN32
	static LARGE_INTEGER iCounterFreq;
	LARGE_INTEGER iCounter;
	QueryPerformanceFrequency(&iCounterFreq);
	QueryPerformanceCounter(&iCounter);
#ifdef USE_DELAY
	static double lastTime;
	lastTime = (double)iCounter.QuadPart * 1000.0 / (double)iCounterFreq.QuadPart;
#endif
#endif	//_WIN32

	//	printf("VM IP at %d\n",(int32_t) ip - (int32_t) mem_cs);

#ifndef CORE_DEBUGGING_MODE
VMLOOP_LABEL
#endif

#ifdef UPDATE_IP
	IP = uint(ip - mem_cs);
	//LOG("IP 0x%04X\n", IP);
#endif

#ifdef MEMORY_DEBUG
	InstCount++;
	if(uint(ip - mem_cs) >= (CODE_SEGMENT_SIZE - 4)) {
		uint currentIP = uint(ip - mem_cs);
		DUMPHEX(IP);
		DUMPHEX(currentIP);
		DUMPINT(InstCount);
		IP = currentIP;
		BIG_PHAT_ERROR(ERR_IMEM_OOB);
	}
#endif	//MEMORY_DEBUG
#if defined(INSTRUCTION_PROFILING) && defined(UPDATE_IP) && defined(MEMORY_DEBUG)
	instruction_count[IP]++;
#endif

#ifdef USE_DELAY
#ifdef _WIN32
	if(InstCount>DELAY_BATCH) {
		InstCount = 0;
		QueryPerformanceCounter(&iCounter);
		double time = (double)iCounter.QuadPart * 1000.0 / (double)iCounterFreq.QuadPart;

		while((time-lastTime)<DELAY_BATCH_TIME) {
			double diff = time-lastTime;
			double a = DELAY_BATCH_TIME;
			QueryPerformanceCounter(&iCounter);
			time = (double)iCounter.QuadPart * 1000.0 / (double)iCounterFreq.QuadPart;
		}

		lastTime = time;
	}
#endif	//_WIN32
#endif	//USE_DELAY

#ifdef LOG_STATE_CHANGE
	logStateChange((int)(ip-mem_cs));
#endif
	op = *ip++;

#ifdef CORE_DEBUGGING_MODE
	static int ra = REG(REG_ra);
	if(ra != REG(REG_ra)) {
		ra = REG(REG_ra);
		LOGC("ra: 0x%x\n", ra);
	}
#endif

	switch (op)
	{
		OPC(ADD)	FETCH_RD_RS	ARITH(rd, RD, +, RS);	EOP;
		OPC(ADDI)	FETCH_RD_CONST	ARITH(rd, RD, +, IMM);	EOP;
		OPC(SUB)	FETCH_RD_RS	ARITH(rd, RD, -, RS);	EOP;
		OPC(SUBI)	FETCH_RD_CONST	ARITH(rd, RD, -, IMM);	EOP;
		OPC(MUL)	FETCH_RD_RS	ARITH(rd, RD, *, RS);	EOP;
		OPC(MULI)	FETCH_RD_CONST	ARITH(rd, RD, *, IMM);	EOP;
		OPC(AND)	FETCH_RD_RS	ARITH(rd, RD, &, RS);	EOP;
		OPC(ANDI)	FETCH_RD_CONST	ARITH(rd, RD, &, IMM);	EOP;
		OPC(OR)	FETCH_RD_RS	ARITH(rd, RD, |, RS);	EOP;
		OPC(ORI)	FETCH_RD_CONST	ARITH(rd, RD, |, IMM);	EOP;
		OPC(XOR)	FETCH_RD_RS	ARITH(rd, RD, ^, RS);	EOP;
		OPC(XORI)	FETCH_RD_CONST	ARITH(rd, RD, ^, IMM);	EOP;
		OPC(DIVU)	FETCH_RD_RS	DIVIDE(rd, RDU, RSU);	EOP;
		OPC(DIVUI)	FETCH_RD_CONST	DIVIDE(rd, RDU, IMMU);	EOP;
		OPC(DIV)	FETCH_RD_RS	DIVIDE(rd, RD, RS);		EOP;
		OPC(DIVI)	FETCH_RD_CONST	DIVIDE(rd, RD, IMM);	EOP;
		OPC(SLL)	FETCH_RD_RS	ARITH(rd, RDU, <<, RSU);	EOP;
		OPC(SLLI)	FETCH_RD_IMM8	ARITH(rd, RDU, <<, IMMU);	EOP;
		OPC(SRA)	FETCH_RD_RS	ARITH(rd, RD, >>, RS);	EOP;
		OPC(SRAI)	FETCH_RD_IMM8	ARITH(rd, RD, >>, IMM);	EOP;
		OPC(SRL)	FETCH_RD_RS	ARITH(rd, RDU, >>, RSU);	EOP;
		OPC(SRLI)	FETCH_RD_IMM8	ARITH(rd, RDU, >>, IMMU);	EOP;

		OPC(NOT)	FETCH_RD_RS	WRITE_REG(rd, ~RS);	EOP;
		OPC(NEG)	FETCH_RD_RS	WRITE_REG(rd, -RS);	EOP;

		OPC(PUSH)	FETCH_RD_RS
		{
			byte r = rd;
			byte n = (rs - rd) + 1;
			if(rd < 2 || int(rd) + n > 32 || n == 0) {
				DUMPINT(rd);
				DUMPINT(n);
				BIG_PHAT_ERROR(ERR_ILLEGAL_INSTRUCTION_FORM); //raise hell
			}

			do {
				//REG(REG_sp) -= 4;
				ARITH(REG_sp, regs[REG_sp], -, 4);
				MEM(int32_t, REG(REG_sp), WRITE) = REG(r);
				LOGC("\t%i 0x%x", r, REG(r));
				r++;
			} while(--n);
		}
		EOP;

		OPC(POP) FETCH_RD_RS
		{
			byte r = rs;
			byte n = (rs - rd) + 1;
			if(rd > 31 || int(rs) - n < 1 || n == 0) {
				DUMPINT(rd);
				DUMPINT(rs);
				DUMPINT(n);
				BIG_PHAT_ERROR(ERR_ILLEGAL_INSTRUCTION_FORM); //raise hell
			}

			do {
				REG(r) = MEM(int32_t, REG(REG_sp), READ);
				//REG(REG_sp) += 4;
				ARITH(REG_sp, regs[REG_sp], +, 4);
				LOGC("\t%i 0x%x", r, REG(r));
				r--;
			} while(--n);
		}
		EOP;

		OPC(FPUSH) FETCH_FRD_FRS
		{
			byte r = rd;
			byte n = (rs - rd) + 1;
			if(int(rd) + n > 15 || n == 0) {
				DUMPINT(rd);
				DUMPINT(n);
				BIG_PHAT_ERROR(ERR_ILLEGAL_INSTRUCTION_FORM); //raise hell
			}

			do {
				ARITH(REG_sp, regs[REG_sp], -, 8);
				MEM(int32_t, REG(REG_sp), WRITE) = freg[r].i[0];
				MEM(int32_t, REG(REG_sp) + 4, WRITE) = freg[r].i[1];
				LOGC("\t%i", r);
				r++;
			} while(--n);
		}
		EOP;

		OPC(FPOP) FETCH_FRD_FRS
		{
			byte r = rs;
			byte n = (rs - rd) + 1;
			if(rd > 15 || n <= 0) {
				DUMPINT(rd);
				DUMPINT(rs);
				DUMPINT(n);
				BIG_PHAT_ERROR(ERR_ILLEGAL_INSTRUCTION_FORM); //raise hell
			}

			do {
				freg[r].i[0] = MEM(int32_t, REG(REG_sp), READ);
				freg[r].i[1] = MEM(int32_t, REG(REG_sp) + 4, READ);
				ARITH(REG_sp, regs[REG_sp], +, 8);
				LOGC("\t%i", r);
				r--;
			} while(--n);
		}
		EOP;

		OPC(LDB)
		{
			FETCH_RD_RS_CONST
			WRITE_REG(rd, MEM(char, RS + IMM, READ));
			LOGC("\t%i", RD);
		}
		EOP;

		OPC(LDH)
		{
			FETCH_RD_RS_CONST
			WRITE_REG(rd, MEM(short, RS + IMM, READ));
			LOGC("\t%i", RD);
		}
		EOP;

		OPC(LDW)
		{
			FETCH_RD_RS_CONST
			WRITE_REG(rd, MEM(int32_t, RS + IMM, READ));
			LOGC("\t[0x%x] %i", RS + IMM, RD);
		}
		EOP;

		OPC(STB)
		{
			FETCH_RD_RS_CONST
			MEM(byte, RD + IMM, WRITE) = RS;
		}
		EOP;

		OPC(STH)
		{
			FETCH_RD_RS_CONST
			MEM(unsigned short, RD + IMM, WRITE) = RS;
		}
		EOP;

		OPC(STW)
		{
			FETCH_RD_RS_CONST
			MEM(unsigned int, RD + IMM, WRITE) = RS;
			LOGC("\t[0x%x] = 0x%x", RD + IMM, RS);
		}
		EOP;

		OPC(LDI)	FETCH_RD_CONST	WRITE_REG(rd, IMM);	EOP;
		OPC(LDR)	FETCH_RD_RS	WRITE_REG(rd, RS);	EOP;

		OPC(LDDR) FETCH_RD_RS WRITE_REG(rd, RS); WRITE_REG(rd+1, REG(rs+1)); EOP;
		OPC(LDDI) FETCH_RD_CONST WRITE_REG(rd, IMM); FETCH_CONST; WRITE_REG(rd+1, IMM); EOP;

		OPC(FLOATS) FETCH_FRD_RS FRD.d = (double)(signed)RS; EOP;
		OPC(FLOATUNS) FETCH_FRD_RS FRD.d = (double)(unsigned)RS; EOP;

		OPC(FLOATD)
		{
			FETCH_FRD_RS;
			FREG temp;
			temp.i[0] = RS;
			temp.i[1] = REG(rs+1);
			FRD.d = (double)(signed long long)temp.ll;
		} EOP;

		OPC(FLOATUND)
		{
			FETCH_FRD_RS;
			FREG temp;
			temp.i[0] = RS;
			temp.i[1] = REG(rs+1);
			FRD.d = (double)(unsigned long long)temp.ll;
		} EOP;

		OPC(FSTRS) {
			FETCH_RD_FRS MA_FV fv;
			fv.f = (float)FRS.d;
			WRITE_REG(rd, fv.i);
		} EOP;
		OPC(FSTRD) FETCH_RD_FRS WRITE_REG(rd, FRS.i[0]); WRITE_REG(rd+1, FRS.i[1]); EOP;

		OPC(FLDRS) FETCH_FRD_RS { MA_FV fv; fv.i = RS; FRD.d = (double)fv.f; } EOP;
		OPC(FLDRD) FETCH_FRD_RS FRD.i[0] = RS; FRD.i[1] = REG(rs+1); EOP;

		OPC(FLDR) FETCH_FRD_FRS FRD.d = FRS.d; EOP;

		OPC(FLDIS) FETCH_FRD_CONST { MA_FV fv; fv.i = IMM; FRD.d = (double)fv.f; } EOP;
		OPC(FLDID) FETCH_FRD_CONST FRD.i[0] = IMM; FETCH_CONST; FRD.i[1] = IMM; EOP;

		OPC(FIX_TRUNCS) FETCH_RD_FRS WRITE_REG(rd, (int)FRS.d); EOP;
		OPC(FIX_TRUNCD)
		{
			FETCH_RD_FRS;
			FREG temp;
			temp.ll = (long long)FRS.d;
			WRITE_REG(rd, temp.i[0]);
			WRITE_REG(rd+1, temp.i[1]);
		} EOP;

		OPC(FIXUN_TRUNCS) FETCH_RD_FRS WRITE_REG(rd, (unsigned int)FRS.d); EOP;
		OPC(FIXUN_TRUNCD)
		{
			FETCH_RD_FRS;
			FREG temp;
			temp.ll = (unsigned long long)FRS.d;
			WRITE_REG(rd, temp.i[0]);
			WRITE_REG(rd+1, temp.i[1]);
		} EOP;

		OPC(FSTS)
		{
			FETCH_RD_FRS_CONST
			MA_FV fv;
			fv.f = (float)FRS.d;
			MEM(unsigned int, RD + IMM, WRITE) = fv.i;
			LOGC("\t[0x%x] = 0x%08x", RD + IMM, fv.i);
		} EOP;

		OPC(FSTD)
		{
			FETCH_RD_FRS_CONST
			int addr = RD + IMM;
			MEM(unsigned int, addr, WRITE) = FRS.i[0];
			MEM(unsigned int, addr + 4, WRITE) = FRS.i[1];
			LOGC("\t[0x%x] = 0x%08x%08x", addr, FRS.i[1], FRS.i[0]);
		} EOP;

		OPC(FLDS)
		{
			FETCH_RD_RS_CONST
			MA_FV fv;
			fv.i = MEM(int32_t, RS + IMM, READ);
			FRD.d = (double)fv.f;
			LOGC("\t[0x%x] 0x%08x (%g)", RS + IMM, fv.i, FRD.d);
		} EOP;

		OPC(FLDD)
		{
			FETCH_RD_RS_CONST
			int addr = RS + IMM;
			FRD.i[0] = MEM(int32_t, addr, READ);
			FRD.i[1] = MEM(int32_t, addr + 4, READ);
			LOGC("\t[0x%x] 0x%08x%08x (%g)", addr, FRD.i[1], FRD.i[0], FRD.d);
		} EOP;

		OPC(FADD) FETCH_FRD_FRS FRD.d += FRS.d; EOP;
		OPC(FSUB) FETCH_FRD_FRS FRD.d -= FRS.d; EOP;
		OPC(FMUL) FETCH_FRD_FRS FRD.d *= FRS.d; EOP;
		OPC(FDIV)
		{
			FETCH_FRD_FRS;
#ifndef ALLOW_FLOAT_DIVISION_BY_ZERO
			if(FRS.d == 0.0
#ifdef EMULATOR
				&& !gSyscall->mAllowDivZero
#endif	//EMULATOR
				)
			{
				BIG_PHAT_ERROR(ERR_DIVISION_BY_ZERO);
			}
#endif	//ALLOW_FLOAT_DIVISION_BY_ZERO
			FRD.d /= FRS.d;
		}
		EOP;

		OPC(FSQRT) FETCH_FRD_FRS FRD.d = sqrt(FRS.d); EOP;
		OPC(FSIN) FETCH_FRD_FRS FRD.d = sin(FRS.d); EOP;
		OPC(FCOS) FETCH_FRD_FRS FRD.d = cos(FRS.d); EOP;
		OPC(FEXP) FETCH_FRD_FRS FRD.d = exp(FRS.d); EOP;
		OPC(FLOG) FETCH_FRD_FRS FRD.d = log(FRS.d); EOP;
		OPC(FPOW) FETCH_FRD_FRS FRD.d = pow(FRD.d, FRS.d); EOP;
		OPC(FATAN2) FETCH_FRD_FRS FRD.d = atan2(FRD.d, FRS.d); EOP;

		OPC(LDD)
		{
			FETCH_RD_RS_CONST
			int addr = RS + IMM;
			WRITE_REG(rd, MEM(int32_t, addr, READ));
			WRITE_REG(rd+1, MEM(int32_t, addr + 4, READ));
			LOGC("\t[0x%x] 0x%08x%08x", addr, regs[rd+1], RD);
		}
		EOP;

		OPC(STD)
		{
			FETCH_RD_RS_CONST
			int addr = RD + IMM;
			MEM(unsigned int, addr, WRITE) = RS;
			MEM(unsigned int, addr + 4, WRITE) = regs[rs+1];
			LOGC("\t[0x%x] = 0x%08x%08x", addr, regs[rs+1], RS);
		}
		EOP;

		OPC(RET)
			fakePop();
			JMP_GENERIC(REG(REG_ra));
		EOP;

		OPC(CALLR)
			FETCH_RD
			CALL_RD
			fakePush(REG(REG_ra), RD);
		EOP;
		OPC(CALLI)
			FETCH_CONST
			CALL_IMM
			fakePush(REG(REG_ra), IMM);
		EOP;

		OPC(JC_EQ) 	FETCH_RD_RS_CONST	if (RD == RS)	{ JMP_IMM; } 	EOP;
		OPC(JC_NE)	FETCH_RD_RS_CONST	if (RD != RS)	{ JMP_IMM; }	EOP;
		OPC(JC_GE)	FETCH_RD_RS_CONST	if (RD >= RS)	{ JMP_IMM; }	EOP;
		OPC(JC_GT)	FETCH_RD_RS_CONST	if (RD >  RS)	{ JMP_IMM; }	EOP;
		OPC(JC_LE)	FETCH_RD_RS_CONST	if (RD <= RS)	{ JMP_IMM; }	EOP;
		OPC(JC_LT)	FETCH_RD_RS_CONST	if (RD <  RS)	{ JMP_IMM; }	EOP;

		OPC(FJC_EQ) 	FETCH_FRD_FRS_CONST	if (FRD.d == FRS.d)	{ JMP_IMM; } 	EOP;
		OPC(FJC_NE)	FETCH_FRD_FRS_CONST	if (FRD.d != FRS.d)	{ JMP_IMM; }	EOP;
		OPC(FJC_GE)	FETCH_FRD_FRS_CONST	if (FRD.d >= FRS.d)	{ JMP_IMM; }	EOP;
		OPC(FJC_GT)	FETCH_FRD_FRS_CONST	if (FRD.d >  FRS.d)	{ JMP_IMM; }	EOP;
		OPC(FJC_LE)	FETCH_FRD_FRS_CONST	if (FRD.d <= FRS.d)	{ JMP_IMM; }	EOP;
		OPC(FJC_LT)	FETCH_FRD_FRS_CONST	if (FRD.d <  FRS.d)	{ JMP_IMM; }	EOP;

		OPC(JC_LTU)	FETCH_RD_RS_CONST	if (RDU <  RSU)	{ JMP_IMM; }	EOP;
		OPC(JC_GEU)	FETCH_RD_RS_CONST	if (RDU >= RSU)	{ JMP_IMM; }	EOP;
		OPC(JC_GTU)	FETCH_RD_RS_CONST	if (RDU >  RSU)	{ JMP_IMM; }	EOP;
		OPC(JC_LEU)	FETCH_RD_RS_CONST	if (RDU <= RSU)	{ JMP_IMM; }	EOP;

		OPC(JPI)	FETCH_CONST		JMP_IMM		EOP;

		//OPC(XB)		FETCH_RD_RS		RD = (int)((char) RS); EOP;
		OPC(XB)	FETCH_RD_RS	RD = ((RS & 0x80) == 0) ? (RS & 0xFF) : (RS | ~0xFF); EOP;
		//OPC(XH)		FETCH_RD_RS		RD = (int)((short) RS);	EOP;
		OPC(XH)	FETCH_RD_RS	RD = ((RS & 0x8000) == 0) ? (RS & 0xFFFF) : (RS | ~0xFFFF); EOP;

		OPC(SYSCALL)
		{
			int syscallNumber = IB;
			fakePush((int32_t) (ip - mem_cs), -syscallNumber);
			InvokeSysCall(syscallNumber);
			fakePop();
			if (VM_Yield)
				return ip;
		}
		EOP;

		OPC(CASE) FETCH_RD; FETCH_CONST; {
			uint CaseStart = IMM;
			FETCH_CONST; uint CaseLength = IMM;
			FETCH_CONST; uint tableAddress = IMM;
			FETCH_CONST; uint defaultLabel = IMM;
			uint index = RD - CaseStart;
			if(index <= CaseLength) {
				JMP_GENERIC(MEM(int, tableAddress + index*sizeof(int), READ));
			} else {
				JMP_GENERIC(defaultLabel);
			}
		} EOP;

#if 0
#ifdef ENABLE_DEBUGGER
		OPC(DBG_OP) {
			int pc = IP;
			switch(Debugger::brk()) {
	case Debugger::BRK_CONTINUE:
	default:
		BIG_PHAT_ERROR(0);
			}
			if(pc!=IP) {
				ip = mem_cs+IP;
			}
		} EOP;
#endif
#else
#if 0//def GDB_DEBUG
		OPC(DBG_OP) {
			ip--;
			if(mGdbOn && mGdbSignal != eStep) {
				mGdbSignal = eBreakpoint;
			} else {
				//if mGdbOn, the debugger tried to step on a breakpoint.
				//it shouldn't do that, because the core can't handle it.
				//it should have overwritten the breakpoint opcode with the original.
				DEBIG_PHAT_ERROR;
			}
		} EOP;
#endif
#endif

	OPC(NOP); EOP;

	default:
		//VM_State = -3;				// Bad instruction
		LOG("Illegal instruction 0x%02X @ 0x%04X\n", op, (int)(size_t)(ip - mem_cs) - 1);
		BIG_PHAT_ERROR(ERR_ILLEGAL_INSTRUCTION);
		//return ip;
	}
#ifdef CORE_DEBUGGING_MODE
	RUN_LOOP;
#endif
}
