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

#ifndef _ARM_RECOMPILER_H_
#define _ARM_RECOMPILER_H_

#include "Recompiler.h"

#ifdef USE_ARM_RECOMPILER

#define AVMPLUS_ARM

#define AvmAssert(x)
typedef unsigned int uint32;
//class PrintWriter;

//#define DEBUG_DISASM

#ifdef _WIN32
#define USE_ENVIRONMENT_REGISTERS
#endif

#ifdef DEBUG_DISASM
#define AVMPLUS_VERBOSE
#endif

#include "ArmAssembler.h"
typedef avmplus::ArmAssembler AA;

struct RegisterMapElement {
	int msReg;
	AA::Register armReg;
};

namespace MoSync {
#ifdef __SYMBIAN32__
	class SafeChunk {
	public:
		SafeChunk();
		~SafeChunk();
		void* allocate(int size);
		void close();
		void* address();
	private:
		RChunk mChunk;
		bool mChunkIsOpen;
	};
#endif

	class ArmRecompiler : public Recompiler <ArmRecompiler> {
	public:
		friend class Recompiler<ArmRecompiler>;

		ArmRecompiler();

		void* allocateCodeMemory(int size);
		void* allocateEntryPoint(int size);
		void freeCodeMemory(void *addr);
		void freeEntryPoint(void *addr);
		void flushInstructionCache(void *addr, int len);
		int protectMemory(void *addr, int len);

		int run(int ip);
#ifndef _android
		void init(Core::VMCore *core, int *VM_Yield);
#else
		void init(Core::VMCore *core, int *VM_Yield, JNIEnv* jniEnv, jobject jthis);
#endif
		void close();

	protected:

#ifdef __SYMBIAN32__
		SafeChunk mCodeChunk, mEntryChunk;
#endif

#define REGISTER_ADDR AA::FP // pointer to gCore->regs

#ifdef USE_ENVIRONMENT_REGISTERS
#define MEMORY_ADDR(temp) AA::R10
#define MEMORY_MASK(temp) AA::R4
#define PIPE_TO_ARM_MAP(temp) AA::R5

#define FREE_ARM_REGISTERS\
	AA::Register freeArmRegisters[] = {\
	AA::R6,\
	AA::R7,\
	AA::R8,\
	AA::R9,\
	AA::LR,\
	AA::SP,\
	AA::IP \
}

#define NUM_STATICALLY_ALLOCATED_REGISTERS 7

	void loadEnvironmentRegisters(AA& _assm) {
		_assm.MOV_imm32(MEMORY_ADDR(AA::Unknown), (int)mEnvironment.mem_ds);
		_assm.MOV_imm32(MEMORY_MASK(AA::Unknown), (int)mEnvironment.dataMask);
		_assm.MOV_imm32(PIPE_TO_ARM_MAP(AA::Unknown), (int)mPipeToArmInstMap, false);
	}
#else

AA::Register MEMORY_ADDR(AA::Register temp) { assm.MOV_imm32(temp, (int)(size_t)mEnvironment.mem_ds); return temp; }
AA::Register MEMORY_MASK(AA::Register temp) { assm.MOV_imm32(temp, (int)mEnvironment.dataMask); return temp; }
AA::Register PIPE_TO_ARM_MAP(AA::Register temp) { assm.MOV_imm32(temp, (int)(size_t)mPipeToArmInstMap); return temp; }

#define FREE_ARM_REGISTERS\
	AA::Register freeArmRegisters[] = {\
	AA::R4,\
	AA::R5,\
	AA::R6,\
	AA::R7,\
	AA::R8 \
}
// Symbian can't recompile using any of the following registers AA::R9, AA::R10, AA::SP, AA::LR, AA::IP

#ifdef _android
#define NUM_STATICALLY_ALLOCATED_REGISTERS 5
#else
#define NUM_STATICALLY_ALLOCATED_REGISTERS 0
#endif

#define loadEnvironmentRegisters(x)
#endif


		void beginInstruction(int ip);

		void analyze();

		void beginPass();
		void endPass();
		void beginFunction(Function *f);
		void endFunction(Function *f);

		// declare instruction visitors, so that you get
		// a compilation errors if you have unimplemented visitors.
		// note that you may have different visitors for
		// different passes. in that case, setup the visitors
		// in the beginPass method.
		INSTRUCTIONS(DECLARE_DEFAULT_VISITOR_ELEM)

		// TODO implement division with assembly?
		void divu(int rd, int rs);
		void divui(int rd, int imm32);
		void div(int rd, int rs);
		void divi(int rd, int imm32);

		typedef void (ArmRecompiler::* OneArgFunc)(int arg1);
		typedef void (ArmRecompiler::* TwoArgFunc)(int arg1, int arg2);

		void saveStackPointer(AA &assm, AA::Register temp);
		void loadStackPointer(AA &assm, AA::Register temp);
		void generateEntryPoint();
		void returnFromRecompiledCode();

#ifdef LOG_STATE_CHANGE
		void logStateChange(int ip);
#endif
		void debugBreak(int ip);

		void emitOneArgFuncCall(OneArgFunc argFunc, int arg1);
		void emitTwoArgFuncCall(TwoArgFunc argFunc, int arg1, int arg2);


		AA::Register findStaticRegister(int msReg);
		AA::Register getStaticRegister(int i);
		void saveStaticRegister(AA &assm, int i);
		void loadStaticRegister(AA &assm, int i);
		void saveStaticRegisters(AA &assm);
		void loadStaticRegisters(AA &assm);
		AA::Register getSaveRegister(int mosync_reg, AA::Register arm_r);
		void saveRegister(int mosync_reg, AA::Register arm_r);
		AA::Register getSaveRegister(int mosync_reg);
		AA::Register loadRegister(int msreg, AA::Register armreg, bool shouldCopy=false);
		AA::Register loadRegister(int msreg);

		AA::FloatReg getFloatTempReg();
		AA::DoubleReg getSaveDoubleReg(int mosync_reg);
		AA::DoubleReg loadDoubleReg(int mosync_reg);
		void saveDoubleReg(int mosync_reg, AA::DoubleReg);
		AA::Register getDISaveRegister(int mosync_reg);
		void saveDIRegister(int mosync_reg, AA::Register);
		AA::Register loadDIRegister(int mosync_reg);
		AA::Register getTempRegister();
		AA::Register getDITempRegister();

		void floatd(int,int);
		void floatund(int,int);
		void fix_truncd(int,int);
		void fixun_truncd(int,int);
		void fsin(int);
		void fcos(int);
		void fexp(int);
		void flog(int);
		void fpow(int,int);
		void fatan2(int,int);

		void visitFJC(AA::ConditionCode code);

		RegisterMapElement registerMapping[NUM_STATICALLY_ALLOCATED_REGISTERS];
		avmplus::ArmAssembler assm;
		avmplus::ArmAssembler entryPoint;

		AA::MDInstruction *mPipeToArmInstMap;

		int mArmStackPointer;
		int mArmCodeSize;

		// Number of the next register to allocate.
		// Resets every instruction.
		int mDoubleRegAlloc;	// start at DR0.
		int mRegisterAlloc;	// start at R4.

		AA::MDInstruction tempInst[512];

		int shiftAriMatcher();
		void shiftAriVisitor();
	};

} // namespace MoSync

#endif	//USE_ARM_RECOMPILER

#endif
