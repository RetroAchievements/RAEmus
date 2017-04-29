#include "gens.h"

#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <stdio.h>
#include <string.h>

//	##RA
#include "BuildVer.h"
#include "RA_Resource.h"
#include "RA_Interface.h"
#include "RA_Implementation.h"

#include "DialogProc.h"

 
#include "G_main.h"
#include "G_ddraw.h"
#include "G_dsound.h"
#include "G_Input.h"
#include "Debug.h"
#include "rom.h"
#include "save.h"
#include "resource.h"
#include "misc.h"
#include "blit.h"
#include "Cpu_68k.h"
#include "Star_68k.h"
#include "Cpu_SH2.h"
#include "Cpu_Z80.h"
#include "z80.h"
#include "mem_M68K.h"
#include "mem_S68K.h"
#include "mem_SH2.h"
#include "mem_Z80.h"
#include "io.h"
#include "psg.h"
#include "ym2612.h"
#include "pwm.h"
#include "scrshot.h"
#include "vdp_io.h"
#include "vdp_rend.h"
#include "vdp_32X.h"
#include "LC89510.h"
#include "gfx_cd.h"
#include "cd_aspi.h"
#include "net.h"
#include "pcm.h"
#include "htmlhelp.h"
#include "CCnet.h"
#include "wave.h"

#include <CommCtrl.h>
#include <direct.h>


extern "C" void Read_To_68K_Space(int adr);

#define WM_KNUX WM_USER + 3
#define GENS_VERSION   2.10
#define GENS_VERSION_H 2 * 65536 + 10

#define MINIMIZE								\
if (Sound_Initialised) Clear_Sound_Buffer();	\
if (Full_Screen)								\
{												\
	Set_Render(hWnd, 0, -1, false);				\
}

#define MENU_L(smenu, pos, flags, id, str, suffixe, def)										\
GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
strcat_s( Str_Tmp, (suffixe) );																	\
InsertMenu((smenu), (pos), (flags), (id), Str_Tmp);

#define WORD_L(id, str, suffixe, def)															\
GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
strcat_s( Str_Tmp, (suffixe) );																	\
SetDlgItemText(hDlg, id, Str_Tmp);

#define MESSAGE_L(str, def, time)																	\
{																									\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
	Put_Info(Str_Tmp, (time));																		\
}

#define MESSAGE_NUM_L(str, def, num, time)															\
{																									\
	char mes_tmp[1024];																				\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
	sprintf_s(mes_tmp, 1024, Str_Tmp, (num));														\
	Put_Info(mes_tmp, (time));																		\
}


HINSTANCE ghInstance;
HACCEL hAccelTable;
WNDCLASS WndClass;
HWND HWnd;
HMENU Gens_Menu;

char Str_Tmp[1024];
char Gens_Path[1024];
char Language_Path[1024];
char CGOffline_Path[1024];
char Manual_Path[1024];

char **language_name = NULL;
struct Rom *Game = NULL;
int Active = 0;
int Paused = 0;
int Net_Play = 0;
int Full_Screen = -1;
int Resolution = 1;
int Fast_Blur = 0;
int Render_W = 1;
int Render_FS = 1;
int Show_FPS = 0;
int Show_Message = 0;
int Show_LED = 0;
int Auto_Pause = 0;
int Auto_Fix_CS = 0;
int Language = 0;
int Country = -1;
int Country_Order[3];
int Kaillera_Client_Running = 0;
int Intro_Style = 0;
int SegaCD_Accurate = 0;
int Gens_Running = 0;
int WinNT_Flag = 0;
int Gens_Priority;
int SS_Actived;

POINT Window_Pos;

LRESULT WINAPI WinProc(HWND, UINT, WPARAM, LPARAM);
int Set_Render(HWND hWnd, int Full, int Num, BOOL fForceWindowReposition);
HMENU Build_Main_Menu(void);


int Change_VSync(HWND hWnd)
{
	int *p_vsync;
	
	if (Full_Screen)
	{
		End_DDraw();
		p_vsync = &FS_VSync;
	}
	else p_vsync = &W_VSync;
	
	*p_vsync = 1 - *p_vsync;
	
	if (*p_vsync) MESSAGE_L("Vertical Sync Enabled", "Vertical Sync Enabled", 1000)
	else MESSAGE_L("Vertical Sync Disabled", "Vertical Sync Disabled", 1000)

	Build_Main_Menu();
	if (Full_Screen) return Init_DDraw(HWnd);
	else return 1;
}


int Set_Frame_Skip(HWND hWnd, int Num)
{
	Frame_Skip = Num;

	if (Frame_Skip != -1)
		MESSAGE_NUM_L("Frame skip set to %d", "Frame skip set to %d", Frame_Skip, 1500)
	else
		MESSAGE_L("Frame skip set to Auto", "Frame skip set to Auto", 1500)

	Build_Main_Menu();
	return(1);
}


int Set_Current_State(HWND hWnd, int Num)
{
	FILE *f;
	
	Current_State = Num;

	if (f = Get_State_File())
	{
		fclose(f);
		MESSAGE_NUM_L("SLOT %d [OCCUPIED]", "SLOT %d [OCCUPIED]", Current_State, 1500)
	}
	else
	{
		MESSAGE_NUM_L("SLOT %d [EMPTY]", "SLOT %d [EMPTY]", Current_State, 1500)
	}

	Build_Main_Menu();
	return 1;
}


int Change_Stretch(void)
{
	if ((Full_Screen) && (Render_FS > 1)) return(0);
	
	Flag_Clr_Scr = 1;

	if (Stretch = (1 - Stretch))
		MESSAGE_L("Stretched mode", "Stretched mode", 1000)
	else
		MESSAGE_L("Correct ratio mode", "Correct ratio mode", 1000)

	Build_Main_Menu();
	return(1);
}


int Change_Blit_Style(void)
{
	if ((!Full_Screen) || (Render_FS > 1)) return(0);

	Flag_Clr_Scr = 1;

	if (Blit_Soft = (1 - Blit_Soft))
		MESSAGE_L("Force software blit for Full-Screen", "Force software blit for Full-Screen", 1000)
	else
		MESSAGE_L("Enable hardware blit for Full-Screen", "Enable hardware blit for Full-Screen", 1000)

	return(1);
}


int Set_Sprite_Over(HWND hWnd, int Num)
{
	if (Sprite_Over = Num)
		MESSAGE_L("Sprite Limit Enabled", "Sprite Limit Enabled", 1000)
	else
		MESSAGE_L("Sprite Limit Disabled", "Sprite Limit Disabled", 1000)

	Build_Main_Menu();
	return(1);
}


int Change_Debug(HWND hWnd, int Debug_Mode)
{
	if (!Game) return 0;
		
	Flag_Clr_Scr = 1;
	Clear_Sound_Buffer();

	if (Debug_Mode == Debug) Debug = 0;
	else Debug = Debug_Mode;
	
	Build_Main_Menu();
	return 1;
}


int Change_Fast_Blur(HWND hWnd)
{
	Flag_Clr_Scr = 1;

	if (Fast_Blur = (1 - Fast_Blur))
		MESSAGE_L("Fast Blur Enabled", "Fast Blur Enabled", 1000)
	else
		MESSAGE_L("Fast Blur Disabled", "Fast Blur Disabled", 1000)

	Build_Main_Menu();
	return(1);
}


// Pass a value of -1 for "Num" to have Set_Render preserve the render mode
// while switching between full/windowed modes.
int Set_Render(HWND hWnd, int Full, int Num, BOOL fForceWindowReposition)
{
	Clear_Sound_Buffer();

	int Old_Rend, *Rend;
	void (**Blit)(unsigned char*, int, int, int, int);
	
	if (Full)
	{
		Rend = &Render_FS;
		Blit = &Blit_FS;
	}
	else
	{
		Rend = &Render_W;
		Blit = &Blit_W;
	}

	Old_Rend = *Rend;
	Flag_Clr_Scr = 1;

	switch(Num)
	{
		case -1:
			switch(Old_Rend)
			{
				case 0:
					if (Have_MMX) *Blit = Blit_X1_MMX;
					else *Blit = Blit_X1;
					break;

				case 1:
					if (Have_MMX) *Blit = Blit_X2_MMX;
					else *Blit = Blit_X2;
					break;

				case 2:
					if (Have_MMX) *Blit = Blit_X2_Int_MMX;
					else *Blit =  Blit_X2_Int;
					break;

				case 3:
					if (Have_MMX) *Blit = Blit_Scanline_MMX;
					else *Blit = Blit_Scanline;
					break;

				case 4:
					if (Have_MMX) *Blit = Blit_Scanline_50_MMX;
					else
					{
						*Rend = 1;
						*Blit = Blit_X2;
					}
					break;

				case 5:
					if (Have_MMX) *Blit = Blit_Scanline_25_MMX;
					else
					{
						*Rend = 1;
						*Blit = Blit_X2;
					}
					break;

				case 6:
					if (Have_MMX) *Blit = Blit_Scanline_Int_MMX;
					else *Blit = Blit_Scanline_Int;
					break;

				case 7:
					if (Have_MMX) *Blit = Blit_Scanline_50_Int_MMX;
					else
					{
						*Rend = 1;
						*Blit = Blit_X2;
					}
					break;

				case 8:
					if (Have_MMX) *Blit = Blit_Scanline_25_Int_MMX;
					else
					{
						*Rend = 1;
						*Blit = Blit_X2;
					}
					break;

				case 9:
					if (Have_MMX) *Blit = Blit_2xSAI_MMX;
					else
					{
						*Rend = 1;
						*Blit = Blit_X2;
					}
					break;

				default:
					*Rend = 1;
					if (Have_MMX) *Blit = Blit_X2_MMX;
					else *Blit = Blit_X2;
					break;
			}
			break;

		case 0:
			*Rend = 0;
			if (Have_MMX) *Blit = Blit_X1_MMX;
			else *Blit = Blit_X1;
			fForceWindowReposition = true;
			MESSAGE_L("Render selected : NORMAL", "Render selected : NORMAL", 1500)
			break;

		case 1:
			*Rend = 1;
			if (Have_MMX) *Blit = Blit_X2_MMX;
			else *Blit = Blit_X2;
			fForceWindowReposition = true;
			MESSAGE_L("Render selected : DOUBLE", "Render selected : DOUBLE", 1500)
			break;

		case 2:
			*Rend = 2;
			if (Have_MMX) *Blit = Blit_X2_Int_MMX;
			else *Blit = Blit_X2_Int;
			fForceWindowReposition = true;
			MESSAGE_L("Render selected : INTERPOLATED", "Render selected : INTERPOLATED", 1500)
			break;

		case 3:
			*Rend = 3;
			if (Have_MMX) *Blit = Blit_Scanline_MMX;
			else *Blit = Blit_Scanline;
			fForceWindowReposition = true;
			MESSAGE_L("Render selected : FULL SCANLINE", "Render selected : FULL SCANLINE", 1500)
			break;

		case 4:
			fForceWindowReposition = true;
			if (Have_MMX)
			{
				*Rend = 4;
				*Blit = Blit_Scanline_50_MMX;
				MESSAGE_L("Render selected : 50% SCANLINE", "Render selected : 50% SCANLINE", 1500)
			}
			else
			{
				*Rend = 6;
				*Blit = Blit_Scanline_Int;
				MESSAGE_L("Render selected : INTERPOLATED SCANLINE", "Render selected : INTERPOLATED SCANLINE", 1500)
			}
			break;

		case 5:
			fForceWindowReposition = true;
			if (Have_MMX)
			{
				*Rend = 5;
				*Blit = Blit_Scanline_25_MMX;
				MESSAGE_L("Render selected : 25% SCANLINE", "Render selected : 25% SCANLINE", 1500)
			}
			else
			{
				*Rend = 3;
				*Blit = Blit_Scanline;
				MESSAGE_L("Render selected : FULL SCANLINE", "Render selected : FULL SCANLINE", 1500)
			}
			break;

		case 6:
			fForceWindowReposition = true;
			*Rend = 6;
			if (Have_MMX) *Blit = Blit_Scanline_Int_MMX;
			else *Blit = Blit_Scanline_Int;
			MESSAGE_L("Render selected : INTERPOLATED SCANLINE", "Render selected : INTERPOLATED SCANLINE", 1500)
			break;

		case 7:
			fForceWindowReposition = true;
			if (Have_MMX)
			{
				*Rend = 7;
				*Blit = Blit_Scanline_50_Int_MMX;
				MESSAGE_L("Render selected : INTERPOLATED 50% SCANLINE", "Render selected : INTERPOLATED 50% SCANLINE", 1500)
			}
			else
			{
				*Rend = 6;
				*Blit = Blit_Scanline_Int;
				MESSAGE_L("Render selected : INTERPOLATED SCANLINE", "Render selected : INTERPOLATED SCANLINE", 1500)
			}
			break;

		case 8:
			fForceWindowReposition = true;
			if (Have_MMX)
			{
				*Rend = 8;
				*Blit = Blit_Scanline_25_Int_MMX;
				MESSAGE_L("Render selected : INTERPOLATED 25% SCANLINE", "Render selected : INTERPOLATED 25% SCANLINE", 1500)
			}
			else
			{
				*Rend = 6;
				*Blit = Blit_Scanline_Int;
				MESSAGE_L("Render selected : INTERPOLATED SCANLINE", "Render selected : INTERPOLATED SCANLINE", 1500)
			}
			break;

		case 9:
			fForceWindowReposition = true;
			if (Have_MMX)
			{
				*Rend = 9;
				*Blit = Blit_2xSAI_MMX;
				MESSAGE_L("Render selected : 2XSAI KREED'S ENGINE", "Render selected : 2XSAI KREED'S ENGINE", 1500)
			}
			else
			{
				*Rend = 6;
				*Blit = Blit_Scanline_Int;
				MESSAGE_L("Render selected : INTERPOLATED SCANLINE", "Render selected : INTERPOLATED SCANLINE", 1500)
			}
			break;

		default:
			fForceWindowReposition = true;
			*Rend = 1;
			if (Have_MMX) *Blit = Blit_X2_MMX;
			else *Blit = Blit_X2;
			MESSAGE_L("Render selected : DOUBLE", "Render selected : DOUBLE", 1500)
			break;
	}

	RECT r;
	
	if (Sound_Initialised) Clear_Sound_Buffer();
	End_DDraw();
	Full_Screen = Full;

	// The first render mode (0) is the "normal" mode, which runs in half the 
	// resolution of all the other modes.  So if we're switching between mode 0 and
	// any other mode, we have to reposition the window.
	// HACK -- if we skip window repositioning, then some weird bugs surface
	// that sometimes cause Gens to hang or not shut down cleanly!
	//if (fForceWindowReposition || (Num==0 && Old_Rend!=0) || (Num!=0 && Old_Rend==0))
	{
		if (Full)
		{
			while (ShowCursor(true) < 1);
			while (ShowCursor(false) >= 0);
			
			SetWindowLong(hWnd, GWL_STYLE, WS_VISIBLE| WS_MAXIMIZE);
			SetWindowPos(hWnd, NULL, 0, 0, 320 * ((*Rend == 0)?1:2), 240 * ((*Rend == 0)?1:2), SWP_NOZORDER | SWP_NOACTIVATE);
		}
		else
		{
 			DEVMODE dm;
 	
 			memset(&dm, 0, sizeof(DEVMODE));
 			dm.dmSize = sizeof(DEVMODE);
 			dm.dmBitsPerPel = 16;
 			dm.dmFields = DM_BITSPERPEL;
 	
 			ChangeDisplaySettings(&dm, 0);
 	
 			while (ShowCursor(false) >= 0);
 			while (ShowCursor(true) < 1);
 	
			if( fForceWindowReposition )
			{
 				SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_OVERLAPPEDWINDOW);
 				//GetWindowRect(hWnd, &r);
 				//if( r.bottom - r.top < 0 || r.right - r.left < 0 )
 				SetRect(&r, 0, 0, 320 * ((*Rend == 0)?1:2), 240 * ((*Rend == 0)?1:2));
	 
 				AdjustWindowRectEx(&r, GetWindowLong(hWnd, GWL_STYLE), 1, GetWindowLong(hWnd, GWL_EXSTYLE));
 				SetWindowPos(hWnd, NULL, Window_Pos.x, Window_Pos.y, r.right - r.left, r.bottom - r.top, SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}
	}
	
	Build_Main_Menu();
	int nResult = Init_DDraw(HWnd);
	
	RA_InitDirectX();

	return( nResult );
}


int Change_SegaCD_Synchro(void)
{
	if (SegaCD_Accurate)
	{
		SegaCD_Accurate = 0;

		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP;
		}

		MESSAGE_L("SegaCD normal mode", "SegaCD normal mode", 1500)
	}
	else
	{
		SegaCD_Accurate = 1;

		if (SegaCD_Started)
		{
			Update_Frame = Do_SegaCD_Frame_Cycle_Accurate;
			Update_Frame_Fast = Do_SegaCD_Frame_No_VDP_Cycle_Accurate;
		}

		MESSAGE_L("SegaCD perfect synchro mode (SLOW)", "SegaCD perfect synchro mode (slower)", 1500)
	}

	Build_Main_Menu();
	return 1;
}


int Change_SegaCD_SRAM_Size(int num)
{
	if (num == -1)
	{
		BRAM_Ex_State &= 1;
		MESSAGE_L("SegaCD SRAM cart removed", "SegaCD SRAM cart removed", 1500)
	}
	else
	{
		char bsize[256];
	
		BRAM_Ex_State |= 0x100;
		BRAM_Ex_Size = num;

		sprintf_s(bsize, 256, "SegaCD SRAM cart plugged (%d Kb)", 8 << num);
		MESSAGE_L(bsize, bsize, 1500)
	}

	Build_Main_Menu();
	return 1;
}


int Change_Z80(HWND hWnd)
{
	if (Z80_State & 1)
	{
		Z80_State &= ~1;
		MESSAGE_L("Z80 Disabled", "Z80 Disabled", 1000)
	}
	else
	{
		Z80_State |= 1;
		MESSAGE_L("Z80 Enabled", "Z80 Enabled", 1000)
	}

	Build_Main_Menu();
	return(1);
}


int Change_DAC(HWND hWnd)
{
	if (DAC_Enable)
	{
		DAC_Enable = 0;
		MESSAGE_L("DAC Disabled", "DAC Disabled", 1000)
	}
	else
	{
		DAC_Enable = 1;
		MESSAGE_L("DAC Enabled", "DAC Enabled", 1000)
	}

	Build_Main_Menu();
	return(1);
}


int Change_DAC_Improv(HWND hWnd)
{
	if (DAC_Improv)
	{
		DAC_Improv = 0;
		MESSAGE_L("Normal DAC sound", "Normal DAC sound", 1000)
	}
	else
	{
		DAC_Improv = 1;
		MESSAGE_L("Improved DAC sound (voices)", "Improved DAC sound (voices)", 1000)
	}

	return(1);
}


int Change_YM2612(HWND hWnd)
{
	if (YM2612_Enable)
	{
		YM2612_Enable = 0;
		MESSAGE_L("YM2612 Disabled", "YM2612 Disabled", 1000)
	}
	else
	{
		YM2612_Enable = 1;
		MESSAGE_L("YM2612 Enabled", "YM2612 Enabled", 1000)
	}

	Build_Main_Menu();
	return(1);
}


int Change_YM2612_Improv(HWND hWnd)
{
	unsigned char Reg_1[0x200];

	if (YM2612_Improv)
	{
		YM2612_Improv = 0;
		MESSAGE_L("Normal YM2612 emulation", "Normal YM2612 emulation", 1000)
	}
	else
	{
		YM2612_Improv = 1;
		MESSAGE_L("High Quality YM2612 emulation", "High Quality YM2612 emulation", 1000)
	}

	YM2612_Save(Reg_1);

	if (CPU_Mode)
	{
		YM2612_Init(CLOCK_PAL / 7, Sound_Rate, YM2612_Improv);
	}
	else
	{
		YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	}

	YM2612_Restore(Reg_1);

	Build_Main_Menu();
	return 1;
}


int Change_PSG(HWND hWnd)
{
	if (PSG_Enable)
	{
		PSG_Enable = 0;
		MESSAGE_L("PSG Disabled", "PSG Disabled", 1000)
	}
	else
	{
		PSG_Enable = 1;
		MESSAGE_L("PSG Enabled", "PSG Enabled", 1000)
	}

	Build_Main_Menu();
	return 1;
}


int Change_PSG_Improv(HWND hWnd)
{
	if (PSG_Improv)
	{
		PSG_Improv = 0;
		MESSAGE_L("Normal PSG sound", "Normal PSG sound", 1000)
	}
	else
	{
		PSG_Improv = 1;
		MESSAGE_L("Improved PSG sound", "Improved PSG sound", 1000)
	}

	return 1;
}


int Change_PCM(HWND hWnd)
{
	if (PCM_Enable)
	{
		PCM_Enable = 0;
		MESSAGE_L("PCM Sound Disabled", "PCM Sound Disabled", 1000)
	}
	else
	{
		PCM_Enable = 1;
		MESSAGE_L("PCM Sound Enabled", "PCM Sound Enabled", 1000)
	}

	Build_Main_Menu();
	return 1;
}


int Change_PWM(HWND hWnd)
{
	if (PWM_Enable)
	{
		PWM_Enable = 0;
		MESSAGE_L("PWM Sound Disabled", "PWM Sound Disabled", 1000)
	}
	else
	{
		PWM_Enable = 1;
		MESSAGE_L("PWM Sound Enabled", "PWM Sound Enabled", 1000)
	}

	Build_Main_Menu();
	return 1;
}


int Change_CDDA(HWND hWnd)
{
	if (CDDA_Enable)
	{
		CDDA_Enable = 0;
		MESSAGE_L("CD Audio Sound Disabled", "CD Audio Sound Disabled", 1000)
	}
	else
	{
		CDDA_Enable = 1;
		MESSAGE_L("CD Audio Enabled", "CD Audio Enabled", 1000)
	}

	Build_Main_Menu();
	return(1);
}


int	Change_Sound(HWND hWnd)
{
	if (Sound_Enable)
	{
		End_Sound();

		Sound_Enable = 0;
		YM2612_Enable = 0;
		PSG_Enable = 0;
		DAC_Enable = 0;
		PCM_Enable = 0;
		PWM_Enable = 0;
		CDDA_Enable = 0;

		MESSAGE_L("Sound Disabled", "Sound Disabled", 1500)
	}
	else
	{
		if (!Init_Sound(hWnd))
		{
			Sound_Enable = 0;
			YM2612_Enable = 0;
			PSG_Enable = 0;
			DAC_Enable = 0;
			PCM_Enable = 0;
			PWM_Enable = 0;
			CDDA_Enable = 0;

			return 0;
		}

		Sound_Enable = 1;
		Play_Sound();

		if (!(Z80_State & 1)) Change_Z80(hWnd);

		YM2612_Enable = 1;
		PSG_Enable = 1;
		DAC_Enable = 1;
		PCM_Enable = 1;
		PWM_Enable = 1;
		CDDA_Enable = 1;

		MESSAGE_L("Sound Enabled", "Sound Enabled", 1500)
	}

	Build_Main_Menu();
	return 1;
}


int Change_Sample_Rate(HWND hWnd, int Rate)
{
	unsigned char Reg_1[0x200];

	switch (Rate)
	{
	case 0:
		Sound_Rate = 11025;
		MESSAGE_L("Sound rate set to 11025", "Sound rate set to 11025", 2500)
		break;

	case 1:
		Sound_Rate = 22050;
		MESSAGE_L("Sound rate set to 22050", "Sound rate set to 22050", 2500)
		break;

	case 2:
		Sound_Rate = 44100;
		MESSAGE_L("Sound rate set to 44100", "Sound rate set to 44100", 2500)
		break;
	}

	if (Sound_Enable)
	{
		PSG_Save_State();
		YM2612_Save(Reg_1);

		End_Sound();
		Sound_Enable = 0;

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

		if (SegaCD_Started) Set_Rate_PCM(Sound_Rate);
		YM2612_Restore(Reg_1);
		PSG_Restore_State();
		
		if(!Init_Sound(hWnd)) return(0);

		Sound_Enable = 1;
		Play_Sound();
	}
	
	Build_Main_Menu();
	return(1);
}


int Change_Sound_Stereo(HWND hWnd)
{
	unsigned char Reg_1[0x200];

	if (Sound_Stereo)
	{
		Sound_Stereo = 0;
		MESSAGE_L("Mono sound", "Mono sound", 1000)
	}
	else
	{
		Sound_Stereo = 1;
		MESSAGE_L("Stereo sound", "Stereo sound", 1000)
	}

	if (Sound_Enable)
	{
		PSG_Save_State();
		YM2612_Save(Reg_1);

		End_Sound();
		Sound_Enable = 0;

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

		if (SegaCD_Started) Set_Rate_PCM(Sound_Rate);
		YM2612_Restore(Reg_1);
		PSG_Restore_State();
		
		if(!Init_Sound(hWnd)) return(0);

		Sound_Enable = 1;
		Play_Sound();
	}

	Build_Main_Menu();
	return(1);
}


int Change_Country(HWND hWnd, int Num)
{
	unsigned char Reg_1[0x200];

	Flag_Clr_Scr = 1;

	switch(Country = Num)
	{
		default:
		case -1:
			if (Genesis_Started || _32X_Started) Detect_Country_Genesis();
			else if (SegaCD_Started) Detect_Country_SegaCD();
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

	if (CPU_Mode)
	{
		CPL_Z80 = Round_Double((((double) CLOCK_PAL / 15.0) / 50.0) / 312.0);
		CPL_M68K = Round_Double((((double) CLOCK_PAL / 7.0) / 50.0) / 312.0);
		CPL_MSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) MSH2_Speed) / 100.0);
		CPL_SSH2 = Round_Double(((((((double) CLOCK_PAL / 7.0) * 3.0) / 50.0) / 312.0) * (double) SSH2_Speed) / 100.0);

		VDP_Num_Lines = 312;
		VDP_Status |= 0x0001;
		_32X_VDP.Mode &= ~0x8000;

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
		_32X_VDP.Mode |= 0x8000;

		CD_Access_Timer = 2096;
		Timer_Step = 135708;
	}

	if (Sound_Enable)
	{
		PSG_Save_State();
		YM2612_Save(Reg_1);

		End_Sound();
		Sound_Enable = 0;

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

		if (SegaCD_Started) Set_Rate_PCM(Sound_Rate);
		YM2612_Restore(Reg_1);
		PSG_Restore_State();
		
		if(!Init_Sound(hWnd)) return(0);

		Sound_Enable = 1;
		Play_Sound();
	}

	if (Game_Mode)
	{
		if (CPU_Mode) MESSAGE_L("Europe system (50 FPS)", "Europe system (50 FPS)", 1500)
		else MESSAGE_L("USA system (60 FPS)", "USA system (60 FPS)", 1500)
	}
	else
	{
		if (CPU_Mode) MESSAGE_L("Japan system (50 FPS)", "Japan system (50 FPS)", 1500)
		else MESSAGE_L("Japan system (60 FPS)", "Japan system (60 FPS)", 1500)
	}

	UpdateAppTitle();

	Build_Main_Menu();
	return 1;
}


int Change_Country_Order(int Num)
{
	char c_str[4][4] = {"USA", "JAP", "EUR"};
	char str_w[128];
	int sav = Country_Order[Num];
		
	if (Num == 1) Country_Order[1] = Country_Order[0];
	else if (Num == 2)
	{
		Country_Order[2] = Country_Order[1];
		Country_Order[1] = Country_Order[0];
	}
	Country_Order[0] = sav;

	if (Country == -1) Change_Country(HWnd, -1);		// Update Country

	wsprintf(str_w, "Country detec.order : %s %s %s", c_str[Country_Order[0]], c_str[Country_Order[1]], c_str[Country_Order[2]]);
	MESSAGE_L(str_w, str_w, 1500)

	Build_Main_Menu();
	return(1);
}


int Check_If_Kaillera_Running(void)
{
	if (Kaillera_Client_Running)
	{
		if (Sound_Initialised) Clear_Sound_Buffer();
		MessageBox(HWnd, "You can't do it during netplay, you have to close rom and kaillera client before", "info", MB_OK);
		return 1;
	}

	return 0;
}


int WINAPI Play_Net_Game(char *game, int player, int maxplayers)
{
	MSG msg;
	char name[2048];
	HANDLE f;
	WIN32_FIND_DATA fd;

	SetCurrentDirectory(Rom_Dir);

	sprintf_s(name, 2048, "%s.*", game);
	memset(&fd, 0, sizeof(fd));
	fd.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
	f = FindFirstFile(name, &fd);

	if (f == INVALID_HANDLE_VALUE) return 1;

	if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
	{
		sprintf_s( name, 2048, "%s%s", Rom_Dir, fd.cFileName );
		Pre_Load_Rom(HWnd, name);
	}

	if ((!Genesis_Started) && (!_32X_Started)) return 1;
	
	Net_Play = 1;
	SetFocus(HWnd);

	if (maxplayers > 4) maxplayers = 4;
	if (player > 4) player = 0;

	Controller_1_Type &= 0xF;
	Controller_2_Type &= 0xF;
	if (maxplayers > 2) Controller_1_Type |= 0x10;
	Make_IO_Table();

	Kaillera_Keys[0] = Kaillera_Keys[1] = Kaillera_Keys[2] = Kaillera_Keys[3] = 0xFF;
	Kaillera_Keys[4] = Kaillera_Keys[5] = Kaillera_Keys[6] = Kaillera_Keys[7] = 0xFF;
	Kaillera_Keys[8] = Kaillera_Keys[9] = 0xFF;
	Kaillera_Error = 0;

	while (Net_Play)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0)) return msg.wParam;
			if (!TranslateAccelerator (HWnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			}
		}
		else if ((Active) && (!Paused))
		{
			Update_Emulation_Netplay(HWnd, player, maxplayers);
		}
		else
		{
			Flip(HWnd);
			Sleep(100);
		}
	}

	//Kaillera_End_Game();

	return 0;
}


int Start_Netplay(void)
{
// 	kailleraInfos K_Infos;
// 	char name[2048];
// 	char *Liste_Games = NULL, *LG = NULL, *Min_Game, *Cur_Game;
// 	int cursize = 8192, num = 0, dep = 0, len;
// 	int backup_infos[32];
// 	HANDLE f;
// 	WIN32_FIND_DATA fd;
// 
// 	if (Kaillera_Initialised == 0)
// 	{
// 		MessageBox(HWnd, "You need the KAILLERACLIENT.DLL file to enable this feature", "Info", MB_OK);
// 		return 0;
// 	}
// 		
// 	if (Kaillera_Client_Running) return 0;
// 
// 	SetCurrentDirectory(Rom_Dir);
// 
// 	Liste_Games = (char *) malloc(cursize);
// 	Liste_Games[0] = 0;
// 	Liste_Games[1] = 0;
// 
// 	memset(&fd, 0, sizeof(fd));
// 	fd.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
// 	f = FindFirstFile("*.bin", &fd);
// 	if (f != INVALID_HANDLE_VALUE)
// 	{
// 		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
// 		{
// 			len = strlen(fd.cFileName) - 4;
// 			fd.cFileName[len++] = 0;
// 			if ((dep + len) > cursize)
// 			{
// 				cursize += 8192;
// 				Liste_Games = (char*) realloc(Liste_Games, cursize);
// 			}
// 			strcpy(Liste_Games + dep, fd.cFileName);
// 			dep += len;
// 			num++;
// 		}
// 
// 		while (FindNextFile(f, &fd))
// 		{
// 			if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
// 			{
// 				len = strlen(fd.cFileName) - 4;
// 				fd.cFileName[len++] = 0;
// 				if ((dep + len) > cursize)
// 				{
// 					cursize += 8192;
// 					Liste_Games = (char*) realloc(Liste_Games, cursize);
// 				}
// 				strcpy(Liste_Games + dep, fd.cFileName);
// 				dep += len;
// 				num++;
// 			}
// 		}
// 	}
// 
// 	memset(&fd, 0, sizeof(fd));
// 	fd.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
// 	f = FindFirstFile("*.smd", &fd);
// 	if (f != INVALID_HANDLE_VALUE)
// 	{
// 		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
// 		{
// 			len = strlen(fd.cFileName) - 4;
// 			fd.cFileName[len++] = 0;
// 			if ((dep + len) > cursize)
// 			{
// 				cursize += 8192;
// 				Liste_Games = (char*) realloc(Liste_Games, cursize);
// 			}
// 			strcpy(Liste_Games + dep, fd.cFileName);
// 			dep += len;
// 			num++;
// 		}
// 
// 		while (FindNextFile(f, &fd))
// 		{
// 			if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
// 			{
// 				len = strlen(fd.cFileName) - 4;
// 				fd.cFileName[len++] = 0;
// 				if ((dep + len) > cursize)
// 				{
// 					cursize += 8192;
// 					Liste_Games = (char*) realloc(Liste_Games, cursize);
// 				}
// 				strcpy(Liste_Games + dep, fd.cFileName);
// 				dep += len;
// 				num++;
// 			}
// 		}
// 	}
// 
// 	memset(&fd, 0, sizeof(fd));
// 	fd.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
// 	f = FindFirstFile("*.32X", &fd);
// 	if (f != INVALID_HANDLE_VALUE)
// 	{
// 		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
// 		{
// 			len = strlen(fd.cFileName) - 4;
// 			fd.cFileName[len++] = 0;
// 			if ((dep + len) > cursize)
// 			{
// 				cursize += 8192;
// 				Liste_Games = (char*) realloc(Liste_Games, cursize);
// 			}
// 			strcpy(Liste_Games + dep, fd.cFileName);
// 			dep += len;
// 			num++;
// 		}
// 
// 		while (FindNextFile(f, &fd))
// 		{
// 			if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
// 			{
// 				len = strlen(fd.cFileName) - 4;
// 				fd.cFileName[len++] = 0;
// 				if ((dep + len) > cursize)
// 				{
// 					cursize += 8192;
// 					Liste_Games = (char*) realloc(Liste_Games, cursize);
// 				}
// 				strcpy(Liste_Games + dep, fd.cFileName);
// 				dep += len;
// 				num++;
// 			}
// 		}
// 	}
// 
// 	memset(&fd, 0, sizeof(fd));
// 	fd.dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;
// 	f = FindFirstFile("*.zip", &fd);
// 	if (f != INVALID_HANDLE_VALUE)
// 	{
// 		if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
// 		{
// 			len = strlen(fd.cFileName) - 4;
// 			fd.cFileName[len++] = 0;
// 			if ((dep + len) > cursize)
// 			{
// 				cursize += 8192;
// 				Liste_Games = (char*) realloc(Liste_Games, cursize);
// 			}
// 			strcpy(Liste_Games + dep, fd.cFileName);
// 			dep += len;
// 			num++;
// 		}
// 
// 		while (FindNextFile(f, &fd))
// 		{
// 			if(!(fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
// 			{
// 				len = strlen(fd.cFileName) - 4;
// 				fd.cFileName[len++] = 0;
// 				if ((dep + len) > cursize)
// 				{
// 					cursize += 8192;
// 					Liste_Games = (char*) realloc(Liste_Games, cursize);
// 				}
// 				strcpy(Liste_Games + dep, fd.cFileName);
// 				dep += len;
// 				num++;
// 			}
// 		}
// 	}
// 
// 	Liste_Games[dep] = 0;
// 	LG = (char*) malloc(dep);
// 	dep = 0;
// 	
// 	for(; num > 0; num--)
// 	{
// 		Min_Game = Cur_Game = Liste_Games;
// 
// 		while(*Cur_Game)
// 		{
// 			if (stricmp(Cur_Game, Min_Game) < 0) Min_Game = Cur_Game; 
// 			Cur_Game += strlen(Cur_Game) + 1;
// 		}
// 
// 		strlwr(Min_Game);
// 		strcpy(LG + dep, Min_Game);
// 		dep += strlen(Min_Game) + 1;
// 		Min_Game[0] = -1;
// 	}
// 
// 	GetWindowText(HWnd, name, 2046);
// 	backup_infos[0] = Controller_1_Type;
// 	backup_infos[1] = Controller_2_Type;
// 
// 	memset(&K_Infos, 0, sizeof(K_Infos));
// 	
// 	K_Infos.appName = "Gens 2.10";
// 	K_Infos.gameList = LG;
// 	K_Infos.gameCallback = Play_Net_Game;
// 
// //	K_Infos.chatReceivedCallback = NULL;
// //	K_Infos.clientDroppedCallback = NULL;
// //	K_Infos.moreInfosCallback = NULL;
// 
// 	Kaillera_Set_Infos(&K_Infos);
// 
// 	Kaillera_Client_Running = 1;
// 	Kaillera_Select_Server_Dialog(NULL);
// 	Kaillera_Client_Running = 0;
// 
// 	Controller_1_Type = backup_infos[0];
// 	Controller_2_Type = backup_infos[1];
// 	Make_IO_Table();
// 	SetWindowText(HWnd, name);
// 
// 	free(Liste_Games);
// 	free(LG);

	return 1;
}


#ifdef CC_SUPPORT
void CC_End_Callback(char mess[256])
{
	MessageBox(HWnd, mess, "Console Classix", MB_OK);

	if (Sound_Initialised) Clear_Sound_Buffer();
	Debug = 0;
	Free_Rom(Game);
	Build_Main_Menu();
}
#endif


BOOL Init(HINSTANCE hInst, int nCmdShow)
{
	Net_Play = 0;
	Full_Screen = -1;
	VDP_Num_Vis_Lines = 224;
	Resolution = 1;
	W_VSync = 0;
	FS_VSync = 0;
	Stretch = 0;
	Sprite_Over = 1;
	Render_W = 1;
	Render_FS = 1;
	Show_Message = 1;

	Sound_Enable = 0;
	Sound_Segs = 8;
	Sound_Stereo = 1;
	Sound_Initialised = 0;
	Sound_Is_Playing = 0;
	WAV_Dumping = 0;
	GYM_Dumping = 0;

	Game = NULL;
	Genesis_Started = 0;
	SegaCD_Started = 0;
	_32X_Started = 0;
	Debug = 0;
	CPU_Mode = 0;
	Window_Pos.x = 0;
	Window_Pos.y = 0;

	WndClass.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = WinProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hInst;
	WndClass.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_SONIC));
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = NULL;
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = "Gens";

	RegisterClass(&WndClass);

	ghInstance = hInst;

	HWnd = CreateWindowEx(
		NULL,
		"Gens",
		"RAGens REWiND - Idle",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		320 * 2,
		240 * 2,
		NULL,
		NULL,
		hInst,
		NULL);

	if (!HWnd) return FALSE;

	hAccelTable = LoadAccelerators(hInst, MAKEINTRESOURCE(RAC));
  
	Identify_CPU();

	//i = GetVersion();
 
	// Get major and minor version numbers of Windows

	//if (((i & 0xFF) > 4) || (i & 0x80000000))
	//	WinNT_Flag = 0;
	//else 
	//	WinNT_Flag = 1;

	GetCurrentDirectory( 1024, Gens_Path );
	GetCurrentDirectory( 1024, Language_Path );
	GetCurrentDirectory( 1024, Str_Tmp );
	strcpy_s( Manual_Path, 1024, "" );
	strcpy_s( CGOffline_Path, 1024, "" );

	strcat_s(Gens_Path, "\\");
	strcat_s(Language_Path, "\\language.dat");
	strcat_s(Str_Tmp, "\\gens.cfg");

	RA_Init( HWnd, RA_Gens, RAGENS_VERSION );
	RA_InitShared();

	MSH2_Init();
	SSH2_Init();
	M68K_Init();
	S68K_Init();
	Z80_Init();

	YM2612_Init(CLOCK_NTSC / 7, Sound_Rate, YM2612_Improv);
	PSG_Init(CLOCK_NTSC / 15, Sound_Rate);
	PWM_Init();

	Load_Config(Str_Tmp, NULL);
	ShowWindow(HWnd, nCmdShow);

	if (!Init_Input(hInst, HWnd))
	{
		End_Sound();
		End_DDraw();
		return FALSE;
	}

	Init_CD_Driver();
	Init_Network();
	Init_Tab();
	Build_Main_Menu();

	SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &SS_Actived, 0);
	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, FALSE, NULL, 0);

	switch(Gens_Priority)
	{
		case 0:
			SetThreadPriority(hInst, THREAD_PRIORITY_BELOW_NORMAL);
			break;

		case 2:
			SetThreadPriority(hInst, THREAD_PRIORITY_ABOVE_NORMAL);
			break;

		case 3:
			SetThreadPriority(hInst, THREAD_PRIORITY_HIGHEST);
			break;

		case 5:
			SetThreadPriority(hInst, THREAD_PRIORITY_TIME_CRITICAL);
			break;
	}

	//	Needed? SD
	InitCommonControls();

	Gens_Running = 1;

	Build_Main_Menu();

	UpdateAppTitle();

	RA_AttemptLogin( true );

	return TRUE;
}


void End_All(void)
{
	End_Sound();
	Free_Rom(Game);
	End_DDraw();
	End_Input();
	YM2612_End();
	End_CD_Driver();
	End_Network();

	RA_Shutdown();

	SystemParametersInfo(SPI_SETSCREENSAVEACTIVE, SS_Actived, NULL, 0);
}


int PASCAL WinMain(HINSTANCE hInst,	HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow)
{
	MSG msg;

	Init(hInst, nCmdShow);

	// Have to do it *before* load by command line
	Init_Genesis_Bios();

	if( lpCmdLine[ 0 ] != '\0' )
	{
		//	Fetch target ROM path from cmd line
		int iter = 0;
		char buffer[ 1024 ];
		if( lpCmdLine[ iter ] == '"' )
		{
			iter++;
			while ((lpCmdLine[ iter ] != '"') && (lpCmdLine[ iter ] != 0))
			{
				buffer[ iter - 1 ] = lpCmdLine[ iter ];
				iter++;
			}

			buffer[ iter - 1 ] = '\0';
		}
		else
		{
			while( lpCmdLine[ iter ] != 0 )
			{
				buffer[ iter ] = lpCmdLine[ iter ];
				iter++;
			}

			buffer[ iter ] = '\0';
		}

		RA_AttemptLogin( true );
		Pre_Load_Rom( HWnd, buffer );

#ifdef CC_SUPPORT
		}
#endif
	}

	while (Gens_Running)
	{
		while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
		{
			if (!GetMessage(&msg, NULL, 0, 0)) Gens_Running = 0;
			if (!TranslateAccelerator (HWnd, hAccelTable, &msg))
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg);
			}
		}

		RA_HandleHTTPResults();

#ifdef GENS_DEBUG
		if (Debug)						// DEBUG
		{
			Update_Debug_Screen();
			Flip(HWnd);
		}
		else
#endif
		if (Genesis_Started || _32X_Started || SegaCD_Started)
		{
			if( Frame_Skip == -1 )
				Sleep(10);	//	Low-intensity CPU to aid laptop support

			if ((Active) && (!Paused))	// EMULATION
			{
				Update_Emulation(HWnd);
			}
			else						// EMULATION PAUSED
			{
				Flip(HWnd);

				//	Note: removed sleep as it interfered with AchievementOverlay SD
				//if( !Paused )
				//	Sleep(10);
			}
		}
		else if (GYM_Playing)			// PLAY GYM
		{
			Play_GYM();
			Update_Gens_Logo(HWnd);
		}
		else if (Intro_Style == 1)		// GENS LOGO EFFECT
		{
			Update_Gens_Logo(HWnd);
			Sleep(5);
		}
		else if (Intro_Style == 2)		// STRANGE EFFECT
		{
			Update_Crazy_Effect(HWnd);
			Sleep(10);
		}
		else if (Intro_Style == 3)		// GENESIS BIOS
		{
			Do_Genesis_Frame();

			Flip(HWnd);
			Sleep(20);
		}
		else							// BLANK SCREEN (MAX IDLE)
		{
			Clear_Back_Screen(HWnd);
			Flip(HWnd);
			Sleep(200);
		}

		if( Paused )
		{
			Clear_Sound_Buffer();
		}
	}

	End_All();

	strcpy_s(Str_Tmp, 1024, Gens_Path);
	strcat_s(Str_Tmp, "Gens.cfg");
	Save_Config(Str_Tmp);

	ChangeDisplaySettings(NULL, 0);

	DestroyWindow(HWnd);

	while(PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE))
	{
		if (!GetMessage(&msg, NULL, 0, 0)) return msg.wParam;
	}

	return 0;
}


long PASCAL WinProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	RECT r;

	switch(message)
	{
		case WM_ACTIVATE:
			{
				if (Gens_Running == 0) break;
				
				// Is main window minimized?
				BOOL fMinimized = HIWORD(wParam);

				if (LOWORD(wParam) == WA_INACTIVE)
				{
					// Main window is being deactivated.
					
					if (Full_Screen)
					{
						// Switch out of fullscreen mode, but don't
						// reposition the window.
						Set_Render(hWnd, 0, -1, false);
					}

					if (Auto_Pause)
					{
						Active = 0;
						if (!Paused) Pause_Screen();
						Clear_Sound_Buffer();
					}
				}
				else
				{
					// Main window is being activated.
					Active = 1;
					
					// Switch back to windowed or full-screen mode.
					Set_Render(hWnd, Full_Screen, -1, false);
				}	
				
				//	NB> Should really handle maximized as well!
			}
			break;

		case WM_MENUSELECT:
 		case WM_ENTERSIZEMOVE:
			Clear_Sound_Buffer();
			break;

 		case WM_EXITSIZEMOVE:
			if (!Full_Screen)
			{
				GetWindowRect(HWnd, &r);
				Window_Pos.x = r.left;
				Window_Pos.y = r.top;
			}
			break;

		case WM_CLOSE:
			if ((Check_If_Kaillera_Running())) return 0;
			if( !RA_ConfirmLoadNewRom( TRUE ) )
				return 0;

			Clear_Sound_Buffer();
			Gens_Running = 0;
			return 0;
		
		case WM_RBUTTONDOWN:
			if (Full_Screen)
			{
				Clear_Sound_Buffer();
				SetCursorPos(40, 30);
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
				Restore_Primary();
				TrackPopupMenu(Gens_Menu, TPM_LEFTALIGN | TPM_TOPALIGN, 20, 20, NULL, hWnd, NULL);
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}
			break;

		case WM_CREATE:
			Active = 1;
			break;
		
		case WM_PAINT:
			Clear_Primary_Screen(HWnd);
			Flip(hWnd);
			RA_OnPaint( hWnd );
			break;
		
		case WM_COMMAND:
			if ((LOWORD(wParam) >= ID_HELP_LANG) && (LOWORD(wParam) < ID_HELP_LANG + 50))
			{
				Language = LOWORD(wParam) - ID_HELP_LANG;
				Build_Main_Menu();
				return 0;
			}
			else switch(LOWORD(wParam))
			{
				case ID_FILES_QUIT:
					PostMessage(hWnd, WM_CLOSE, 0, 0);
					return 0;

				case ID_FILES_OPENROM:
					if ((Check_If_Kaillera_Running())) return 0;
					MINIMIZE
					if (GYM_Playing) Stop_Play_GYM();

					if( !RA_ConfirmLoadNewRom( FALSE ) )
						return 0;

					return Get_Rom(hWnd);


				case ID_FILES_OPENRECENTROM0:
				case ID_FILES_OPENRECENTROM1:
				case ID_FILES_OPENRECENTROM2:
				case ID_FILES_OPENRECENTROM3:
				case ID_FILES_OPENRECENTROM4:
				case ID_FILES_OPENRECENTROM5:
				case ID_FILES_OPENRECENTROM6:
				case ID_FILES_OPENRECENTROM7:
				case ID_FILES_OPENRECENTROM8:
					if ((Check_If_Kaillera_Running())) return 0;
					if (GYM_Playing) Stop_Play_GYM();

					if( !RA_ConfirmLoadNewRom( FALSE ) )
						return 0;

					return Pre_Load_Rom(HWnd, Recent_Rom[LOWORD(wParam) - ID_FILES_OPENRECENTROM0]);

				case ID_FILES_BOOTCD:
					if (Num_CD_Drive == 0) return 1;
					if (Check_If_Kaillera_Running()) return 0;
					if (GYM_Playing) Stop_Play_GYM();
					Free_Rom(Game);			// Don't forget it !
					SegaCD_Started = Init_SegaCD(NULL);
					Build_Main_Menu();
					return SegaCD_Started;

				case ID_FILES_OPENCLOSECD:
					if (SegaCD_Started) Change_CD();
					return 0;

// 				case ID_FILES_NETPLAY:
// 					MINIMIZE
// 					if (GYM_Playing) Stop_Play_GYM();
// 					Start_Netplay();
// 					return 0;

				//case IDM_RA_FILES_TEST1:

				//	//Force to double render
				//	Set_Render(hWnd, (Full_Screen = 0), 1, false);

				//	int nMainWidth, nMainHeight;
				//	RECT r, r1;

				//	GetWindowRect(HWnd, &r);
				//	nMainWidth = r.right-r.left;
				//	nMainHeight = r.bottom-r.top;

				//	SetWindowPos( HWnd, NULL, 0, 0, nMainWidth, nMainHeight, 0 );

				//	//	Poke the windows to wake them up
				//	RA_InvokeDialog( IDM_RA_FILES_ACHIEVEMENTS );
				//	RA_InvokeDialog( IDM_RA_FILES_ACHIEVEMENTEDITOR );
				//	RA_InvokeDialog( IDM_RA_FILES_MEMORYFINDER );
				//	//SendMessage( HWnd, WM_COMMAND, IDM_RA_FILES_ACHIEVEMENTS, 0 );
				//	//SendMessage( HWnd, WM_COMMAND, IDM_RA_FILES_ACHIEVEMENTEDITOR, 0 );
				//	//SendMessage( HWnd, WM_COMMAND, IDM_RA_FILES_MEMORYFINDER, 0 );

				//	//	Move the windows to secure locations (stuck to main wnd)
				//	//GetWindowRect( g_MemoryDialog.GetHWND(), &r1 );
				//	//SetWindowPos( g_MemoryDialog.GetHWND(), NULL, nMainWidth, 0, r1.right-r1.left, r1.bottom-r1.top, 0 );
				//	//GetWindowRect( g_AchievementsDialog.GetHWND(), &r1 );
				//	//SetWindowPos( g_AchievementsDialog.GetHWND(), NULL, 0, nMainHeight, r1.right-r1.left, r1.bottom-r1.top, 0 );
				//	//GetWindowRect( g_AchievementEditorDialog.GetHWND(), &r1 );
				//	//SetWindowPos( g_AchievementEditorDialog.GetHWND(), NULL, nMainWidth, nMainHeight, r1.right-r1.left, r1.bottom-r1.top, 0 );

				//	//	Load Sonic 1 if we can

				//	if( !RA_ConfirmLoadNewRom( FALSE ) )
				//		return 0;

				//	Pre_Load_Rom(HWnd, "S:\\Downloads\\GoodGen\\GoodGen v3.00 2011-12-28.zip\\Sonic The Hedgehog (W) (REV00) [!]\\Sonic The Hedgehog (W) (REV00) [!].gen" );
				//	break;

				case ID_FILES_CLOSEROM:
					if (Sound_Initialised) Clear_Sound_Buffer();

					if( !RA_ConfirmLoadNewRom( TRUE ) )
						return 0;
					
					Debug = 0;
					if (Net_Play)
					{
						if (Full_Screen) Set_Render(hWnd, 0, -1, true);
					}
					Free_Rom(Game);
					Build_Main_Menu();
					return 0;
		
				case ID_FILES_GAMEGENIE:
					if (Check_If_Kaillera_Running()) return 0;
					MINIMIZE
					DialogBox( ghInstance, MAKEINTRESOURCE(IDD_GAMEGENIE), hWnd, GGenieProc );
					Build_Main_Menu();
					return 0;

				case ID_FILES_LOADSTATE:
					if (Check_If_Kaillera_Running()) 
						return 0;
					if( Game == NULL )
						return 0;

					Str_Tmp[0] = 0;
					Get_State_File_Name(Str_Tmp);
					Load_State(Str_Tmp);
					return 0;

				case ID_FILES_LOADSTATEAS:
					if (Check_If_Kaillera_Running()) 
						return 0;
					if( Game == NULL )
						return 0;

					Str_Tmp[0] = 0;
					Change_File_L(Str_Tmp, State_Dir, "Load state", "State Files\0*.gs*\0All Files\0*.*\0\0", "");
					Load_State(Str_Tmp);
					return 0;

				case ID_FILES_SAVESTATE:
					if (Check_If_Kaillera_Running()) 
						return 0;

					if( Game == NULL )
						return 0;

					Str_Tmp[0] = 0;
					Get_State_File_Name(Str_Tmp);
					Save_State(Str_Tmp);
					return 0;

				case ID_FILES_SAVESTATEAS:
					if (Check_If_Kaillera_Running()) 
						return 0;

					if( Game == NULL )
						return 0;

					Change_File_S(Str_Tmp, State_Dir, "Save state", "State Files\0*.gs*\0All Files\0*.*\0\0", "");
					Save_State(Str_Tmp);
					return 0;

				case ID_FILES_PREVIOUSSTATE:
					Set_Current_State(hWnd, (Current_State + 9) % 10);
					return 0;

				case ID_FILES_NEXTSTATE:
					Set_Current_State(hWnd, (Current_State + 1) % 10);
					return 0;

				case ID_GRAPHICS_VSYNC:
					Change_VSync(hWnd);
					return 0;

				case ID_GRAPHICS_SWITCH_MODE:
					if (Full_Screen) 
					{
						Set_Render(hWnd, 0, -1, true);
						Sleep(500);
					}
					else 
					{
						Set_Render(hWnd, 1, -1, true);
						Sleep(500);
					}
					return 0;

				case ID_GRAPHICS_COLOR_ADJUST:
					if (Check_If_Kaillera_Running()) return 0;
					MINIMIZE
					DialogBox( ghInstance, MAKEINTRESOURCE(IDD_COLOR), hWnd, ColorProc );
					return 0;

				case ID_GRAPHICS_RENDER_NORMAL:
					Set_Render(hWnd, Full_Screen, 0, false);
					return 0;

				case ID_GRAPHICS_RENDER_DOUBLE:
					Set_Render(hWnd, Full_Screen, 1, false);
					return 0;

				case ID_GRAPHICS_RENDER_DOUBLE_INT:
					Set_Render(hWnd, Full_Screen, 2, false);
					return 0;

				case ID_GRAPHICS_RENDER_FULLSCANLINE:
					Set_Render(hWnd, Full_Screen, 3, false);
					return 0;

				case ID_GRAPHICS_RENDER_50SCANLINE:
					Set_Render(hWnd, Full_Screen, 4, false);
					return 0;

				case ID_GRAPHICS_RENDER_25SCANLINE:
					Set_Render(hWnd, Full_Screen, 5, false);
					return 0;

				case ID_GRAPHICS_RENDER_INTESCANLINE:
					Set_Render(hWnd, Full_Screen, 6, false);
					return 0;

				case ID_GRAPHICS_RENDER_INT50SCANLIN:
					Set_Render(hWnd, Full_Screen, 7, false);
					return 0;

				case ID_GRAPHICS_RENDER_INT25SCANLIN:
					Set_Render(hWnd, Full_Screen, 8, false);
					return 0;

				case ID_GRAPHICS_RENDER_2XSAI:
					Set_Render(hWnd, Full_Screen, 9, false);
					return 0;

				case ID_GRAPHICS_PREVIOUS_RENDER:
					if ((Full_Screen) && (Render_FS > 0)) Set_Render(hWnd, 1, Render_FS - 1, false);
					else if ((!Full_Screen) && (Render_W > 0)) Set_Render(hWnd, 0, Render_W - 1, false);
					return 0;

				case ID_GRAPHICS_NEXT_RENDER:
					if ((Full_Screen) && (Render_FS < 9)) Set_Render(hWnd, 1, Render_FS + 1, false);
					else if ((!Full_Screen) && (Render_W < 9)) Set_Render(hWnd, 0, Render_W + 1, false);
					return 0;

				case ID_GRAPHICS_STRETCH:
					Change_Stretch();
					return 0;

				case ID_GRAPHICS_FORCESOFT:
					Change_Blit_Style();
					return 0;
				
				case ID_GRAPHICS_FRAMESKIP_AUTO:
					Set_Frame_Skip(hWnd, -1);
					return 0;

				case ID_GRAPHICS_FRAMESKIP_0:
				case ID_GRAPHICS_FRAMESKIP_1:
				case ID_GRAPHICS_FRAMESKIP_2:
				case ID_GRAPHICS_FRAMESKIP_3:
				case ID_GRAPHICS_FRAMESKIP_4:
				case ID_GRAPHICS_FRAMESKIP_5:
				case ID_GRAPHICS_FRAMESKIP_6:
				case ID_GRAPHICS_FRAMESKIP_7:
				case ID_GRAPHICS_FRAMESKIP_8:
					Set_Frame_Skip(hWnd, LOWORD(wParam) - ID_GRAPHICS_FRAMESKIP_0);
					return 0;

				case ID_GRAPHICS_FRAMESKIP_MOINS:
					if (Frame_Skip == -1)
					{
						Set_Frame_Skip(hWnd, 0);
					}
					else
					{
						if (Frame_Skip > 0) Set_Frame_Skip(hWnd, Frame_Skip - 1);
					}
					return 0;

				case ID_GRAPHICS_FRAMESKIP_PLUS:
					if (Frame_Skip == -1)
					{
						Set_Frame_Skip(hWnd, 1);
					}
					else
					{
						if (Frame_Skip < 8) Set_Frame_Skip(hWnd, Frame_Skip + 1);
					}
					return 0;

				case ID_GRAPHICS_SPRITEOVER:
					Set_Sprite_Over(hWnd, Sprite_Over ^ 1);
					return 0;

				case ID_GRAPHICS_SHOT:
					Clear_Sound_Buffer();
					Take_Shot();
					Build_Main_Menu();
					return 0;

				case ID_FILES_CHANGESTATE_0:
				case ID_FILES_CHANGESTATE_1:
				case ID_FILES_CHANGESTATE_2:
				case ID_FILES_CHANGESTATE_3:
				case ID_FILES_CHANGESTATE_4:
				case ID_FILES_CHANGESTATE_5:
				case ID_FILES_CHANGESTATE_6:
				case ID_FILES_CHANGESTATE_7:
				case ID_FILES_CHANGESTATE_8:
				case ID_FILES_CHANGESTATE_9:
					Set_Current_State(hWnd, LOWORD(wParam) - ID_FILES_CHANGESTATE_0);
					return 0;

#ifdef GENS_DEBUG
				case ID_CPU_DEBUG_GENESIS_68000:
					Change_Debug(hWnd, 1);
					return 0;

				case ID_CPU_DEBUG_GENESIS_Z80:
					Change_Debug(hWnd, 2);
					return 0;

				case ID_CPU_DEBUG_GENESIS_VDP:
					Change_Debug(hWnd, 3);
					return 0;

				case ID_CPU_DEBUG_SEGACD_68000:
					Change_Debug(hWnd, 4);
					return 0;

				case ID_CPU_DEBUG_SEGACD_CDC:
					Change_Debug(hWnd, 5);
					return 0;

				case ID_CPU_DEBUG_SEGACD_GFX:
					Change_Debug(hWnd, 6);
					return 0;

				case ID_CPU_DEBUG_32X_MAINSH2:
					Change_Debug(hWnd, 7);
					return 0;

				case ID_CPU_DEBUG_32X_SUBSH2:
					Change_Debug(hWnd, 8);
					return 0;

				case ID_CPU_DEBUG_32X_VDP:
					Change_Debug(hWnd, 9);
					return 0;
#endif

				case ID_CPU_RESET:
					if (Check_If_Kaillera_Running()) return 0;

					if (Genesis_Started)
					{
						Reset_Genesis();
						//g_MainOverlay.OnLoad_NewRom();//TBD
						MESSAGE_L("Genesis reseted", "Genesis reset", 1500)
					}
					else if (_32X_Started)
					{
						Reset_32X();
						MESSAGE_L("32X reseted", "32X reset", 1500)
					}
					else if (SegaCD_Started)
					{
						Reset_SegaCD();
						MESSAGE_L("SegaCD reseted", "SegaCD reset", 1500)
					}
					return 0;

				case ID_CPU_RESET68K:
					if (Check_If_Kaillera_Running()) return 0;
					if (Game)
					{
						Paused = 0;
						main68k_reset();
						if (Genesis_Started) MESSAGE_L("68000 CPU reseted", "68000 CPU reseted", 1000)
						else if (SegaCD_Started) MESSAGE_L("Main 68000 CPU reseted", "Main 68000 CPU reseted", 1000)
					}
					return 0;

				case ID_CPU_RESET_MSH2:
					if (Check_If_Kaillera_Running()) return 0;
					if ((Game) && (_32X_Started))
					{
						Paused = 0;
						SH2_Reset(&M_SH2, 1);
						MESSAGE_L("Master SH2 reseted", "Master SH2 reseted", 1000)
					}
					return 0;

				case ID_CPU_RESET_SSH2:
					if (Check_If_Kaillera_Running()) return 0;
					if ((Game) && (_32X_Started))
					{
						Paused = 0;
						SH2_Reset(&S_SH2, 1);
						MESSAGE_L("Slave SH2 reseted", "Slave SH2 reseted", 1000)
					}
					return 0;

				case ID_CPU_RESET_SUB68K:
					if (Check_If_Kaillera_Running()) return 0;
					if ((Game) && (SegaCD_Started))
					{
						Paused = 0;
						sub68k_reset();
						MESSAGE_L("Sub 68000 CPU reseted", "Sub 68000 CPU reseted", 1000)
					}
					return 0;

				case ID_CPU_RESETZ80:
					if (Check_If_Kaillera_Running()) return 0;
					if (Game)
					{
						z80_Reset(&M_Z80);
						MESSAGE_L("CPU Z80 reseted", "CPU Z80 reseted", 1000)
					}
					return 0;

				case ID_CPU_ACCURATE_SYNCHRO:
					Change_SegaCD_Synchro();
					return 0;

				case ID_CPU_COUNTRY_AUTO:
					Change_Country(hWnd, -1);
					return 0;

				case ID_CPU_COUNTRY_JAPAN:
					Change_Country(hWnd, 0);
					return 0;

				case ID_CPU_COUNTRY_USA:
					Change_Country(hWnd, 1);
					return 0;

				case ID_CPU_COUNTRY_EUROPE:
					Change_Country(hWnd, 2);
					return 0;

				case ID_CPU_COUNTRY_MISC:
					Change_Country(hWnd, 3);
					return 0;

				case ID_CPU_COUNTRY_ORDER + 0:
				case ID_CPU_COUNTRY_ORDER + 1:
				case ID_CPU_COUNTRY_ORDER + 2:
					Change_Country_Order(LOWORD(wParam) - ID_CPU_COUNTRY_ORDER);
					return 0;

				case ID_SOUND_Z80ENABLE:
					Change_Z80(hWnd);
					return 0;

				case ID_SOUND_YM2612ENABLE:
					Change_YM2612(hWnd);
					return 0;

				case ID_SOUND_PSGENABLE:
					Change_PSG(hWnd);
					return 0;

				case ID_SOUND_DACENABLE:
					Change_DAC(hWnd);
					return 0;

				case ID_SOUND_PCMENABLE:
					Change_PCM(hWnd);
					return 0;

				case ID_SOUND_PWMENABLE:
					Change_PWM(hWnd);
					return 0;

				case ID_SOUND_CDDAENABLE:
					Change_CDDA(hWnd);
					return 0;

				case ID_SOUND_DACIMPROV:
					Change_DAC_Improv(hWnd);
					return 0;

				case ID_SOUND_PSGIMPROV:
					Change_PSG_Improv(hWnd);
					return 0;

				case ID_SOUND_YMIMPROV:
					Change_YM2612_Improv(hWnd);
					return 0;

				case ID_SOUND_ENABLE:
					Change_Sound(hWnd);
					return 0;

				case ID_SOUND_RATE_11000:
					Change_Sample_Rate(hWnd, 0);
					return 0;

				case ID_SOUND_RATE_22000:
					Change_Sample_Rate(hWnd, 1);
					return 0;

				case ID_SOUND_RATE_44000:
					Change_Sample_Rate(hWnd, 2);
					return 0;

				case ID_SOUND_STEREO:
					Change_Sound_Stereo(hWnd);
					return 0;

				case ID_SOUND_STARTWAVDUMP:
					if (WAV_Dumping) Stop_WAV_Dump();
					else Start_WAV_Dump();
					Build_Main_Menu();
					return 0;

				case ID_SOUND_STARTGYMDUMP:
					if (GYM_Dumping) Stop_GYM_Dump();
					else Start_GYM_Dump();
					Build_Main_Menu();
					return 0;

				case ID_SOUND_PLAYGYM:
					MINIMIZE
					if (!Genesis_Started && !SegaCD_Started && !_32X_Started)
					{
						if (GYM_Playing) Stop_Play_GYM();
						else Start_Play_GYM();
					}
					Build_Main_Menu();
					return 0;

				case ID_OPTIONS_FASTBLUR:
					Change_Fast_Blur(hWnd);
					return 0;

				case ID_OPTIONS_SHOWFPS:
					if (Show_FPS) Show_FPS = 0;
					else Show_FPS = 1;
					return 0;

				case ID_OPTIONS_GENERAL:
					if (Check_If_Kaillera_Running()) return 0;
					MINIMIZE
					DialogBox( ghInstance, MAKEINTRESOURCE(IDD_OPTION), hWnd, OptionProc );
					Build_Main_Menu();
					return 0;

				case ID_OPTIONS_JOYPADSETTING:
					if (Check_If_Kaillera_Running()) return 0;
					MINIMIZE
					End_Input();
					DialogBox( ghInstance, MAKEINTRESOURCE(IDD_CONTROLLER), hWnd, ControllerProc );
					if (!Init_Input(ghInstance, HWnd)) return false;
					Build_Main_Menu();
					return 0;

				case ID_OPTIONS_CHANGEDIR:
					if (Check_If_Kaillera_Running()) return 0;
					MINIMIZE
					DialogBox( ghInstance, MAKEINTRESOURCE(IDD_DIRECTORIES), hWnd, DirectoriesProc );
					Build_Main_Menu();
					return 0;

				case ID_OPTIONS_CHANGEFILES:
					if (Check_If_Kaillera_Running()) return 0;
					MINIMIZE
					DialogBox( ghInstance, MAKEINTRESOURCE(IDD_FILES), hWnd, FilesProc );
					Build_Main_Menu();
					return 0;

				case ID_OPTION_CDDRIVE_0:
				case ID_OPTION_CDDRIVE_1:
				case ID_OPTION_CDDRIVE_2:
				case ID_OPTION_CDDRIVE_3:
				case ID_OPTION_CDDRIVE_4:
				case ID_OPTION_CDDRIVE_5:
				case ID_OPTION_CDDRIVE_6:
				case ID_OPTION_CDDRIVE_7:
					if (Num_CD_Drive > (LOWORD(wParam) - ID_OPTION_CDDRIVE_0))
					{
						CUR_DEV = LOWORD(wParam) - ID_OPTION_CDDRIVE_0;
					}
					Build_Main_Menu();
					return 0;

				case ID_OPTION_SRAMSIZE_0:
					Change_SegaCD_SRAM_Size(-1);
					return 0;

				case ID_OPTION_SRAMSIZE_8:
					Change_SegaCD_SRAM_Size(0);
					return 0;

				case ID_OPTION_SRAMSIZE_16:
					Change_SegaCD_SRAM_Size(1);
					return 0;

				case ID_OPTION_SRAMSIZE_32:
					Change_SegaCD_SRAM_Size(2);
					return 0;

				case ID_OPTION_SRAMSIZE_64:
					Change_SegaCD_SRAM_Size(3);
					return 0;

				case ID_OPTIONS_SAVECONFIG:
					strcpy_s(Str_Tmp, 1024, Gens_Path);
					strcat_s(Str_Tmp, "Gens.cfg");
					Save_Config(Str_Tmp);
					return 0;

				case ID_OPTIONS_LOADCONFIG:
					if (Check_If_Kaillera_Running()) return 0;
					MINIMIZE
					Load_As_Config(hWnd, Game);
					return 0;

				case ID_OPTIONS_SAVEASCONFIG:
					MINIMIZE
					Save_As_Config(hWnd);
					return 0;

				case ID_HELP_ABOUT:
					Clear_Sound_Buffer();
					DialogBox( ghInstance, MAKEINTRESOURCE(IDD_ABOUTDIAL), hWnd, AboutProc );
					return 0;

				case ID_HELP_HELP:
					if (Game)
					{
						if (Genesis_Started)
						{
							HtmlHelp(GetDesktopWindow(), CGOffline_Path, HH_HELP_CONTEXT, Calculate_CRC32());
						}
					}
					else
					{
						if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
						{
							strcpy_s(Str_Tmp, 1024, Manual_Path);
							strcat_s(Str_Tmp, " index.html");
							system(Str_Tmp);
						}
					}

					/* Gens manual file :
					**
					**	File menu						manual.exe helpfilemenu.html
					**	Graphics menu					manual.exe helpgraphicsmenu.html
					**	CPU menu						manual.exe helpcpumenu.html
					**	Sound menu						manual.exe helpsoundmenu.html
					**	Options menu					manual.exe helpoptionmenu.html
					**	Netplay							manual.exe helpnetplay.html
					**
					**	Game Genie						manual.exe helpgamegenie.html
					**	Misc/General options			manual.exe helpmisc.html
					**	Joypad/Controllers settings		manual.exe helpjoypads.html
					**	Directories/file configuration	manual.exe helpdir.html
					**
					**	Help menu						manual.exe helphelpmenu.html
					**	Mega-CD                         manual.exe helpmegacd.html
					**	FAQ                             manual.exe helpfaq.html
					**	Default Keys/keyboard shortcuts manual.exe helpkeys.html
					**	Multitap                        manual.exe helpmultitap.html
					*/
					return 0;

				case ID_HELP_MENU_FILE:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpfilemenu.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_MENU_GRAPHICS:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpgraphicsmenu.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_MENU_CPU:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpcpumenu.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_MENU_SOUND:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpsoundmenu.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_MENU_OPTIONS:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpoptionmenu.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_NETPLAY:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpnetplay.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_MEGACD:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpmegacd.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_FAQ:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpfaq.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_HELP_KEYS:
					if (Detect_Format(Manual_Path) != -1)		// Can be used to test if file exist
					{
						strcpy_s(Str_Tmp, 1024, Manual_Path);
						strcat_s(Str_Tmp, " helpkeys.html");
						system(Str_Tmp);
					}
					return 0;

				case ID_EMULATION_PAUSED:
					if (Debug)
					{
						Change_Debug(HWnd, 0);
						Paused = 0;
						Build_Main_Menu();
					}
					else if (Paused)
					{
						Paused = 0;
						//Play_Sound();
					}
					else
					{
						Paused = 1;
						Pause_Screen();
						Clear_Sound_Buffer();
						//Stop_Sound();
					}

					RA_SetPaused( Paused==1 );
					return 0;
			}

			//	##RA
			if( LOWORD(wParam) >= IDM_RA_MENUSTART &&
				LOWORD(wParam) < IDM_RA_MENUEND )
			{
				MINIMIZE

				Clear_Sound_Buffer();

				RA_InvokeDialog( LOWORD(wParam) );

				return 0;
			}

			break;

/*
			// A device has be modified (new CD inserted for instance)
			case WM_DEVICECHANGE:
				ASPI_Mechanism_State(0, NULL);
				break;
*/

#ifdef GENS_DEBUG
		case WM_KEYDOWN:
			if (Debug) Debug_Event((lParam >> 16) & 0x7F);
			break;
#endif

		case WM_KNUX:
			MESSAGE_L("Communicating", "Communicating ...", 1000)

			switch(wParam)
			{
				case 0:
					switch(lParam)
					{
						case 0:
							return 4;

						case 1:
							GetWindowText(HWnd, Str_Tmp, 1024);
							return (long) (char *) Str_Tmp;

						case 2:
							return 5;

						case 3:
							return GENS_VERSION_H;

						default:
							return -1;
					}

				case 1:
					switch(lParam)
					{
						case 0:
							return((long) (unsigned short *)&Ram_68k[0]);
						case 1:
							return(64 * 1024);
						case 2:
							return(1);
						default:
							return(-1);
					}

				case 2:
					switch(lParam)
					{
						case 0:
							return((long) (unsigned char *)&Ram_Z80[0]);
						case 1:
							return(8 * 1024);
						case 2:
							return(0);
						default:
							return(-1);
					}

				case 3:
					switch(lParam)
					{
						case 0:
							return((long) (char *)&Rom_Data[0]);
						case 1:
							return(0);
						case 2:
							return(Rom_Size);
						default:
							return(-1);
					}

				case 4:
					switch(lParam)
					{
						case 0:
							return(0);
						case 1:
							return((Game != NULL)?1:0);
						case 2:
							return(0);
						default:
							return(-1);
					}

				default:
					return(-1);
			}
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}


int Build_Language_String(void)
{
	unsigned long nb_lue = 1;
	int sec_alloue = 1, poscar = 0;
	enum etat_sec {DEB_LIGNE, SECTION, NORMAL} etat = DEB_LIGNE;
	HANDLE LFile;
	char c;

	if (language_name)
	{
		free(language_name);
		language_name = NULL;
	}

	language_name = (char**)malloc(sec_alloue * sizeof(char*));
	language_name[0] = NULL;

	LFile = CreateFile(Language_Path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
 	
	while(nb_lue)
	{
		ReadFile(LFile, &c, 1, &nb_lue, NULL);
		
		switch(etat)
		{
			case DEB_LIGNE:
				switch(c)
				{
					case '[':
						etat = SECTION;
						sec_alloue++;
						language_name = (char**)realloc(language_name, sec_alloue * sizeof(char*));
						language_name[sec_alloue - 2] = (char*)malloc(32 * sizeof(char));
						language_name[sec_alloue - 1] = NULL;
						poscar = 0;
						break;

					case '\n':
						break;

					default: etat = NORMAL;
						break;
				}
				break;

			case NORMAL:
				switch(c)
				{
					case '\n':
						etat = DEB_LIGNE;
						break;

					default:
						break;
				}
				break;

			case SECTION:
				switch(c)
				{
					case ']':
						language_name[sec_alloue - 2][poscar] = 0;
						etat = DEB_LIGNE;
						break;

					default:
						if(poscar < 32)
							language_name[sec_alloue - 2][poscar++] = c;
						break;
				}
				break;
		}
	}

	CloseHandle(LFile);

	if (sec_alloue == 1)
	{
		language_name = (char**)realloc(language_name, 2 * sizeof(char*));
		language_name[0] = (char*)malloc(32 * sizeof(char));
		strcpy_s(language_name[0], 32, "English");
		language_name[1] = NULL;
		WritePrivateProfileString("English", "Menu Language", "&English menu", Language_Path);
	}

	return(0);	
}


HMENU Build_Main_Menu(void)
{
	unsigned int Flags;
	int i, Rend;

	HMENU MainMenu;
	HMENU Files;
	HMENU Graphics;
	HMENU CPU;
	HMENU Sound;
	HMENU Options;
	HMENU Help;

	HMENU FilesChangeState;
	HMENU FilesHistory;
	HMENU GraphicsRender;
	HMENU GraphicsFrameSkip;
#ifdef GENS_DEBUG
	HMENU CPUDebug;
#endif
	HMENU CPUCountry;
	HMENU CPUCountryOrder;
	HMENU SoundRate;
	HMENU OptionsCDDrive;
	HMENU OptionsSRAMSize;

	DestroyMenu(Gens_Menu);

	Build_Language_String();

	if (Full_Screen)
	{
		MainMenu = CreatePopupMenu();
		Rend = Render_FS;
	}
	else
	{
		MainMenu = CreateMenu();
		Rend = Render_W;
	}

	Files = CreatePopupMenu();
	Graphics = CreatePopupMenu();
	CPU = CreatePopupMenu();
	Sound = CreatePopupMenu();
	Options = CreatePopupMenu();
	Help = CreatePopupMenu();
	FilesChangeState = CreatePopupMenu();
	FilesHistory = CreatePopupMenu();
	GraphicsRender = CreatePopupMenu();
	GraphicsFrameSkip = CreatePopupMenu();
#ifdef GENS_DEBUG
	CPUDebug = CreatePopupMenu();
#endif
	CPUCountry = CreatePopupMenu();
	CPUCountryOrder = CreatePopupMenu();
	SoundRate = CreatePopupMenu();
	OptionsCDDrive = CreatePopupMenu();
	OptionsSRAMSize = CreatePopupMenu();

	// Création des sous-menu pricipaux

	Flags = MF_BYPOSITION | MF_POPUP | MF_STRING;

	MENU_L(MainMenu, 0, Flags, (UINT)Files, "File", "", "&File");
	MENU_L(MainMenu, 1, Flags, (UINT)Graphics, "Graphic", "", "&Graphic");
	MENU_L(MainMenu, 2, Flags, (UINT)CPU, "CPU", "", "&CPU");
	MENU_L(MainMenu, 3, Flags, (UINT)Sound, "Sound", "", "&Sound");
	MENU_L(MainMenu, 4, Flags, (UINT)Options, "Option", "", "&Option");
	MENU_L(MainMenu, 5, Flags, (UINT)Help, "Help", "", "&Help");


	// Menu Files 
	
	Flags = MF_BYPOSITION | MF_STRING;
	
	MENU_L(Files, 0, Flags, ID_FILES_OPENROM, "Open Rom", "\tCtrl+O", "&Open ROM");
	MENU_L(Files, 1, Flags, ID_FILES_CLOSEROM, "Free Rom", "\tCtrl+C", "&Close ROM");

	i = 2;

	MENU_L(Files, i++, Flags, ID_FILES_BOOTCD, "Boot CD", "\tCtrl+B", "&Boot CD");

// 	if (Kaillera_Initialised)
// 	{
// 		MENU_L(Files, i++, Flags, ID_FILES_NETPLAY, "Netplay", "", "&Netplay");
// 	}

	if( strcmp( RA_Username(), "Scott" ) == 0 )
	{
		InsertMenu(Files, i++, MF_SEPARATOR, NULL, NULL);

		MENU_L(Files, i++, Flags, ID_FILES_GAMEGENIE, "Game Genie", "", "&Game Genie");
	}

	InsertMenu(Files, i++, MF_SEPARATOR, NULL, NULL);

	MENU_L(Files, i++, Flags, ID_FILES_LOADSTATEAS, "Load State as", "\tShift+F8", "&Load State ...");
	MENU_L(Files, i++, Flags, ID_FILES_SAVESTATEAS, "Save State as", "\tShift+F5", "&Save State as...");
	MENU_L(Files, i++, Flags, ID_FILES_LOADSTATE, "Load State", "\tF8", "Quick &Load");
	MENU_L(Files, i++, Flags, ID_FILES_SAVESTATE, "Save State", "\tF5", "Quick &Save");
	MENU_L(Files, i++, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)FilesChangeState, "Change State", "\tF6-F7", "C&hange State");

	InsertMenu(Files, i++, MF_SEPARATOR, NULL, NULL);

	if (strcmp(Recent_Rom[0], ""))
	{
		MENU_L(Files, i++, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)FilesHistory, "Rom History", "", "&ROM History");
		InsertMenu(Files, i++, MF_SEPARATOR, NULL, NULL);
	}

	MENU_L(Files, i++, Flags, ID_FILES_QUIT, "Quit", "", "&Quit");



	// Menu FilesChangeState
	
	for(i = 0; i < 10; i++)
	{
		wsprintf(Str_Tmp ,"&%d", i);

		if(Current_State == i)
			InsertMenu(FilesChangeState, i, Flags | MF_CHECKED, ID_FILES_CHANGESTATE_0 + i, Str_Tmp);
		else
			InsertMenu(FilesChangeState, i, Flags | MF_UNCHECKED, ID_FILES_CHANGESTATE_0 + i, Str_Tmp);
	}


	// Menu FilesHistory
	
	for(i = 0; i < 9; i++)
	{
		if (strcmp(Recent_Rom[i], ""))
		{
			char tmp[1024];

			switch (Detect_Format(Recent_Rom[i]) >> 1)			// do not exist anymore
			{
				default:
					strcpy_s(tmp, 1024, "[---]\t- ");
					break;

				case 1:
					strcpy_s(tmp, 1024, "[MD]\t- ");
					break;

				case 2:
					strcpy_s(tmp, 1024, "[32X]\t- ");
					break;

				case 3:
					strcpy_s(tmp, 1024, "[SCD]\t- ");
					break;

				case 4:
					strcpy_s(tmp, 1024, "[SCDX]\t- ");
					break;
			}

			Get_Name_From_Path(Recent_Rom[i], Str_Tmp);
			strcat_s(tmp, Str_Tmp);
			InsertMenu(FilesHistory, i, Flags, ID_FILES_OPENRECENTROM0 + i, tmp);

		}
		else break;
	}

	
	// Menu Graphics

	Flags = MF_BYPOSITION | MF_STRING;
	
	if (Full_Screen)
	{
		MENU_L(Graphics, 0, Flags, ID_GRAPHICS_SWITCH_MODE, "Windowed", "\tAlt+Enter", "&Windowed");
	}
	else
	{
		MENU_L(Graphics, 0, Flags, ID_GRAPHICS_SWITCH_MODE, "Full Screen", "\tAlt+Enter", "&Full Screen");
	}

	if ((Full_Screen && FS_VSync) || (!Full_Screen && W_VSync))
	{
		MENU_L(Graphics, 1, Flags | MF_CHECKED, ID_GRAPHICS_VSYNC, "VSync", "\tShift+F3", "&VSync");
	}
	else
	{
		MENU_L(Graphics, 1, Flags | MF_UNCHECKED, ID_GRAPHICS_VSYNC, "VSync", "\tShift+F3", "&VSync");
	}

	if ((Full_Screen) && (Render_FS > 1))
	{
		MENU_L(Graphics, 2, Flags | MF_UNCHECKED | MF_GRAYED, ID_GRAPHICS_STRETCH, "Stretch", "\tShift+F2", "&Stretch");
	}
	else
	{
		if (Stretch)
		{
			MENU_L(Graphics, 2, Flags | MF_CHECKED, ID_GRAPHICS_STRETCH, "Stretch", "\tShift+F2", "&Stretch");
		}
		else
		{
			MENU_L(Graphics, 2, Flags | MF_UNCHECKED, ID_GRAPHICS_STRETCH, "Stretch", "\tShift+F2", "&Stretch");
		}
	}

	MENU_L(Graphics, 3, Flags, ID_GRAPHICS_COLOR_ADJUST, "Color", "", "&Color Adjust...");
	MENU_L(Graphics, 4, Flags | MF_POPUP, (UINT)GraphicsRender, "Render", "", "&Render");
	InsertMenu(Graphics, 5, MF_SEPARATOR, NULL, NULL);

	if (Sprite_Over)
	{
		MENU_L(Graphics, 6, Flags | MF_CHECKED, ID_GRAPHICS_SPRITEOVER, "Sprite Limit", "", "&Sprite Limit");
	}
	else
	{
		MENU_L(Graphics, 6, Flags | MF_UNCHECKED, ID_GRAPHICS_SPRITEOVER, "Sprite Limit", "", "&Sprite Limit");
	}

	InsertMenu(Graphics, 7, MF_SEPARATOR, NULL, NULL);
	MENU_L(Graphics, 8, MF_BYPOSITION | MF_POPUP | MF_STRING, (UINT)GraphicsFrameSkip, "Frame Skip", "", "&Frame Skip");
	InsertMenu(Graphics, 9, MF_SEPARATOR, NULL, NULL);
	MENU_L(Graphics, 10, Flags | MF_UNCHECKED, ID_GRAPHICS_SHOT, "Screen Shot", "\tShift+Backspc", "&Screen Shot");


	// Menu GraphicsRender


	if (Rend == 0)
	{
		MENU_L(GraphicsRender, 0, MF_BYPOSITION | MF_STRING | MF_CHECKED, ID_GRAPHICS_RENDER_NORMAL, "Normal", "", "&Normal");
	}
	else
	{
		MENU_L(GraphicsRender, 0, MF_BYPOSITION | MF_STRING | MF_UNCHECKED, ID_GRAPHICS_RENDER_NORMAL, "Normal", "", "&Normal");
	}

	if (Rend == 1)
	{
		MENU_L(GraphicsRender, 1, MF_BYPOSITION | MF_STRING | MF_CHECKED, ID_GRAPHICS_RENDER_DOUBLE, "Double", "", "&Double");
	}
	else
	{
		MENU_L(GraphicsRender, 1, MF_BYPOSITION | MF_STRING | MF_UNCHECKED, ID_GRAPHICS_RENDER_DOUBLE, "Double", "", "&Double");
	}

	if (Rend == 2)
	{
		MENU_L(GraphicsRender, 2, MF_BYPOSITION | MF_CHECKED, ID_GRAPHICS_RENDER_DOUBLE_INT, "Interpolated", "", "&Interpolated");
	}
	else
	{
		MENU_L(GraphicsRender, 2, MF_BYPOSITION | MF_UNCHECKED, ID_GRAPHICS_RENDER_DOUBLE_INT, "Interpolated", "", "&Interpolated");
	}

	if (Rend == 3)
	{
		MENU_L(GraphicsRender, 3, MF_BYPOSITION | MF_CHECKED, ID_GRAPHICS_RENDER_FULLSCANLINE, "Scanline", "", "&Scanline");
	}
	else
	{
		MENU_L(GraphicsRender, 3, MF_BYPOSITION | MF_UNCHECKED, ID_GRAPHICS_RENDER_FULLSCANLINE, "Scanline", "", "&Scanline");
	}

	i = 4;
	
	if (Have_MMX)
	{
		if (Rend == 4)
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_STRING | MF_CHECKED, ID_GRAPHICS_RENDER_50SCANLINE, "50% Scanline", "", "&50% Scanline");
		}
		else
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_STRING | MF_UNCHECKED, ID_GRAPHICS_RENDER_50SCANLINE, "50% Scanline", "", "&50% Scanline");
		}

		if (Rend == 5)
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_CHECKED, ID_GRAPHICS_RENDER_25SCANLINE, "25% Scanline", "", "&25% Scanline");
		}
		else
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_UNCHECKED, ID_GRAPHICS_RENDER_25SCANLINE, "25% Scanline", "", "&25% Scanline");
		}
	}

	if (Rend == 6)
	{
		MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_STRING | MF_CHECKED, ID_GRAPHICS_RENDER_INTESCANLINE, "Interpolated Scanline", "", "&Interpolated Scanline");
	}
	else
	{
		MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_STRING | MF_UNCHECKED, ID_GRAPHICS_RENDER_INTESCANLINE, "Interpolated Scanline", "", "&Interpolated Scanline");
	}

	if (Have_MMX)
	{
		if (Rend == 7)
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_STRING | MF_CHECKED, ID_GRAPHICS_RENDER_INT50SCANLIN, "Interpolated 50% Scanline", "", "Interpolated 50% Scanline");
		}
		else
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_STRING | MF_UNCHECKED, ID_GRAPHICS_RENDER_INT50SCANLIN, "Interpolated 50% Scanline", "", "Interpolated 50% Scanline");
		}

		if (Rend == 8)
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_CHECKED, ID_GRAPHICS_RENDER_INT25SCANLIN, "Interpolated 25% Scanline", "", "Interpolated 25% Scanline");
		}
		else
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_UNCHECKED, ID_GRAPHICS_RENDER_INT25SCANLIN, "Interpolated 25% Scanline", "", "Interpolated 25% Scanline");
		}

		if (Rend == 9)
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_CHECKED, ID_GRAPHICS_RENDER_2XSAI, "2xSAI (Kreed)", "", "2xSAI (&Kreed)");
		}
		else
		{
			MENU_L(GraphicsRender, i++, MF_BYPOSITION | MF_UNCHECKED, ID_GRAPHICS_RENDER_2XSAI, "2xSAI (Kreed)", "", "2xSAI (&Kreed)");
		}
	}


	// Menu GraphicsFrameSkip

	Flags = MF_BYPOSITION | MF_STRING;

	if (Frame_Skip == -1)
	{
		MENU_L(GraphicsFrameSkip, 0, Flags | MF_CHECKED, ID_GRAPHICS_FRAMESKIP_AUTO, "Auto", "", "&Auto");
	}
	else
	{
		MENU_L(GraphicsFrameSkip, 0, Flags | MF_UNCHECKED, ID_GRAPHICS_FRAMESKIP_AUTO, "Auto", "", "&Auto");
	}

	for(i = 0; i < 9; i++)
	{
		wsprintf(Str_Tmp ,"&%d", i);

		if (Frame_Skip == i)
			InsertMenu(GraphicsFrameSkip, i + 1, Flags | MF_CHECKED, ID_GRAPHICS_FRAMESKIP_0 + i, Str_Tmp);
		else
			InsertMenu(GraphicsFrameSkip, i + 1, Flags | MF_UNCHECKED, ID_GRAPHICS_FRAMESKIP_0 + i, Str_Tmp);
	}
	
	// Menu CPU

	i = 0;

#ifdef GENS_DEBUG
	MENU_L(CPU, i++, Flags | MF_POPUP, (UINT)CPUDebug, "Debug", "", "&Debug");
	InsertMenu(CPU, i++, MF_SEPARATOR, NULL, NULL);
#endif

	MENU_L(CPU, i++, Flags | MF_POPUP, (UINT)CPUCountry, "Country", "", "&Country");
	InsertMenu(CPU, i++, MF_SEPARATOR, NULL, NULL);
	MENU_L(CPU, i++, Flags, ID_CPU_RESET, "Hard Reset", "\tTAB", "&Hard Reset");

	if (SegaCD_Started)
	{
		MENU_L(CPU, i++, Flags, ID_CPU_RESET68K, "Reset main 68000", "", "Reset &main 68000");
		MENU_L(CPU, i++, Flags, ID_CPU_RESET_SUB68K, "Reset sub 68000", "", "Reset &sub 68000");
	}
	else if (_32X_Started)
	{
		MENU_L(CPU, i++, Flags, ID_CPU_RESET68K, "Reset 68K", "", "Reset &68000");
		MENU_L(CPU, i++, Flags, ID_CPU_RESET_MSH2, "Reset master SH2", "", "Reset master SH2");
		MENU_L(CPU, i++, Flags, ID_CPU_RESET_SSH2, "Reset slave SH2", "", "Reset slave SH2");
	}
	else
	{
		MENU_L(CPU, i++, Flags, ID_CPU_RESET68K, "Reset 68K", "", "Reset &68000");
	}

	MENU_L(CPU, i++, Flags, ID_CPU_RESETZ80, "Reset Z80", "", "Reset &Z80");

	if (!Genesis_Started && !_32X_Started)
	{
		InsertMenu(CPU, i++, MF_SEPARATOR, NULL, NULL);

		if (SegaCD_Accurate)
		{
			MENU_L(CPU, i++, Flags | MF_CHECKED, ID_CPU_ACCURATE_SYNCHRO, "Perfect Synchro", "", "&Perfect Synchro (SLOW)");
		}
		else
		{
			MENU_L(CPU, i++, Flags | MF_UNCHECKED, ID_CPU_ACCURATE_SYNCHRO, "Perfect Synchro", "", "&Perfect Synchro (SLOW)");
		}
	}

#ifdef GENS_DEBUG
	// Menu CPU Debug

	if (Debug == 1) Flags |= MF_CHECKED;
	else Flags &= ~MF_CHECKED;

	MENU_L(CPUDebug, 0, Flags, ID_CPU_DEBUG_GENESIS_68000, "Genesis - 68000", "", "&Genesis - 68000");

	if (Debug == 2) Flags |= MF_CHECKED;
	else Flags &= ~MF_CHECKED;

	MENU_L(CPUDebug, 1, Flags, ID_CPU_DEBUG_GENESIS_Z80, "Genesis - Z80", "", "Genesis - &Z80");

	if (Debug == 3) Flags |= MF_CHECKED;
	else Flags &= ~MF_CHECKED;

	MENU_L(CPUDebug, 2, Flags, ID_CPU_DEBUG_GENESIS_VDP, "Genesis - VDP", "", "Genesis - &VDP");

	i = 3;

	if (SegaCD_Started)
	{
		if (Debug == (i + 1)) Flags |= MF_CHECKED;
		else Flags &= ~MF_CHECKED;

		MENU_L(CPUDebug, i++, Flags, ID_CPU_DEBUG_SEGACD_68000, "SegaCD - 68000", "", "&SegaCD - 68000");

		if (Debug == (i + 1)) Flags |= MF_CHECKED;
		else Flags &= ~MF_CHECKED;

		MENU_L(CPUDebug, i++, Flags, ID_CPU_DEBUG_SEGACD_CDC, "SegaCD - CDC", "", "SegaCD - &CDC");

		if (Debug == (i + 1)) Flags |= MF_CHECKED;
		else Flags &= ~MF_CHECKED;

		MENU_L(CPUDebug, i++, Flags, ID_CPU_DEBUG_SEGACD_GFX, "SegaCD - GFX", "", "SegaCD - GF&X");
	}

	if (_32X_Started)
	{
		if (Debug == (i + 1)) Flags |= MF_CHECKED;
		else Flags &= ~MF_CHECKED;

		MENU_L(CPUDebug, i++, Flags, ID_CPU_DEBUG_32X_MAINSH2, "32X - main SH2", "", "32X - main SH2");

		if (Debug == (i + 1)) Flags |= MF_CHECKED;
		else Flags &= ~MF_CHECKED;

		MENU_L(CPUDebug, i++, Flags, ID_CPU_DEBUG_32X_SUBSH2, "32X - sub SH2", "", "32X - sub SH2");

		if (Debug == (i + 1)) Flags |= MF_CHECKED;
		else Flags &= ~MF_CHECKED;

		MENU_L(CPUDebug, i++, Flags, ID_CPU_DEBUG_32X_VDP, "32X - VDP", "", "32X - VDP");
	}
#endif


	// Menu CPU Country

	Flags = MF_BYPOSITION | MF_STRING;

	if (Country == -1)
	{
		MENU_L(CPUCountry, 0, Flags | MF_CHECKED, ID_CPU_COUNTRY_AUTO, "Auto detect", "", "&Auto detect");
	}
	else
	{
		MENU_L(CPUCountry, 0, Flags | MF_UNCHECKED, ID_CPU_COUNTRY_AUTO, "Auto detect", "", "&Auto detect");
	}
	if (Country == 0)
	{
		MENU_L(CPUCountry, 1, Flags | MF_CHECKED, ID_CPU_COUNTRY_JAPAN, "Japan (NTSC)", "", "&Japan (NTSC)");
	}
	else
	{
		MENU_L(CPUCountry, 1, Flags | MF_UNCHECKED, ID_CPU_COUNTRY_JAPAN, "Japan (NTSC)", "", "&Japan (NTSC)");
	}
	if (Country == 1)
	{
		MENU_L(CPUCountry, 2, Flags | MF_CHECKED, ID_CPU_COUNTRY_USA, "USA (NTSC)", "", "&USA (NTSC)");
	}
	else
	{
		MENU_L(CPUCountry, 2, Flags | MF_UNCHECKED, ID_CPU_COUNTRY_USA, "USA (NTSC)", "", "&USA (NTSC)");
	}
	if (Country == 2)
	{
		MENU_L(CPUCountry, 3, Flags | MF_CHECKED, ID_CPU_COUNTRY_EUROPE, "Europe (PAL)", "", "&Europe (PAL)");
	}
	else
	{
		MENU_L(CPUCountry, 3, Flags | MF_UNCHECKED, ID_CPU_COUNTRY_EUROPE, "Europe (PAL)", "", "&Europe (PAL)");
	}
	if (Country == 3)
	{
		MENU_L(CPUCountry, 4, Flags | MF_CHECKED, ID_CPU_COUNTRY_MISC, "Japan (PAL)", "", "Japan (PAL)");
	}
	else
	{
		MENU_L(CPUCountry, 4, Flags | MF_UNCHECKED, ID_CPU_COUNTRY_MISC, "Japan (PAL)", "", "Japan (PAL)");
	}

	InsertMenu(CPUCountry, 5, MF_SEPARATOR, NULL, NULL);

	MENU_L(CPUCountry, 6, Flags | MF_POPUP, (UINT)CPUCountryOrder, "Auto detection order", "", "&Auto detection order");


	// Menu CPU Prefered Country 

	for(i = 0; i < 3; i++)
	{
		if (Country_Order[i] == 0)
		{
			MENU_L(CPUCountryOrder, i, Flags, ID_CPU_COUNTRY_ORDER + i, "USA (NTSC)", "", "&USA (NTSC)");
		}
		else if (Country_Order[i] == 1)
		{
			MENU_L(CPUCountryOrder, i, Flags, ID_CPU_COUNTRY_ORDER + i, "Japan (NTSC)", "", "&Japan (NTSC)");
		}
		else
		{
			MENU_L(CPUCountryOrder, i, Flags, ID_CPU_COUNTRY_ORDER + i, "Europe (PAL)", "", "&Europe (PAL)");
		}
	}


	// Menu Sound

	if (Sound_Enable)
	{
		MENU_L(Sound, 0, Flags | MF_CHECKED, ID_SOUND_ENABLE, "Enable", "", "&Enable");
	}
	else
	{
		MENU_L(Sound, 0, Flags | MF_UNCHECKED, ID_SOUND_ENABLE, "Enable", "", "&Enable");
	}

	InsertMenu(Sound, 1, MF_SEPARATOR, NULL, NULL);

	MENU_L(Sound, 2, Flags | MF_POPUP, (UINT)SoundRate, "Rate", "", "&Rate");

	if (Sound_Stereo)
	{
		MENU_L(Sound, 3, Flags | MF_CHECKED, ID_SOUND_STEREO, "Stereo", "", "&Stereo");
	}
	else
	{
		MENU_L(Sound, 3, Flags | MF_UNCHECKED, ID_SOUND_STEREO, "Stereo", "", "&Stereo");
	}

	InsertMenu(Sound, 4, MF_SEPARATOR, NULL, NULL);

	if (Z80_State & 1)
		InsertMenu(Sound, 5, Flags | MF_CHECKED, ID_SOUND_Z80ENABLE, "&Z80");
	else
		InsertMenu(Sound, 5, Flags | MF_UNCHECKED, ID_SOUND_Z80ENABLE, "&Z80");

	if (YM2612_Enable)
		InsertMenu(Sound, 6, Flags | MF_CHECKED, ID_SOUND_YM2612ENABLE, "&YM2612");
	else
		InsertMenu(Sound, 6, Flags | MF_UNCHECKED, ID_SOUND_YM2612ENABLE, "&YM2612");

	if (PSG_Enable)
		InsertMenu(Sound, 7, Flags | MF_CHECKED, ID_SOUND_PSGENABLE, "&PSG");
	else
		InsertMenu(Sound, 7, Flags | MF_UNCHECKED, ID_SOUND_PSGENABLE, "&PSG");

	if (DAC_Enable)
		InsertMenu(Sound, 8, Flags | MF_CHECKED, ID_SOUND_DACENABLE, "&DAC");
	else
		InsertMenu(Sound, 8, Flags | MF_UNCHECKED, ID_SOUND_DACENABLE, "&DAC");

	i = 9;
	
	if (!Genesis_Started && !_32X_Started)
	{
		if (PCM_Enable)
			InsertMenu(Sound, i++, Flags | MF_CHECKED, ID_SOUND_PCMENABLE, "P&CM");
		else
			InsertMenu(Sound, i++, Flags | MF_UNCHECKED, ID_SOUND_PCMENABLE, "P&CM");
	}

	if (!Genesis_Started && !SegaCD_Started)
	{
		if (PWM_Enable)
			InsertMenu(Sound, i++, Flags | MF_CHECKED, ID_SOUND_PWMENABLE, "P&WM");
		else
			InsertMenu(Sound, i++, Flags | MF_UNCHECKED, ID_SOUND_PWMENABLE, "P&WM");
	}

	if (!Genesis_Started && !_32X_Started)
	{
		if (CDDA_Enable)
			InsertMenu(Sound, i++, Flags | MF_CHECKED, ID_SOUND_CDDAENABLE, "CDD&A");
		else
			InsertMenu(Sound, i++, Flags | MF_UNCHECKED, ID_SOUND_CDDAENABLE, "CDD&A");
	}

	InsertMenu(Sound, i++, MF_SEPARATOR, NULL, NULL);

	if (YM2612_Improv)
	{
		MENU_L(Sound, i++, Flags | MF_CHECKED, ID_SOUND_YMIMPROV, "YM2612 High Quality", "", "YM2612 High &Quality");
	}
	else
	{
		MENU_L(Sound, i++, Flags | MF_UNCHECKED, ID_SOUND_YMIMPROV, "YM2612 High Quality", "", "YM2612 High &Quality");
	}

	InsertMenu(Sound, i++, MF_SEPARATOR, NULL, NULL);

	if (WAV_Dumping)
	{
		MENU_L(Sound, i++, Flags, ID_SOUND_STARTWAVDUMP, "Stop Dump", "", "Stop WAV Dump");
	}
	else
	{
		MENU_L(Sound, i++, Flags, ID_SOUND_STARTWAVDUMP, "Start Dump", "", "Start WAV Dump");
	}

	if (GYM_Dumping)
	{
		MENU_L(Sound, i++, Flags, ID_SOUND_STARTGYMDUMP, "Stop GYM Dump", "", "Stop GYM Dump");
	}
	else
	{
		MENU_L(Sound, i++, Flags, ID_SOUND_STARTGYMDUMP, "Start GYM Dump", "", "Start GYM Dump");
	}

	// Sous-Menu SoundRate

	if (Sound_Rate == 11025)
		InsertMenu(SoundRate, 0, Flags | MF_CHECKED, ID_SOUND_RATE_11000, "&11025");
	else
		InsertMenu(SoundRate, 0, Flags | MF_UNCHECKED, ID_SOUND_RATE_11000, "&11025");

	if (Sound_Rate == 22050)
		InsertMenu(SoundRate, 1, Flags | MF_CHECKED, ID_SOUND_RATE_22000, "&22050");
	else
		InsertMenu(SoundRate, 1, Flags | MF_UNCHECKED, ID_SOUND_RATE_22000, "&22050");

	if (Sound_Rate == 44100)
		InsertMenu(SoundRate, 2, Flags | MF_CHECKED, ID_SOUND_RATE_44000, "&44100");
	else
		InsertMenu(SoundRate, 2, Flags | MF_UNCHECKED, ID_SOUND_RATE_44000, "&44100");

	
	// Menu Options

	MENU_L(Options, 0, MF_BYPOSITION | MF_STRING, ID_OPTIONS_GENERAL, "Misc", "", "&Misc...");
	MENU_L(Options, 1, Flags, ID_OPTIONS_JOYPADSETTING, "Joypad", "", "&Joypads...");
	MENU_L(Options, 2, Flags, ID_OPTIONS_CHANGEDIR, "Directories", "", "&Directories...");
	MENU_L(Options, 3, Flags, ID_OPTIONS_CHANGEFILES, "Bios/Misc Files", "", "Bios/Misc &Files...");

	InsertMenu(Options, 4, MF_SEPARATOR, NULL, NULL);
	MENU_L(Options, 5, Flags | MF_POPUP, (UINT)OptionsCDDrive, "Current CD Drive", "", "Current CD Drive");
	MENU_L(Options, 6, Flags | MF_POPUP, (UINT)OptionsSRAMSize, "Sega CD SRAM Size", "", "Sega CD SRAM Size");
	InsertMenu(Options, 7, MF_SEPARATOR, NULL, NULL);

	MENU_L(Options, 8, Flags, ID_OPTIONS_LOADCONFIG, "Load config", "", "&Load config");
	MENU_L(Options, 9, Flags, ID_OPTIONS_SAVEASCONFIG, "Save config as", "", "&Save config as");


	// Sous-Menu CDDrive

	if (Num_CD_Drive)
	{
		char drive_name[100];

		for(i = 0; i < Num_CD_Drive; i++)
		{
			ASPI_Get_Drive_Info(i, (unsigned char *) drive_name);

			if (CUR_DEV == i)
			{
				InsertMenu(OptionsCDDrive, i, Flags | MF_CHECKED, ID_OPTION_CDDRIVE_0 + i, &drive_name[8]);
			}
			else
			{
				InsertMenu(OptionsCDDrive, i, Flags | MF_UNCHECKED, ID_OPTION_CDDRIVE_0 + i, &drive_name[8]);
			}
		}
	}
	else
	{
		MENU_L(OptionsCDDrive, 0, Flags | MF_GRAYED, NULL, "No drive detected", "", "No Drive Detected");
	}


	// Sous-Menu SRAMSize

	if (BRAM_Ex_State & 0x100)
	{
		MENU_L(OptionsSRAMSize, 0, Flags | MF_UNCHECKED, ID_OPTION_SRAMSIZE_0, "None", "", "&None");

		for (i = 0; i < 4; i++)
		{
			char bsize[16];

			sprintf_s(bsize, 16, "&%d Kb", 8 << i);

			if (BRAM_Ex_Size == i)
			{
				InsertMenu(OptionsSRAMSize, i + 1, Flags | MF_CHECKED, ID_OPTION_SRAMSIZE_8 + i, bsize);
			}
			else
			{
				InsertMenu(OptionsSRAMSize, i + 1, Flags | MF_UNCHECKED, ID_OPTION_SRAMSIZE_8 + i, bsize);
			}
		}
	}
	else
	{
		MENU_L(OptionsSRAMSize, 0, Flags | MF_CHECKED, ID_OPTION_SRAMSIZE_0, "None", "", "&None");

		for (i = 0; i < 4; i++)
		{
			char bsize[16];

			sprintf_s(bsize, 16, "&%d Kb", 8 << i);

			InsertMenu(OptionsSRAMSize, i + 1, Flags | MF_UNCHECKED, ID_OPTION_SRAMSIZE_8 + i, bsize);
		}
	}


	// Menu Help

	i = 0;
	
	while (language_name[i])
	{
		GetPrivateProfileString(language_name[i], "Menu Language", "Undefined language", Str_Tmp, 1024, Language_Path);
		if (Language == i)
			InsertMenu(Help, i, Flags | MF_CHECKED, ID_HELP_LANG + i, Str_Tmp);
		else 
			InsertMenu(Help, i, Flags | MF_UNCHECKED, ID_HELP_LANG + i, Str_Tmp);

		i++;
	}

	InsertMenu(Help, i++, MF_SEPARATOR, NULL, NULL);

	if (Detect_Format(Manual_Path) != -1)		// can be used to detect if file exist
	{
		MENU_L(Help, i++, Flags, ID_HELP_MENU_FILE, "File menu" ,"", "&File menu");
		MENU_L(Help, i++, Flags, ID_HELP_MENU_GRAPHICS, "Graphics menu" ,"", "&Graphics menu");
		MENU_L(Help, i++, Flags, ID_HELP_MENU_CPU, "CPU menu" ,"", "&CPU menu");
		MENU_L(Help, i++, Flags, ID_HELP_MENU_SOUND, "Sound menu" ,"", "&Sound menu");
		MENU_L(Help, i++, Flags, ID_HELP_MENU_OPTIONS, "Options menu" ,"", "&Options menu");

		InsertMenu(Help, i++, MF_SEPARATOR, NULL, NULL);

		MENU_L(Help, i++, Flags, ID_HELP_NETPLAY, "Netplay" ,"", "&Netplay");
		MENU_L(Help, i++, Flags, ID_HELP_MEGACD, "Mega/Sega CD" ,"", "&Mega/Sega CD");
		MENU_L(Help, i++, Flags, ID_HELP_FAQ, "FAQ" ,"", "&FAQ");
		MENU_L(Help, i++, Flags, ID_HELP_KEYS, "Shortcuts" ,"", "&Defaults keys && Shortcuts");

		InsertMenu(Help, i++, MF_SEPARATOR, NULL, NULL);
	}

	MENU_L(Help, i++, Flags, ID_HELP_ABOUT, "About" ,"", "&About");

	//	##RA embed RA
	HMENU hRA = RA_CreatePopupMenu();
	if( hRA )
		AppendMenu( MainMenu, MF_POPUP|MF_STRING, (UINT_PTR)hRA, TEXT("RetroAchievements") );

	Gens_Menu = MainMenu;

	if (Full_Screen) SetMenu(HWnd, NULL);
	else SetMenu(HWnd, Gens_Menu);

	return(Gens_Menu);
}

void UpdateAppTitle()
{
	char detailBuffer[1024];
	if (Genesis_Started)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			sprintf_s(detailBuffer, "- Megadrive : %s", Game->Rom_Name_W);
		else
			sprintf_s(detailBuffer, "- Genesis : %s", Game->Rom_Name_W);
	}
	else if (_32X_Started)
	{
		if (CPU_Mode == 1)
			sprintf_s(detailBuffer, "- 32X (PAL) : %s", Game->Rom_Name_W);
		else
			sprintf_s(detailBuffer, "- 32X (NTSC) : %s", Game->Rom_Name_W);
	}
	else if (SegaCD_Started)
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			sprintf_s(detailBuffer, "- MegaCD : %s", Rom_Name);
		else
			sprintf_s(detailBuffer, "- SegaCD : %s", Rom_Name);
	}
	else
	{
		if ((CPU_Mode == 1) || (Game_Mode == 0))
			sprintf_s(detailBuffer, "- Idle");
		else
			sprintf_s(detailBuffer, "- Idle");
	}
	
	RA_UpdateAppTitle( detailBuffer );
}
