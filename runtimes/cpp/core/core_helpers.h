
#define FETCH_INT	FETCH_IMM32

#define FETCH_IMM8	imm32 = IB; LOGC(" n%i", imm32);
// little endian
#define FETCH_IMM32	imm32 = IB; imm32 += IB << 8; imm32 += IB << 16;\
	imm32 += IB << 24; LOGC(" n%i(0x%x)", imm32, imm32);

#define FETCH_RD_RS		FETCH_RD FETCH_RS
#define FETCH_RD_CONST		FETCH_RD FETCH_CONST
#define FETCH_RD_RS_CONST	FETCH_RD FETCH_RS FETCH_CONST
#define FETCH_RD_IMM8		FETCH_RD FETCH_IMM8

#define FETCH_FRD_RS		FETCH_FRD FETCH_RS
#define FETCH_FRD_FRS		FETCH_FRD FETCH_FRS
#define FETCH_FRD_CONST	FETCH_FRD FETCH_CONST
#define FETCH_FRD_FRS_CONST	FETCH_FRD FETCH_FRS FETCH_CONST
#define FETCH_RD_FRS_CONST	FETCH_RD FETCH_FRS FETCH_CONST
#define FETCH_RD_FRS FETCH_RD FETCH_FRS
