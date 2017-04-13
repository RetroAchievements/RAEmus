#include <stdio.h>

#include "gens.h"
#include "G_main.h"
#include "G_ddraw.h"
#include "G_dsound.h"
#include "G_input.h"
#include "rom.h"
#include "mem_M68K.h"
#include "mem_S68K.h"
#include "mem_SH2.h"
#include "ym2612.h"
#include "psg.h"
#include "Cpu_68k.h"
#include "Cpu_Z80.h"
#include "Cpu_SH2.h"
#include "z80.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32X.h"
#include "io.h"
#include "misc.h"
#include "save.h"
#include "ggenie.h"
#include "cd_sys.h"
#include "LC89510.h"
#include "gfx_cd.h"
#include "wave.h"
#include "pcm.h"
#include "pwm.h"
#include "cd_sys.h"
#include "cd_file.h"

//	##RA
#include "../RA_Integration/RA_Interface.h"



int Debug;
int Frame_Skip;
int Frame_Number;
int DAC_Improv;
int RMax_Level;
int GMax_Level;
int BMax_Level;
int Contrast_Level;
int Brightness_Level;
int Greyscale;
int Invert_Color;

unsigned char CD_Data[1024];		// Used for hard reset to know the game name


int Round_Double(double val)
{
	if ((val - (double) (int) val) > 0.5) return (int) (val + 1);
	else return (int) val;
}


void Init_Tab(void)
{
	int x, y, dep;

	for(x = 0; x < 1024; x++)
	{
		for(y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x3FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 9)] = (unsigned short) dep;
		}
	}

	for(x = 0; x < 512; x++)
	{
		for(y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0x1FC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 8) + 0x8000] = (unsigned short) (dep + 0x8000);
		}
	}

	for(x = 0; x < 256; x++)
	{
		for(y = 0; y < 64; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 8);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xC000] = (unsigned short) (dep + 0xC000);
		}
	}

	for(x = 0; x < 256; x++)
	{
		for(y = 0; y < 32; y++)
		{
			dep = (x & 3) + (((x & 0xFC) >> 2) << 7);
			dep += ((y & 7) << 2) + (((y & 0xF8) >> 3) << 5);
			dep >>= 1;
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xE000] = (unsigned short) (dep + 0xE000);
			Cell_Conv_Tab[(x >> 1) + (y << 7) + 0xF000] = (unsigned short) (dep + 0xF000);
		}
	}

	for(x = 0; x < 512; x++) Z80_M68K_Cycle_Tab[x] = (int) ((double) x * 7.0 / 15.0);
}


void Recalculate_Palettes(void)
{
	int i;
	int r, g, b;
	int rf, gf, bf;
	int bright, cont;

	for(r = 0; r < 0x10; r++)
	{
		for(g = 0; g < 0x10; g++)
		{
			for(b = 0; b < 0x10; b++)
			{
				rf = (r & 0xE) << 2;
				gf = (g & 0xE) << 2;
				bf = (b & 0xE) << 2;

				rf = (double) (rf) * ((double) (RMax_Level) / 224.0);
				gf = (double) (gf) * ((double) (GMax_Level) / 224.0);
				bf = (double) (bf) * ((double) (BMax_Level) / 224.0);

				// Compute colors here (64 levels)

				bright = Brightness_Level;
				bright -= 100;
				bright *= 32;
				bright /= 100;

				rf += bright;
				gf += bright;
				bf += bright;

				if (rf < 0) rf = 0;
				else if (rf > 0x3F) rf = 0x3F;
				if (gf < 0) gf = 0;
				else if (gf > 0x3F) gf = 0x3F;
				if (bf < 0) bf = 0;
				else if (bf > 0x3F) bf = 0x3F;

				cont = Contrast_Level;

				rf = (rf * cont) / 100;
				gf = (gf * cont) / 100;
				bf = (bf * cont) / 100;
		
				if (rf < 0) rf = 0;
				else if (rf > 0x3F) rf = 0x3F;
				if (gf < 0) gf = 0;
				else if (gf > 0x3F) gf = 0x3F;
				if (bf < 0) bf = 0;
				else if (bf > 0x3F) bf = 0x3F;

				if (Mode_555 & 1)
				{
					rf = (rf >> 1) << 10;
					gf = (gf >> 1) << 5;
				}
				else
				{
					rf = (rf >> 1) << 11;
					gf = (gf >> 0) << 5;
				}
				bf = (bf >> 1) << 0;

				Palette[(b << 8) | (g << 4) | r] = rf | gf | bf;
			}
		}
	}

	for(i = 0; i < 0x10000; i++)
	{
		b = ((i >> 10) & 0x1F) << 1;
		g = ((i >>  5) & 0x1F) << 1;
		r = ((i >>  0) & 0x1F) << 1;

		r = (double) (r) * ((double) (RMax_Level) / 248.0);
		g = (double) (g) * ((double) (GMax_Level) / 248.0);
		b = (double) (b) * ((double) (BMax_Level) / 248.0);

		// Compute colors here (64 levels)

		bright = Brightness_Level;
		bright -= 100;
		bright *= 32;
		bright /= 100;

		r += bright;
		g += bright;
		b += bright;

		if (r < 0) r = 0;
		else if (r > 0x3F) r = 0x3F;
		if (g < 0) g = 0;
		else if (g > 0x3F) g = 0x3F;
		if (b < 0) b = 0;
		else if (b > 0x3F) b = 0x3F;

		cont = Contrast_Level;

		r = (r * cont) / 100;
		g = (g * cont) / 100;
		b = (b * cont) / 100;
		
		if (r < 0) r = 0;
		else if (r > 0x3F) r = 0x3F;
		if (g < 0) g = 0;
		else if (g > 0x3F) g = 0x3F;
		if (b < 0) b = 0;
		else if (b > 0x3F) b = 0x3F;

		if (Mode_555 & 1)
		{
			r = (r >> 1) << 10;
			g = (g >> 1) << 5;
		}
		else
		{
			r = (r >> 1) << 11;
			g = (g >> 0) << 5;
		}
		b = (b >> 1) << 0;

		_32X_Palette_16B[i] = r | g | b;
	}

	if (Greyscale)
	{
		for(i = 0; i < 0x1000; i++)
		{
			if (Mode_555 & 1)
			{
				r = ((Palette[i] >> 10) & 0x1F) << 1;
				g = ((Palette[i] >> 5) & 0x1F) << 1;
			}
			else
			{
				r = ((Palette[i] >> 11) & 0x1F) << 1;
				g = (Palette[i] >> 5) & 0x3F;
			}

			b = ((Palette[i] >> 0) & 0x1F) << 1;

			r = (r * unsigned int (0.30 * 65536.0)) >> 16;
			g = (g * unsigned int (0.59 * 65536.0)) >> 16;
			b = (b * unsigned int (0.11 * 65536.0)) >> 16;

			r = g = b = r + g + b;

			if (Mode_555 & 1)
			{
				r = (r >> 1) << 10;
				g = (g >> 1) << 5;
			}
			else
			{
				r = (r >> 1) << 11;
				g = (g >> 0) << 5;
			}

			b = (b >> 1) << 0;

			Palette[i] = r | g | b;
		}

		for(i = 0; i < 0x10000; i++)
		{
			if (Mode_555 & 1)
			{
				r = ((_32X_Palette_16B[i] >> 10) & 0x1F) << 1;
				g = ((_32X_Palette_16B[i] >> 5) & 0x1F) << 1;
			}
			else
			{
				r = ((_32X_Palette_16B[i] >> 11) & 0x1F) << 1;
				g = (_32X_Palette_16B[i] >> 5) & 0x3F;
			}

			b = ((_32X_Palette_16B[i] >> 0) & 0x1F) << 1;

			r = (r * unsigned int (0.30 * 65536.0)) >> 16;
			g = (g * unsigned int (0.59 * 65536.0)) >> 16;
			b = (b * unsigned int (0.11 * 65536.0)) >> 16;

			r = g = b = r + g + b;

			if (Mode_555 & 1)
			{
				r = (r >> 1) << 10;
				g = (g >> 1) << 5;
			}
			else
			{
				r = (r >> 1) << 11;
				g = (g >> 0) << 5;
			}

			b = (b >> 1) << 0;

			_32X_Palette_16B[i] = r | g | b;
		}
	}

	if (Invert_Color)
	{
		for(i = 0; i < 0x1000; i++)
		{
			Palette[i] ^= 0xFFFF;
		}

		for(i = 0; i < 0x10000; i++)
		{
			_32X_Palette_16B[i] ^= 0xFFFF;
		}
	}

	for (i = 0; i < 0x100; i++)
	{
		_32X_VDP_CRam_Ajusted[i] = _32X_Palette_16B[_32X_VDP_CRam[i]];

	}
}


void Check_Country_Order(void)
{
	if ((Country_Order[0] == Country_Order[1]) || (Country_Order[0] == Country_Order[2]) || (Country_Order[1] == Country_Order[2]) || (Country_Order[0] == Country_Order[2])
		|| (Country_Order[0] > 2) || (Country_Order[0] < 0) || (Country_Order[1] > 2) || (Country_Order[1] < 0) || (Country_Order[2] > 2) || (Country_Order[2] < 0))
	{
		Country_Order[0] = 0;
		Country_Order[1] = 1;
		Country_Order[2] = 2;
	}
}


char *Detect_Country_SegaCD(void)
{
	if (CD_Data[0x10B] == 0x64)
	{
		Game_Mode = 1;
		CPU_Mode = 1;
		return EU_CD_Bios;
	}
	else if (CD_Data[0x10B] == 0xA1)
	{
		Game_Mode = 0;
		CPU_Mode = 0;
		return JA_CD_Bios;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
		return US_CD_Bios;
	}
}


void Detect_Country_Genesis(void)
{
	int c_tab[3] = {4, 1, 8};
	int gm_tab[3] = {1, 0, 1};
	int cm_tab[3] = {0, 0, 1};
	int i, coun = 0;
	char c;
	
	if (!_strnicmp((char *) &Rom_Data[0x1F0], "eur", 3)) coun |= 8;
	else if (!_strnicmp((char *) &Rom_Data[0x1F0], "usa", 3)) coun |= 4;
	else if (!_strnicmp((char *) &Rom_Data[0x1F0], "jap", 3)) coun |= 1;
	else for(i = 0; i < 4; i++)
	{
		c = toupper(Rom_Data[0x1F0 + i]);
		
		if (c == 'U') coun |= 4;
		else if (c == 'J') coun |= 1;
		else if (c == 'E') coun |= 8;
		else if (c < 16) coun |= c;
		else if ((c >= '0') && (c <= '9')) coun |= c - '0';
		else if ((c >= 'A') && (c <= 'F')) coun |= c - 'A' + 10;
	}

	if (coun & c_tab[Country_Order[0]])
	{
		Game_Mode = gm_tab[Country_Order[0]];
		CPU_Mode = cm_tab[Country_Order[0]];
	}
	else if (coun & c_tab[Country_Order[1]])
	{
		Game_Mode = gm_tab[Country_Order[1]];
		CPU_Mode = cm_tab[Country_Order[1]];
	}
	else if (coun & c_tab[Country_Order[2]])
	{
		Game_Mode = gm_tab[Country_Order[2]];
		CPU_Mode = cm_tab[Country_Order[2]];
	}
	else if (coun & 2)
	{
		Game_Mode = 0;
		CPU_Mode = 1;
	}
	else
	{
		Game_Mode = 1;
		CPU_Mode = 0;
	}

	if (Game_Mode)
	{
		if (CPU_Mode) Put_Info("Europe system (50 FPS)", 1500);
		else Put_Info("USA system (60 FPS)", 1500);
	}
	else
	{
		if (CPU_Mode) Put_Info("Japan system (50 FPS)", 1500);
		else Put_Info("Japan system (60 FPS)", 1500);
	}

	if (CPU_Mode)
	{
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;
	}
	else
	{
		_32X_VDP.Mode |= 0x8000;
		VDP_Status &= 0xFFFE;
	}
}







/*************************************/
/*              GENESIS              */
/*************************************/


void Init_Genesis_Bios(void)
{
	FILE *f;

	if (f = fopen(Genesis_Bios, "rb"))
	{
		fread(&Genesis_Rom[0], 1, 2 * 1024, f);
		Byte_Swap(&Genesis_Rom[0], 2 * 1024);
		fclose(f);
	}
	else memset(Genesis_Rom, 0, 2 * 1024);

	Rom_Size =  2 * 1024;
	memcpy(Rom_Data, Genesis_Rom, 2 * 1024);
	Game_Mode = 0;
	CPU_Mode = 0;
	VDP_Num_Vis_Lines = 224;
	M68K_Reset(0);
	Z80_Reset();
	Reset_VDP();
	CPL_Z80 = Round_Double((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
	CPL_M68K = Round_Double((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
	VDP_Num_Lines = 262;
	VDP_Status &= 0xFFFE;
	YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
}


int Init_Genesis(struct Rom *MD_Rom)
{
	char Str_Err[256];

	Flag_Clr_Scr = 1;
	Debug = Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	Controller_1_COM = Controller_2_COM = 0;

	if (!Kaillera_Client_Running)
	{
		if ((MD_Rom->Ram_Infos[8] == 'R') && (MD_Rom->Ram_Infos[9] == 'A') && (MD_Rom->Ram_Infos[10] & 0x40))
		{
			SRAM_Start = MD_Rom->Ram_Start_Adress & 0x0F80000;		// multiple de 0x080000
			SRAM_End = MD_Rom->Ram_End_Adress;
		}
		else
		{
			SRAM_Start = 0x200000;
			SRAM_End = 0x200000 + (64 * 1024) - 1;
		}

		if ((SRAM_Start > SRAM_End) || ((SRAM_End - SRAM_Start) >= (64 * 1024)))
			SRAM_End = SRAM_Start + (64 * 1024) - 1;

		if (Rom_Size <= (2 * 1024 * 1024))
		{
			SRAM_ON = 1;
			SRAM_Write = 1;
		}

		SRAM_Start &= 0xFFFFFFFE;
		SRAM_End |= 0x00000001;

//		sprintf(Str_Err, "deb = %.8X end = %.8X", SRAM_Start, SRAM_End);
//		MessageBox(NULL, Str_Err, "", MB_OK);

		if ((SRAM_End - SRAM_Start) <= 2) SRAM_Custom = 1;
		else SRAM_Custom = 0;

		Load_SRAM();
	}
	
	switch(Country)
	{
		default:
		case -1:
			Detect_Country_Genesis();
			break;

		case 0:
			Game_Mode = 0;
			CPU_Mode = 0;

			break;

		case 1:
			Game_Mode = 1;
			CPU_Mode = 0;
			break;

		case 2:
			Game_Mode = 1;
			CPU_Mode = 1;
			break;

		case 3:
			Game_Mode = 0;
			CPU_Mode = 1;
			break;
	}

	UpdateAppTitle();

	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	 	// Version de la megadrive (0x0 - 0xF)

	Byte_Swap(Rom_Data, Rom_Size);

	M68K_Reset(0);
	Z80_Reset();
	Reset_VDP();

	if (CPU_Mode)
	{
		CPL_Z80 = Round_Double((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = Round_Double((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) SSH2_Speed) / 100.0);

		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;

		YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, Sound_Rate);
	}
	else
	{
		CPL_Z80 = Round_Double((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = Round_Double((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = Round_Double(((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double(((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) SSH2_Speed) / 100.0);

		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;

		YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
	}

	if (Auto_Fix_CS) Fix_Checksum();

	if (Sound_Enable)
	{
		End_Sound();

		if (!Init_Sound(HWnd)) Sound_Enable = 0;
		else Play_Sound();
	}

	Load_Patch_File();
	Build_Main_Menu();

	Last_Time = GetTickCount();
	New_Time = 0;
	Used_Time = 0;

	Update_Frame = Do_Genesis_Frame;
	Update_Frame_Fast = Do_Genesis_Frame_No_VDP;

	return 1;
}


void Reset_Genesis()
{
	Controller_1_COM = Controller_2_COM = 0;
	Paused = 0;

	if (Rom_Size <= (2 * 1024 * 1024))
	{
		SRAM_ON = 1;
		SRAM_Write = 1;
	}
	else
	{
		SRAM_ON = 0;
		SRAM_Write = 0;
	}

	M68K_Reset(0);
	Z80_Reset();
	Reset_VDP();
	YM2612_Reset();

	if (CPU_Mode) VDP_Status |= 1;
	else VDP_Status &= ~1;

	if (Auto_Fix_CS) Fix_Checksum();
}


int Do_VDP_Only()
{
	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
		Render_Line();

	return(0);
}


int Do_Genesis_Frame_No_VDP(void)
{
	int *buf[2];
	int HInt_Counter;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);

	Patch_Codes();

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;							// Clear V Blank
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;		// Hint_Counter = step H interrupt

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1
//		main68k_exec(Cycles_M68K - 436);
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;					// HBlank = 0

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		main68k_exec(Cycles_M68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec(Cycles_M68K - 360);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;			// HBlank = 0
	VDP_Status |= 0x0080;			// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line();
	z80_Interrupt(&M_Z80, 0xFF);

	main68k_exec(Cycles_M68K);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1
//		main68k_exec(Cycles_M68K - 436);
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;					// HBlank = 0

		main68k_exec(Cycles_M68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}

	PSG_Special_Update();
	YM2612_Special_Update();

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	return(1);
}


int Do_Genesis_Frame()
{
	int *buf[2];
	int HInt_Counter;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);

	Patch_Codes();

	RA_DoAchievementsFrame();

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;							// Clear V Blank
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;					// Hint_Counter = step d'interruption H

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;			// HBlank = 1
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;			// HBlank = 0

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		Render_Line();

		main68k_exec(Cycles_M68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}

	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec(Cycles_M68K - 360);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;			// HBlank = 0
	VDP_Status |= 0x0080;			// V Int happened

	VDP_Int |= 0x8;
	Update_IRQ_Line();
	z80_Interrupt(&M_Z80, 0xFF);

	main68k_exec(Cycles_M68K);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1
//		main68k_exec(Cycles_M68K - 436);
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;					// HBlank = 0

		main68k_exec(Cycles_M68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}

	PSG_Special_Update();
	YM2612_Special_Update();

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	return(1);
}






/*************************************/
/*                32X                */
/*************************************/


int Init_32X(struct Rom *MD_Rom)
{
	char Str_Err[256];
	FILE *f;
	int i;

	if (f = fopen(_32X_Genesis_Bios, "rb"))
	{
		fread(&_32X_Genesis_Rom[0], 1, 256, f);
		Byte_Swap(&_32X_Genesis_Rom[0], 256);
		fclose(f);
	}
	else
	{
		MessageBox(NULL, "Your 32X bios files aren't correctly configured :\nGenesis 32X bios not found.\nGo to menu 'Option -> Bios/Misc Files' to set up them", "Error", MB_OK);
		return 0;
	}

	if (f = fopen(_32X_Master_Bios, "rb"))
	{
		fread(&_32X_MSH2_Rom[0], 1, 2 * 1024, f);
		fclose(f);
	}
	else
	{
		MessageBox(NULL, "Your 32X bios files aren't correctly configured :\nMaster SH2 bios not found.\nGo to menu 'Option -> Bios/Misc Files' to set up them", "Error", MB_OK);
		return 0;
	}

	if (f = fopen(_32X_Slave_Bios, "rb"))
	{
		fread(&_32X_SSH2_Rom[0], 1, 1 * 1024, f);
		fclose(f);
	}
	else
	{
		MessageBox(NULL, "Your 32X bios files aren't correctly configured :\nSlave SH2 bios not found.\nGo to menu 'Option -> Bios/Misc Files' to set up them", "Error", MB_OK);
		return 0;
	}

	Flag_Clr_Scr = 1;
	Debug = Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	Controller_1_COM = Controller_2_COM = 0;

	if (!Kaillera_Client_Running)
	{
		if ((MD_Rom->Ram_Infos[8] == 'R') && (MD_Rom->Ram_Infos[9] == 'A') && (MD_Rom->Ram_Infos[10] & 0x40))
		{
			SRAM_Start = MD_Rom->Ram_Start_Adress & 0x0F80000;		// multiple de 0x080000
			SRAM_End = MD_Rom->Ram_End_Adress;
		}
		else
		{
			SRAM_Start = 0x200000;
			SRAM_End = 0x200000 + (64 * 1024) - 1;
		}

		if ((SRAM_Start > SRAM_End) || ((SRAM_End - SRAM_Start) >= (64 * 1024)))
			SRAM_End = SRAM_Start + (64 * 1024) - 1;

		if (Rom_Size <= (2 * 1024 * 1024))
		{
			SRAM_ON = 1;
			SRAM_Write = 1;
		}

		SRAM_Start &= 0xFFFFFFFE;
		SRAM_End |= 0x00000001;

//		sprintf(Str_Err, "deb = %.8X end = %.8X", SRAM_Start, SRAM_End);
//		MessageBox(NULL, Str_Err, "", MB_OK);

		if ((SRAM_End - SRAM_Start) <= 2) SRAM_Custom = 1;
		else SRAM_Custom = 0;

		Load_SRAM();
	}
	
	switch(Country)
	{
		default:
		case -1:
			Detect_Country_Genesis();
			break;

		case 0:
			Game_Mode = 0;
			CPU_Mode = 0;

			break;

		case 1:
			Game_Mode = 1;
			CPU_Mode = 0;
			break;

		case 2:
			Game_Mode = 1;
			CPU_Mode = 1;
			break;

		case 3:
			Game_Mode = 0;
			CPU_Mode = 1;
			break;
	}

	if (CPU_Mode == 1)
		sprintf_s(Str_Err, 256, "Gens - 32X (PAL) : %s",MD_Rom->Rom_Name_W);
	else
		sprintf_s(Str_Err, 256, "Gens - 32X (NTSC) : %s",MD_Rom->Rom_Name_W);

	SetWindowText(HWnd, Str_Err);

	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	 	// Version de la megadrive (0x0 - 0xF)

	memcpy(_32X_Rom, Rom_Data, 4 * 1024 * 1024);	// no byteswapped image (for SH2)
	Byte_Swap(Rom_Data, Rom_Size);					// byteswapped image (for 68000)

	MSH2_Reset();
	SSH2_Reset();
	M68K_Reset(1);
	Z80_Reset();
	Reset_VDP();
	_32X_VDP_Reset();
	_32X_Set_FB();
	PWM_Init();

	if (CPU_Mode)
	{
		CPL_Z80 = Round_Double((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = Round_Double((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) SSH2_Speed) / 100.0);

		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;

		YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, Sound_Rate);
	}
	else
	{
		CPL_Z80 = Round_Double((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = Round_Double((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = Round_Double(((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double(((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) SSH2_Speed) / 100.0);

		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;
		_32X_VDP.Mode |= 0x8000;

		YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
	}

	_32X_VDP.State |= 0x2000;

	if (Auto_Fix_CS) Fix_Checksum();

	if (Sound_Enable)
	{
		End_Sound();

		if (!Init_Sound(HWnd)) Sound_Enable = 0;
		else Play_Sound();
	}

	Load_Patch_File();
	Build_Main_Menu();

	Last_Time = GetTickCount();
	New_Time = 0;
	Used_Time = 0;

	Update_Frame = Do_32X_Frame;
	Update_Frame_Fast = Do_32X_Frame_No_VDP;

	// We patch the Master SH2 bios with ROM bios
	// this permit 32X games with older BIOS version to run correctly
	// Ecco 32X demo needs it

	for(i = 0; i < 0x400; i++) _32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];

	return 1;
}


void Reset_32X()
{
	int i;

	Paused = 0;
	Controller_1_COM = Controller_2_COM = 0;
	_32X_ADEN = _32X_RES = _32X_FM = _32X_RV = 0;

	if (Rom_Size <= (2 * 1024 * 1024))
	{
		SRAM_ON = 1;
		SRAM_Write = 1;
	}
	else
	{
		SRAM_ON = 0;
		SRAM_Write = 0;
	}

	MSH2_Reset();
	SSH2_Reset();
	M68K_Reset(1);
	Z80_Reset();
	Reset_VDP();
	_32X_VDP_Reset();
	_32X_Set_FB();
	YM2612_Reset();
	PWM_Init();

	if (CPU_Mode)
	{
		VDP_Status |= 1;
		_32X_VDP.Mode &= ~0x8000;
	}
	else
	{
		VDP_Status &= ~1;
		_32X_VDP.Mode |= 0x8000;
	}

	_32X_VDP.State |= 0x2000;

	if (Auto_Fix_CS) Fix_Checksum();

	// We patch the Master SH2 bios with ROM bios
	// this permit 32X games with older BIOS version to run correctly
	// Ecco 32X demo needs it

	for(i = 0; i < 0x400; i++) _32X_MSH2_Rom[i + 0x36C] = _32X_Rom[i + 0x400];
}


int Do_32X_Frame_No_VDP()
{
	int i, j, k, l, p_i, p_j, p_k, p_l, *buf[2];
	int HInt_Counter, HInt_Counter_32X;
	int CPL_PWM;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	CPL_PWM = CPL_M68K * 3;

	PWM_Cycles = Cycles_SSH2 = Cycles_MSH2 = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;

	main68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);
	SH2_Clear_Odo(&M_SH2);
	SH2_Clear_Odo(&S_SH2);
	PWM_Clear_Timer();

	Patch_Codes();

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;							// Clear V Blank
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;
	_32X_VDP.State &= ~0x8000;

	HInt_Counter = VDP_Reg.H_Int;					// Hint_Counter = step d'interruption H
	HInt_Counter_32X = _32X_HIC;

	p_i = 84;
	p_j = (p_i * CPL_MSH2) / CPL_M68K;
	p_k = (p_i * CPL_SSH2) / CPL_M68K;
	p_l = p_i * 3;

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;			// HBlank = 1
		_32X_VDP.State |= 0x6000;

		main68k_exec(i - p_i);
		SH2_Exec(&M_SH2, j - p_j);
		SH2_Exec(&S_SH2, k - p_k);
		PWM_Update_Timer(l - p_l);

		VDP_Status &= ~0x0004;			// HBlank = 0
		_32X_VDP.State &= ~0x6000;

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
			if (_32X_MINT & 0x04) SH2_Interrupt(&M_SH2, 10);
			if (_32X_SINT & 0x04) SH2_Interrupt(&S_SH2, 10);
		}

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			SH2_Exec(&M_SH2, j);
			SH2_Exec(&S_SH2, k);
			PWM_Update_Timer(l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}

		main68k_exec(Cycles_M68K);
		SH2_Exec(&M_SH2, Cycles_MSH2);
		SH2_Exec(&S_SH2, Cycles_SSH2);
		PWM_Update_Timer(PWM_Cycles);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}

	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

	i = Cycles_M68K + p_i;
	j = Cycles_MSH2 + p_j;
	k = Cycles_SSH2 + p_k;
	l = PWM_Cycles + p_l;

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_MSH2 += CPL_MSH2;
	Cycles_SSH2 += CPL_SSH2;
	Cycles_Z80 += CPL_Z80;
	PWM_Cycles += CPL_PWM;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	if (--HInt_Counter_32X < 0)
	{
		HInt_Counter_32X = _32X_HIC;
		if (_32X_MINT & 0x04) SH2_Interrupt(&M_SH2, 10);
		if (_32X_SINT & 0x04) SH2_Interrupt(&S_SH2, 10);
	}

	VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	_32X_VDP.State |= 0xE000;		// VBlank = 1, HBlank = 1, PEN = 1

	if (_32X_VDP.State & 0x10000) _32X_VDP.State |= 1;
	else _32X_VDP.State &= ~1;

	_32X_Set_FB();

	while (i < (Cycles_M68K - 360))
	{
		main68k_exec(i);
		SH2_Exec(&M_SH2, j);
		SH2_Exec(&S_SH2, k);
		PWM_Update_Timer(l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}

	main68k_exec(Cycles_M68K - 360);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= ~0x0004;			// HBlank = 0
	_32X_VDP.State &= ~0x4000;
	VDP_Status |= 0x0080;			// V Int happened

	VDP_Int |= 0x8;
	Update_IRQ_Line();
	if (_32X_MINT & 0x08) SH2_Interrupt(&M_SH2, 12);
	if (_32X_SINT & 0x08) SH2_Interrupt(&S_SH2, 12);
	z80_Interrupt(&M_Z80, 0xFF);

	while (i < Cycles_M68K)
	{
		main68k_exec(i);
		SH2_Exec(&M_SH2, j);
		SH2_Exec(&S_SH2, k);
		PWM_Update_Timer(l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}

	main68k_exec(Cycles_M68K);
	SH2_Exec(&M_SH2, Cycles_MSH2);
	SH2_Exec(&S_SH2, Cycles_SSH2);
	PWM_Update_Timer(PWM_Cycles);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;			// HBlank = 1
		_32X_VDP.State |= 0x6000;

		main68k_exec(i - p_i);
		SH2_Exec(&M_SH2, j - p_j);
		SH2_Exec(&S_SH2, k - p_k);
		PWM_Update_Timer(l - p_l);

		VDP_Status &= ~0x0004;			// HBlank = 0
		_32X_VDP.State &= ~0x6000;

		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
			if ((_32X_MINT & 0x04) && (_32X_MINT & 0x80)) SH2_Interrupt(&M_SH2, 10);
			if ((_32X_SINT & 0x04) && (_32X_SINT & 0x80)) SH2_Interrupt(&S_SH2, 10);
		}

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			SH2_Exec(&M_SH2, j);
			SH2_Exec(&S_SH2, k);
			PWM_Update_Timer(l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}

		main68k_exec(Cycles_M68K);
		SH2_Exec(&M_SH2, Cycles_MSH2);
		SH2_Exec(&S_SH2, Cycles_SSH2);
		PWM_Update_Timer(PWM_Cycles);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}

	PSG_Special_Update();
	YM2612_Special_Update();

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	return 1;
}


int Do_32X_Frame()
{
	int i, j, k, l, p_i, p_j, p_k, p_l, *buf[2];
	int HInt_Counter, HInt_Counter_32X;
	int CPL_PWM;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	CPL_PWM = CPL_M68K * 3;

	PWM_Cycles = Cycles_SSH2 = Cycles_MSH2 = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;

	main68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);
	SH2_Clear_Odo(&M_SH2);
	SH2_Clear_Odo(&S_SH2);
	PWM_Clear_Timer();

	Patch_Codes();

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;							// Clear V Blank
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;
	_32X_VDP.State &= ~0x8000;

	HInt_Counter = VDP_Reg.H_Int;					// Hint_Counter = step d'interruption H
	HInt_Counter_32X = _32X_HIC;

	p_i = 84;
	p_j = (p_i * CPL_MSH2) / CPL_M68K;
	p_k = (p_i * CPL_SSH2) / CPL_M68K;
	p_l = p_i * 3;

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;			// HBlank = 1
		_32X_VDP.State |= 0x6000;

		main68k_exec(i - p_i);
		SH2_Exec(&M_SH2, j - p_j);
		SH2_Exec(&S_SH2, k - p_k);
		PWM_Update_Timer(l - p_l);

		VDP_Status &= ~0x0004;			// HBlank = 0
		_32X_VDP.State &= ~0x6000;

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
			if (_32X_MINT & 0x04) SH2_Interrupt(&M_SH2, 10);
			if (_32X_SINT & 0x04) SH2_Interrupt(&S_SH2, 10);
		}

		Render_Line_32X();

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			SH2_Exec(&M_SH2, j);
			SH2_Exec(&S_SH2, k);
			PWM_Update_Timer(l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}

		main68k_exec(Cycles_M68K);
		SH2_Exec(&M_SH2, Cycles_MSH2);
		SH2_Exec(&S_SH2, Cycles_SSH2);
		PWM_Update_Timer(PWM_Cycles);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}

	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

	i = Cycles_M68K + p_i;
	j = Cycles_MSH2 + p_j;
	k = Cycles_SSH2 + p_k;
	l = PWM_Cycles + p_l;

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_MSH2 += CPL_MSH2;
	Cycles_SSH2 += CPL_SSH2;
	Cycles_Z80 += CPL_Z80;
	PWM_Cycles += CPL_PWM;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	if (--HInt_Counter_32X < 0)
	{
		HInt_Counter_32X = _32X_HIC;
		if (_32X_MINT & 0x04) SH2_Interrupt(&M_SH2, 10);
		if (_32X_SINT & 0x04) SH2_Interrupt(&S_SH2, 10);
	}

	VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	_32X_VDP.State |= 0xE000;		// VBlank = 1, HBlank = 1, PEN = 1

	if (_32X_VDP.State & 0x10000) _32X_VDP.State |= 1;
	else _32X_VDP.State &= ~1;

	_32X_Set_FB();

	while (i < (Cycles_M68K - 360))
	{
		main68k_exec(i);
		SH2_Exec(&M_SH2, j);
		SH2_Exec(&S_SH2, k);
		PWM_Update_Timer(l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}

	main68k_exec(Cycles_M68K - 360);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= ~0x0004;			// HBlank = 0
	_32X_VDP.State &= ~0x4000;
	VDP_Status |= 0x0080;			// V Int happened

	VDP_Int |= 0x8;
	Update_IRQ_Line();
	if (_32X_MINT & 0x08) SH2_Interrupt(&M_SH2, 12);
	if (_32X_SINT & 0x08) SH2_Interrupt(&S_SH2, 12);
	z80_Interrupt(&M_Z80, 0xFF);

	while (i < Cycles_M68K)
	{
		main68k_exec(i);
		SH2_Exec(&M_SH2, j);
		SH2_Exec(&S_SH2, k);
		PWM_Update_Timer(l);
		i += p_i;
		j += p_j;
		k += p_k;
		l += p_l;
	}

	main68k_exec(Cycles_M68K);
	SH2_Exec(&M_SH2, Cycles_MSH2);
	SH2_Exec(&S_SH2, Cycles_SSH2);
	PWM_Update_Timer(PWM_Cycles);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		PWM_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];

		i = Cycles_M68K + (p_i * 2);
		j = Cycles_MSH2 + (p_j * 2);
		k = Cycles_SSH2 + (p_k * 2);
		l = PWM_Cycles + (p_l * 2);

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_MSH2 += CPL_MSH2;
		Cycles_SSH2 += CPL_SSH2;
		Cycles_Z80 += CPL_Z80;
		PWM_Cycles += CPL_PWM;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;			// HBlank = 1
		_32X_VDP.State |= 0x6000;

		main68k_exec(i - p_i);
		SH2_Exec(&M_SH2, j - p_j);
		SH2_Exec(&S_SH2, k - p_k);
		PWM_Update_Timer(l - p_l);

		VDP_Status &= ~0x0004;			// HBlank = 0
		_32X_VDP.State &= ~0x6000;

		if (--HInt_Counter_32X < 0)
		{
			HInt_Counter_32X = _32X_HIC;
			if ((_32X_MINT & 0x04) && (_32X_MINT & 0x80)) SH2_Interrupt(&M_SH2, 10);
			if ((_32X_SINT & 0x04) && (_32X_SINT & 0x80)) SH2_Interrupt(&S_SH2, 10);
		}

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			SH2_Exec(&M_SH2, j);
			SH2_Exec(&S_SH2, k);
			PWM_Update_Timer(l);
			i += p_i;
			j += p_j;
			k += p_k;
			l += p_l;
		}

		main68k_exec(Cycles_M68K);
		SH2_Exec(&M_SH2, Cycles_MSH2);
		SH2_Exec(&S_SH2, Cycles_SSH2);
		PWM_Update_Timer(PWM_Cycles);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);
	}

	PSG_Special_Update();
	YM2612_Special_Update();

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	return 1;
}



/*************************************/
/*              SEGA CD              */
/*************************************/


int Init_SegaCD(char *iso_name)
{
	char Str_Err[256], *Bios_To_Use;

	SetWindowText(HWnd, "Gens - Sega CD : initialising, please wait ...");

	if (Reset_CD((char *) CD_Data, iso_name))
	{
		SetWindowText(HWnd, "Gens - Idle");
		return 0;
	}

	switch(Country)
	{
		default:
		case -1:
			Bios_To_Use = Detect_Country_SegaCD();
			break;

		case 0:
			Game_Mode = 0;
			CPU_Mode = 0;
			Bios_To_Use = JA_CD_Bios;
			break;

		case 1:
			Game_Mode = 1;
			CPU_Mode = 0;
			Bios_To_Use = US_CD_Bios;
			break;

		case 2:
			Game_Mode = 1;
			CPU_Mode = 1;
			Bios_To_Use = EU_CD_Bios;
			break;

		case 3:
			Game_Mode = 0;
			CPU_Mode = 1;
			Bios_To_Use = JA_CD_Bios;
			break;
	}

	if (Load_Bios(HWnd, Bios_To_Use) == NULL)
	{
		MessageBox(NULL, "Your BIOS files aren't correctly configured, do it with 'Option -> BIOS/Misc Files...' menu.", "Warning", MB_OK | MB_ICONEXCLAMATION);
		SetWindowText(HWnd, "Gens - Idle");
		return 0;
	}

	Update_CD_Rom_Name((char *) &CD_Data[32]);

	if ((CPU_Mode == 1) || (Game_Mode == 0))
		sprintf_s(Str_Err, 256, "Gens - MegaCD : %s", Rom_Name);
	else
		sprintf_s(Str_Err, 256, "Gens - SegaCD : %s", Rom_Name);

	SetWindowText(HWnd, Str_Err);

	Flag_Clr_Scr = 1;
	Debug = Paused = Frame_Number = 0;
	SRAM_Start = SRAM_End = SRAM_ON = SRAM_Write = 0;
	BRAM_Ex_State &= 0x100;
	Controller_1_COM = Controller_2_COM = 0;
	
	if (CPU_Mode)
	{
		CPL_Z80 = Round_Double((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = Round_Double((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) SSH2_Speed) / 100.0);

		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;

		CD_Access_Timer = 2080;
		Timer_Step = 136752;
	}
	else
	{
		CPL_Z80 = Round_Double((((double) CLOCK_NTSC / 15.0) / 60.0) / 262.0);
		CPL_M68K = Round_Double((((double) CLOCK_NTSC / 7.0) / 60.0) / 262.0);
		CPL_MSH2 = Round_Double(((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double(((((((double) CLOCK_NTSC / 7.0) * 3.0) / 60.0) / 262.0) * (double) SSH2_Speed) / 100.0);

		VDP_Num_Lines = 262;
		VDP_Status &= 0xFFFE;

		CD_Access_Timer = 2096;
		Timer_Step = 135708;
	}

	VDP_Num_Vis_Lines = 224;
	Gen_Version = 0x20 + 0x0;	 	// Version de la megadrive (0x0 - 0xF)

	Rom_Data[0x72] = 0xFF;
	Rom_Data[0x73] = 0xFF;
	Byte_Swap(Rom_Data, Rom_Size);

	M68K_Reset(2);
	S68K_Reset();
	Z80_Reset();
	Reset_VDP();
	LC89510_Reset();
	Init_RS_GFX();

	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_PAL / 15, Sound_Rate);
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
		PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
	}

	Init_PCM(Sound_Rate);

	if (Sound_Enable)
	{
		End_Sound();

		if (!Init_Sound(HWnd)) Sound_Enable = 0;
		else Play_Sound();
	}

	Load_BRAM();				// Initialise BRAM
	Load_Patch_File();			// Only used to reset Patch structure
	Build_Main_Menu();

	Last_Time = GetTickCount();
	New_Time = 0;
	Used_Time = 0;

	if (SegaCD_Accurate)
	{
		Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
	}
	else
	{
		Update_Frame = Do_SegaCD_Frame;
		Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
	}

	return 1;
}


int Reload_SegaCD(char *iso_name)
{
	char Str_Err[256];

	Save_BRAM();

	SetWindowText(HWnd, "Gens - Sega CD : re-initialising, please wait ...");

	Reset_CD((char *) CD_Data, iso_name);
	Update_CD_Rom_Name((char *) &CD_Data[32]);

	if ((CPU_Mode == 1) || (Game_Mode == 0)) sprintf(Str_Err, "Gens - MegaCD : %s", Rom_Name);
	else sprintf(Str_Err, "Gens - SegaCD : %s", Rom_Name);

	SetWindowText(HWnd, Str_Err);

	Load_BRAM();

	return 1;
}


void Reset_SegaCD()
{
	char *Bios_To_Use;

	if (CPU_Mode) Bios_To_Use = EU_CD_Bios;
	else if (Game_Mode) Bios_To_Use = US_CD_Bios;
	else Bios_To_Use = JA_CD_Bios;
							
	SetCurrentDirectory(Gens_Path);

	if (Detect_Format(Bios_To_Use) == -1)
	{
		MessageBox(HWnd, "Some bios files are missing !\nConfigure them in the option menu.\n", "Warning", MB_ICONEXCLAMATION | MB_OK);
		return;
	}

	Controller_1_COM = Controller_2_COM = 0;
	SRAM_ON = 0;
	SRAM_Write = 0;
	Paused = 0;
	BRAM_Ex_State &= 0x100;

	if (!_stricmp("ZIP", &Bios_To_Use[strlen(Bios_To_Use) - 3]))
	{
		Game = Load_Rom_Zipped(HWnd, Bios_To_Use, 0);
	}
	else
	{
		Game = Load_Rom(HWnd, Bios_To_Use, 0);
	}

	Update_CD_Rom_Name((char *) &CD_Data[32]);

	Rom_Data[0x72] = 0xFF;
	Rom_Data[0x73] = 0xFF;

	Byte_Swap(Rom_Data, Rom_Size);

	M68K_Reset(2);
	S68K_Reset();
	Z80_Reset();
	LC89510_Reset();
	Reset_VDP();
	Init_RS_GFX();
	Reset_PCM();
	YM2612_Reset();

	if (CPU_Mode) VDP_Status |= 1;
	else VDP_Status &= ~1;
}


int Do_SegaCD_Frame_No_VDP(void)
{
	int *buf[2];
	int HInt_Counter;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	CPL_S68K = 795;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	sub68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;		// Hint_Counter = step H interrupt

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;					// HBlank = 0

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert();

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1) Cycles_S68K += CPL_S68K;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec(Cycles_M68K - 360);
	sub68k_exec(Cycles_S68K - 586);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;			// HBlank = 0
	VDP_Status |= 0x0080;			// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line();
	z80_Interrupt(&M_Z80, 0xFF);

	main68k_exec(Cycles_M68K);
	sub68k_exec(Cycles_S68K);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	Update_SegaCD_Timer();

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;					// HBlank = 0

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}

	buf[0] = Seg_L;
	buf[1] = Seg_R;

	PSG_Special_Update();
	YM2612_Special_Update();
	Update_CD_Audio(buf, Seg_Lenght);

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	return(1);
}


int Do_SegaCD_Frame_No_VDP_Cycle_Accurate(void)
{
	int *buf[2], i, j;
	int HInt_Counter;

	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	CPL_S68K = 795;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	sub68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;		// Hint_Counter = step H interrupt

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1

		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec(i);
			i += 24;

			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K - 404);
		sub68k_exec(Cycles_S68K - 658);

		/* end instruction by instruction execution */

		VDP_Status &= 0xFFFB;					// HBlank = 0

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			i += 24;

			if (j < Cycles_S68K)
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);

		/* end instruction by instruction execution */

		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}
	
	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert();

	i = Cycles_M68K + 24;
	j = Cycles_S68K + 39;

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1) Cycles_S68K += CPL_S68K;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	VDP_Status |= 0x000C;			// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)

	/* instruction by instruction execution */

	while (i < (Cycles_M68K - 360))
	{
		main68k_exec(i);
		i += 24;

		if (j < (Cycles_S68K - 586))
		{
			sub68k_exec(j);
			j += 39;
		}
	}

	main68k_exec(Cycles_M68K - 360);
	sub68k_exec(Cycles_S68K - 586);

	/* end instruction by instruction execution */

	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;			// HBlank = 0
	VDP_Status |= 0x0080;			// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line();
	z80_Interrupt(&M_Z80, 0xFF);

	/* instruction by instruction execution */
		
	while (i < Cycles_M68K)
	{
		main68k_exec(i);
		i += 24;

		if (j < Cycles_S68K)
		{
			sub68k_exec(j);
			j += 39;
		}
	}

	main68k_exec(Cycles_M68K);
	sub68k_exec(Cycles_S68K);

	/* end instruction by instruction execution */

	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	Update_SegaCD_Timer();

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1

		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec(i);
			i += 24;

			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K - 404);
		sub68k_exec(Cycles_S68K - 658);

		/* end instruction by instruction execution */

		VDP_Status &= 0xFFFB;					// HBlank = 0

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			i += 24;

			if (j < Cycles_S68K)
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);

		/* end instruction by instruction execution */

		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}

	buf[0] = Seg_L;
	buf[1] = Seg_R;

	PSG_Special_Update();
	YM2612_Special_Update();
	Update_CD_Audio(buf, Seg_Lenght);

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	return(1);
}


int Do_SegaCD_Frame(void)
{
	int *buf[2];
	int HInt_Counter;
 
	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	CPL_S68K = 795;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	sub68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;		// Hint_Counter = step d'interruption H

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;			// HBlank = 1
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;			// HBlank = 0

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		Render_Line();

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}

	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert();

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1) Cycles_S68K += CPL_S68K;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	VDP_Status |= 0x000C;				// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)
	main68k_exec(Cycles_M68K - 360);
	sub68k_exec(Cycles_S68K - 586);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;				// HBlank = 0
	VDP_Status |= 0x0080;				// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line();
	z80_Interrupt(&M_Z80, 0xFF);

	main68k_exec(Cycles_M68K);
	sub68k_exec(Cycles_S68K);
	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	Update_SegaCD_Timer();

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1
		main68k_exec(Cycles_M68K - 404);
		VDP_Status &= 0xFFFB;					// HBlank = 0

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);
		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}

	buf[0] = Seg_L;
	buf[1] = Seg_R;

	PSG_Special_Update();
	YM2612_Special_Update();
	Update_CD_Audio(buf, Seg_Lenght);

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	if (Show_LED)
	{
		if (LED_Status & 2)
		{
			MD_Screen[336 * 220 + 12] = 0x03E0;
			MD_Screen[336 * 220 + 13] = 0x03E0;
			MD_Screen[336 * 220 + 14] = 0x03E0;
			MD_Screen[336 * 220 + 15] = 0x03E0;
			MD_Screen[336 * 222 + 12] = 0x03E0;
			MD_Screen[336 * 222 + 13] = 0x03E0;
			MD_Screen[336 * 222 + 14] = 0x03E0;
			MD_Screen[336 * 222 + 15] = 0x03E0;
		}

		if (LED_Status & 1)
		{
			MD_Screen[336 * 220 + 12 + 8] = 0xF800;
			MD_Screen[336 * 220 + 13 + 8] = 0xF800;
			MD_Screen[336 * 220 + 14 + 8] = 0xF800;
			MD_Screen[336 * 220 + 15 + 8] = 0xF800;
			MD_Screen[336 * 222 + 12 + 8] = 0xF800;
			MD_Screen[336 * 222 + 13 + 8] = 0xF800;
			MD_Screen[336 * 222 + 14 + 8] = 0xF800;
			MD_Screen[336 * 222 + 15 + 8] = 0xF800;
		}
	}

	return(1);
}


int Do_SegaCD_Frame_Cycle_Accurate(void)
{
	int *buf[2], i, j;
	int HInt_Counter;
 
	if ((CPU_Mode) && (VDP_Reg.Set2 & 0x8))	VDP_Num_Vis_Lines = 240;
	else VDP_Num_Vis_Lines = 224;

	CPL_S68K = 795;

	YM_Buf[0] = PSG_Buf[0] = Seg_L;
	YM_Buf[1] = PSG_Buf[1] = Seg_R;
	YM_Len = PSG_Len = 0;

	Cycles_S68K = Cycles_M68K = Cycles_Z80 = 0;
	Last_BUS_REQ_Cnt = -1000;
	main68k_tripOdometer();
	sub68k_tripOdometer();
	z80_Clear_Odo(&M_Z80);

	VRam_Flag = 1;

	VDP_Status &= 0xFFF7;
	if (VDP_Reg.Set4 & 0x2) VDP_Status ^= 0x0010;

	HInt_Counter = VDP_Reg.H_Int;		// Hint_Counter = step d'interruption H

	for(VDP_Current_Line = 0; VDP_Current_Line < VDP_Num_Vis_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;			// HBlank = 1

		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec(i);
			i += 24;

			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K - 404);
		sub68k_exec(Cycles_S68K - 658);

		/* end instruction by instruction execution */

		VDP_Status &= 0xFFFB;			// HBlank = 0

		if (--HInt_Counter < 0)
		{
			HInt_Counter = VDP_Reg.H_Int;
			VDP_Int |= 0x4;
			Update_IRQ_Line();
		}

		Render_Line();

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			i += 24;

			if (j < Cycles_S68K)
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);

		/* end instruction by instruction execution */

		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}

	buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
	buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
	if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
	YM_Len += Sound_Extrapol[VDP_Current_Line][1];
	PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
	Update_CDC_TRansfert();

	i = Cycles_M68K + 24;
	j = Cycles_S68K + 39;

	Fix_Controllers();
	Cycles_M68K += CPL_M68K;
	Cycles_Z80 += CPL_Z80;
	if (S68K_State == 1) Cycles_S68K += CPL_S68K;
	if (DMAT_Lenght) main68k_addCycles(Update_DMA());

	if (--HInt_Counter < 0)
	{
		VDP_Int |= 0x4;
		Update_IRQ_Line();
	}

	VDP_Status |= 0x000C;				// VBlank = 1 et HBlank = 1 (retour de balayage vertical en cours)

	/* instruction by instruction execution */

	while (i < (Cycles_M68K - 360))
	{
		main68k_exec(i);
		i += 24;

		if (j < (Cycles_S68K - 586))
		{
			sub68k_exec(j);
			j += 39;
		}
	}

	main68k_exec(Cycles_M68K - 360);
	sub68k_exec(Cycles_S68K - 586);

	/* end instruction by instruction execution */

	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80 - 168);
	else z80_Set_Odo(&M_Z80, Cycles_Z80 - 168);

	VDP_Status &= 0xFFFB;				// HBlank = 0
	VDP_Status |= 0x0080;				// V Int happened
	VDP_Int |= 0x8;
	Update_IRQ_Line();
	z80_Interrupt(&M_Z80, 0xFF);

	/* instruction by instruction execution */
		
	while (i < Cycles_M68K)
	{
		main68k_exec(i);
		i += 24;

		if (j < Cycles_S68K)
		{
			sub68k_exec(j);
			j += 39;
		}
	}

	main68k_exec(Cycles_M68K);
	sub68k_exec(Cycles_S68K);

	/* end instruction by instruction execution */

	if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
	else z80_Set_Odo(&M_Z80, Cycles_Z80);

	Update_SegaCD_Timer();

	for(VDP_Current_Line++; VDP_Current_Line < VDP_Num_Lines; VDP_Current_Line++)
	{
		buf[0] = Seg_L + Sound_Extrapol[VDP_Current_Line][0];
		buf[1] = Seg_R + Sound_Extrapol[VDP_Current_Line][0];
		if (PCM_Enable) Update_PCM(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM2612_DacAndTimers_Update(buf, Sound_Extrapol[VDP_Current_Line][1]);
		YM_Len += Sound_Extrapol[VDP_Current_Line][1];
		PSG_Len += Sound_Extrapol[VDP_Current_Line][1];
		Update_CDC_TRansfert();

		i = Cycles_M68K + 24;
		j = Cycles_S68K + 39;

		Fix_Controllers();
		Cycles_M68K += CPL_M68K;
		Cycles_Z80 += CPL_Z80;
		if (S68K_State == 1) Cycles_S68K += CPL_S68K;
		if (DMAT_Lenght) main68k_addCycles(Update_DMA());

		VDP_Status |= 0x0004;					// HBlank = 1

		/* instruction by instruction execution */
		
		while (i < (Cycles_M68K - 404))
		{
			main68k_exec(i);
			i += 24;

			if (j < (Cycles_S68K - 658))
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K - 404);
		sub68k_exec(Cycles_S68K - 658);

		/* end instruction by instruction execution */

		VDP_Status &= 0xFFFB;					// HBlank = 0

		/* instruction by instruction execution */
		
		while (i < Cycles_M68K)
		{
			main68k_exec(i);
			i += 24;					// Chuck Rock intro need faster timing ... strange.

			if (j < Cycles_S68K)
			{
				sub68k_exec(j);
				j += 39;
			}
		}

		main68k_exec(Cycles_M68K);
		sub68k_exec(Cycles_S68K);

		/* end instruction by instruction execution */

		if (Z80_State == 3) z80_Exec(&M_Z80, Cycles_Z80);
		else z80_Set_Odo(&M_Z80, Cycles_Z80);

		Update_SegaCD_Timer();
	}

	buf[0] = Seg_L;
	buf[1] = Seg_R;

	PSG_Special_Update();
	YM2612_Special_Update();
	Update_CD_Audio(buf, Seg_Lenght);

	if (WAV_Dumping) Update_WAV_Dump();
	if (GYM_Dumping) Update_GYM_Dump((unsigned char) 0, (unsigned char) 0, (unsigned char) 0);

	if (Show_LED)
	{
		if (LED_Status & 2)
		{
			MD_Screen[336 * 220 + 12] = 0x03E0;
			MD_Screen[336 * 220 + 13] = 0x03E0;
			MD_Screen[336 * 220 + 14] = 0x03E0;
			MD_Screen[336 * 220 + 15] = 0x03E0;
			MD_Screen[336 * 222 + 12] = 0x03E0;
			MD_Screen[336 * 222 + 13] = 0x03E0;
			MD_Screen[336 * 222 + 14] = 0x03E0;
			MD_Screen[336 * 222 + 15] = 0x03E0;
		}

		if (LED_Status & 1)
		{
			MD_Screen[336 * 220 + 12 + 8] = 0xF800;
			MD_Screen[336 * 220 + 13 + 8] = 0xF800;
			MD_Screen[336 * 220 + 14 + 8] = 0xF800;
			MD_Screen[336 * 220 + 15 + 8] = 0xF800;
			MD_Screen[336 * 222 + 12 + 8] = 0xF800;
			MD_Screen[336 * 222 + 13 + 8] = 0xF800;
			MD_Screen[336 * 222 + 14 + 8] = 0xF800;
			MD_Screen[336 * 222 + 15 + 8] = 0xF800;
		}
	}

	return 1;
}