#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <io.h>

#include "save.h"
#include "cpu_68k.h"
#include "cpu_sh2.h"
#include "sh2.h"
#include "z80.h"
#include "cd_aspi.h"
#include "gens.h"
#include "G_main.h"
#include "G_ddraw.h"
#include "G_dsound.h"
#include "G_input.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32X.h"
#include "rom.h"
#include "mem_M68K.h"
#include "mem_S68K.h"
#include "mem_SH2.h"
#include "mem_Z80.h"
#include "ym2612.h"
#include "psg.h"
#include "pcm.h"
#include "pwm.h"
#include "scrshot.h"
#include "ggenie.h"
#include "io.h"
#include "misc.h"
#include "cd_sys.h"

// ##RA
#include "../../../RA_Integration/RA_Interface.h"

int Current_State = 0;
char State_Dir[1024] = "";
char SRAM_Dir[1024] = "";
char BRAM_Dir[1024] = "";
unsigned char State_Buffer[MAX_STATE_FILE_LENGHT];

// ##RW
int numMemstates = 0;
int memstateSize = 0;
int memstateAllocated = 0;
int maxMemTakenByMemstates = 128 * 1024 * 1024; //##RW001 128MB Memory taken by Memstates (doubled the buffer size)

struct Memstate *ptr_Memstates, *prev_Memstate, *next_Memstate; 

char cTemp[255];

void allocate_Memstates(int datasize)
{
	int maxnumstates = 100;

	maxnumstates = maxMemTakenByMemstates / datasize;

	if(memstateAllocated == 1)
      return;

	struct Memstate *created_state, *first_state;
	int i;
	memstateSize = GENESIS_STATE_FILE_LENGHT_EX; // 152912byte
	numMemstates = maxnumstates;
	if(numMemstates <= 0)
	{
		memstateAllocated = 0;
		return;
	}
	else
	{
		memstateAllocated = 1;
	}

	// reserves first State
	created_state = (struct Memstate *) malloc(sizeof(Memstate));
	created_state->hasData = false;
	created_state->buf = (BYTE *) malloc(datasize);
	first_state = created_state;
	prev_Memstate = first_state;

	for(i=1; i < numMemstates; i++)
	{
		created_state = (struct Memstate *) malloc(sizeof(Memstate));
		created_state->hasData = false;
		created_state->buf = (BYTE *) malloc(datasize);
		created_state->prev = prev_Memstate;
		prev_Memstate->next = created_state;
		prev_Memstate = created_state;
	}

	// make list circular
	created_state->next = first_state;
	first_state->prev = created_state;
	ptr_Memstates = first_state;
}

void free_Memstates()
{
	if(memstateAllocated == 0)
		return;

	struct Memstate *now_state;

	now_state = ptr_Memstates;
	prev_Memstate = now_state->prev;
	prev_Memstate->next = NULL;

	while(true)
	{
		if(now_state == NULL)
			break;
		next_Memstate = now_state->next;
		free(now_state->buf);
		free(now_state);
		now_state = next_Memstate;
	}

	memstateAllocated = 0;
}

void save_Memstate()
{
	// ##RW001
	if( RA_HardcoreModeIsActive() )
	{
		return;
	}
	if(memstateAllocated == 0)
		return;
	// hier ptr_Memstates->buf saven!
	Save_Memstate(ptr_Memstates->buf);
	ptr_Memstates->hasData = true;
	ptr_Memstates = ptr_Memstates->next;
}

int load_Memstate()
{
	// #RA
	if( RA_HardcoreModeIsActive() )
	{
		MessageBox( HWnd, "Please disable Hardcore mode when using REWiND!", "Warning", MB_OK );
		return 0;
	}

	if(memstateAllocated == 0)
		return 0;

	int ret_val = 0;
	prev_Memstate = ptr_Memstates->prev;

	if(prev_Memstate->hasData)
	{
		// hier aus prev_Memstate->buf lesen
		Load_Memstate(prev_Memstate->buf);
		prev_Memstate->hasData = false;
		ptr_Memstates = ptr_Memstates->prev;
		ret_val = 1;
	}

	return ret_val;
}

// ##RW - End



int Change_File_S(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext)
{
	OPENFILENAME ofn;

	SetCurrentDirectory(Gens_Path);

	if (!strcmp(Dest, ""))
	{
		strcpy(Dest, "default.");
		strcat(Dest, Ext);
	}

	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = HWnd;
	ofn.hInstance = ghInstance;
	ofn.lpstrFile = Dest;
	ofn.nMaxFile = 2047;
	ofn.lpstrFilter = Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = Dir;
	ofn.lpstrTitle = Titre;
	ofn.lpstrDefExt = Ext;
	ofn.Flags = 0;

	if (GetSaveFileName(&ofn)) return 1;

	return 0;
}

int Change_File_L(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext)
{
	OPENFILENAME ofn;

	SetCurrentDirectory(Gens_Path);

	if (!strcmp(Dest, ""))
	{
		strcpy(Dest, "default.");
		strcat(Dest, Ext);
	}

	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = HWnd;
	ofn.hInstance = ghInstance;
	ofn.lpstrFile = Dest;
	ofn.nMaxFile = 2047;
	ofn.lpstrFilter = Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = Dir;
	ofn.lpstrTitle = Titre;
	ofn.lpstrDefExt = Ext;
	ofn.Flags = 0;

	if (GetOpenFileName(&ofn)) return 1;

	return 0;
}


int Change_Dir(char *Dest, char *Dir, char *Titre, char *Filter, char *Ext)
{
	OPENFILENAME ofn;
	int i;

	SetCurrentDirectory(Gens_Path);

	strcpy(Dest, "default.");
	strcat(Dest, Ext);
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = HWnd;
	ofn.hInstance = ghInstance;
	ofn.lpstrFile = Dest;
	ofn.nMaxFile = 2047;
	ofn.lpstrFilter = Filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = Dir;
	ofn.lpstrTitle = Titre;
	ofn.lpstrDefExt = Ext;
	ofn.Flags = 0;

	if (GetSaveFileName(&ofn))
	{
		i = strlen(Dest) - 1;
		while ((i > 0) && (Dest[i] != '\\')) i--;
		if (!i) return 0;
		Dest[++i] = 0;
		return 1;
	}

	return 0;
}


FILE *Get_State_File()
{
	char Name[2048];
	char Ext[5] = ".gsX";

	SetCurrentDirectory(Gens_Path);

	Ext[3] = '0' + Current_State;
	strcpy_s(Name, 2048, State_Dir);
	strcat_s(Name, 2048, Rom_Name);
	strcat_s(Name, 2048, Ext);

	return fopen(Name, "rb");
}


void Get_State_File_Name(char *name)
{
	char Ext[5] = ".gsX";

	SetCurrentDirectory(Gens_Path);

	Ext[3] = '0' + Current_State;
	strcpy(name, State_Dir);
	strcat(name, Rom_Name);
	strcat(name, Ext);
}



int Load_State(char *Name)
{
	// #RA
	if( RA_HardcoreModeIsActive() )
	{
		if( MessageBox( HWnd, "Hardcore mode is active. If you load a state, you will disable Hardcore Mode. Continue?", "Warning", MB_YESNO ) == IDNO )
			return 0;
	}

	FILE *f;
	unsigned char *buf;
	int len;

	if (Genesis_Started) len = GENESIS_STATE_FILE_LENGHT_EX;
	else if (SegaCD_Started) len = SEGACD_STATE_FILE_LENGHT;
	else if (_32X_Started) len = G32X_STATE_FILE_LENGHT;
	else return 0;

	buf = State_Buffer;

	if ((f = fopen(Name, "rb")) == NULL) return 0;

	memset(buf, 0, len);
	if (fread(buf, 1, len, f))
	{
		z80_Reset(&M_Z80);
//		main68k_reset();
//		YM2612ResetChip(0);
//		Reset_VDP();

		if (SegaCD_Started)
		{



		}

		if (_32X_Started)
		{


		}

		Import_Genesis(buf);
		buf += 0x22500;
		if (SegaCD_Started)
		{
//			Import_SegaCD(buf);
			buf += 0xE0000;
		}
		if (_32X_Started)
		{
			Import_32X(buf);
			buf += 0x82A00;
		}

		Flag_Clr_Scr = 1;
		CRam_Flag = 1;
		VRam_Flag = 1;

		// #RA
		// Attempt to restore state of core achievement set
		RA_OnLoadState( Name );

		sprintf(Str_Tmp, "STATE %d LOADED", Current_State);
		Put_Info(Str_Tmp, 2000);
	}

	fclose(f);

	return 1;
}

//pReader is typedef unsigned char (_RAMByteReadFn)( unsigned int nOffset );
//pWriter is typedef void (_RAMByteWriteFn)( unsigned int nOffs, unsigned int nVal );

extern unsigned char RAMByteReader( unsigned int nOffs );
extern void RAMByteWriter( unsigned int nOffs, unsigned int nVal );
extern unsigned char RAMByteReaderSegaCD( unsigned int nOffs );
extern void RAMByteWriterSegaCD( unsigned int nOffs, unsigned int nVal );

int Load_Memstate(BYTE *memBuf)
{
	BYTE *buf;
	int len;

	//if     (SMS_Started)     len = SMS_STATE_FILE_LENGHT;
	if(Genesis_Started) len = GENESIS_STATE_FILE_LENGHT_EX;
	else if(SegaCD_Started)  len = SEGACD_STATE_FILE_LENGHT;
	else if(_32X_Started)    len = G32X_STATE_FILE_LENGHT;
	else return 0;

	buf = memBuf;

	//if(SMS_Started)
	//{
	//	Import_SMS(memBuf);
	//}
	//else
	//{
		z80_Reset(&M_Z80);
//    main68k_reset();
//    YM2612ResetChip(0);
//    Reset_VDP();

		Import_Genesis(memBuf);

		buf += 0x22500;
		if(SegaCD_Started)
		{
			//Import_SegaCD(buf);
			buf += 0xE0000;
		}
		if(_32X_Started)
		{
			Import_32X(buf);
			buf += 0x82A00;
		}

		CRam_Flag = 1;
		VRam_Flag = 1;

		// #RA
		// Attempt to restore state of core achievement set and set hardcore mode = false
		// RA_OnLoadState( Name );

		if( RA_HardcoreModeIsActive() )
		{
			if( SegaCD_Started )
			{
				RA_InstallMemoryBank( 0, &RAMByteReaderSegaCD, &RAMByteWriterSegaCD, 512 * 1024 );
				RA_OnLoadNewRom( CD_Data, 512 );
			}
			else
			{
				RA_InstallMemoryBank( 0, &RAMByteReader, &RAMByteWriter, 64 * 1024 );
				RA_OnLoadNewRom( Rom_Data, 6*1024*1024 );
			}
		}

  //}

  return 1;
}

int Save_State(char *Name)
{
	// #RA
	if( RA_HardcoreModeIsActive() )
	{
		if( MessageBox( HWnd, "Hardcore mode is active. If you save a state, you will disable Hardcore Mode. Continue?", "Warning", MB_YESNO ) == IDNO )
			return 0;
	}

	FILE *f;
	unsigned char *buf;
	int len;

	buf = State_Buffer;

	if ((f = fopen(Name, "wb")) == NULL) return 0;

	if (Genesis_Started) len = GENESIS_STATE_FILE_LENGHT;
	else if (SegaCD_Started) len = SEGACD_STATE_FILE_LENGHT;
	else if (_32X_Started) len = G32X_STATE_FILE_LENGHT;

	memset(buf, 0, len);
	Export_Genesis(buf);
	buf += 0x22500;
	if (SegaCD_Started)
	{
//		Export_SegaCD(buf);
		buf += 0xE0000;
	}
	if (_32X_Started)
	{
		Export_32X(buf);
		buf += 0x82A00;
	}
	
	fwrite(State_Buffer, 1, len, f);
	fclose(f);

	// #RA	Attempt to save state of core achievement set
	RA_OnSaveState( Name );

	sprintf(Str_Tmp, "STATE %d SAVED", Current_State);
	Put_Info(Str_Tmp, 2000);

	return 1;
}

int Save_Memstate(BYTE *memBuf)
{
	BYTE  *buf;
  int     len;

  buf = memBuf;

  /*
  if(Compress_Save_States)
  { if((gz = gzopen(Name, "wb5")) == NULL) return 0; }
  else
  {
    f = CreateFile(Name, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_ARCHIVE, NULL);
    if(f == INVALID_HANDLE_VALUE) return 0;
  }
  */

  //if     (SMS_Started)     len = SMS_STATE_FILE_LENGHT;
  if(Genesis_Started) len = GENESIS_STATE_FILE_LENGHT;
  else if(SegaCD_Started)  len = SEGACD_STATE_FILE_LENGHT;
  else if(_32X_Started)    len = G32X_STATE_FILE_LENGHT;

  memset(memBuf, 0, len);

  //if(SMS_Started)
  //{
  //  Export_SMS(memBuf);
  //}
  //else
  //{
    Export_Genesis(memBuf);
		buf += 0x22500;
    if(SegaCD_Started)
    {
      //Export_SegaCD(buf);
      buf += 0xE0000;
    }
    if(_32X_Started)
    {
      Export_32X(buf);
      buf += 0x82A00;
    }
  //}

  /*
  if(Compress_Save_States)
  {
    gzwrite(gz, State_Buffer, len);
    gzclose(gz);
    if(Use_State_MiniShot)
    {
      f = CreateFile(Name, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
      if(f != INVALID_HANDLE_VALUE)
      { SetFilePointer(f, 0, NULL, FILE_END);
        Create_MiniShot();
        WriteFile(f, MiniShot_Data, sizeof(MiniShot_Data), &d, NULL);
        CloseHandle(f); }
    }
  }
  else
  {
    WriteFile(f, State_Buffer, len, &d, NULL);
    if(Use_State_MiniShot)
    { Create_MiniShot();
      WriteFile(f, MiniShot_Data, sizeof(MiniShot_Data), &d, NULL); }
    CloseHandle(f);
  }

  sprintf(Str_Tmp, "STATE %d SAVED", Current_State);
  Put_Info(Str_Tmp, 3000);
  Erase_MiniShot();
  */

  return 1;
}

/*

GST genecyst save file

Range        Size   Description
-----------  -----  -----------
00000-00002  3      "GST"
00006-00007  2      "\xE0\x40"
000FA-00112  24     VDP registers
00112-00191  128    Color RAM
00192-001E1  80     Vertical scroll RAM
001E4-003E3  512    YM2612 registers
00474-02473  8192   Z80 RAM
02478-12477  65536  68K RAM
12478-22477  65536  Video RAM

main 68000 registers
--------------------

00080-0009F : D0-D7
000A0-000BF : A0-A7
000C8 : PC
000D0 : SR
000D2 : USP
000D6 : SSP

Z80 registers
-------------

00404 : AF
00408 : BC
0040C : DE
00410 : HL
00414 : IX
00418 : IY
0041C : PC
00420 : SP
00424 : AF'
00428 : BC'
0042C : DE'
00430 : HL'

00434 : I
00435 : Unknow
00436 : IFF1 = IFF2
00437 : Unknow

The 'R' register is not supported.

Z80 State
---------

00438 : Z80 RESET
00439 : Z80 BUSREQ
0043A : Unknow
0043B : Unknow

0043C : Z80 BANK (DWORD)

Gens and Kega ADD
-----------------

00040 : last VDP Control data written (DWORD)
00044 : second write flag (1 for second write) 
00045 : DMA Fill flag (1 mean next data write will cause a DMA fill)
00048 : VDP write address (DWORD)

00050 : Version       (Genecyst=0 ; Kega=5 ; Gens=5)
00051 : Emulator ID   (Genecyst=0 ; Kega=0 ; Gens=1)
00052 : System ID     (Genesis=0 ; SegaCD=1 ; 32X=2 ; SegaCD32X=3)

00060-00070 : PSG registers (WORD).


SEGA CD
-------

+00000-00FFF : Gate array & sub 68K
+01000-80FFF : Prg RAM
+81000-C0FFF : Word RAM (2M mode arrangement)
+C1000-D0FFF : PCM RAM
+D1000-DFFFF : CDD & CDC data (16 kB cache include)

32X
---

main SH2
--------

+00000-00FFF : cache
+01000-011FF : IO registers
+01200-0123F : R0-R15
+01240 : SR
+01244 : GBR
+01248 : VBR
+0124C : MACL
+01250 : MACH
+01254 : PR
+01258 : PC
+0125C : State

sub SH2
-------

+01400-023FF : cache
+02400-025FF : IO registers
+02600-0263F : R0-R15
+02640 : SR
+02644 : GBR
+02648 : VBR
+0264C : MACL
+02650 : MACH
+02654 : PR
+02658 : PC
+0265C : State

others
------
// Fix 32X save state :
// enregistrer correctement les registres systèmes ...

+02700 : ADEN bit (bit 0)
+02701 : FM bit (bit 7)
+02702 : Master SH2 INT mask register
+02703 : Slave SH2 INT mask register
+02704 : 68000 32X rom bank register
+02705 : RV (Rom to VRAM DMA allowed) bit (bit 0)
+02710-0273F : FIFO stuff (not yet done)
+02740-0274F : 32X communication buffer
+02750-02759 : PWM registers
+02760-0276F : 32X VDP registers
+02800-029FF : 32X palette
+02A00-429FF : SDRAM
+42A00-829FF : FB1 & FB2

*/

void Import_Genesis(unsigned char *Data)
{
	unsigned char Reg_1[0x200], Version, *src;
	int i;

	VDP_Int = 0;
	DMAT_Lenght = 0;
	Version = Data[0x50];

	for(i = 0; i < 0x80; i++) CRam[i] = Data[i + 0x112];
	for(i = 0; i < 0x50; i++) VSRam[i] = Data[i + 0x192];
	for(i = 0; i < 0x2000; i++) Ram_Z80[i] = Data[i + 0x474];
	
	for(i = 0; i < 0x10000; i += 2)
	{
		Ram_68k[i + 0] = Data[i + 0x2478 + 1];
		Ram_68k[i + 1] = Data[i + 0x2478 + 0];
	}

	for(i = 0; i < 0x10000; i += 2)
	{
		VRam[i + 0] = Data[i + 0x12478 + 1];
		VRam[i + 1] = Data[i + 0x12478 + 0];
	}

	for(i = 0; i < 0x200; i++) Reg_1[i] = Data[i + 0x1E4];
	YM2612_Restore(Reg_1);

	if ((Version >= 2) && (Version < 4))
	{
		src = (unsigned char *) &Ctrl;
		for(i = 0; i < 7 * 4; i++) *src++ = Data[i + 0x30];

		Z80_State &= ~6;
		if (Data[0x440] & 1) Z80_State |= 2;
		if (Data[0x444] & 1) Z80_State |= 4;

		src = (unsigned char *) &Bank_Z80;
		for(i = 0; i < 4; i++) *src++ = Data[i + 0x448];

		src = (unsigned char *) &PSG_Save;
		for(i = 0; i < 8 * 4; i++) *src++ = Data[i + 0x224B8];
		PSG_Restore_State();
	}
	else if ((Version >= 4) || (Version == 0)) 		// New version compatible with Kega.
	{
		Z80_State &= ~6;

		if (Version == 4)
		{
			M_Z80.IM = Data[0x437];
			M_Z80.IFF.b.IFF1 = (Data[0x438] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (Data[0x438] & 1) << 2;

			Z80_State |= (Data[0x439] & 1) << 1;
		}
		else
		{
			M_Z80.IM = 1;
			M_Z80.IFF.b.IFF1 = (Data[0x436] & 1) << 2;
			M_Z80.IFF.b.IFF2 = (Data[0x436] & 1) << 2;

			Z80_State |= ((Data[0x439] & 1) ^ 1) << 1;
			Z80_State |= ((Data[0x438] & 1) ^ 1) << 2;
		}

		src = (unsigned char *) &Ctrl;
		for(i = 0; i < 7 * 4; i++) *src++ = 0;

		Write_VDP_Ctrl(Data[0x40] + (Data[0x41] << 8));
		Write_VDP_Ctrl(Data[0x42] + (Data[0x43] << 8));

		Ctrl.Flag = Data[0x44];
		Ctrl.DMA = (Data[0x45] & 1) << 2;
		Ctrl.Address = Data[0x48] + (Data[0x49] << 8);
		
		src = (unsigned char *) &Bank_Z80;
		for(i = 0; i < 4; i++) *src++ = Data[i + 0x43C];

		if (Version >= 4)
		{
			for(i = 0; i < 8; i++) PSG_Save[i] = Data[i * 2 + 0x60] + (Data[i * 2 + 0x61] << 8);
			PSG_Restore_State();
		}
	}

	z80_Set_AF(&M_Z80, Data[0x404] + (Data[0x405] << 8));
	M_Z80.BC.w.BC = Data[0x408] + (Data[0x409] << 8);
	M_Z80.DE.w.DE = Data[0x40C] + (Data[0x40D] << 8);
	M_Z80.HL.w.HL = Data[0x410] + (Data[0x411] << 8);
	M_Z80.IX.w.IX = Data[0x414] + (Data[0x415] << 8);
	M_Z80.IY.w.IY = Data[0x418] + (Data[0x419] << 8);
	z80_Set_PC(&M_Z80, Data[0x41C] + (Data[0x41D] << 8));
	M_Z80.SP.w.SP = Data[0x420] + (Data[0x421] << 8);
	z80_Set_AF2(&M_Z80, Data[0x424] + (Data[0x425] << 8));
	M_Z80.BC2.w.BC2 = Data[0x428] + (Data[0x429] << 8);
	M_Z80.DE2.w.DE2 = Data[0x42C] + (Data[0x42D] << 8);
	M_Z80.HL2.w.HL2 = Data[0x430] + (Data[0x431] << 8);
	M_Z80.I = Data[0x434] & 0xFF;

	main68k_GetContext(&Context_68K);

	for(i = 0; i < 24; i++) Set_VDP_Reg(i, Data[0xFA + i]);
	
	src = (unsigned char *) &Context_68K.dreg[0];
	for(i = 0; i < 8 * 2 * 4; i++) *src++ = Data[0x80 + i];

	src = (unsigned char *) &Context_68K.pc;
	for(i = 0; i < 4; i++) *src++ = Data[0xC8 + i];

	src = (unsigned char *) &Context_68K.sr;
	for(i = 0; i < 2; i++) *src++ = Data[0xD0 + i];

	if ((Version >= 3) || (Version == 0))
	{
		if (Data[0xD1] & 0x20)
		{
			// Supervisor
			src = (unsigned char *) &Context_68K.asp;
			for(i = 0; i < 2; i++) *src++ = Data[0xD2 + i];
		}
		else
		{
			// User
			src = (unsigned char *) &Context_68K.asp;
			for(i = 0; i < 2; i++) *src++ = Data[0xD6 + i];
		}
	}

	main68k_SetContext(&Context_68K);
}


void Export_Genesis(unsigned char *Data)
{
	unsigned char Reg_1[0x200], *src;
	int i;

	while (DMAT_Lenght) Update_DMA();		// Be sure to finish DMA before save

	Data[0x00] = 'G';
	Data[0x01] = 'S';
	Data[0x02] = 'T';
	Data[0x03] = 0x40;
	Data[0x04] = 0xE0;

	Data[0x50] = 5;		// Version
	Data[0x51] = 0;		// Gens

	PSG_Save_State();

	for(i = 0; i < 8; i++)
	{
		Data[0x60 + i * 2] = PSG_Save[i] & 0xFF;
		Data[0x61 + i * 2] = (PSG_Save[i] >> 8) & 0xFF;
	}

	main68k_GetContext(&Context_68K);

	src = (unsigned char *) &(Context_68K.dreg[0]);
	for(i = 0; i < 8 * 2 * 4; i++) Data[0x80 + i] = *src++;

	src = (unsigned char *) &(Context_68K.pc);
	for(i = 0; i < 4; i++) Data[0xC8 + i] = *src++;

	src = (unsigned char *) &(Context_68K.sr);
	for(i = 0; i < 2; i++) Data[0xD0 + i] = *src++;

	if (Context_68K.sr & 0x2000)
	{
		src = (unsigned char *) &(Context_68K.asp);
		for(i = 0; i < 4; i++) Data[0xD2 + i] = *src++;
		src = (unsigned char *) &(Context_68K.areg[7]);
		for(i = 0; i < 4; i++) Data[0xD6 + i] = *src++;
	}
	else
	{
		src = (unsigned char *) &(Context_68K.asp);
		for(i = 0; i < 4; i++) Data[0xD6 + i] = *src++;
		src = (unsigned char *) &(Context_68K.areg[7]);
		for(i = 0; i < 4; i++) Data[0xD2 + i] = *src++;
	}

	src = (unsigned char *) &(Ctrl.Data);
	for(i = 0; i < 4; i++) Data[0x40 + i] = *src++;

	Data[0x44] = Ctrl.Flag;
	Data[0x45] = (Ctrl.DMA >> 2) & 1;

	Data[0x48] = Ctrl.Address & 0xFF;
	Data[0x49] = (Ctrl.Address >> 8) & 0xFF;

	VDP_Reg.DMA_Lenght_L = VDP_Reg.DMA_Lenght & 0xFF;
	VDP_Reg.DMA_Lenght_H = (VDP_Reg.DMA_Lenght >> 8) & 0xFF;

	VDP_Reg.DMA_Src_Adr_L = VDP_Reg.DMA_Address & 0xFF;
	VDP_Reg.DMA_Src_Adr_M = (VDP_Reg.DMA_Address >> 8) & 0xFF;
	VDP_Reg.DMA_Src_Adr_H = (VDP_Reg.DMA_Address >> 16) & 0xFF;

	VDP_Reg.DMA_Src_Adr_H |= Ctrl.DMA_Mode & 0xC0;

	src = (unsigned char *) &(VDP_Reg.Set1);
	for(i = 0; i < 24; i++)
	{
		Data[0xFA + i] = *src;
		src += 4;
	}

	for(i = 0; i < 0x80; i++) Data[i + 0x112] = CRam[i];
	for(i = 0; i < 0x50; i++) Data[i + 0x192] = VSRam[i];

	YM2612_Save(Reg_1);
	for(i = 0; i < 0x200; i++) Data[i + 0x1E4] = Reg_1[i];

	Data[0x404] = (unsigned char) (z80_Get_AF(&M_Z80) & 0xFF);
	Data[0x405] = (unsigned char) (z80_Get_AF(&M_Z80) >> 8);
	Data[0x408] = (unsigned char) (M_Z80.BC.w.BC & 0xFF);
	Data[0x409] = (unsigned char) (M_Z80.BC.w.BC >> 8);
	Data[0x40C] = (unsigned char) (M_Z80.DE.w.DE & 0xFF);
	Data[0x40D] = (unsigned char) (M_Z80.DE.w.DE >> 8);
	Data[0x410] = (unsigned char) (M_Z80.HL.w.HL & 0xFF);
	Data[0x411] = (unsigned char) (M_Z80.HL.w.HL >> 8);
	Data[0x414] = (unsigned char) (M_Z80.IX.w.IX & 0xFF);
	Data[0x415] = (unsigned char) (M_Z80.IX.w.IX >> 8);
	Data[0x418] = (unsigned char) (M_Z80.IY.w.IY & 0xFF);
	Data[0x419] = (unsigned char) (M_Z80.IY.w.IY >> 8);
	Data[0x41C] = (unsigned char) (z80_Get_PC(&M_Z80) & 0xFF);
	Data[0x41D] = (unsigned char) (z80_Get_PC(&M_Z80) >> 8);
	Data[0x420] = (unsigned char) (M_Z80.SP.w.SP & 0xFF);
	Data[0x421] = (unsigned char) (M_Z80.SP.w.SP >> 8);
	Data[0x424] = (unsigned char) (z80_Get_AF2(&M_Z80) & 0xFF);
	Data[0x425] = (unsigned char) (z80_Get_AF2(&M_Z80) >> 8);
	Data[0x428] = (unsigned char) (M_Z80.BC2.w.BC2 & 0xFF);
	Data[0x429] = (unsigned char) (M_Z80.BC2.w.BC2 >> 8);
	Data[0x42C] = (unsigned char) (M_Z80.DE2.w.DE2 & 0xFF);
	Data[0x42D] = (unsigned char) (M_Z80.DE2.w.DE2 >> 8);
	Data[0x430] = (unsigned char) (M_Z80.HL2.w.HL2 & 0xFF);
	Data[0x431] = (unsigned char) (M_Z80.HL2.w.HL2 >> 8);
	Data[0x434] = (unsigned char) (M_Z80.I);
	Data[0x436] = (unsigned char) (M_Z80.IFF.b.IFF1 >> 2);

	Data[0x438] = (unsigned char) (((Z80_State & 4) >> 2) ^ 1);
	Data[0x439] = (unsigned char) (((Z80_State & 2) >> 1) ^ 1);

	src = (unsigned char *) &Bank_Z80;
	for(i = 0; i < 4; i++) Data[0x43C + i] = *src++;

	for(i = 0; i < 0x2000; i++) Data[i + 0x474] = Ram_Z80[i];

	for(i = 0; i < 0x10000; i += 2)
	{
		Data[i + 0x2478 + 1] = Ram_68k[i + 0];
		Data[i + 0x2478 + 0] = Ram_68k[i + 1];
	}

	for(i = 0; i < 0x10000; i += 2)
	{
		Data[i + 0x12478 + 1] = VRam[i + 0];
		Data[i + 0x12478 + 0] = VRam[i + 1];
	}
}


void Import_32X(unsigned char *Data)
{
	unsigned char *src;
	int i;

	for(i = 0; i < 0x1000; i++) M_SH2.Cache[i] = Data[i];
	for(i = 0; i < 0x200; i++) SH2_Write_Byte(&M_SH2, 0xFFFFFE00 + i, Data[0x1000 + i]);

	src = (unsigned char *) &(M_SH2.R[0]);
	for(i = 0; i < 16 * 4; i++) *src++ = Data[0x1200 + i];

	SH2_Set_SR(&M_SH2, (Data[0x1243] << 24) + (Data[0x1242] << 16) + (Data[0x1241] << 8) + Data[0x1240]);
	SH2_Set_GBR(&M_SH2, (Data[0x1247] << 24) + (Data[0x1246] << 16) + (Data[0x1245] << 8) + Data[0x1244]);
	SH2_Set_VBR(&M_SH2, (Data[0x124B] << 24) + (Data[0x124A] << 16) + (Data[0x1249] << 8) + Data[0x1248]);
	SH2_Set_MACL(&M_SH2, (Data[0x124F] << 24) + (Data[0x124E] << 16) + (Data[0x124D] << 8) + Data[0x124C]);
	SH2_Set_MACH(&M_SH2, (Data[0x1253] << 24) + (Data[0x1252] << 16) + (Data[0x1251] << 8) + Data[0x1250]);
	SH2_Set_PR(&M_SH2, (Data[0x1257] << 24) + (Data[0x1256] << 16) + (Data[0x1255] << 8) + Data[0x1254]);
	SH2_Set_PC(&M_SH2, (Data[0x125B] << 24) + (Data[0x125A] << 16) + (Data[0x1259] << 8) + Data[0x1258]);
	M_SH2.Status = (Data[0x125F] << 24) + (Data[0x125E] << 16) + (Data[0x125D] << 8) + Data[0x125C];

	for(i = 0; i < 0x1000; i++) S_SH2.Cache[i] = Data[i + 0x1400];
	for(i = 0; i < 0x200; i++) SH2_Write_Byte(&S_SH2, 0xFFFFFE00 + i, Data[0x2400 + i]);

	src = (unsigned char *) &(S_SH2.R[0]);
	for(i = 0; i < 16 * 4; i++) *src++ = Data[0x2600 + i];

	SH2_Set_SR(&S_SH2, (Data[0x2643] << 24) + (Data[0x2642] << 16) + (Data[0x2641] << 8) + Data[0x2640]);
	SH2_Set_GBR(&S_SH2, (Data[0x2647] << 24) + (Data[0x2646] << 16) + (Data[0x2645] << 8) + Data[0x2644]);
	SH2_Set_VBR(&S_SH2, (Data[0x264B] << 24) + (Data[0x264A] << 16) + (Data[0x2649] << 8) + Data[0x2648]);
	SH2_Set_MACL(&S_SH2, (Data[0x264F] << 24) + (Data[0x264E] << 16) + (Data[0x264D] << 8) + Data[0x264C]);
	SH2_Set_MACH(&S_SH2, (Data[0x2653] << 24) + (Data[0x2652] << 16) + (Data[0x2651] << 8) + Data[0x2650]);
	SH2_Set_PR(&S_SH2, (Data[0x2657] << 24) + (Data[0x2656] << 16) + (Data[0x2655] << 8) + Data[0x2654]);
	SH2_Set_PC(&S_SH2, (Data[0x265B] << 24) + (Data[0x265A] << 16) + (Data[0x2659] << 8) + Data[0x2658]);
	S_SH2.Status = (Data[0x265F] << 24) + (Data[0x265E] << 16) + (Data[0x265D] << 8) + Data[0x265C];

	_32X_ADEN = Data[0x2700] & 1;
	_32X_RV = Data[0x2705] & 1;
	M68K_32X_Mode();
	_32X_FM = Data[0x2701] & 0x80;
	_32X_Set_FB();
	_32X_MINT = Data[0x2702];
	_32X_SINT = Data[0x2703];
	Bank_SH2 = Data[0x2704];
	M68K_Set_32X_Rom_Bank();
	
	/*******
	FIFO stuff to add here...	
	 *******/
	for(i = 0; i < 0x10; i++) SH2_Write_Byte(&M_SH2, 0x4020 + i, Data[0x2740 + i]);
	for(i = 0; i < 4; i++) SH2_Write_Byte(&M_SH2, 0x4030 + i, Data[0x2750 + i]);
	/*******
	Extra PWM stuff to add here...	
	 *******/

	// Do it to allow VDP write on 32X side
	_32X_FM = 0x80;
	_32X_Set_FB();

	for(i = 0; i < 0x10; i++) SH2_Write_Byte(&M_SH2, 0x4100 + i, Data[0x2760 + i]);
	for(i = 0; i < 0x200; i += 2) SH2_Write_Word(&M_SH2, 0x4200 + i, Data[0x2800 + i + 0] + (Data[0x2800 + i + 1] << 8));
	for(i = 0; i < 0x40000; i++) SH2_Write_Byte(&M_SH2, 0x6000000 + i, Data[0x2A00 + i]);

	_32X_FM = Data[0x2701] & 0x80;
	_32X_Set_FB();

	if (SH2_Read_Word(&M_SH2, 0x410A) & 1)
	{
		for(i = 0; i < 0x20000; i++) _32X_VDP_Ram[i] = Data[0x42A00 + i];
		for(i = 0; i < 0x20000; i++) _32X_VDP_Ram[i + 0x20000] = Data[0x62A00 + i];
	}
	else
	{
		for(i = 0; i < 0x20000; i++) _32X_VDP_Ram[i + 0x20000] = Data[0x42A00 + i];
		for(i = 0; i < 0x20000; i++) _32X_VDP_Ram[i] = Data[0x62A00 + i];
	}
}


void Export_32X(unsigned char *Data)
{
	unsigned char *src;
	int i;

	for(i = 0; i < 0x1000; i++) Data[i] = M_SH2.Cache[i];
	for(i = 0; i < 0x200; i++) Data[0x1000 + i] = M_SH2.IO_Reg[i];

	src = (unsigned char *) &(M_SH2.R[0]);
	for(i = 0; i < 16 * 4; i++) Data[0x1200 + i] = *src++;

	i = SH2_Get_SR(&M_SH2);
	Data[0x1240] = (unsigned char) (i & 0xFF);
	Data[0x1241] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x1242] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x1243] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_GBR(&M_SH2);
	Data[0x1244] = (unsigned char) (i & 0xFF);
	Data[0x1245] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x1246] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x1247] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_VBR(&M_SH2);
	Data[0x1248] = (unsigned char) (i & 0xFF);
	Data[0x1249] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x124A] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x124B] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_MACL(&M_SH2);
	Data[0x124C] = (unsigned char) (i & 0xFF);
	Data[0x124D] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x124E] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x124F] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_MACH(&M_SH2);
	Data[0x1250] = (unsigned char) (i & 0xFF);
	Data[0x1251] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x1252] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x1253] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_PR(&M_SH2);
	Data[0x1254] = (unsigned char) (i & 0xFF);
	Data[0x1255] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x1256] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x1257] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_PC(&M_SH2);
	Data[0x1258] = (unsigned char) (i & 0xFF);
	Data[0x1259] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x125A] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x125B] = (unsigned char) ((i >> 24) & 0xFF);
	i = M_SH2.Status;
	Data[0x125C] = (unsigned char) (i & 0xFF);
	Data[0x125D] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x125E] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x125F] = (unsigned char) ((i >> 24) & 0xFF);

	for(i = 0; i < 0x1000; i++) Data[0x1400 + i] = S_SH2.Cache[i];
	for(i = 0; i < 0x200; i++) Data[0x2400 + i] = S_SH2.IO_Reg[i];

	src = (unsigned char *) &(S_SH2.R[0]);
	for(i = 0; i < 16 * 4; i++) Data[0x2600 + i] = *src++;

	i = SH2_Get_SR(&S_SH2);
	Data[0x2640] = (unsigned char) (i & 0xFF);
	Data[0x2641] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x2642] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x2643] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_GBR(&S_SH2);
	Data[0x2644] = (unsigned char) (i & 0xFF);
	Data[0x2645] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x2646] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x2647] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_VBR(&S_SH2);
	Data[0x2648] = (unsigned char) (i & 0xFF);
	Data[0x2649] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x264A] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x264B] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_MACL(&S_SH2);
	Data[0x264C] = (unsigned char) (i & 0xFF);
	Data[0x264D] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x264E] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x264F] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_MACH(&S_SH2);
	Data[0x2650] = (unsigned char) (i & 0xFF);
	Data[0x2651] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x2652] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x2653] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_PR(&S_SH2);
	Data[0x2654] = (unsigned char) (i & 0xFF);
	Data[0x2655] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x2656] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x2657] = (unsigned char) ((i >> 24) & 0xFF);
	i = SH2_Get_PC(&S_SH2);
	Data[0x2658] = (unsigned char) (i & 0xFF);
	Data[0x2659] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x265A] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x265B] = (unsigned char) ((i >> 24) & 0xFF);
	i = S_SH2.Status;
	Data[0x265C] = (unsigned char) (i & 0xFF);
	Data[0x265D] = (unsigned char) ((i >> 8) & 0xFF);
	Data[0x265E] = (unsigned char) ((i >> 16) & 0xFF);
	Data[0x265F] = (unsigned char) ((i >> 24) & 0xFF);

	// Do it to allow VDP write on 32X side
	Data[0x2700] = _32X_ADEN & 1;
	Data[0x2705] = _32X_RV & 1;
	Data[0x2701] = _32X_FM & 0x80;
	Data[0x2702] = _32X_MINT;
	Data[0x2703] = _32X_SINT;
	Data[0x2704] = Bank_SH2;
	
	/*******
	FIFO stuff to add here...	
	 *******/
	for(i = 0; i < 0x10; i++) Data[0x2740 + i] = SH2_Read_Byte(&M_SH2, 0x4020 + i);
	for(i = 0; i < 4; i++) Data[0x2750 + i] = SH2_Read_Byte(&M_SH2, 0x4030 + i);
	/*******
	Extra PWM stuff to add here...	
	 *******/

	// Do it to allow VDP write on 32X side
	_32X_FM = 0x80;
	_32X_Set_FB();

	for(i = 0; i < 0x10; i++) Data[0x2760 + i] = SH2_Read_Byte(&M_SH2, 0x4100 + i);
	for(i = 0; i < 0x200; i += 2)
	{
		Data[0x2800 + i + 0] = (unsigned char) (SH2_Read_Word(&M_SH2, 0x4200 + i) & 0xFF);
		Data[0x2800 + i + 1] = (unsigned char) (SH2_Read_Word(&M_SH2, 0x4200 + i) >> 8);
	}
	for(i = 0; i < 0x40000; i++) Data[0x2A00 + i] = SH2_Read_Byte(&M_SH2, 0x6000000 + i);

	_32X_FM = Data[0x2701] & 0x80;
	_32X_Set_FB();

	if (SH2_Read_Word(&M_SH2, 0x410A) & 1)
	{
		for(i = 0; i < 0x20000; i++) Data[0x42A00 + i] = _32X_VDP_Ram[i];
		for(i = 0; i < 0x20000; i++) Data[0x62A00 + i] = _32X_VDP_Ram[i + 0x20000];
	}
	else
	{
		for(i = 0; i < 0x20000; i++) Data[0x42A00 + i] = _32X_VDP_Ram[i + 0x20000];
		for(i = 0; i < 0x20000; i++) Data[0x62A00 + i] = _32X_VDP_Ram[i];
	}
}


int Save_Config(char *File_Name)
{
	char Conf_File[1024];

	strcpy(Conf_File, File_Name);

	WritePrivateProfileString("General", "Rom path", Rom_Dir, Conf_File);
	WritePrivateProfileString("General", "Save path", State_Dir, Conf_File);
	WritePrivateProfileString("General", "SRAM path", SRAM_Dir, Conf_File);
	WritePrivateProfileString("General", "BRAM path", BRAM_Dir, Conf_File);
	WritePrivateProfileString("General", "Dump path", Dump_Dir, Conf_File);
	WritePrivateProfileString("General", "Dump GYM path", Dump_GYM_Dir, Conf_File);
	WritePrivateProfileString("General", "Screen Shot path", ScrShot_Dir, Conf_File);
	WritePrivateProfileString("General", "Patch path", Patch_Dir, Conf_File);
	WritePrivateProfileString("General", "IPS Patch path", IPS_Dir, Conf_File);

	WritePrivateProfileString("General", "Genesis Bios", Genesis_Bios, Conf_File);

	WritePrivateProfileString("General", "USA CD Bios", US_CD_Bios, Conf_File);
	WritePrivateProfileString("General", "EUROPE CD Bios", EU_CD_Bios, Conf_File);
	WritePrivateProfileString("General", "JAPAN CD Bios", JA_CD_Bios, Conf_File);

	WritePrivateProfileString("General", "32X 68000 Bios", _32X_Genesis_Bios, Conf_File);
	WritePrivateProfileString("General", "32X Master SH2 Bios", _32X_Master_Bios, Conf_File);
	WritePrivateProfileString("General", "32X Slave SH2 Bios", _32X_Slave_Bios, Conf_File);

	WritePrivateProfileString("General", "Rom 1", Recent_Rom[0], Conf_File);
	WritePrivateProfileString("General", "Rom 2", Recent_Rom[1], Conf_File);
	WritePrivateProfileString("General", "Rom 3", Recent_Rom[2], Conf_File);
	WritePrivateProfileString("General", "Rom 4", Recent_Rom[3], Conf_File);
	WritePrivateProfileString("General", "Rom 5", Recent_Rom[4], Conf_File);
	WritePrivateProfileString("General", "Rom 6", Recent_Rom[5], Conf_File);
	WritePrivateProfileString("General", "Rom 7", Recent_Rom[6], Conf_File);
	WritePrivateProfileString("General", "Rom 8", Recent_Rom[7], Conf_File);
	WritePrivateProfileString("General", "Rom 9", Recent_Rom[8], Conf_File);

	wsprintf(Str_Tmp, "%d", File_Type_Index);
	WritePrivateProfileString("General", "File type index", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", Current_State);
	WritePrivateProfileString("General", "State Number", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Language);
	WritePrivateProfileString("General", "Language", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Window_Pos.x);
	WritePrivateProfileString("General", "Window X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Window_Pos.y);
	WritePrivateProfileString("General", "Window Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Effect_Color);
	WritePrivateProfileString("General", "Free Mode Color", Str_Tmp, Conf_File);

	//##RW001
	wsprintf(Str_Tmp, "%d", MemstateFrameSkip);
	WritePrivateProfileString("REWiND", "REWiND Frameskip", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", Full_Screen & 1);
	WritePrivateProfileString("Graphics", "Full Screen", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", FS_VSync & 1);
	WritePrivateProfileString("Graphics", "Full Screen VSync", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", W_VSync & 1);
	WritePrivateProfileString("Graphics", "Windows VSync", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Render_W);
	WritePrivateProfileString("Graphics", "Render Windowed", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Render_FS);
	WritePrivateProfileString("Graphics", "Render Fullscreen", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Stretch & 1);
	WritePrivateProfileString("Graphics", "Stretch", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Blit_Soft & 1);
	WritePrivateProfileString("Graphics", "Software Blit", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", Contrast_Level);
	WritePrivateProfileString("Graphics", "Contrast", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Brightness_Level);
	WritePrivateProfileString("Graphics", "Brightness", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Greyscale & 1);
	WritePrivateProfileString("Graphics", "Greyscale", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Invert_Color & 1);
	WritePrivateProfileString("Graphics", "Invert", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", Sprite_Over & 1);
	WritePrivateProfileString("Graphics", "Sprite limit", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Frame_Skip);
	WritePrivateProfileString("Graphics", "Frame skip", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", Sound_Enable & 1);
	WritePrivateProfileString("Sound", "State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Sound_Rate);
	WritePrivateProfileString("Sound", "Rate", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Sound_Stereo);
	WritePrivateProfileString("Sound", "Stereo", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Z80_State & 1);
	WritePrivateProfileString("Sound", "Z80 State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", YM2612_Enable & 1);
	WritePrivateProfileString("Sound", "YM2612 State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", PSG_Enable & 1);
	WritePrivateProfileString("Sound", "PSG State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", DAC_Enable & 1);
	WritePrivateProfileString("Sound", "DAC State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", PCM_Enable & 1);
	WritePrivateProfileString("Sound", "PCM State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", PWM_Enable & 1);
	WritePrivateProfileString("Sound", "PWM State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", CDDA_Enable & 1);
	WritePrivateProfileString("Sound", "CDDA State", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", YM2612_Improv & 1);
	WritePrivateProfileString("Sound", "YM2612 Improvement", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", DAC_Improv & 1);
	WritePrivateProfileString("Sound", "DAC Improvement", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", PSG_Improv & 1);
	WritePrivateProfileString("Sound", "PSG Improvement", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", Country);
	WritePrivateProfileString("CPU", "Country", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Country_Order[0]);
	WritePrivateProfileString("CPU", "Prefered Country 1", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Country_Order[1]);
	WritePrivateProfileString("CPU", "Prefered Country 2", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Country_Order[2]);
	WritePrivateProfileString("CPU", "Prefered Country 3", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", SegaCD_Accurate);
	WritePrivateProfileString("CPU", "Perfect synchro between main and sub CPU (Sega CD)", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", MSH2_Speed);
	WritePrivateProfileString("CPU", "Main SH2 Speed", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", SSH2_Speed);
	WritePrivateProfileString("CPU", "Slave SH2 Speed", Str_Tmp, Conf_File);

	wsprintf(Str_Tmp, "%d", Fast_Blur & 1);
	WritePrivateProfileString("Options", "Fast Blur", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Show_FPS & 1);
	WritePrivateProfileString("Options", "FPS", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", FPS_Style);
	WritePrivateProfileString("Options", "FPS Style", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Show_Message & 1);
	WritePrivateProfileString("Options", "Message", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Message_Style);
	WritePrivateProfileString("Options", "Message Style", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Show_LED & 1);
	WritePrivateProfileString("Options", "LED", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Auto_Fix_CS & 1);
	WritePrivateProfileString("Options", "Auto Fix Checksum", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Auto_Pause & 1);
	WritePrivateProfileString("Options", "Auto Pause", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", CUR_DEV);
	WritePrivateProfileString("Options", "CD Drive", Str_Tmp, Conf_File);

	if (BRAM_Ex_State & 0x100)
	{
		wsprintf(Str_Tmp, "%d", BRAM_Ex_Size);
		WritePrivateProfileString("Options", "Ram Cart Size", Str_Tmp, Conf_File);
	}
	else
	{
		WritePrivateProfileString("Options", "Ram Cart Size", "-1", Conf_File);
	}

	WritePrivateProfileString("Options", "GCOffline path", CGOffline_Path, Conf_File);
	WritePrivateProfileString("Options", "Gens manual path", Manual_Path, Conf_File);

	wsprintf(Str_Tmp, "%d", Controller_1_Type & 0x13);
	WritePrivateProfileString("Input", "P1.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Up);
	WritePrivateProfileString("Input", "P1.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Down);
	WritePrivateProfileString("Input", "P1.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Left);
	WritePrivateProfileString("Input", "P1.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Right);
	WritePrivateProfileString("Input", "P1.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Start);
	WritePrivateProfileString("Input", "P1.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].A);
	WritePrivateProfileString("Input", "P1.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].B);
	WritePrivateProfileString("Input", "P1.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].C);
	WritePrivateProfileString("Input", "P1.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Mode);
	WritePrivateProfileString("Input", "P1.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].X);
	WritePrivateProfileString("Input", "P1.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Y);
	WritePrivateProfileString("Input", "P1.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Z);
	WritePrivateProfileString("Input", "P1.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].RA_Home);
	WritePrivateProfileString("Input", "P1.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[0].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P1.Rewind", Str_Tmp, Conf_File);	// ##RW

	wsprintf(Str_Tmp, "%d", Controller_1B_Type & 0x03);
	WritePrivateProfileString("Input", "P1B.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Up);
	WritePrivateProfileString("Input", "P1B.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Down);
	WritePrivateProfileString("Input", "P1B.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Left);
	WritePrivateProfileString("Input", "P1B.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Right);
	WritePrivateProfileString("Input", "P1B.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Start);
	WritePrivateProfileString("Input", "P1B.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].A);
	WritePrivateProfileString("Input", "P1B.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].B);
	WritePrivateProfileString("Input", "P1B.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].C);
	WritePrivateProfileString("Input", "P1B.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Mode);
	WritePrivateProfileString("Input", "P1B.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].X);
	WritePrivateProfileString("Input", "P1B.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Y);
	WritePrivateProfileString("Input", "P1B.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Z);
	WritePrivateProfileString("Input", "P1B.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].RA_Home);
	WritePrivateProfileString("Input", "P1B.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[2].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P1B.Rewind", Str_Tmp, Conf_File);	// ##RW

	wsprintf(Str_Tmp, "%d", Controller_1C_Type & 0x03);
	WritePrivateProfileString("Input", "P1C.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Up);
	WritePrivateProfileString("Input", "P1C.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Down);
	WritePrivateProfileString("Input", "P1C.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Left);
	WritePrivateProfileString("Input", "P1C.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Right);
	WritePrivateProfileString("Input", "P1C.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Start);
	WritePrivateProfileString("Input", "P1C.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].A);
	WritePrivateProfileString("Input", "P1C.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].B);
	WritePrivateProfileString("Input", "P1C.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].C);
	WritePrivateProfileString("Input", "P1C.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Mode);
	WritePrivateProfileString("Input", "P1C.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].X);
	WritePrivateProfileString("Input", "P1C.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Y);
	WritePrivateProfileString("Input", "P1C.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Z);
	WritePrivateProfileString("Input", "P1C.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].RA_Home);
	WritePrivateProfileString("Input", "P1C.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[3].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P1C.Rewind", Str_Tmp, Conf_File);	// ##RW

	wsprintf(Str_Tmp, "%d", Controller_1D_Type & 0x03);
	WritePrivateProfileString("Input", "P1D.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Up);
	WritePrivateProfileString("Input", "P1D.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Down);
	WritePrivateProfileString("Input", "P1D.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Left);
	WritePrivateProfileString("Input", "P1D.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Right);
	WritePrivateProfileString("Input", "P1D.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Start);
	WritePrivateProfileString("Input", "P1D.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].A);
	WritePrivateProfileString("Input", "P1D.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].B);
	WritePrivateProfileString("Input", "P1D.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].C);
	WritePrivateProfileString("Input", "P1D.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Mode);
	WritePrivateProfileString("Input", "P1D.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].X);
	WritePrivateProfileString("Input", "P1D.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Y);
	WritePrivateProfileString("Input", "P1D.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Z);
	WritePrivateProfileString("Input", "P1D.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].RA_Home);
	WritePrivateProfileString("Input", "P1D.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[4].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P1D.Rewind", Str_Tmp, Conf_File);	// ##RW

	wsprintf(Str_Tmp, "%d", Controller_2_Type & 0x13);
	WritePrivateProfileString("Input", "P2.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Up);
	WritePrivateProfileString("Input", "P2.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Down);
	WritePrivateProfileString("Input", "P2.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Left);
	WritePrivateProfileString("Input", "P2.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Right);
	WritePrivateProfileString("Input", "P2.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Start);
	WritePrivateProfileString("Input", "P2.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].A);
	WritePrivateProfileString("Input", "P2.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].B);
	WritePrivateProfileString("Input", "P2.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].C);
	WritePrivateProfileString("Input", "P2.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Mode);
	WritePrivateProfileString("Input", "P2.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].X);
	WritePrivateProfileString("Input", "P2.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Y);
	WritePrivateProfileString("Input", "P2.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Z);
	WritePrivateProfileString("Input", "P2.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].RA_Home);
	WritePrivateProfileString("Input", "P2.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[1].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P2.Rewind", Str_Tmp, Conf_File);	// ##RW

	wsprintf(Str_Tmp, "%d", Controller_2B_Type & 0x03);
	WritePrivateProfileString("Input", "P2B.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Up);
	WritePrivateProfileString("Input", "P2B.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Down);
	WritePrivateProfileString("Input", "P2B.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Left);
	WritePrivateProfileString("Input", "P2B.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Right);
	WritePrivateProfileString("Input", "P2B.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Start);
	WritePrivateProfileString("Input", "P2B.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].A);
	WritePrivateProfileString("Input", "P2B.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].B);
	WritePrivateProfileString("Input", "P2B.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].C);
	WritePrivateProfileString("Input", "P2B.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Mode);
	WritePrivateProfileString("Input", "P2B.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].X);
	WritePrivateProfileString("Input", "P2B.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Y);
	WritePrivateProfileString("Input", "P2B.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Z);
	WritePrivateProfileString("Input", "P2B.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].RA_Home);
	WritePrivateProfileString("Input", "P2B.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[5].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P2B.Rewind", Str_Tmp, Conf_File);	// ##RW

	wsprintf(Str_Tmp, "%d", Controller_2C_Type & 0x03);
	WritePrivateProfileString("Input", "P2C.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Up);
	WritePrivateProfileString("Input", "P2C.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Down);
	WritePrivateProfileString("Input", "P2C.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Left);
	WritePrivateProfileString("Input", "P2C.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Right);
	WritePrivateProfileString("Input", "P2C.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Start);
	WritePrivateProfileString("Input", "P2C.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].A);
	WritePrivateProfileString("Input", "P2C.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].B);
	WritePrivateProfileString("Input", "P2C.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].C);
	WritePrivateProfileString("Input", "P2C.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Mode);
	WritePrivateProfileString("Input", "P2C.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].X);
	WritePrivateProfileString("Input", "P2C.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Y);
	WritePrivateProfileString("Input", "P2C.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Z);
	WritePrivateProfileString("Input", "P2C.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].RA_Home);
	WritePrivateProfileString("Input", "P2C.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[6].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P2C.Rewind", Str_Tmp, Conf_File);	// ##RW

	wsprintf(Str_Tmp, "%d", Controller_2D_Type & 0x03);
	WritePrivateProfileString("Input", "P2D.Type", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Up);
	WritePrivateProfileString("Input", "P2D.Up", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Down);
	WritePrivateProfileString("Input", "P2D.Down", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Left);
	WritePrivateProfileString("Input", "P2D.Left", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Right);
	WritePrivateProfileString("Input", "P2D.Right", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Start);
	WritePrivateProfileString("Input", "P2D.Start", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].A);
	WritePrivateProfileString("Input", "P2D.A", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].B);
	WritePrivateProfileString("Input", "P2D.B", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].C);
	WritePrivateProfileString("Input", "P2D.C", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Mode);
	WritePrivateProfileString("Input", "P2D.Mode", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].X);
	WritePrivateProfileString("Input", "P2D.X", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Y);
	WritePrivateProfileString("Input", "P2D.Y", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Z);
	WritePrivateProfileString("Input", "P2D.Z", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].RA_Home);
	WritePrivateProfileString("Input", "P2D.Home", Str_Tmp, Conf_File);
	wsprintf(Str_Tmp, "%d", Keys_Def[7].Rewind);							// ##RW
	WritePrivateProfileString("Input", "P2D.Rewind", Str_Tmp, Conf_File);	// ##RW

	//	Always write user
	//WritePrivateProfileString("Achievements", "User", RAUsers::LocalUser.m_sUsername, Conf_File);
	//if( RAUsers::LocalUser.m_bStoreToken )
	//	WritePrivateProfileString("Achievements", "Token", RAUsers::LocalUser.m_sToken, Conf_File);
	//else
	//	WritePrivateProfileString("Achievements", "Token", "", Conf_File);

	//WritePrivateProfileString("Achievements", "RAKnownVersion", g_sKnownRAVersion, Conf_File );

	return 1;
}


int Save_As_Config(HWND hWnd)
{
	char Name[2048];
	OPENFILENAME ofn;

	SetCurrentDirectory(Gens_Path);

	strcpy(&Name[0], "Gens.cfg");
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = ghInstance;
	ofn.lpstrFile = Name;
	ofn.nMaxFile = 2047;
	ofn.lpstrFilter = "Config Files\0*.cfg\0All Files\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = Gens_Path;
	ofn.lpstrDefExt = "cfg";
	ofn.Flags = 0;

	if(GetSaveFileName(&ofn))
	{
		Save_Config(Name);
		strcpy(Str_Tmp, "config saved in ");
		strcat(Str_Tmp, Name);
		Put_Info(Str_Tmp, 2000);
		return 1;
	}
	else return 0;
}


int Load_Config(char *File_Name, void *Game_Active)
{
	int new_val;
	char Conf_File[1024];

	SetCurrentDirectory(Gens_Path);
	strcpy(Conf_File, File_Name);

	CRam_Flag = 1;

	GetPrivateProfileString("General", "Rom path", ".\\", &Rom_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "Save path", Rom_Dir, &State_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "SRAM path", Rom_Dir, &SRAM_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "BRAM path", Rom_Dir, &BRAM_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "Dump path", Rom_Dir, &Dump_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "Dump GYM path", Rom_Dir, &Dump_GYM_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "Screen Shot path", Rom_Dir, &ScrShot_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "Patch path", Rom_Dir, &Patch_Dir[0], 1024, Conf_File);
	GetPrivateProfileString("General", "IPS Patch path", Rom_Dir, &IPS_Dir[0], 1024, Conf_File);

	GetPrivateProfileString("General", "Genesis Bios", Rom_Dir, &Genesis_Bios[0], 1024, Conf_File);

	GetPrivateProfileString("General", "USA CD Bios", Rom_Dir, &US_CD_Bios[0], 1024, Conf_File);
	GetPrivateProfileString("General", "EUROPE CD Bios", Rom_Dir, &EU_CD_Bios[0], 1024, Conf_File);
	GetPrivateProfileString("General", "JAPAN CD Bios", Rom_Dir, &JA_CD_Bios[0], 1024, Conf_File);

	GetPrivateProfileString("General", "32X 68000 Bios", Rom_Dir, &_32X_Genesis_Bios[0], 1024, Conf_File);
	GetPrivateProfileString("General", "32X Master SH2 Bios", Rom_Dir, &_32X_Master_Bios[0], 1024, Conf_File);
	GetPrivateProfileString("General", "32X Slave SH2 Bios", Rom_Dir, &_32X_Slave_Bios[0], 1024, Conf_File);

	GetPrivateProfileString("General", "Rom 1", "", &Recent_Rom[0][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 2", "", &Recent_Rom[1][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 3", "", &Recent_Rom[2][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 4", "", &Recent_Rom[3][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 5", "", &Recent_Rom[4][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 6", "", &Recent_Rom[5][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 7", "", &Recent_Rom[6][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 8", "", &Recent_Rom[7][0], 1024, Conf_File);
	GetPrivateProfileString("General", "Rom 9", "", &Recent_Rom[8][0], 1024, Conf_File);

	File_Type_Index = GetPrivateProfileInt("General", "File type index", 1, Conf_File);

	Current_State = GetPrivateProfileInt("General", "State Number", 0, Conf_File);
	Language = GetPrivateProfileInt("General", "Language", 0, Conf_File);
	Window_Pos.x = GetPrivateProfileInt("General", "Window X", 0, Conf_File);
	Window_Pos.y = GetPrivateProfileInt("General", "Window Y", 0, Conf_File);
	Intro_Style = GetPrivateProfileInt("General", "Intro Style", 2, Conf_File);
	Effect_Color = GetPrivateProfileInt("General", "Free Mode Color", 7, Conf_File);
	Sleep_Time = GetPrivateProfileInt("General", "Allow Idle", 0, Conf_File) & 1;
	Gens_Priority = GetPrivateProfileInt("General", "Priority", 1, Conf_File);

	MemstateFrameSkip = GetPrivateProfileInt("REWiND", "REWiND Frameskip", 0, Conf_File);	//##RW001

	if (GetPrivateProfileInt("Graphics", "Force 555", 0, Conf_File)) Mode_555 = 3;
	else if (GetPrivateProfileInt("Graphics", "Force 565", 0, Conf_File)) Mode_555 = 2;
	else Mode_555 = 0;

	RMax_Level = GetPrivateProfileInt("Graphics", "Red Max", 255, Conf_File);
	GMax_Level = GetPrivateProfileInt("Graphics", "Green Max", 255, Conf_File);
	BMax_Level = GetPrivateProfileInt("Graphics", "Blue Max", 255, Conf_File);
	Contrast_Level = GetPrivateProfileInt("Graphics", "Contrast", 100, Conf_File);
	Brightness_Level = GetPrivateProfileInt("Graphics", "Brightness", 100, Conf_File);
	Greyscale = GetPrivateProfileInt("Graphics", "Greyscale", 0, Conf_File);
	Invert_Color = GetPrivateProfileInt("Graphics", "Invert", 0, Conf_File);

	Recalculate_Palettes();

	FS_VSync = GetPrivateProfileInt("Graphics", "Full Screen VSync", 1, Conf_File);
	W_VSync = GetPrivateProfileInt("Graphics", "Windows VSync", 1, Conf_File);
	Full_Screen = GetPrivateProfileInt("Graphics", "Full Screen", 0, Conf_File);
	Render_W = GetPrivateProfileInt("Graphics", "Render Windowed", 1, Conf_File);
	Render_FS = GetPrivateProfileInt("Graphics", "Render Fullscreen", 1, Conf_File);

	Set_Render(HWnd, Full_Screen, -1, 1);

	Stretch = GetPrivateProfileInt("Graphics", "Stretch", 0, Conf_File);
	Blit_Soft = GetPrivateProfileInt("Graphics", "Software Blit", 0, Conf_File);
	Sprite_Over = GetPrivateProfileInt("Graphics", "Sprite limit", 1, Conf_File);
	Frame_Skip = GetPrivateProfileInt("Graphics", "Frame skip", -1, Conf_File);

	Sound_Rate = GetPrivateProfileInt("Sound", "Rate", 22050, Conf_File);
	Sound_Stereo = GetPrivateProfileInt("Sound", "Stereo", 1, Conf_File);

	if (GetPrivateProfileInt("Sound", "Z80 State", 1, Conf_File)) Z80_State |= 1;
	else Z80_State &= ~1;

	new_val = GetPrivateProfileInt("Sound", "State", 1, Conf_File);
	if (new_val != Sound_Enable)
	{
		if (Change_Sound(HWnd))
		{
			YM2612_Enable = GetPrivateProfileInt("Sound", "YM2612 State", 1, Conf_File);
			PSG_Enable = GetPrivateProfileInt("Sound", "PSG State", 1, Conf_File);
			DAC_Enable = GetPrivateProfileInt("Sound", "DAC State", 1, Conf_File);
			PCM_Enable = GetPrivateProfileInt("Sound", "PCM State", 1, Conf_File);
			PWM_Enable = GetPrivateProfileInt("Sound", "PWM State", 1, Conf_File);
			CDDA_Enable = GetPrivateProfileInt("Sound", "CDDA State", 1, Conf_File);
		}
	}
	else
	{
		YM2612_Enable = GetPrivateProfileInt("Sound", "YM2612 State", 1, Conf_File);
		PSG_Enable = GetPrivateProfileInt("Sound", "PSG State", 1, Conf_File);
		DAC_Enable = GetPrivateProfileInt("Sound", "DAC State", 1, Conf_File);
		PCM_Enable = GetPrivateProfileInt("Sound", "PCM State", 1, Conf_File);
		PWM_Enable = GetPrivateProfileInt("Sound", "PWM State", 1, Conf_File);
		CDDA_Enable = GetPrivateProfileInt("Sound", "CDDA State", 1, Conf_File);
	}

	YM2612_Improv = GetPrivateProfileInt("Sound", "YM2612 Improvement", 0, Conf_File);
	DAC_Improv = GetPrivateProfileInt("Sound", "DAC Improvement", 0, Conf_File);
	PSG_Improv = GetPrivateProfileInt("Sound", "PSG Improvement", 0, Conf_File);

	Country = GetPrivateProfileInt("CPU", "Country", -1, Conf_File);
	Country_Order[0] = GetPrivateProfileInt("CPU", "Prefered Country 1", 0, Conf_File);
	Country_Order[1] = GetPrivateProfileInt("CPU", "Prefered Country 2", 1, Conf_File);
	Country_Order[2] = GetPrivateProfileInt("CPU", "Prefered Country 3", 2, Conf_File);

	SegaCD_Accurate = GetPrivateProfileInt("CPU", "Perfect synchro between main and sub CPU (Sega CD)", 0, Conf_File);

	MSH2_Speed = GetPrivateProfileInt("CPU", "Main SH2 Speed", 100, Conf_File);
	SSH2_Speed = GetPrivateProfileInt("CPU", "Slave SH2 Speed", 100, Conf_File);

	if (MSH2_Speed < 0) MSH2_Speed = 0;
	if (SSH2_Speed < 0) SSH2_Speed = 0;

	Check_Country_Order();

	Fast_Blur = GetPrivateProfileInt("Options", "Fast Blur", 0, Conf_File);
	Show_FPS = GetPrivateProfileInt("Options", "FPS", 0, Conf_File);
	FPS_Style = GetPrivateProfileInt("Options", "FPS Style", 0, Conf_File);
	Show_Message = GetPrivateProfileInt("Options", "Message", 1, Conf_File);
	Message_Style = GetPrivateProfileInt("Options", "Message Style", 0, Conf_File);
	Show_LED = GetPrivateProfileInt("Options", "LED", 1, Conf_File);
	Auto_Fix_CS = GetPrivateProfileInt("Options", "Auto Fix Checksum", 0, Conf_File);
	Auto_Pause = GetPrivateProfileInt("Options", "Auto Pause", 0, Conf_File);
	CUR_DEV = GetPrivateProfileInt("Options", "CD Drive", 0, Conf_File);
	BRAM_Ex_Size = GetPrivateProfileInt("Options", "Ram Cart Size", 3, Conf_File);

	if (BRAM_Ex_Size == -1)
	{
		BRAM_Ex_State &= 1;
		BRAM_Ex_Size = 0;
	}
	else BRAM_Ex_State |= 0x100;

	GetPrivateProfileString("Options", "GCOffline path", "GCOffline.chm", CGOffline_Path, 1024, Conf_File);
	GetPrivateProfileString("Options", "Gens manual path", "manual.exe", Manual_Path, 1024, Conf_File);

	Controller_1_Type = GetPrivateProfileInt("Input", "P1.Type", 1, Conf_File);
	Keys_Def[0].Up = GetPrivateProfileInt("Input", "P1.Up", DIK_UP, Conf_File);
	Keys_Def[0].Down = GetPrivateProfileInt("Input", "P1.Down", DIK_DOWN, Conf_File);
	Keys_Def[0].Left = GetPrivateProfileInt("Input", "P1.Left", DIK_LEFT, Conf_File);
	Keys_Def[0].Right = GetPrivateProfileInt("Input", "P1.Right", DIK_RIGHT, Conf_File);
	Keys_Def[0].Start = GetPrivateProfileInt("Input", "P1.Start", DIK_RETURN, Conf_File);
	Keys_Def[0].A = GetPrivateProfileInt("Input", "P1.A", DIK_A, Conf_File);
	Keys_Def[0].B = GetPrivateProfileInt("Input", "P1.B", DIK_S, Conf_File);
	Keys_Def[0].C = GetPrivateProfileInt("Input", "P1.C", DIK_D, Conf_File);
	Keys_Def[0].Mode = GetPrivateProfileInt("Input", "P1.Mode", DIK_RSHIFT, Conf_File);
	Keys_Def[0].X = GetPrivateProfileInt("Input", "P1.X", DIK_Z, Conf_File);
	Keys_Def[0].Y = GetPrivateProfileInt("Input", "P1.Y", DIK_X, Conf_File);
	Keys_Def[0].Z = GetPrivateProfileInt("Input", "P1.Z", DIK_C, Conf_File);
	Keys_Def[0].RA_Home = GetPrivateProfileInt("Input", "P1.Home", DIK_ESCAPE, Conf_File);
	Keys_Def[0].Rewind = GetPrivateProfileInt("Input", "P1.Rewind", DIK_V, Conf_File);		// ##RW

	//	Default 360 layout
	Controller_1B_Type = GetPrivateProfileInt("Input", "P1B.Type", 1, Conf_File);
	Keys_Def[2].Up = GetPrivateProfileInt("Input", "P1B.Up", 4225, Conf_File);
	Keys_Def[2].Down = GetPrivateProfileInt("Input", "P1B.Down", 4227, Conf_File);
	Keys_Def[2].Left = GetPrivateProfileInt("Input", "P1B.Left", 4228, Conf_File);
	Keys_Def[2].Right = GetPrivateProfileInt("Input", "P1B.Right", 4226, Conf_File);
	Keys_Def[2].Start = GetPrivateProfileInt("Input", "P1B.Start", 4119, Conf_File);
	Keys_Def[2].A = GetPrivateProfileInt("Input", "P1B.A", 4114, Conf_File);
	Keys_Def[2].B = GetPrivateProfileInt("Input", "P1B.B", 4115, Conf_File);
	Keys_Def[2].C = GetPrivateProfileInt("Input", "P1B.C", 4113, Conf_File);
	Keys_Def[2].Mode = GetPrivateProfileInt("Input", "P1B.Mode", 4120, Conf_File);
	Keys_Def[2].X = GetPrivateProfileInt("Input", "P1B.X", 4116, Conf_File);
	Keys_Def[2].Y = GetPrivateProfileInt("Input", "P1B.Y", 4117, Conf_File);
	Keys_Def[2].Z = GetPrivateProfileInt("Input", "P1B.Z", 4112, Conf_File);
	Keys_Def[2].RA_Home = GetPrivateProfileInt("Input", "P1B.Home", 4118, Conf_File);
	Keys_Def[2].Rewind = GetPrivateProfileInt("Input", "P1B.Rewind", 0000, Conf_File);	// ##RW TODO: 360 Rewind standard button

	Controller_1C_Type = GetPrivateProfileInt("Input", "P1C.Type", 0, Conf_File);
	Keys_Def[3].Up = GetPrivateProfileInt("Input", "P1C.Up", 0, Conf_File);
	Keys_Def[3].Down = GetPrivateProfileInt("Input", "P1C.Down", 0, Conf_File);
	Keys_Def[3].Left = GetPrivateProfileInt("Input", "P1C.Left", 0, Conf_File);
	Keys_Def[3].Right = GetPrivateProfileInt("Input", "P1C.Right", 0, Conf_File);
	Keys_Def[3].Start = GetPrivateProfileInt("Input", "P1C.Start", 0, Conf_File);
	Keys_Def[3].A = GetPrivateProfileInt("Input", "P1C.A", 0, Conf_File);
	Keys_Def[3].B = GetPrivateProfileInt("Input", "P1C.B", 0, Conf_File);
	Keys_Def[3].C = GetPrivateProfileInt("Input", "P1C.C", 0, Conf_File);
	Keys_Def[3].Mode = GetPrivateProfileInt("Input", "P1C.Mode", 0, Conf_File);
	Keys_Def[3].X = GetPrivateProfileInt("Input", "P1C.X", 0, Conf_File);
	Keys_Def[3].Y = GetPrivateProfileInt("Input", "P1C.Y", 0, Conf_File);
	Keys_Def[3].Z = GetPrivateProfileInt("Input", "P1C.Z", 0, Conf_File);
	Keys_Def[3].RA_Home = GetPrivateProfileInt("Input", "P1C.Home", 0, Conf_File);
	Keys_Def[3].Rewind = GetPrivateProfileInt("Input", "P1C.Rewind", 0, Conf_File);	// ##RW

	Controller_1D_Type = GetPrivateProfileInt("Input", "P1D.Type", 0, Conf_File);
	Keys_Def[4].Up = GetPrivateProfileInt("Input", "P1D.Up", 0, Conf_File);
	Keys_Def[4].Down = GetPrivateProfileInt("Input", "P1D.Down", 0, Conf_File);
	Keys_Def[4].Left = GetPrivateProfileInt("Input", "P1D.Left", 0, Conf_File);
	Keys_Def[4].Right = GetPrivateProfileInt("Input", "P1D.Right", 0, Conf_File);
	Keys_Def[4].Start = GetPrivateProfileInt("Input", "P1D.Start", 0, Conf_File);
	Keys_Def[4].A = GetPrivateProfileInt("Input", "P1D.A", 0, Conf_File);
	Keys_Def[4].B = GetPrivateProfileInt("Input", "P1D.B", 0, Conf_File);
	Keys_Def[4].C = GetPrivateProfileInt("Input", "P1D.C", 0, Conf_File);
	Keys_Def[4].Mode = GetPrivateProfileInt("Input", "P1D.Mode", 0, Conf_File);
	Keys_Def[4].X = GetPrivateProfileInt("Input", "P1D.X", 0, Conf_File);
	Keys_Def[4].Y = GetPrivateProfileInt("Input", "P1D.Y", 0, Conf_File);
	Keys_Def[4].Z = GetPrivateProfileInt("Input", "P1D.Z", 0, Conf_File);
	Keys_Def[4].RA_Home = GetPrivateProfileInt("Input", "P1D.Home", 0, Conf_File);
	Keys_Def[4].Rewind = GetPrivateProfileInt("Input", "P1D.Rewind", 0, Conf_File);	// ##RW


	Controller_2_Type = GetPrivateProfileInt("Input", "P2.Type", 1, Conf_File);
	Keys_Def[1].Up = GetPrivateProfileInt("Input", "P2.Up", DIK_Y, Conf_File);
	Keys_Def[1].Down = GetPrivateProfileInt("Input", "P2.Down", DIK_H, Conf_File);
	Keys_Def[1].Left = GetPrivateProfileInt("Input", "P2.Left", DIK_G, Conf_File);
	Keys_Def[1].Right = GetPrivateProfileInt("Input", "P2.Right", DIK_J, Conf_File);
	Keys_Def[1].Start = GetPrivateProfileInt("Input", "P2.Start", DIK_U, Conf_File);
	Keys_Def[1].A = GetPrivateProfileInt("Input", "P2.A", DIK_K, Conf_File);
	Keys_Def[1].B = GetPrivateProfileInt("Input", "P2.B", DIK_L, Conf_File);
	Keys_Def[1].C = GetPrivateProfileInt("Input", "P2.C", DIK_M, Conf_File);
	Keys_Def[1].Mode = GetPrivateProfileInt("Input", "P2.Mode", DIK_T, Conf_File);
	Keys_Def[1].X = GetPrivateProfileInt("Input", "P2.X", DIK_I, Conf_File);
	Keys_Def[1].Y = GetPrivateProfileInt("Input", "P2.Y", DIK_O, Conf_File);
	Keys_Def[1].Z = GetPrivateProfileInt("Input", "P2.Z", DIK_P, Conf_File);
	Keys_Def[1].RA_Home = GetPrivateProfileInt("Input", "P2.Home", 0, Conf_File);
	Keys_Def[1].Rewind = GetPrivateProfileInt("Input", "P2.Rewind", 0, Conf_File);	// ##RW

	Controller_2B_Type = GetPrivateProfileInt("Input", "P2B.Type", 0, Conf_File);
	Keys_Def[5].Up = GetPrivateProfileInt("Input", "P2B.Up", 0, Conf_File);
	Keys_Def[5].Down = GetPrivateProfileInt("Input", "P2B.Down", 0, Conf_File);
	Keys_Def[5].Left = GetPrivateProfileInt("Input", "P2B.Left", 0, Conf_File);
	Keys_Def[5].Right = GetPrivateProfileInt("Input", "P2B.Right", 0, Conf_File);
	Keys_Def[5].Start = GetPrivateProfileInt("Input", "P2B.Start", 0, Conf_File);
	Keys_Def[5].A = GetPrivateProfileInt("Input", "P2B.A", 0, Conf_File);
	Keys_Def[5].B = GetPrivateProfileInt("Input", "P2B.B", 0, Conf_File);
	Keys_Def[5].C = GetPrivateProfileInt("Input", "P2B.C", 0, Conf_File);
	Keys_Def[5].Mode = GetPrivateProfileInt("Input", "P2B.Mode", 0, Conf_File);
	Keys_Def[5].X = GetPrivateProfileInt("Input", "P2B.X", 0, Conf_File);
	Keys_Def[5].Y = GetPrivateProfileInt("Input", "P2B.Y", 0, Conf_File);
	Keys_Def[5].Z = GetPrivateProfileInt("Input", "P2B.Z", 0, Conf_File);
	Keys_Def[5].RA_Home = GetPrivateProfileInt("Input", "P2B.Home", 0, Conf_File);
	Keys_Def[5].Rewind = GetPrivateProfileInt("Input", "P2B.Rewind", 0, Conf_File);	// ##RW

	Controller_2C_Type = GetPrivateProfileInt("Input", "P2C.Type", 0, Conf_File);
	Keys_Def[6].Up = GetPrivateProfileInt("Input", "P2C.Up", 0, Conf_File);
	Keys_Def[6].Down = GetPrivateProfileInt("Input", "P2C.Down", 0, Conf_File);
	Keys_Def[6].Left = GetPrivateProfileInt("Input", "P2C.Left", 0, Conf_File);
	Keys_Def[6].Right = GetPrivateProfileInt("Input", "P2C.Right", 0, Conf_File);
	Keys_Def[6].Start = GetPrivateProfileInt("Input", "P2C.Start", 0, Conf_File);
	Keys_Def[6].A = GetPrivateProfileInt("Input", "P2C.A", 0, Conf_File);
	Keys_Def[6].B = GetPrivateProfileInt("Input", "P2C.B", 0, Conf_File);
	Keys_Def[6].C = GetPrivateProfileInt("Input", "P2C.C", 0, Conf_File);
	Keys_Def[6].Mode = GetPrivateProfileInt("Input", "P2C.Mode", 0, Conf_File);
	Keys_Def[6].X = GetPrivateProfileInt("Input", "P2C.X", 0, Conf_File);
	Keys_Def[6].Y = GetPrivateProfileInt("Input", "P2C.Y", 0, Conf_File);
	Keys_Def[6].Z = GetPrivateProfileInt("Input", "P2C.Z", 0, Conf_File);
	Keys_Def[6].RA_Home = GetPrivateProfileInt("Input", "P2C.Home", 0, Conf_File);
	Keys_Def[6].Rewind = GetPrivateProfileInt("Input", "P2C.Rewind", 0, Conf_File);	// ##RW

	Controller_2D_Type = GetPrivateProfileInt("Input", "P2D.Type", 0, Conf_File);
	Keys_Def[7].Up = GetPrivateProfileInt("Input", "P2D.Up", 0, Conf_File);
	Keys_Def[7].Down = GetPrivateProfileInt("Input", "P2D.Down", 0, Conf_File);
	Keys_Def[7].Left = GetPrivateProfileInt("Input", "P2D.Left", 0, Conf_File);
	Keys_Def[7].Right = GetPrivateProfileInt("Input", "P2D.Right", 0, Conf_File);
	Keys_Def[7].Start = GetPrivateProfileInt("Input", "P2D.Start", 0, Conf_File);
	Keys_Def[7].A = GetPrivateProfileInt("Input", "P2D.A", 0, Conf_File);
	Keys_Def[7].B = GetPrivateProfileInt("Input", "P2D.B", 0, Conf_File);
	Keys_Def[7].C = GetPrivateProfileInt("Input", "P2D.C", 0, Conf_File);
	Keys_Def[7].Mode = GetPrivateProfileInt("Input", "P2D.Mode", 0, Conf_File);
	Keys_Def[7].X = GetPrivateProfileInt("Input", "P2D.X", 0, Conf_File);
	Keys_Def[7].Y = GetPrivateProfileInt("Input", "P2D.Y", 0, Conf_File);
	Keys_Def[7].Z = GetPrivateProfileInt("Input", "P2D.Z", 0, Conf_File);
	Keys_Def[7].RA_Home = GetPrivateProfileInt("Input", "P2D.Home", 0, Conf_File);
	Keys_Def[7].Rewind = GetPrivateProfileInt("Input", "P2D.Rewind", 0, Conf_File);	// ##RW


	//char bufferUser[50];
	//char bufferToken[50];
	///GetPrivateProfileString("Achievements", "User", "", bufferUser, 50, Conf_File);
	//GetPrivateProfileString("Achievements", "Token", "", bufferToken, 50, Conf_File);
	//GetPrivateProfileString("Achievements", "RAKnownVersion", "", g_sKnownRAVersion, 50, Conf_File );
	//RAUsers::LocalUser.m_nLatestScore = GetPrivateProfileInt("Achievements", "Points", 0, Conf_File );

	//RAUsers::LocalUser.SetUsername( bufferUser );

	//	bRemember=true because we have data to restore, implying we had/wanted data saved:
	//if( bufferToken[0] != '\0' )
	//{
	//	//	Set a username to imply we may have logged in...
	//	strcpy_s( RAUsers::LocalUser.m_sUsername, 64, bufferUser );
	//	//strcpy_s( RAUsers::LocalUser.m_sToken, 64, bufferToken );

	//	RAUsers::LocalUser.AttemptSilentLogin();
	//}

	Make_IO_Table();
	DestroyMenu(Gens_Menu);

	if (Full_Screen) Build_Main_Menu();
	else SetMenu(HWnd, Build_Main_Menu());		// Update new menu

	return 1;
}


int Load_As_Config(HWND hWnd, void *Game_Active)
{
	char Name[2048];
	OPENFILENAME ofn;

	SetCurrentDirectory(Gens_Path);

	strcpy(&Name[0], "Gens.cfg");
	memset(&ofn, 0, sizeof(OPENFILENAME));

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = ghInstance;
	ofn.lpstrFile = Name;
	ofn.nMaxFile = 2047;
	ofn.lpstrFilter = "Config Files\0*.cfg\0All Files\0*.*\0\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrInitialDir = Gens_Path;
	ofn.lpstrDefExt = "cfg";
	ofn.Flags = OFN_FILEMUSTEXIST;

	if(GetOpenFileName(&ofn))
	{
		Load_Config(Name, Game_Active);
		strcpy(Str_Tmp, "config loaded from ");
		strcat(Str_Tmp, Name);
		Put_Info(Str_Tmp, 2000);
		return 1;
	}
	else return 0;
}


int Load_SRAM(void)
{
	int bResult;
	HANDLE SRAM_File;
	char Name[2048];
	unsigned long Bytes_Read;

	SetCurrentDirectory(Gens_Path);

	memset(SRAM, 0, 64 * 1024);

	strcpy(Name, SRAM_Dir);
	strcat(Name, Rom_Name);
	strcat(Name, ".srm");
	
	SRAM_File = CreateFile(Name, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (SRAM_File == INVALID_HANDLE_VALUE) return 0;

	bResult = ReadFile(SRAM_File, SRAM, 64 * 1024, &Bytes_Read, NULL);

	CloseHandle(SRAM_File);

	strcpy(Str_Tmp, "SRAM loaded from ");
	strcat(Str_Tmp, Name);
	Put_Info(Str_Tmp, 2000);

	return 1;
}


int Save_SRAM(void)
{
	HANDLE SRAM_File;
	int bResult, size_to_save, i;
	char Name[2048];
	unsigned long Bytes_Write;

	SetCurrentDirectory(Gens_Path);

	i = (64 * 1024) - 1;
	while ((i >= 0) && (SRAM[i] == 0)) i--;

	if (i < 0) return 0;

	i++;

	size_to_save = 1;
	while (i > size_to_save) size_to_save <<= 1;

	strcpy(Name, SRAM_Dir);
	strcat(Name, Rom_Name);
	strcat(Name, ".srm");
	
	SRAM_File = CreateFile(Name, GENERIC_WRITE, NULL, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (SRAM_File == INVALID_HANDLE_VALUE) return 0;
	
	bResult = WriteFile(SRAM_File, SRAM, size_to_save, &Bytes_Write, NULL);
	
	CloseHandle(SRAM_File);

	strcpy(Str_Tmp, "SRAM saved in ");
	strcat(Str_Tmp, Name);
	Put_Info(Str_Tmp, 2000);

	return 1;
}


void S_Format_BRAM(unsigned char *buf)
{
	memset(buf, 0x5F, 11);

	buf[0x0F] = 0x40;

	buf[0x11] = 0x7D;
	buf[0x13] = 0x7D;
	buf[0x15] = 0x7D;
	buf[0x17] = 0x7D;

	sprintf((char *) &buf[0x20], "SEGA CD ROM");
	sprintf((char *) &buf[0x30], "RAM CARTRIDGE");

	buf[0x24] = 0x5F;
	buf[0x27] = 0x5F;

	buf[0x2C] = 0x01;

	buf[0x33] = 0x5F;
	buf[0x3D] = 0x5F;
	buf[0x3E] = 0x5F;
	buf[0x3F] = 0x5F;
}


void Format_Backup_Ram(void)
{
	memset(Ram_Backup, 0, 8 * 1024);

	S_Format_BRAM(&Ram_Backup[0x1FC0]);

	memset(Ram_Backup_Ex, 0, 64 * 1024);
}


int Load_BRAM(void)
{
	HANDLE BRAM_File;
	int bResult;
	char Name[2048];
	unsigned long Bytes_Read;

	Format_Backup_Ram();

	SetCurrentDirectory(Gens_Path);

	strcpy(Name, BRAM_Dir);
	strcat(Name, Rom_Name);
	strcat(Name, ".brm");
	
	BRAM_File = CreateFile(Name, GENERIC_READ, FILE_SHARE_READ, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if (BRAM_File == INVALID_HANDLE_VALUE) return 0;

	bResult = ReadFile(BRAM_File, Ram_Backup, 8 * 1024, &Bytes_Read, NULL);
	bResult = ReadFile(BRAM_File, Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, &Bytes_Read, NULL);

	CloseHandle(BRAM_File);

	strcpy(Str_Tmp, "BRAM loaded from ");
	strcat(Str_Tmp, Name);
	Put_Info(Str_Tmp, 2000);

	return 1;
}


int Save_BRAM(void)
{
	HANDLE BRAM_File;
	int bResult;
	char Name[2048];
	unsigned long Bytes_Write;

	SetCurrentDirectory(Gens_Path);

	strcpy(Name, BRAM_Dir);
	strcat(Name, Rom_Name);
	strcat(Name, ".brm");
	
	BRAM_File = CreateFile(Name, GENERIC_WRITE, NULL, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (BRAM_File == INVALID_HANDLE_VALUE) return 0;
	
	bResult = WriteFile(BRAM_File, Ram_Backup, 8 * 1024, &Bytes_Write, NULL);
	bResult = WriteFile(BRAM_File, Ram_Backup_Ex, (8 << BRAM_Ex_Size) * 1024, &Bytes_Write, NULL);
	
	CloseHandle(BRAM_File);

	strcpy(Str_Tmp, "BRAM saved in ");
	strcat(Str_Tmp, Name);
	Put_Info(Str_Tmp, 2000);

	return 1;
}