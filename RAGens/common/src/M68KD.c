#include <stdio.h>
#include "mem_M68K.h"
#include "mem_S68K.h"


static char Dbg_Str[32];
static char Dbg_EA_Str[16];
static char Dbg_Size_Str[3];
static char Dbg_Cond_Str[3];

static unsigned short (*Next_Word)();
static unsigned int (*Next_Long)();


char *Make_Dbg_EA_Str(int Size, int EA_Num, int Reg_Num)
{
	int i;
	Dbg_EA_Str[15] = 0;

	switch(EA_Num)
	{
		case 0:
			// 000 rrr  Dr
			sprintf_s(Dbg_EA_Str, 16, "D%.1d%c", Reg_Num, 0);
			break;

		case 1:
			// 001 rrr  Ar
			sprintf_s(Dbg_EA_Str, 16, "A%.1d%c", Reg_Num, 0);
			break;

		case 2:
			// 010 rrr  (Ar)
			sprintf_s(Dbg_EA_Str, 16, "(A%.1d)%c", Reg_Num, 0);
			break;

		case 3:
			// 011 rrr  (Ar)+
			sprintf_s(Dbg_EA_Str, 16, "(A%.1d)+%c", Reg_Num, 0);
			break;

		case 4:
			// 100 rrr  -(Ar)
			sprintf_s(Dbg_EA_Str, 16, "-(A%.1d)%c", Reg_Num, 0);
			break;

		case 5:
			// 101 rrr  d16(Ar)     dddddddd dddddddd
			sprintf_s(Dbg_EA_Str, 16, "$%.4X(A%.1d)%c", Next_Word(), Reg_Num, 0);
			break;

		case 6:
			// 110 rrr  d8(Ar,ix)   aiiizcc0 dddddddd
			i = Next_Word() & 0xFFFF;
			if (i & 0x8000)
				sprintf_s(Dbg_EA_Str, 16, "$%.2X(A%.1d,A%.1d)%c", i & 0xFF, Reg_Num, (i >> 12) & 0x7, 0);
			else
  				sprintf_s(Dbg_EA_Str, 16, "$%.2X(A%.1d,D%.1d)%c", i & 0xFF, Reg_Num, (i >> 12) & 0x7, 0);
			break;

		case 7:
			switch(Reg_Num)
			{
				case 0:
					// 111 000  addr16      dddddddd dddddddd
					sprintf_s(Dbg_EA_Str, 16, "($%.4X)%c", Next_Word(), 0);
					break;

				case 1:
					// 111 001  addr32      dddddddd dddddddd ddddddddd dddddddd
					sprintf_s(Dbg_EA_Str, 16, "($%.8X)%c", Next_Long(), 0);
					break;

				case 2:
					// 111 010  d16(PC)     dddddddd dddddddd
					sprintf_s(Dbg_EA_Str, 16, "$%.4X(PC)%c", Next_Word(), 0);
					break;

				case 3:
					// 111 011  d8(PC,ix)   aiiiz000 dddddddd
					i = Next_Word() & 0xFFFF;
					if (i & 0x8000)
						sprintf_s(Dbg_EA_Str, 16, "$%.2X(PC,A%.1d)%c", i & 0xFF, (i >> 12) & 0x7, 0);
					else
						sprintf_s(Dbg_EA_Str, 16, "$%.2X(PC,D%.1d)%c", i & 0xFF, (i >> 12) & 0x7, 0);
					break;

				case 4:
					// 111 100  imm/implied
					switch(Size)
					{
						case 0:
							sprintf_s(Dbg_EA_Str, 16, "#$%.2X%c", Next_Word() & 0xFF, 0);
							break;

						case 1:
							sprintf_s(Dbg_EA_Str, 16, "#$%.4X%c", Next_Word(), 0);
							break;

						case 2:
							sprintf_s(Dbg_EA_Str, 16, "#$%.8X%c", Next_Long(), 0);
							break;
					}
					break;
			}
			break;
		}

	return(Dbg_EA_Str);
}


char *Make_Dbg_Size_Str(int Size)
{
	Dbg_Size_Str[2] = 0;
	sprintf_s(Dbg_Size_Str, 3, ".?");

	switch(Size)
	{
		case 0:
			sprintf_s(Dbg_Size_Str, 3, ".B");
			break;

		case 1:
			sprintf_s(Dbg_Size_Str, 3, ".W");
			break;

		case 2:
			sprintf_s(Dbg_Size_Str, 3, ".L");
			break;
	}

	return(Dbg_Size_Str);
}


char *Make_Dbg_Size_Str_2(int Size)
{
	Dbg_Size_Str[2] = 0;
	sprintf_s(Dbg_Size_Str, 3, ".?");

	switch(Size)
	{
		case 0:
			sprintf_s(Dbg_Size_Str, 3, ".W");
			break;

		case 1:
			sprintf_s(Dbg_Size_Str, 3, ".L");
			break;
	}

	return(Dbg_Size_Str);
}

char *Make_Dbg_Cond_Str(int Cond)
{
	Dbg_Cond_Str[2] = 0;
	sprintf_s(Dbg_Size_Str, 3, "??");

	switch(Cond)
	{
		case 0:
			sprintf_s(Dbg_Size_Str, 3, "Tr");
			break;

		case 1:
			sprintf_s(Dbg_Size_Str, 3, "Fa");
			break;

		case 2:
			sprintf_s(Dbg_Size_Str, 3, "HI");
			break;

		case 3:
			sprintf_s(Dbg_Size_Str, 3, "LS");
			break;

		case 4:
			sprintf_s(Dbg_Size_Str, 3, "CC");
			break;

		case 5:
			sprintf_s(Dbg_Size_Str, 3, "CS");
			break;

		case 6:
			sprintf_s(Dbg_Size_Str, 3, "NE");
			break;

		case 7:
			sprintf_s(Dbg_Size_Str, 3, "EQ");
			break;

		case 8:
			sprintf_s(Dbg_Size_Str, 3, "VC");
			break;

		case 9:
			sprintf_s(Dbg_Size_Str, 3, "VS");
			break;

		case 10:
			sprintf_s(Dbg_Size_Str, 3, "PL");
			break;

		case 11:
			sprintf_s(Dbg_Size_Str, 3, "MI");
			break;

		case 12:
			sprintf_s(Dbg_Size_Str, 3, "GE");
			break;

		case 13:
			sprintf_s(Dbg_Size_Str, 3, "LT");
			break;

		case 14:
			sprintf_s(Dbg_Size_Str, 3, "GT");
			break;

		case 15:
			sprintf_s(Dbg_Size_Str, 3, "LE");
			break;
    }

	return(Dbg_Cond_Str);
}


char *M68KDisasm(unsigned short (*NW)(), unsigned int (*NL)())
{
	int i;
	unsigned short OPC;
	char Tmp_Str[32];

	Dbg_Str[31] = 0;
	Tmp_Str[31] = 0;

	Next_Word = NW;
	Next_Long = NL;

	OPC = Next_Word();

	sprintf_s(Dbg_Str, 32, "Unknow Opcode%c", 0);

	switch(OPC >> 12)
	{
		case 0:

		if (OPC & 0x100)
		{
			if ((OPC & 0x038) == 0x8)
			{
				if (OPC & 0x080)
					//MOVEP.z Ds,d16(Ad)
					sprintf_s(Dbg_Str, 32, "MOVEP%-3sD%.1d,#$%.4X(A%.1d)%c", Make_Dbg_Size_Str_2((OPC & 0x40) >> 6), (OPC & 0xE00) >> 9, Next_Word(), OPC & 0x7, 0);
				else
					//MOVEP.z d16(As),Dd
					sprintf_s(Dbg_Str, 32, "MOVEP%-3s#$%.4X(A%.1d),D%.1d%c", Make_Dbg_Size_Str_2((OPC & 0x40) >> 6), Next_Word(), OPC & 0x7, (OPC & 0xE00) >> 9, 0);
			}
			else
			{
				switch((OPC >> 6) & 0x3)
				{
					case 0:
						//BTST  Ds,a
						sprintf_s(Dbg_Str, 32, "BTST    D%.1d,%s%c", (OPC & 0xE00) >> 9, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 7), 0);
						break;

					case 1:
						//BCHG  Ds,a
						sprintf_s(Dbg_Str, 32, "BCHG    D%.1d,%s%c", (OPC & 0xE00) >> 9, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 7), 0);
						break;

					case 2:
						//BCLR  Ds,a
						sprintf_s(Dbg_Str, 32, "BCLR    D%.1d,%s%c", (OPC & 0xE00) >> 9, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 7), 0);
						break;

					case 3:
						//BSET  Ds,a
						sprintf_s(Dbg_Str, 32, "BSET    D%.1d,%s%c", (OPC & 0xE00) >> 9, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 7), 0);
						break;
				}
			}
		}
		else
		{
			switch((OPC >> 6) & 0x3F)
			{
				case 0:
					//ORI.B  #k,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "ORI.B   #$%.2X,%s%c", i, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 1:
					//ORI.W  #k,a
					i = Next_Word() & 0xFFFF;
					sprintf_s(Dbg_Str, 32, "ORI.W   #$%.4X,%s%c", i, Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 2:
					//ORI.L  #k,a
					i = Next_Long();
					sprintf_s(Dbg_Str, 32, "ORI.L   #$%.8X,%s%c", i, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 8:
					//ANDI.B  #k,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "ANDI.B  #$%.2X,%s%c", i & 0xFF, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 9:
					//ANDI.W  #k,a
					i = Next_Word() & 0xFFFF;
					sprintf_s(Dbg_Str, 32, "ANDI.W  #$%.4X,%s%c", i, Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 10:
					//ANDI.L  #k,a
					i = Next_Long();
					sprintf_s(Dbg_Str, 32, "ANDI.L  #$%.8X,%s%c", i, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 16:
					//SUBI.B  #k,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "SUBI.B  #$%.2X,%s%c", i & 0xFF, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 17:
					//SUBI.W  #k,a
					i = Next_Word() & 0xFFFF;
					sprintf_s(Dbg_Str, 32, "SUBI.W  #$%.4X,%s%c", i, Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 18:
					//SUBI.L  #k,a
					i = Next_Long();
					sprintf_s(Dbg_Str, 32, "SUBI.L  #$%.8X,%s%c", i, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 24:
					//ADDI.B  #k,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "ADDI.B  #$%.2X,%s%c", i & 0xFF, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 25:
					//ADDI.W  #k,a
					i = Next_Word() & 0xFFFF;
					sprintf_s(Dbg_Str, 32, "ADDI.W  #$%.4X,%s%c", i, Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 26:
					//ADDI.L  #k,a
					i = Next_Long();
					sprintf_s(Dbg_Str, 32, "ADDI.L  #$%.8X,%s%c", i, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 32:
					//BTST  #n,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "BTST    #%d,%s%c", i, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 33:
					//BCHG  #n,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "BCHG    #%d,%s%c", i, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 34:
					//BCLR  #n,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "BCLR    #%d,%s%c", i, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 35:
					//BSET  #n,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "BSET    #%d,%s%c", i, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 40:
					//EORI.B  #k,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "EORI.B  #$%.2X,%s%c", i & 0xFF, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 41:
					//EORI.W  #k,a
					i = Next_Word() & 0xFFFF;
					sprintf_s(Dbg_Str, 32, "EORI.W  #$%.4X,%s%c", i, Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 42:
					//EORI.L  #k,a
					i = Next_Long();
					sprintf_s(Dbg_Str, 32, "EORI.L  #$%.8X,%s%c", i, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 48:
					//CMPI.B  #k,a
					i = Next_Word() & 0xFF;
					sprintf_s(Dbg_Str, 32, "CMPI.B  #$%.2X,%s%c", i & 0xFF, Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 49:
					//CMPI.W  #k,a
					i = Next_Word() & 0xFFFF;
					sprintf_s(Dbg_Str, 32, "CMPI.W  #$%.4X,%s%c", i, Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 50:
					//CMPI.L  #k,a
					i = Next_Long();
					sprintf_s(Dbg_Str, 32, "CMPI.L  #$%.8X,%s%c", i, Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;
			}
		}
		break;

		case 1:
			//MOVE.b  as,ad
			sprintf_s(Tmp_Str, 32, "%s%c", Make_Dbg_EA_Str(0, (OPC >> 3) & 0x7, OPC & 0x7), 0);
			sprintf_s(Dbg_Str, 32, "MOVE.b  %s,%s%c", Tmp_Str, Make_Dbg_EA_Str(0, (OPC >> 6) & 0x7, (OPC >> 9) & 0x7), 0);
			break;

		case 2:
			//MOVE.l  as,ad
			sprintf_s(Tmp_Str, 32, "%s%c", Make_Dbg_EA_Str(2, (OPC >> 3) & 0x7, OPC & 0x7), 0);
			sprintf_s(Dbg_Str, 32, "MOVE.l  %s,%s%c", Tmp_Str, Make_Dbg_EA_Str(2, (OPC >> 6) & 0x7, (OPC >> 9) & 0x7), 0);
			break;

		case 3:
			//MOVE.w  as,ad
			sprintf_s(Tmp_Str, 32, "%s%c", Make_Dbg_EA_Str(1, (OPC >> 3) & 0x7, OPC & 0x7), 0);
			sprintf_s(Dbg_Str, 32, "MOVE.w  %s,%s%c", Tmp_Str, Make_Dbg_EA_Str(1, (OPC >> 6) & 0x7, (OPC >> 9) & 0x7), 0);
			break;

		case 4:
			//SPECIALS ...

			if (OPC & 0x100)
			{
				if (OPC & 0x40)
					//LEA  a,Ad
					sprintf_s(Dbg_Str, 32, "LEA     %s,A%.1d%c", Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
				else
					//CHK.W  a,Dd
					sprintf_s(Dbg_Str, 32, "CHK.W   %s,D%.1d%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			}
			else
			{
				switch((OPC >> 6) & 0x3F)
				{
					case 0:	case 1: case 2:
						//NEGX.z  a
						sprintf_s(Dbg_Str, 32, "NEGX%-4s%s%c", Make_Dbg_Size_Str((OPC >> 6) & 3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 3:
						//MOVE  SR,a
						sprintf_s(Dbg_Str, 32, "MOVE    SR,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 8: case 9: case 10:
						//CLR.z  a
						sprintf_s(Dbg_Str, 32, "CLR%-5s%s%c", Make_Dbg_Size_Str((OPC >> 6) & 3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 16: case 17: case 18:
						//NEG.z  a
						sprintf_s(Dbg_Str, 32, "NEG%-5s%s%c", Make_Dbg_Size_Str((OPC >> 6) & 3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 19:
						//MOVE  a,CCR
						sprintf_s(Dbg_Str, 32, "MOVE    %s,CCR%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 24: case 25: case 26:
						//NOT.z  a
						sprintf_s(Dbg_Str, 32, "NOT%-4s%s%c", Make_Dbg_Size_Str((OPC >> 6) & 3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 27:
						//MOVE  a,SR
						sprintf_s(Dbg_Str, 32, "MOVE    %s,SR%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 32:
						//NBCD  a
						sprintf_s(Dbg_Str, 32, "NBCD    %s%c", Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 33:

						if (OPC & 0x38)
							//PEA  a
							sprintf_s(Dbg_Str, 32, "PEA     %s%c", Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						else
							//SWAP.w  Dd
							sprintf_s(Dbg_Str, 32, "SWAP.w  D%d%c", OPC & 0x7, 0);

						break;

					case 34: case 35:

						if (OPC & 0x38)
						{
							//MOVEM.z Reg-List,a
							sprintf_s(Dbg_Str, 32, "MOVEM%-3sReg-List,%s%c", Make_Dbg_Size_Str_2((OPC >> 6) & 1), Make_Dbg_EA_Str((OPC >> 6) & 1 + 1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
							Next_Word();
						}
						else
							//EXT.z  Dd
							sprintf_s(Dbg_Str, 32, "EXT%-5s%s%c", Make_Dbg_Size_Str_2((OPC >> 6) & 1), Make_Dbg_EA_Str((OPC >> 6) & 1 + 1, (OPC & 0x38) >> 3, OPC & 0x7), 0);

						break;

					case 40: case 41: case 42:
						//TST.z a
						sprintf_s(Dbg_Str, 32, "TST%-5s%s%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), Make_Dbg_EA_Str((OPC >> 6) & 0x3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 43:
						//TAS.b a
						sprintf_s(Dbg_Str, 32, "TAS.B %s%c", Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 48: case 49:
						//Bad Opcode
						sprintf_s(Dbg_Str, 32, "Bad Opcode%c", 0);
						break;

					case 50: case 51:
						//MOVEM.z a,Reg-List
						sprintf_s(Dbg_Str, 32, "MOVEM%-3s%s,Reg-List%c", Make_Dbg_Size_Str_2((OPC >> 6) & 1), Make_Dbg_EA_Str((OPC >> 6) & 1 + 1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						Next_Word();
						break;

					case 57:

						switch((OPC >> 3) & 0x7)
						{
							case 0: case 1:
								//TRAP  #vector
								sprintf_s(Dbg_Str, 32, "TRAP    #$%.1X%c", OPC & 0xF, 0);
								break;

							case 2:
								//LINK As,#k16
								sprintf_s(Dbg_Str, 32, "LINK    A%.1d,#$%.4X%c", OPC & 0x7, Next_Word(), 0);
								break;

							case 3:
								//ULNK Ad
								sprintf_s(Dbg_Str, 32, "ULNK    A%.1d%c", OPC & 0x7, 0);
								break;

							case 4:
								//MOVE As,USP
								sprintf_s(Dbg_Str, 32, "MOVE    A%.1d,USP%c",OPC & 0x7, 0);
								break;

							case 5:
								//MOVE USP,Ad
								sprintf_s(Dbg_Str, 32, "MOVE    USP,A%.1d%c",OPC & 0x7, 0);
								break;

							case 6:

								switch(OPC & 0x7)
								{
									case 0:
										//RESET
										sprintf_s(Dbg_Str, 32, "RESET%c", 0);
										break;

									case 1:
										//NOP
										sprintf_s(Dbg_Str, 32, "NOP%c", 0);
										break;

									case 2:
										//STOP #k16
										sprintf_s(Dbg_Str, 32, "STOP    #$%.4X%c", Next_Word(), 0);
										break;

									case 3:
										//RTE
										sprintf_s(Dbg_Str, 32, "RTE%c", 0);
										break;

									case 4:
										//Bad Opcode
										sprintf_s(Dbg_Str, 32, "Bad Opcode%c", 0);
										break;

									case 5:
										//RTS
										sprintf_s(Dbg_Str, 32, "RTS%c", 0);
										break;

									case 6:
										//TRAPV
										sprintf_s(Dbg_Str, 32, "TRAPV%c", 0);
										break;

									case 7:
										//RTR
										sprintf_s(Dbg_Str, 32, "RTR%c", 0);
										break;
								}
								break;
						}
						break;

					case 58:
						//JSR  a
						sprintf_s(Dbg_Str, 32, "JSR     %s%c", Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 59:
						//JMP  a
						sprintf_s(Dbg_Str, 32, "JMP     %s%c", Make_Dbg_EA_Str(2, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;
				}
			}
			break;

		case 5:

			if ((OPC & 0xC0) == 0xC0)
			{
				if ((OPC & 0x38) == 0x08)
					//DBCC  Ds,label
					sprintf_s(Dbg_Str, 32, "DB%-6sD%.1d,#$%.4X%c", Make_Dbg_Cond_Str((OPC >> 8) & 0xF), OPC & 0x7, Next_Word(), 0);
				else
					//STCC.b  a
					sprintf_s(Dbg_Str, 32, "ST%-6s%s%c", Make_Dbg_Cond_Str((OPC >> 8) & 0xF), Make_Dbg_EA_Str(0, (OPC & 0x38) >> 3, OPC & 0x7), 0);
				break;
			}
			else
			{
				if (OPC & 0x100)
					//SUBQ.z  #k3,a
					sprintf_s(Dbg_Str, 32, "SUBQ%-4s#%.1d,%s%c", Make_Dbg_Size_Str((OPC >> 6) & 3), (OPC >> 9) & 0x7, Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
				else
					//ADDQ.z  #k3,a
					sprintf_s(Dbg_Str, 32, "ADDQ%-4s#%.1d,%s%c", Make_Dbg_Size_Str((OPC >> 6) & 3), (OPC >> 9) & 0x7, Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
				break;
			}
			break;

		case 6:

			if (OPC & 0xFF)
			{
				if ((OPC & 0xF00) == 0x100)
				{
					//BSR  label
					sprintf_s(Dbg_Str, 32, "BSR     #$%.2X%c", OPC & 0xFF, 0);
					break;
				}

				if (!(OPC & 0xF00))
				{
					//BRA  label
					sprintf_s(Dbg_Str, 32, "BRA     #$%.2X%c", OPC & 0xFF, 0);
					break;
				}

				//BCC  label
				sprintf_s(Dbg_Str, 32, "B%-7s#$%.2X%c", Make_Dbg_Cond_Str((OPC >> 8) & 0xF), OPC & 0xFF, 0);
			}
			else
			{
				if ((OPC & 0xF00) == 0x100)
				{
					//BSR  label
					sprintf_s(Dbg_Str, 32, "BSR     #$%.4X%c", Next_Word(), 0);
					break;
				}

				if (!(OPC & 0xF00))
				{
					//BRA  label
					sprintf_s(Dbg_Str, 32, "BRA     #$%.4X%c", Next_Word(), 0);
					break;
				}

				//BCC  label
				sprintf_s(Dbg_Str, 32, "B%-7s#$%.4X%c", Make_Dbg_Cond_Str((OPC >> 8 ) & 0xF), Next_Word(), 0);
			}
			break;

		case 7:
			//MOVEQ  #k8,Dd
			sprintf_s(Dbg_Str, 32, "MOVEQ   #$%.2X,D%.1d%c", OPC & 0xFF, (OPC >> 9) & 0x7, 0);
			break;

		case 8:

			if (OPC & 0x100)
  			{
				if (!(OPC & 0xF8))
				{
					//SBCD  Ds,Dd
					sprintf_s(Dbg_Str, 32, "SBCD D%.1d,D%.1d%c", OPC & 0x7, (OPC >> 9) & 0x7, 0);
					break;
				}

				if ((OPC & 0xF8) == 0x8)
				{
					//SBCD  -(As),-(Ad)
					sprintf_s(Dbg_Str, 32, "SBCD -(A%.1d),-(A%.1d)%c", OPC & 0x7, (OPC >> 9) & 0x7, 0);
					break;
				}

				if ((OPC & 0xC0) == 0xC0)
					//DIVS.w  a,Dd
					sprintf_s(Dbg_Str, 32, "DIVS.W  %s,D%.1d%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
				else
					//OR.z  Ds,a
					sprintf_s(Dbg_Str, 32, "OR%-6sD%.1d;%s%c", Make_Dbg_Size_Str((OPC >> 6) & 3), (OPC >> 9) & 0x7, Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
			}
			else
			{
				if ((OPC & 0xC0) == 0xC0)
					//DIVU.w  a,Dd
					sprintf_s(Dbg_Str, 32, "DIVU.W  %s,D%.1d%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
				else
					//OR.z  a,Dd
					sprintf_s(Dbg_Str, 32, "OR%-6s%s,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			}
			break;

		case 9:

			if ((OPC & 0xC0) == 0xC0)
				//SUBA.z  a,Ad
				sprintf_s(Dbg_Str, 32, "SUBA%-4s%s,A%.1d%c", Make_Dbg_Size_Str_2((OPC >> 8) & 1), Make_Dbg_EA_Str((OPC >> 8) & 1 + 1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			else
			{
				if (OPC & 0x100)
				{
					if (!(OPC & 0x38))
					{
						//SUBX.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "SUBX%-4sD%.1d,D%.1d%c",	Make_Dbg_Size_Str((OPC >> 6) & 0x3), OPC & 0x7, (OPC >> 9) & 0x7, 0);
						break;
					}

					if ((OPC & 0x38) == 0x8)
					{
						//SUBX.z  -(As),-(Ad)
						sprintf_s(Dbg_Str, 32, "SUBX%-4s-(A%.1d),-(A%.1d)%c",	Make_Dbg_Size_Str((OPC >> 6) & 0x3), OPC & 0x7, (OPC >> 9) & 0x7, 0);
						break;
					}

					//SUB.z  Ds,a
					sprintf_s(Dbg_Str, 32, "SUB%-5sD%.1d,%s%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
				}
				else
					//SUB.z  a,Dd
					sprintf_s(Dbg_Str, 32, "SUB%-5s%s,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			}
			break;

		case 10:
			//Bad Opcode
			sprintf_s(Dbg_Str, 32, "Bad Opcode%c", 0);
			break;

		case 11:

			if ((OPC & 0xC0) == 0xC0)
				//CMPA.z  a,Ad
				sprintf_s(Dbg_Str, 32, "CMPA%-4s%s,A%.1d%c", Make_Dbg_Size_Str_2((OPC >> 8) & 1), Make_Dbg_EA_Str((OPC >> 7) & 1 + 1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			else
			{
				if (OPC & 0x100)
				{
					if ((OPC & 0x38) == 0x8)
					{
						//CMPM.z  (As)+,(Ad)+
						sprintf_s(Dbg_Str, 32, "CMPM%-4s(A%.1d)+,(A%.1d)+%c",	Make_Dbg_Size_Str((OPC >> 6) & 0x3), OPC & 0x7, (OPC >> 9) & 0x7, 0);
						break;
					}

					//EOR.z  Ds,a
					sprintf_s(Dbg_Str, 32, "EOR%-5sD%.1d,%s%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
				}
				else
					//CMP.z  a,Dd
					sprintf_s(Dbg_Str, 32, "CMP%-5s%s,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			}
			break;

		case 12:

			if ((OPC & 0X1F8) == 0x100)
			{
				//ABCD Ds,Dd
				sprintf_s(Dbg_Str, 32, "ABCD    D%.1d,D%.1d%c", OPC & 0x7, (OPC >> 9) & 0x7, 0);
				break;
			}

			if ((OPC & 0X1F8) == 0x140)
			{
				//EXG.l Ds,Dd
				sprintf_s(Dbg_Str, 32, "EXG.L   D%.1d,D%.1d%c", OPC & 0x7, (OPC >> 9) & 0x7, 0);
				break;
			}

			if ((OPC & 0X1F8) == 0x108)
			{
				//ABCD -(As),-(Ad)
				sprintf_s(Dbg_Str, 32, "ABCD    -(A%.1d),-(A%.1d)%c", OPC & 0x7, (OPC >> 9) & 0x7, 0);
				break;
			}

			if ((OPC & 0X1F8) == 0x148)
			{
				//EXG.l As,Ad
				sprintf_s(Dbg_Str, 32, "EXG.L   A%.1d,A%.1d%c", OPC & 0x7, (OPC >> 9) & 0x7, 0);
				break;
			}

			if ((OPC & 0X1F8) == 0x188)
			{
				//EXG.l As,Dd
				sprintf_s(Dbg_Str, 32, "EXG.L   A%.1d,D%.1d%c", OPC & 0x7, (OPC >> 9) & 0x7, 0);
				break;
			}

			switch((OPC	>> 6) & 0x7)
			{
				case 0: case 1: case 2:
					//AND.z  a,Dd
					sprintf_s(Dbg_Str, 32, "AND%-5s%s,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
					break;

				case 3:
					//MULU.w  a,Dd
					sprintf_s(Dbg_Str, 32, "MULU.W  %s,D%.1d%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
					break;

				case 4: case 5: case 6:
					//AND.z  Ds,a
					sprintf_s(Dbg_Str, 32, "AND%-5sD%.1d,%s%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
					break;

				case 7:
					//MULS.w  a,Dd
					sprintf_s(Dbg_Str, 32, "MULS.W  %s,D%.1d%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
					break;
			}
			break;

		case 13:

			if ((OPC & 0xC0) == 0xC0)
				//ADDA.z  a,Ad
				sprintf_s(Dbg_Str, 32, "ADDA%-4s%s,A%.1d%c", Make_Dbg_Size_Str_2((OPC >> 8) & 1), Make_Dbg_EA_Str((OPC >> 8) & 1 + 1, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			else
			{
				if (OPC & 0x100)
				{
					if (!(OPC & 0x38))
					{
						//ADDX.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "ADDX%-4sD%.1d,D%.1d%c",	Make_Dbg_Size_Str((OPC >> 6) & 0x3), OPC & 0x7, (OPC >> 9) & 0x7, 0);
						break;
					}

					if ((OPC & 0x38) == 0x8)
					{
						//ADDX.z  -(As),-(Ad)
						sprintf_s(Dbg_Str, 32, "ADDX%-4s-(A%.1d),-(A%.1d)%c",	Make_Dbg_Size_Str((OPC >> 6) & 0x3), OPC & 0x7, (OPC >> 9) & 0x7, 0);
						break;
					}

					//ADD.z  Ds,a
					sprintf_s(Dbg_Str, 32, "ADD%-5sD%.1d,%s%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), 0);
				}
				else
					//ADD.z  a,Dd
					sprintf_s(Dbg_Str, 32, "ADD%-5s%s,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), Make_Dbg_EA_Str((OPC >> 6) & 3, (OPC & 0x38) >> 3, OPC & 0x7), (OPC >> 9) & 0x7, 0);
			}
			break;

		case 14:

			if ((OPC & 0xC0) == 0xC0)
			{
				switch ((OPC >> 8) & 0x7)
				{
					case 0:
						//ASR.w  #1,a
						sprintf_s(Dbg_Str, 32, "ASR.W   #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;
	
					case 1:
						//ASL.w  #1,a
						sprintf_s(Dbg_Str, 32, "ASL.W   #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 2:
						//LSR.w  #1,a
						sprintf_s(Dbg_Str, 32, "LSR.W   #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 3:
						//LSL.w  #1,a
						sprintf_s(Dbg_Str, 32, "LSL.W   #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 4:
						//ROXR.w  #1,a
						sprintf_s(Dbg_Str, 32, "ROXR.W  #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 5:
						//ROXL.w  #1,a
						sprintf_s(Dbg_Str, 32, "ROXL.W  #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 6:
						//ROR.w  #1,a
						sprintf_s(Dbg_Str, 32, "ROR.W   #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;

					case 7:
						//ROL.w  #1,a
						sprintf_s(Dbg_Str, 32, "ROL.W   #1,%s%c", Make_Dbg_EA_Str(1, (OPC & 0x38) >> 3, OPC & 0x7), 0);
						break;
 
				}
			}
			else
			{
				switch ((OPC >> 3) & 0x3F)
				{
					case 0: case 8: case 16:
						//ASR.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "ASR%-5s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 1: case 9: case 17:
						//LSR.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "LSR%-5s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 2: case 10: case 18:
						//ROXR.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "ROXR%-4s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 3: case 11: case 19:
						//ROR.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "ROR%-5s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 4: case 12: case 20:
						//ASR.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "ASR%-5sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 5: case 13: case 21:
						//LSR.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "LSR%-5sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 6: case 14: case 22:
						//ROXR.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "ROXR%-4sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 7: case 15: case 23:
						//ROR.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "ROR%-5sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 32: case 40: case 48:
						//ASL.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "ASL%-5s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 33: case 41: case 49:
						//LSL.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "LSL%-5s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 34: case 42: case 50:
						//ROXL.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "ROXL%-4s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 35: case 43: case 51:
						//ROL.z  #k,Dd
						sprintf_s(Dbg_Str, 32, "ROL%-5s#%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 36: case 44: case 52:
						//ASL.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "ASL%-5sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 37: case 45: case 53:
						//LSL.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "LSL%-5sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 38: case 46: case 54:
						//ROXL.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "ROXL%-4sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

					case 39: case 47: case 55:
						//ROL.z  Ds,Dd
						sprintf_s(Dbg_Str, 32, "ROL%-5sD%.1d,D%.1d%c", Make_Dbg_Size_Str((OPC >> 6) & 0x3), (OPC >> 9) & 0x7, OPC & 0x7, 0);
						break;

				}
			}
			break;

		case 15:
			//Bad Opcode
			sprintf_s(Dbg_Str, 32, "Bad Opcode%c", 0);
			break;
	}
	
	return(Dbg_Str);
}


