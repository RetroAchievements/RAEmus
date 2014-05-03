/******************************************************************************
	[CPU.h]
		ＣＰＵの記述に必要な宣言や定義を行ないます．

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
//Kitao更新。コメント上の"CPU_INST_BBS1_ZP_REL"の値が0x8Fになっていたのを0x9Fに修正。v1.62

#ifndef CPU_H_INCLUDED
#define CPU_H_INCLUDED

#include <stdio.h>
#include "TypeDefs.h"


/*-----------------------------------------------------------------------------
** 外部公開関数のプロトタイプ宣言です．
**---------------------------------------------------------------------------*/
void		CPU_SetReadFunction(Uint8 (*RdFunc)(Uint32 mpr, Uint32 addr)); //Kitao更新
void		CPU_SetWriteFunction(void (*WrFunc)(Uint32 mpr, Uint32 addr, Uint8 data)); //Kitao更新

Sint32		CPU_Reset();
Sint32		CPU_ExecuteSingleInstruction();

//Kitao追加
void
CPU_ExecuteOperation();

//Kitao追加
void
CPU_SetIntDisable(
	Uint8	intDisable);

//Kitao追加
void
CPU_SelectVDC(
	Sint32	selectVDC);

//Kitao追加
Sint32
CPU_GetClockElapsed();

//Kitao追加
void
CPU_SetDebug(
	BOOL	debug);

//Kitao追加
Uint32
CPU_GetOpCode();

//Kitao追加
Uint8
CPU_ReadCode(
	Uint16	pc);

//Kitao追加
Uint16
CPU_GetPrevPC();

//Kitao追加
Uint8
CPU_GetPrevFlags();

//Kitao追加
void
CPU_SetTurboCycle(
	Sint32	n);

void	CPU_ActivateRDY();
void	CPU_ActivateNMI();
void	CPU_ActivateTIMER();
void	CPU_ActivateIRQ1();
void	CPU_ActivateIRQ2();

void	CPU_InactivateRDY();
void	CPU_InactivateNMI();
void	CPU_InactivateTIMER();
void	CPU_InactivateIRQ1();
void	CPU_InactivateIRQ2();

BOOL	CPU_SaveState(FILE* p);
BOOL	CPU_LoadState(FILE* p);

// debug functions
/*-----------------------------------------------------------------------------
** [CPU_Setxx]
**   レジスタの値を設定します。 
**---------------------------------------------------------------------------*/
void CPU_SetA(Uint8 A);
void CPU_SetX(Uint8 X);
void CPU_SetY(Uint8 Y);
void CPU_SetS(Uint8 S);
void CPU_SetP(Uint8 P);
void CPU_SetPC(Uint16 PC);
void CPU_SetMPR(Sint32 i, Uint32 mpr);

/*-----------------------------------------------------------------------------
** [CPU_Getxx]
**   レジスタの値を取得します。 
**---------------------------------------------------------------------------*/
Uint8 CPU_GetA();
Uint8 CPU_GetX();
Uint8 CPU_GetY();
Uint8 CPU_GetS();
Uint8 CPU_GetP();
Uint16 CPU_GetPC();

Uint32 CPU_GetMPR(Sint32 i);

//Kitao追加
void
CPU_WriteMemory(
	Uint32	addr,
	Uint8	data);

//Kitao追加
void
CPU_WriteMemoryZERO(
	Uint8	addr,
	Uint8	data);

//Kitao追加
void
CPU_WriteMemoryMpr(
	Uint32	mpr,
	Uint32	addr,
	Uint8	data,
	BOOL	bContinuous);


/******************************************************************************
** Defines
******************************************************************************/

/*-----------------------------------------------------------------------------
** RESET / IRQ / NMI vectors
**---------------------------------------------------------------------------*/
#define		CPU_IRQ2VECTOR			(0xFFF6)
#define		CPU_IRQ1VECTOR			(0xFFF8)
#define		CPU_TIMERVECTOR			(0xFFFA)
#define		CPU_NMIVECTOR			(0xFFFC)
#define		CPU_RESETVECTOR			(0xFFFE)

/*-----------------------------------------------------------------------------
** Flags
**---------------------------------------------------------------------------*/
#define		CPU_CF					(0x01)
#define		CPU_ZF					(0x02)
#define		CPU_IF					(0x04)
#define		CPU_DF					(0x08)
#define		CPU_BF					(0x10)
#define		CPU_TF					(0x20)
#define		CPU_VF					(0x40)
#define		CPU_NF					(0x80)

#define		CPU_CLOCKS_PER_SEC		7159090

/*-----------------------------------------------------------------------------
** Instructions
**---------------------------------------------------------------------------*/
enum Instructions
{
	CPU_INST_BRK,					// 0x00 : BRK
	CPU_INST_ORA_IND_X,				// 0x01 : ORA	(IND,X)
	CPU_INST_SXY,					// 0x02 : SXY
	CPU_INST_ST0_IMM,				// 0x03 : ST0	#$nn
	CPU_INST_TSB_ZP,				// 0x04 : TSB	$ZZ
	CPU_INST_ORA_ZP,				// 0x05 : ORA	$ZZ
	CPU_INST_ASL_ZP,				// 0x06 : ASL	$ZZ
	CPU_INST_RMB0_ZP,				// 0x07 : RMB0	$ZZ
	CPU_INST_PHP,					// 0x08 : PHP
	CPU_INST_ORA_IMM,				// 0x09 : ORA	#$nn
	CPU_INST_ASL_ACCUM,				// 0x0A : ASL	A
	CPU_INST_BAD_OP_0B,				// 0x0B : BAD INSTRUCTION
	CPU_INST_TSB_ABS,				// 0x0C : TSB	$hhll
	CPU_INST_ORA_ABS,				// 0x0D : ORA	$hhll
	CPU_INST_ASL_ABS,				// 0x0E : ASL	$hhll
	CPU_INST_BBR0_ZP_REL,			// 0x0F : BBR0	$ZZ,$rr
	CPU_INST_BPL_REL,				// 0x10 : BPL	REL
	CPU_INST_ORA_IND_Y,				// 0x11 : ORA	(IND),Y
	CPU_INST_ORA_IND,				// 0x12 : ORA	(IND)
	CPU_INST_ST1_IMM,				// 0x13 : ST1	#$nn
	CPU_INST_TRB_ZP,				// 0x14 : TRB	$ZZ
	CPU_INST_ORA_ZP_X,				// 0x15 : ORA	$ZZ,X
	CPU_INST_ASL_ZP_X,				// 0x16 : ASL	$ZZ,X
    CPU_INST_RMB1_ZP,				// 0x17 : RMB1	$ZZ
	CPU_INST_CLC,					// 0x18 : CLC
	CPU_INST_ORA_ABS_Y,				// 0x19 : ORA	$hhll,Y
	CPU_INST_INC_ACCUM,				// 0x1A : INC	A
	CPU_INST_BAD_OP_1B,				// 0x1B : BAD INSTRUCTION
	CPU_INST_TRB_ABS,				// 0x1C : TRB	$hhll
	CPU_INST_ORA_ABS_X,				// 0x1D : ORA	$hhll,X
	CPU_INST_ASL_ABS_X,				// 0x1E : ASL	$hhll,X
	CPU_INST_BBR1_ZP_REL,			// 0x1F : BBR1	$ZZ,$rr
	CPU_INST_JSR_ABS,				// 0x20 : JSR	$hhll
	CPU_INST_AND_IND_X,				// 0x21 : AND	(IND,X)
	CPU_INST_SAX,					// 0x22 : SAX
	CPU_INST_ST2_IMM,				// 0x23 : ST2	#$nn
	CPU_INST_BIT_ZP,				// 0x24 : BIT	$ZZ
	CPU_INST_AND_ZP,				// 0x25 : AND	$ZZ
	CPU_INST_ROL_ZP,				// 0x26 : ROL	$ZZ
	CPU_INST_RMB2_ZP,				// 0x27 : RMB2	$ZZ
	CPU_INST_PLP,					// 0x28 : PLP
	CPU_INST_AND_IMM,				// 0x29 : AND	#$nn
	CPU_INST_ROL_ACCUM,				// 0x2A : ROL	A
	CPU_INST_BAD_OP_2B,				// 0x2B : BAD INSTRUCTION
	CPU_INST_BIT_ABS,				// 0x2C : BIT	$hhll
	CPU_INST_AND_ABS,				// 0x2D : AND	$hhll
	CPU_INST_ROL_ABS,				// 0x2E : ROL	$hhll
	CPU_INST_BBR2_ZP_REL,			// 0x2F : BBR2	$ZZ,$rr
	CPU_INST_BMI_REL,				// 0x30 : BMI	$rr
	CPU_INST_AND_IND_Y,				// 0x31 : AND	(IND),Y
	CPU_INST_AND_IND,				// 0x32 : AND	(IND)
	CPU_INST_BAD_OP_33,				// 0x33 : BAD INSTRUCTION
	CPU_INST_BIT_ZP_X,				// 0x34 : BIT	$ZZ,X
	CPU_INST_AND_ZP_X,				// 0x35 : AND	$ZZ,X
	CPU_INST_ROL_ZP_X,				// 0x36 : ROL	$ZZ,X
	CPU_INST_RMB3_ZP,				// 0x37 : RMB3	$ZZ
	CPU_INST_SEC,					// 0x38 : SEC
	CPU_INST_AND_ABS_Y,				// 0x39 : AND	$hhll,Y
	CPU_INST_DEC_ACCUM,				// 0x3A : DEC	A
	CPU_INST_BAD_OP_3B,				// 0x3B : BAD INSTRUCTION
	CPU_INST_BIT_ABS_X,				// 0x3C : BIT	$hhll,X
	CPU_INST_AND_ABS_X,				// 0x3D : AND	$hhll,X
	CPU_INST_ROL_ABS_X,				// 0x3E : ROL	$hhll,X
	CPU_INST_BBR3_ZP_REL,			// 0x3F : BBR3	$ZZ,$rr
	CPU_INST_RTI,					// 0x40 : RTI
	CPU_INST_EOR_IND_X,				// 0x41 : EOR	(IND,X)
	CPU_INST_SAY,					// 0x42 : SAY
	CPU_INST_TMA,					// 0x43 : TMAi
	CPU_INST_BSR_REL,				// 0x44 : BSR	$rr
	CPU_INST_EOR_ZP,				// 0x45 : EOR	$ZZ
	CPU_INST_LSR_ZP,				// 0x46 : LSR	$ZZ
	CPU_INST_RMB4_ZP,				// 0x47 : RMB4	$ZZ
	CPU_INST_PHA,					// 0x48 : PHA
	CPU_INST_EOR_IMM,				// 0x49 : EOR	#$nn
	CPU_INST_LSR_ACCUM,				// 0x4A : LSR	A
	CPU_INST_BAD_OP_4B,				// 0x4B : BAD INSTRUCTION
	CPU_INST_JMP_ABS,				// 0x4C : JMP	$hhll
	CPU_INST_EOR_ABS,				// 0x4D : EOR	$hhll
	CPU_INST_LSR_ABS,				// 0x4E : LSR	$hhll
	CPU_INST_BBR4_ZP_REL,			// 0x4F : BBR4	$ZZ,$rr
	CPU_INST_BVC_REL,				// 0x50 : BVC	$rr
	CPU_INST_EOR_IND_Y,				// 0x51 : EOR	(IND),Y
	CPU_INST_EOR_IND,				// 0x52 : EOR	(IND)
	CPU_INST_TAM,					// 0x53 : TAMi
	CPU_INST_CSL,					// 0x54 : CSL
	CPU_INST_EOR_ZP_X,				// 0x55 : EOR	$ZZ,X
	CPU_INST_LSR_ZP_X,				// 0x56 : LSR	$ZZ,X
	CPU_INST_RMB5_ZP,				// 0x57 : RMB5	$ZZ
	CPU_INST_CLI,					// 0x58 : CLI
	CPU_INST_EOR_ABS_Y,				// 0x59 : EOR	$hhll,Y
	CPU_INST_PHY,					// 0x5A : PHY
	CPU_INST_BAD_OP_5B,				// 0x5B : BAD INSTRUCTION
	CPU_INST_BAD_OP_5C,				// 0x5C : BAD INSTRUCTION
	CPU_INST_EOR_ABS_X,				// 0x5D : EOR	$hhll,X
	CPU_INST_LSR_ABS_X,				// 0x5E : LSR	$hhll,X
	CPU_INST_BBR5_ZP_REL,			// 0x5F : BBR5	$ZZ,$rr
	CPU_INST_RTS,					// 0x60 : RTS
	CPU_INST_ADC_IND_X,				// 0x61 : ADC	($ZZ,X)
	CPU_INST_CLA,					// 0x62 : CLA
	CPU_INST_BAD_OP_63,				// 0x63 : BAD INSTRUCTION
	CPU_INST_STZ_ZP,				// 0x64 : STZ	$ZZ
	CPU_INST_ADC_ZP,				// 0x65 : ADC	$ZZ
	CPU_INST_ROR_ZP,				// 0x66 : ROR	$ZZ
	CPU_INST_RMB6_ZP,				// 0x67 : RMB6	$ZZ
	CPU_INST_PLA,					// 0x68 : PLA
	CPU_INST_ADC_IMM,				// 0x69 : ADC	#$nn
	CPU_INST_ROR_ACCUM,				// 0x6A : ROR	A
	CPU_INST_BAD_OP_6B,				// 0x6B : BAD INSTRUCTION
	CPU_INST_JMP_INDIR,				// 0x6C : JMP	($hhll)
	CPU_INST_ADC_ABS,				// 0x6D : ADC	$hhll
	CPU_INST_ROR_ABS,				// 0x6E : ROR	$hhll
	CPU_INST_BBR6_ZP_REL,			// 0x6F : BBR6	$ZZ,$rr
	CPU_INST_BVS_REL,				// 0x70 : BVS	$rr
	CPU_INST_ADC_IND_Y,				// 0x71 : ADC	($ZZ),Y
	CPU_INST_ADC_IND,				// 0x72 : ADC	($ZZ)
	CPU_INST_TII,					// 0x73 : TII	$SHSL,$DHDL,$LHLL
	CPU_INST_STZ_ZP_X,				// 0x74 : STZ	$ZZ,X
	CPU_INST_ADC_ZP_X,				// 0x75 : ADC	$ZZ,X
	CPU_INST_ROR_ZP_X,				// 0x76 : ROR	$ZZ,X
	CPU_INST_RMB7_ZP,				// 0x77 : RMB7	$ZZ
	CPU_INST_SEI,					// 0x78 : SEI
	CPU_INST_ADC_ABS_Y,				// 0x79 : ADC	$hhll,Y
	CPU_INST_PLY,					// 0x7A : PLY
	CPU_INST_BAD_OP_7B,				// 0x7B : BAD INSTRUCTION
	CPU_INST_JMP_INDIRX,			// 0x7C : JMP	$hhll,X
	CPU_INST_ADC_ABS_X,				// 0x7D : ADC	$hhll,X
	CPU_INST_ROR_ABS_X,				// 0x7E : ROR	$hhll,X
	CPU_INST_BBR7_ZP_REL,			// 0x7F : BBR7	$ZZ,$rr
	CPU_INST_BRA_REL,				// 0x80 : BRA	$rr
	CPU_INST_STA_IND_X,				// 0x81 : STA	(IND,X)
	CPU_INST_CLX,					// 0x82 : CLX
	CPU_INST_TST_IMM_ZP,			// 0x83 : TST	#$nn,$ZZ
	CPU_INST_STY_ZP,				// 0x84 : STY	$ZZ
	CPU_INST_STA_ZP,				// 0x85 : STA	$ZZ
	CPU_INST_STX_ZP,				// 0x86 : STX	$ZZ
	CPU_INST_SMB0_ZP,				// 0x87 : SMB0	$ZZ
	CPU_INST_DEY,					// 0x88 : DEY
	CPU_INST_BIT_IMM,				// 0x89 : BIT	#$nn
	CPU_INST_TXA,					// 0x8A : TXA
	CPU_INST_BAD_OP_8B,				// 0x8B : BAD INSTRUCTION
	CPU_INST_STY_ABS,				// 0x8C : STY	$hhll
	CPU_INST_STA_ABS,				// 0x8D : STA	$hhll
	CPU_INST_STX_ABS,				// 0x8E : STX	$hhll
	CPU_INST_BBS0_ZP_REL,			// 0x8F : BBS0	$ZZ,$rr
	CPU_INST_BCC_REL,				// 0x90 : BCC	$rr
	CPU_INST_STA_IND_Y,				// 0x91 : STA	(IND),Y
	CPU_INST_STA_IND,				// 0x92 : STA	(IND)
	CPU_INST_TST_IMM_ABS,			// 0x93 : TST	#$nn,$hhll
	CPU_INST_STY_ZP_X,				// 0x94 : STY	$ZZ,X
	CPU_INST_STA_ZP_X,				// 0x95 : STA	$ZZ,X
	CPU_INST_STX_ZP_Y,				// 0x96 : STX	$ZZ,Y
	CPU_INST_SMB1_ZP,				// 0x97 : SMB1	$ZZ
	CPU_INST_TYA,					// 0x98 : TYA
	CPU_INST_STA_ABS_Y,				// 0x99 : STA	$hhll,Y
	CPU_INST_TXS,					// 0x9A : TXS
	CPU_INST_BAD_OP_9B,				// 0x9B : BAD INSTRUCTION
	CPU_INST_STZ_ABS,				// 0x9C : STZ	$hhll
	CPU_INST_STA_ABS_X,				// 0x9D : STA	$hhll,X
	CPU_INST_STZ_ABS_X,				// 0x9E : STZ	$hhll,X
	CPU_INST_BBS1_ZP_REL,			// 0x9F : BBS1	$ZZ,$rr
	CPU_INST_LDY_IMM,				// 0xA0 : LDY	#$nn
	CPU_INST_LDA_IND_X,				// 0xA1 : LDA	(IND,X)
	CPU_INST_LDX_IMM,				// 0xA2 : LDX	#$nn
	CPU_INST_TST_IMM_ZP_X,			// 0xA3 : TST	#$nn,$ZZ,X
	CPU_INST_LDY_ZP,				// 0xA4 : LDY	$ZZ
	CPU_INST_LDA_ZP,				// 0xA5 : LDA	$ZZ
	CPU_INST_LDX_ZP,				// 0xA6 : LDX	$ZZ
	CPU_INST_SMB2_ZP,				// 0xA7 : SMB2	$ZZ
	CPU_INST_TAY,					// 0xA8 : TAY
	CPU_INST_LDA_IMM,				// 0xA9 : LDA	#$nn
	CPU_INST_TAX,					// 0xAA : TAX
	CPU_INST_BAD_OP_AB,				// 0xAB : BAD INSTRUCTION
	CPU_INST_LDY_ABS,				// 0xAC : LDY	$hhll
	CPU_INST_LDA_ABS,				// 0xAD : LDA	$hhll
	CPU_INST_LDX_ABS,				// 0xAE : LDX	$hhll
	CPU_INST_BBS2_ZP_REL,			// 0xAF : BBS2	$ZZ,$rr
	CPU_INST_BCS_REL,				// 0xB0 : BCS	$rr
	CPU_INST_LDA_IND_Y,				// 0xB1 : LDA	(IND),Y
	CPU_INST_LDA_IND,				// 0xB2 : LDA 	(IND)
	CPU_INST_TST_IMM_ABS_X,			// 0xB3 : TST	#$nn,$hhll,X
	CPU_INST_LDY_ZP_X,				// 0xB4 : LDY	$ZZ,X
	CPU_INST_LDA_ZP_X,				// 0xB5 : LDA	$ZZ,X
	CPU_INST_LDX_ZP_Y,				// 0xB6 : LDX	$ZZ,Y
	CPU_INST_SMB3_ZP,				// 0xB7 : SMB3	$ZZ
	CPU_INST_CLV,					// 0xB8 : CLV
	CPU_INST_LDA_ABS_Y,				// 0xB9 : LDA	$hhll,Y
	CPU_INST_TSX,					// 0xBA : TSX
	CPU_INST_BAD_OP_BB,				// 0xBB : BAD INSTRUCTION
	CPU_INST_LDY_ABS_X,				// 0xBC : LDY	$hhll,X
	CPU_INST_LDA_ABS_X,				// 0xBD : LDA	$hhll,X
	CPU_INST_LDX_ABS_Y,				// 0xBE : LDX	$hhll,Y
	CPU_INST_BBS3_ZP_REL,			// 0xBF : BBS3	$ZZ,$rr
	CPU_INST_CPY_IMM,				// 0xC0 : CPY	#$nn
	CPU_INST_CMP_IND_X,				// 0xC1 : CMP	(IND,X)
	CPU_INST_CLY,					// 0xC2 : CLY
	CPU_INST_TDD,					// 0xC3 : TDD	$SHSL,$DHDL,$LHLL
	CPU_INST_CPY_ZP,				// 0xC4 : CPY	$ZZ
	CPU_INST_CMP_ZP,				// 0xC5 : CMP	$ZZ
	CPU_INST_DEC_ZP,				// 0xC6 : DEC	$ZZ
	CPU_INST_SMB4_ZP,				// 0xC7 : SMB4	$ZZ
	CPU_INST_INY,					// 0xC8 : INY
	CPU_INST_CMP_IMM,				// 0xC9 : CMP	#$nn
	CPU_INST_DEX,					// 0xCA : DEX
	CPU_INST_BAD_OP_CB,				// 0xCB : BAD INSTRUCTION
	CPU_INST_CPY_ABS,				// 0xCC : CPY	$hhll
	CPU_INST_CMP_ABS,				// 0xCD : CMP	$hhll
	CPU_INST_DEC_ABS,				// 0xCE : DEC	$hhll
	CPU_INST_BBS4_ZP_REL,			// 0xCF : BBS4	$ZZ,$rr
	CPU_INST_BNE_REL,				// 0xD0 : BNE	$rr
	CPU_INST_CMP_IND_Y,				// 0xD1 : CMP	(IND),Y
	CPU_INST_CMP_IND,				// 0xD2 : CMP	(IND)
	CPU_INST_TIN,					// 0xD3 : TIN	$SHSL,$DHDL,$LHLL
	CPU_INST_CSH,					// 0xD4 : CSH
	CPU_INST_CMP_ZP_X,				// 0xD5 : CMP	$ZZ,X
	CPU_INST_DEC_ZP_X,				// 0xD6 : DEC	$ZZ,X
	CPU_INST_SMB5_ZP,				// 0xD7 : SMB5	$ZZ
	CPU_INST_CLD,					// 0xD8 : CLD
	CPU_INST_CMP_ABS_Y,				// 0xD9 : CMP	$hhll,Y
	CPU_INST_PHX,					// 0xDA : PHX
	CPU_INST_BAD_OP_DB,				// 0xDB : BAD INSTRUCTION
	CPU_INST_BAD_OP_DC,				// 0xDC : BAD INSTRUCTION
	CPU_INST_CMP_ABS_X,				// 0xDD : CMP	$hhll,X
	CPU_INST_DEC_ABS_X,				// 0xDE : DEC	$hhll,X
	CPU_INST_BBS5_ZP_REL,			// 0xDF : BBS5	$ZZ,$rr
	CPU_INST_CPX_IMM,				// 0xE0 : CPX	#$nn
	CPU_INST_SBC_IND_X,				// 0xE1 : SBC	(IND,X)
	CPU_INST_BAD_OP_E2,				// 0xE2 : BAD INSTRUCTION
	CPU_INST_TIA,					// 0xE3 : TIA	$SHSL,$DHDL,$LHLL
	CPU_INST_CPX_ZP,				// 0xE4 : CPX	$ZZ
	CPU_INST_SBC_ZP,				// 0xE5 : SBC	$ZZ
	CPU_INST_INC_ZP,				// 0xE6 : INC	$ZZ
	CPU_INST_SMB6_ZP,				// 0xE7 : SMB6	$ZZ
	CPU_INST_INX,					// 0xE8 : INX
	CPU_INST_SBC_IMM,				// 0xE9 : SBC	#$nn
	CPU_INST_NOP,					// 0xEA : NOP
	CPU_INST_BAD_OP_EB,				// 0xEB : BAD INSTRUCTION
	CPU_INST_CPX_ABS,				// 0xEC : CPX	$hhll
	CPU_INST_SBC_ABS,				// 0xED : SBC	$hhll
	CPU_INST_INC_ABS,				// 0xEE : INC	$hhll
	CPU_INST_BBS6_ZP_REL,			// 0xEF : BBS6	$ZZ,$rr
	CPU_INST_BEQ_REL,				// 0xF0 : BEQ	$rr
	CPU_INST_SBC_IND_Y,				// 0xF1 : SBC	(IND),Y
	CPU_INST_SBC_IND,				// 0xF2 : SBC	(IND)
	CPU_INST_TAI,					// 0xF3 : TAI	$SHSL,$DHDL,$LHLL
	CPU_INST_SET,					// 0xF4 : SET
	CPU_INST_SBC_ZP_X,				// 0xF5 : SBC	$ZZ,X
	CPU_INST_INC_ZP_X,				// 0xF6 : INC	$ZZ,X
	CPU_INST_SMB7_ZP,				// 0xF7 : SMB7	$ZZ
	CPU_INST_SED,					// 0xF8 : SED
	CPU_INST_SBC_ABS_Y,				// 0xF9 : SBC	$hhll,Y
	CPU_INST_PLX,					// 0xFA : PLX
	CPU_INST_BAD_OP_FB,				// 0xFB : BAD INSTRUCTION
	CPU_INST_BAD_OP_FC,				// 0xFC : BAD INSTRUCTION
	CPU_INST_SBC_ABS_X,				// 0xFD : SBC	$hhll,X
	CPU_INST_INC_ABS_X,				// 0xFE : INC	$hhll,X
	CPU_INST_BBS7_ZP_REL			// 0xFF : BBS7	$ZZ,$rr
};


#endif /* CPU_H_INCLUDED */
