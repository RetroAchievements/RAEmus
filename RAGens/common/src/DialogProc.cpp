#include "DialogProc.h"

#include "resource.h"
#include <windows.h>
#include <windowsx.h>
#include <stdio.h>
#include <CommCtrl.h>

#include "ggenie.h"
#include "io.h"
#include "vdp_io.h"
#include "gens.h"
#include "save.h"
#include "rom.h"
#include "scrshot.h"
#include "Mem_M68k.h"
#include "G_Main.h"
#include "G_Input.h"
#include "G_ddraw.h"
#include "G_dsound.h"

#include "md5.h"


#ifndef WORD_L
#define WORD_L(id, str, suffixe, def)															\
	GetPrivateProfileString(language_name[Language], (str), (def), Str_Tmp, 1024, Language_Path);	\
	strcat_s(Str_Tmp, 1024, (suffixe));																			\
	SetDlgItemText(hDlg, id, Str_Tmp);
#endif //WORD_L


INT_PTR CALLBACK GGenieProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
 {
 	RECT r;
 	RECT r2;
 	int dx1, dy1, dx2, dy2, i, value;
 	char tmp[1024];
 
 	switch(uMsg)
 	{
 		case WM_INITDIALOG:
 			Build_Language_String();
 
 			GetWindowRect(HWnd, &r);
 			dx1 = (r.right - r.left) / 2;
 			dy1 = (r.bottom - r.top) / 2;
 
 			GetWindowRect(hDlg, &r2);
 			dx2 = (r2.right - r2.left) / 2;
 			dy2 = (r2.bottom - r2.top) / 2;
 
 			SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
 
 			WORD_L(IDC_INFO_GG, "Informations GG", "", "Informations about GG/Patch codes");
 			WORD_L(IDC_GGINFO1, "Game Genie info 1", "", "Both Game Genie code and Patch code are supported.");
 			WORD_L(IDC_GGINFO2, "Game Genie info 2", "", "Highlight a code to activate it.");
 			WORD_L(IDC_GGINFO3, "Game Genie info 3", "", "yntax for Game Genie code :  XXXX-XXXX");
 			WORD_L(IDC_GGINFO4, "Game Genie info 4", "", "Syntax for Patch code :  XXXXXX:YYYY    (address:data)");
 
 			WORD_L(ID_GGADD, "Add code", "", "Add &code");
 			WORD_L(ID_GGREMOVE, "Remove selected codes", "", "&Remove selected codes");
 			WORD_L(ID_GGDESACTIVE, "Desactive all codes", "", "&Desactive all codes");
 			WORD_L(ID_OK, "OK", "", "&OK");
 			WORD_L(ID_CANCEL, "Cancel", "", "&Cancel");
 
 			for(i = 0; i < 256; i++)
 			{
 				if (Liste_GG[i].code[0] != 0)
 				{
 					strcpy_s(Str_Tmp, Liste_GG[i].code);
 					while (strlen(Str_Tmp) < 20) strcat_s(Str_Tmp, 1024, " ");
 					strcat_s(Str_Tmp, 1024, Liste_GG[i].name);
 
 					SendDlgItemMessage(hDlg, IDC_LIST1, LB_ADDSTRING, (WPARAM) 0, (LONG) (LPTSTR) Str_Tmp);
 
 					if (Liste_GG[i].active)
 						SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETSEL, (WPARAM) 1, (LONG) i);
 					else
 						SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETSEL, (WPARAM) 0, (LONG) i);
 
 					if ((Liste_GG[i].restore != 0xFFFFFFFF) && (Liste_GG[i].addr < Rom_Size) && (Genesis_Started))
 					{
 						Rom_Data[Liste_GG[i].addr] = (unsigned char)(Liste_GG[i].restore & 0xFF);
 						Rom_Data[Liste_GG[i].addr + 1] = (unsigned char)((Liste_GG[i].restore & 0xFF00) >> 8);
 					}
 				}
 			}
 			return true;
 			break;
 
 		case WM_COMMAND:
 			switch(LOWORD(wParam))
 			{
 				case ID_GGADD:
 					if (GetDlgItemText(hDlg, IDC_EDIT1, Str_Tmp, 14))
 					{
 						if ((strlen(Str_Tmp) == 9) || (strlen(Str_Tmp) == 11))
 						{						
 							_strupr_s(Str_Tmp);
 							while (strlen(Str_Tmp) < 20) strcat_s(Str_Tmp, 1024, " ");
 
 							GetDlgItemText(hDlg, IDC_EDIT2, (char *) (Str_Tmp + strlen(Str_Tmp)), 240);
 
 							SendDlgItemMessage(hDlg, IDC_LIST1, LB_ADDSTRING, (WPARAM) 0, (LONG) (LPTSTR) Str_Tmp);
 
 							SetDlgItemText(hDlg, IDC_EDIT1, "");
 							SetDlgItemText(hDlg, IDC_EDIT2, "");
 						}
 					}
 					return true;
 					break;
 
 				case ID_GGREMOVE:
 					value = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
 					if (value == LB_ERR) value = 0;
 
 					for(i = value - 1; i >= 0; i--)
 					{
 						if (SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETSEL, (WPARAM) i, NULL) > 0)
 							SendDlgItemMessage(hDlg, IDC_LIST1, LB_DELETESTRING , (WPARAM) i, (LPARAM) 0);
 					}
 					return true;
 					break;
 
 				case ID_GGDESACTIVE:
 					value = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
 					if (value == LB_ERR) value = 0;
 
 					for(i = value - 1; i >= 0; i--)
 					{
 						SendDlgItemMessage(hDlg, IDC_LIST1, LB_SETSEL , (WPARAM) 0, (LPARAM) i);
 					}
 					return true;
 					break;
 
 				case ID_OK:
 					value = SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETCOUNT, (WPARAM) 0, (LPARAM) 0);
 					if (value == LB_ERR) value = 0;
 
 					for(i = 0; i < 256; i++)
 					{
 						Liste_GG[i].code[0] = 0;
 						Liste_GG[i].name[0] = 0;
 						Liste_GG[i].active = 0;
 						Liste_GG[i].addr = 0xFFFFFFFF;
 						Liste_GG[i].data = 0;
 						Liste_GG[i].restore = 0xFFFFFFFF;
 					}
 
 					for(i = 0; i < value; i++)
 					{
 						if (SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETTEXT, (WPARAM) i, (LONG) (LPTSTR) tmp) != LB_ERR)
 						{
 							dx1 = 0;
 
 							while ((tmp[dx1] != ' ') && (tmp[dx1] != 0)) dx1++;
 
 							memcpy(Liste_GG[i].code, tmp, dx1);
 							Liste_GG[i].code[dx1] = 0;
 
 							while ((tmp[dx1] == ' ') && (tmp[dx1] != 0)) dx1++;
 
 							strcpy_s(Liste_GG[i].name, (char *) (tmp + dx1));
 
 							if (SendDlgItemMessage(hDlg, IDC_LIST1, LB_GETSEL, (WPARAM) i, NULL) > 0)
 								Liste_GG[i].active = 1;
 							else Liste_GG[i].active = 0;
 						}
 					}
 
 					for(i = 0; i < value; i++)
 					{
 						if ((Liste_GG[i].code[0] != 0) && (Liste_GG[i].addr == 0xFFFFFFFF) && (Liste_GG[i].data == 0))
 						{
 							decode(Liste_GG[i].code, (patch *) (&(Liste_GG[i].addr)));
 
 							if ((Liste_GG[i].restore == 0xFFFFFFFF) && (Liste_GG[i].addr < Rom_Size) && (Genesis_Started))
 							{
 								Liste_GG[i].restore = (unsigned int) (Rom_Data[Liste_GG[i].addr] & 0xFF);
 								Liste_GG[i].restore += (unsigned int) ((Rom_Data[Liste_GG[i].addr + 1] & 0xFF) << 8);
 							}
 						}
 					}
 
 				case ID_CANCEL:
 					EndDialog(hDlg, true);
 					return true;
 					break;
 
 
 				case ID_HELP_HELP:
 					if (Detect_Format(Manual_Path) != -1)		// can be used to detect if file exist
 					{
 						strcpy_s(Str_Tmp, Manual_Path);
 						strcat_s(Str_Tmp, 1024, " helpgamegenie.html");
 						system(Str_Tmp);
 					}
 					else
 					{
 						MessageBox(NULL, "You need to configure the Manual Path to have help", "info", MB_OK);
 					}
 					return true;
 					break;
 			}
 			break;
 
 		case WM_CLOSE:
 			EndDialog(hDlg, true);
 			return true;
 			break;
 	}
 
 	return false;
 }


INT_PTR CALLBACK DirectoriesProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char Str_Tmp2[1024];
	
	switch(uMsg)
	{
		case WM_INITDIALOG:
			RECT r;
			RECT r2;
			int dx1, dy1, dx2, dy2;

			Build_Language_String();

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			WORD_L(IDD_DIRECTORIES, "Directories configuration", "", "Directories configuration");
			WORD_L(IDC_DIRECTORIES, "Setting directories", "", "Configure directories");
						
			WORD_L(ID_CANCEL, "Cancel", "", "&Cancel");
			WORD_L(ID_OK, "OK", "", "&OK");

			WORD_L(ID_CHANGE_SAVE, "Change", "", "Change");
			WORD_L(ID_CHANGE_SRAM, "Change", "", "Change");
			WORD_L(ID_CHANGE_BRAM, "Change", "", "Change");
			WORD_L(ID_CHANGE_WAV, "Change", "", "Change");
			WORD_L(ID_CHANGE_GYM, "Change", "", "Change");
			WORD_L(ID_CHANGE_SHOT, "Change", "", "Change");
			WORD_L(ID_CHANGE_PATCH, "Change", "", "Change");
			WORD_L(ID_CHANGE_IPS, "Change", "", "Change");

			WORD_L(IDC_STATIC_SAVE, "Save static", "", "SAVE STATE");
			WORD_L(IDC_STATIC_SRAM, "Sram static", "", "SRAM BACKUP");
			WORD_L(IDC_STATIC_BRAM, "Bram static", "", "BRAM BACKUP");
			WORD_L(IDC_STATIC_WAV, "Wav static", "", "WAV DUMP");
			WORD_L(IDC_STATIC_GYM, "Gym static", "", "GYM DUMP");
			WORD_L(IDC_STATIC_SHOT, "Shot static", "", "SCREEN SHOT");
			WORD_L(IDC_STATIC_PATCH, "Patch static", "", "PAT PATCH");
			WORD_L(IDC_STATIC_IPS, "IPS static", "", "IPS PATCH");

			SetDlgItemText(hDlg, IDC_EDIT_SAVE, State_Dir);
			SetDlgItemText(hDlg, IDC_EDIT_SRAM, SRAM_Dir);
			SetDlgItemText(hDlg, IDC_EDIT_BRAM, BRAM_Dir);
			SetDlgItemText(hDlg, IDC_EDIT_WAV, Dump_Dir);
			SetDlgItemText(hDlg, IDC_EDIT_GYM, Dump_GYM_Dir);
			SetDlgItemText(hDlg, IDC_EDIT_SHOT, ScrShot_Dir);
			SetDlgItemText(hDlg, IDC_EDIT_PATCH, Patch_Dir);
			SetDlgItemText(hDlg, IDC_EDIT_IPS, IPS_Dir);

			return true;
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case ID_CHANGE_SAVE:
					GetDlgItemText(hDlg, IDC_EDIT_SAVE, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "Save state directory", "Save state files\0*.gs*\0\0", "gs0"))
						SetDlgItemText(hDlg, IDC_EDIT_SAVE, Str_Tmp);
					break;

				case ID_CHANGE_SRAM:
					GetDlgItemText(hDlg, IDC_EDIT_SRAM, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "SRAM backup directory", "SRAM backup files\0*.srm\0\0", "srm"))
						SetDlgItemText(hDlg, IDC_EDIT_SRAM, Str_Tmp);
					break;

				case ID_CHANGE_BRAM:
					GetDlgItemText(hDlg, IDC_EDIT_BRAM, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "BRAM backup directory", "BRAM backup files\0*.brm\0\0", "brm"))
						SetDlgItemText(hDlg, IDC_EDIT_BRAM, Str_Tmp);
					break;

				case ID_CHANGE_WAV:
					GetDlgItemText(hDlg, IDC_EDIT_WAV, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "Sound WAV dump directory", "Sound WAV dump files\0*.wav\0\0", "wav"))
						SetDlgItemText(hDlg, IDC_EDIT_WAV, Str_Tmp);
					break;

				case ID_CHANGE_GYM:
					GetDlgItemText(hDlg, IDC_EDIT_GYM, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "GYM dump directory", "GYM dump files\0*.gym\0\0", "gym"))
						SetDlgItemText(hDlg, IDC_EDIT_GYM, Str_Tmp);
					break;

				case ID_CHANGE_SHOT:
					GetDlgItemText(hDlg, IDC_EDIT_SHOT, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "Screen-shot directory", "Screen-shot files\0*.bmp\0\0", "bmp"))
						SetDlgItemText(hDlg, IDC_EDIT_SHOT, Str_Tmp);
					break;

				case ID_CHANGE_PATCH:
					GetDlgItemText(hDlg, IDC_EDIT_PATCH, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "PAT Patch directory", "PAT Patch files\0*.pat\0\0", "pat"))
						SetDlgItemText(hDlg, IDC_EDIT_PATCH, Str_Tmp);
					break;

				case ID_CHANGE_IPS:
					GetDlgItemText(hDlg, IDC_EDIT_IPS, Str_Tmp2, 1024);
					if (Change_Dir(Str_Tmp, Str_Tmp2, "IPS Patch directory", "IPS Patch files\0*.ips\0\0", "ips"))
						SetDlgItemText(hDlg, IDC_EDIT_IPS, Str_Tmp);
					break;

				case ID_OK:
					GetDlgItemText(hDlg, IDC_EDIT_SAVE, State_Dir, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_SRAM, SRAM_Dir, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_BRAM, BRAM_Dir, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_WAV, Dump_Dir, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_GYM, Dump_GYM_Dir, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_SHOT, ScrShot_Dir, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_PATCH, Patch_Dir, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_IPS, IPS_Dir, 1024);

				case ID_CANCEL:
					EndDialog(hDlg, true);
					return true;
					break;

				case ID_HELP_HELP:
					if (Detect_Format(Manual_Path) != -1)		// can be used to detect if file exist
					{
						strcpy_s(Str_Tmp, Manual_Path);
						strcat_s(Str_Tmp, 1024, " helpdir.html");
						system(Str_Tmp);
					}
					else
					{
						MessageBox(NULL, "You need to configure the Manual Path to have help", "info", MB_OK);
					}
					return true;
					break;
			}
			break;

		case WM_CLOSE:
			EndDialog(hDlg, true);
			return true;
			break;
	}

	return false;
}


INT_PTR CALLBACK FilesProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	char Str_Tmp2[1024];
	
	switch(uMsg)
	{
		case WM_INITDIALOG:
			RECT r;
			RECT r2;
			int dx1, dy1, dx2, dy2;

			Build_Language_String();

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			WORD_L(IDD_FILES, "Files configuration", "", "Files configuration");
			WORD_L(IDC_GENESISBIOS_FILE, "Setting Genesis bios file", "", "Configure Genesis bios file");
			WORD_L(IDC_32XBIOS_FILES, "Setting 32X bios files", "", "Configure 32X bios files");
			WORD_L(IDC_CDBIOS_FILES, "Setting SEGA CD bios files", "", "Configure SEGA CD bios files");
			WORD_L(IDC_MISC_FILES, "Setting misc files", "", "Configure misc file");
						
			WORD_L(ID_CANCEL, "Cancel", "", "&Cancel");
			WORD_L(ID_OK, "OK", "", "&OK");

			WORD_L(ID_CHANGE_GENESISBIOS, "Change", "", "Change");
			WORD_L(ID_CHANGE_32XGBIOS, "Change", "", "Change");
			WORD_L(ID_CHANGE_32XMBIOS, "Change", "", "Change");
			WORD_L(ID_CHANGE_32XSBIOS, "Change", "", "Change");
			WORD_L(ID_CHANGE_USBIOS, "Change", "", "Change");
			WORD_L(ID_CHANGE_EUBIOS, "Change", "", "Change");
			WORD_L(ID_CHANGE_JABIOS, "Change", "", "Change");
			WORD_L(ID_CHANGE_CGOFFLINE, "Change", "", "Change");
			WORD_L(ID_CHANGE_MANUAL, "Change", "", "Change");

			WORD_L(IDC_STATIC_GENESISBIOS, "Genesis bios static", "", "Genesis");
			WORD_L(IDC_STATIC_32XGBIOS, "M68000 bios static", "", "M68000");
			WORD_L(IDC_STATIC_32XMBIOS, "M SH2 bios static", "", "Master SH2");
			WORD_L(IDC_STATIC_32XSBIOS, "S SH2 bios static", "", "Slave SH2");
			WORD_L(IDC_STATIC_USBIOS, "US bios static", "", "USA");
			WORD_L(IDC_STATIC_EUBIOS, "EU bios static", "", "Europe");
			WORD_L(IDC_STATIC_JABIOS, "JA bios static", "", "Japan");
			WORD_L(IDC_STATIC_CGOFFLINE, "CGOffline static", "", "CGOffline");
			WORD_L(IDC_STATIC_MANUAL, "Manual static", "", "Manual");

			SetDlgItemText(hDlg, IDC_EDIT_GENESISBIOS, Genesis_Bios);
			SetDlgItemText(hDlg, IDC_EDIT_32XGBIOS, _32X_Genesis_Bios);
			SetDlgItemText(hDlg, IDC_EDIT_32XMBIOS, _32X_Master_Bios);
			SetDlgItemText(hDlg, IDC_EDIT_32XSBIOS, _32X_Slave_Bios);
			SetDlgItemText(hDlg, IDC_EDIT_USBIOS, US_CD_Bios);
			SetDlgItemText(hDlg, IDC_EDIT_EUBIOS, EU_CD_Bios);
			SetDlgItemText(hDlg, IDC_EDIT_JABIOS, JA_CD_Bios);
			SetDlgItemText(hDlg, IDC_EDIT_CGOFFLINE, CGOffline_Path);
			SetDlgItemText(hDlg, IDC_EDIT_MANUAL, Manual_Path);

			return true;
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case ID_CHANGE_GENESISBIOS:
					GetDlgItemText(hDlg, IDC_EDIT_GENESISBIOS, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "genesis.bin"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "Genesis bios file", "bios files\0*.bin\0\0", "bin"))
						SetDlgItemText(hDlg, IDC_EDIT_GENESISBIOS, Str_Tmp);
					break;

				case ID_CHANGE_32XGBIOS:
					GetDlgItemText(hDlg, IDC_EDIT_32XGBIOS, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "32X_G_bios.bin"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "32X M68000 bios file", "bios files\0*.bin\0\0", "bin"))
						SetDlgItemText(hDlg, IDC_EDIT_32XGBIOS, Str_Tmp);
					break;

				case ID_CHANGE_32XMBIOS:
					GetDlgItemText(hDlg, IDC_EDIT_32XMBIOS, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "32X_M_bios.bin"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "32X Master SH2 bios file", "bios files\0*.bin\0\0", "bin"))
						SetDlgItemText(hDlg, IDC_EDIT_32XMBIOS, Str_Tmp);
					break;

				case ID_CHANGE_32XSBIOS:
					GetDlgItemText(hDlg, IDC_EDIT_32XSBIOS, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "32X_S_bios.bin"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "32X Slave SH2 bios file", "bios files\0*.bin\0\0", "bin"))
						SetDlgItemText(hDlg, IDC_EDIT_32XSBIOS, Str_Tmp);
					break;

				case ID_CHANGE_USBIOS:
					GetDlgItemText(hDlg, IDC_EDIT_USBIOS, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "us_scd1_9210.bin"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "USA CD bios file", "bios files\0*.bin\0\0", "bin"))
						SetDlgItemText(hDlg, IDC_EDIT_USBIOS, Str_Tmp);
					break;

				case ID_CHANGE_EUBIOS:
					GetDlgItemText(hDlg, IDC_EDIT_EUBIOS, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "eu_mcd1_9210.bin"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "EUROPEAN CD bios file", "bios files\0*.bin\0\0", "bin"))
						SetDlgItemText(hDlg, IDC_EDIT_EUBIOS, Str_Tmp);
					break;

				case ID_CHANGE_JABIOS:
					GetDlgItemText(hDlg, IDC_EDIT_JABIOS, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "jp_mcd1_9112.bin"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "JAPAN CD bios file", "bios files\0*.bin\0\0", "bin"))
						SetDlgItemText(hDlg, IDC_EDIT_JABIOS, Str_Tmp);
					break;

				case ID_CHANGE_CGOFFLINE:
					GetDlgItemText(hDlg, IDC_EDIT_CGOFFLINE, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "GCOffline.chm"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "Genesis Collective - CGOffline file", "html help files\0*.chm\0\0", "chm"))
						SetDlgItemText(hDlg, IDC_EDIT_CGOFFLINE, Str_Tmp);
					break;

				case ID_CHANGE_MANUAL:
					GetDlgItemText(hDlg, IDC_EDIT_MANUAL, Str_Tmp2, 1024);
					strcpy_s(Str_Tmp, "Manual.exe"); 
					if (Change_File_S(Str_Tmp, Str_Tmp2, "Gens Manual", "executable files\0*.exe\0\0", "exe"))
						SetDlgItemText(hDlg, IDC_EDIT_MANUAL, Str_Tmp);
					break;

				case ID_OK:
					GetDlgItemText(hDlg, IDC_EDIT_GENESISBIOS, Genesis_Bios, 1024);

					GetDlgItemText(hDlg, IDC_EDIT_32XGBIOS, _32X_Genesis_Bios, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_32XMBIOS, _32X_Master_Bios, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_32XSBIOS, _32X_Slave_Bios, 1024);

					GetDlgItemText(hDlg, IDC_EDIT_USBIOS, US_CD_Bios, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_EUBIOS, EU_CD_Bios, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_JABIOS, JA_CD_Bios, 1024);

					GetDlgItemText(hDlg, IDC_EDIT_CGOFFLINE, CGOffline_Path, 1024);
					GetDlgItemText(hDlg, IDC_EDIT_MANUAL, Manual_Path, 1024);

				case ID_CANCEL:
					EndDialog(hDlg, true);
					return true;
					break;

				case ID_HELP_HELP:
					if (Detect_Format(Manual_Path) != -1)		// can be used to detect if file exist
					{
						strcpy_s(Str_Tmp, Manual_Path);
						strcat_s(Str_Tmp, 1024, " helpdir.html");
						system(Str_Tmp);
					}
					else
					{
						MessageBox(NULL, "You need to configure the Manual Path to have help", "info", MB_OK);
					}
					return true;
					break;
			}
			break;

		case WM_CLOSE:
			EndDialog(hDlg, true);
			return true;
			break;
	}

	return false;
}


INT_PTR CALLBACK AboutProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	RECT r2;
	int dx1, dy1, dx2, dy2;

	switch(uMsg)
	{
		case WM_INITDIALOG:
			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);
			return true;
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case IDOK:
					if (Full_Screen)
					{
						while (ShowCursor(true) < 0);
						while (ShowCursor(false) >= 0);
					}

					EndDialog(hDlg, true);
					return true;
			}
			break;

		case WM_CLOSE:
			if (Full_Screen)
			{
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}

			EndDialog(hDlg, true);
			return true;
			break;
	}

	return false;
}


INT_PTR CALLBACK ColorProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	RECT r2;
	int dx1, dy1, dx2, dy2;

	switch(uMsg)
	{
		case WM_INITDIALOG:
			Build_Language_String();

			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			WORD_L(IDC_STATIC_CONT, "Contrast", "", "Contrast");
			WORD_L(IDC_STATIC_BRIGHT, "Brightness", "", "Brightness");
			WORD_L(IDC_CHECK_GREYSCALE, "Greyscale", "", "Greyscale");
			WORD_L(IDC_CHECK_INVERT, "Invert", "", "Invert");

			WORD_L(ID_APPLY, "Apply", "", "&Apply");
			WORD_L(ID_CLOSE, "Close", "", "&Close");
			WORD_L(ID_DEFAULT, "Default", "", "&Default");

			SendDlgItemMessage(hDlg, IDC_SLIDER_CONTRASTE, TBM_SETRANGE, (WPARAM) (BOOL) TRUE, (LPARAM) MAKELONG(0, 200));
			SendDlgItemMessage(hDlg, IDC_SLIDER_CONTRASTE, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) (Contrast_Level));
			SendDlgItemMessage(hDlg, IDC_SLIDER_LUMINOSITE, TBM_SETRANGE, (WPARAM) (BOOL) TRUE, (LPARAM) MAKELONG(0, 200));
			SendDlgItemMessage(hDlg, IDC_SLIDER_LUMINOSITE, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) (Brightness_Level));

			SendDlgItemMessage(hDlg, IDC_CHECK_GREYSCALE, BM_SETCHECK, (WPARAM) (Greyscale)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_CHECK_INVERT, BM_SETCHECK, (WPARAM) (Invert_Color)?BST_CHECKED:BST_UNCHECKED, 0);

			return true;
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case ID_CLOSE:
					if (Full_Screen)
					{
						while (ShowCursor(true) < 0);
						while (ShowCursor(false) >= 0);
					}

					Build_Main_Menu();
					EndDialog(hDlg, true);
					return true;

				case ID_APPLY:
					Contrast_Level = SendDlgItemMessage(hDlg, IDC_SLIDER_CONTRASTE, TBM_GETPOS, 0, 0);
					Brightness_Level = SendDlgItemMessage(hDlg, IDC_SLIDER_LUMINOSITE, TBM_GETPOS, 0, 0);
					Greyscale = (SendDlgItemMessage(hDlg, IDC_CHECK_GREYSCALE, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;
					Invert_Color = (SendDlgItemMessage(hDlg, IDC_CHECK_INVERT, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;

					Recalculate_Palettes();
					if (Genesis_Started || _32X_Started || SegaCD_Started)
					{
						CRam_Flag = 1;
						if (!Paused) Update_Emulation_One(HWnd);
					}
					return true;

				case ID_DEFAULT:
					SendDlgItemMessage(hDlg, IDC_SLIDER_CONTRASTE, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) (100));
					SendDlgItemMessage(hDlg, IDC_SLIDER_LUMINOSITE, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) (100));
					SendDlgItemMessage(hDlg, IDC_CHECK_GREYSCALE, BM_SETCHECK, BST_UNCHECKED, 0);
					SendDlgItemMessage(hDlg, IDC_CHECK_INVERT, BM_SETCHECK, BST_UNCHECKED, 0);
					return true;

				case ID_HELP_HELP:
/*					if (Detect_Format(Manual_Path) != -1)		// can be used to detect if file exist
					{
						strcpy_s(Str_Tmp, Manual_Path);
						strcat(Str_Tmp, " helpmisc.html");
						system(Str_Tmp);
					}
					else
					{
						MessageBox(NULL, "You need to configure the Manual Path to have help", "info", MB_OK);
					}
*/					return true;
			}
			break;

		case WM_CLOSE:
			if (Full_Screen)
			{
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}

			EndDialog(hDlg, true);
			return true;
			break;
	}

	return false;
}


INT_PTR CALLBACK OptionProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	RECT r2;
	int dx1, dy1, dx2, dy2;

	switch(uMsg)
	{
		case WM_INITDIALOG:
			Build_Language_String();

			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			WORD_L(IDC_AUTOFIXCHECKSUM, "Auto Fix Checksum", "", "Auto Fix Checksum");
			WORD_L(IDC_AUTOPAUSE, "Auto Pause", "", "Auto Pause");
			WORD_L(IDC_FASTBLUR, "Fast Blur", "", "Fast Blur");
			WORD_L(IDC_SHOWLED, "Show Sega-CD LED", "", "Show Sega-CD LED");
			WORD_L(IDC_ENABLE_FPS, "Enable", "", "Enable");
			WORD_L(IDC_ENABLE_MESSAGE, "Enable", "", "Enable");
			WORD_L(IDC_X2_FPS, "Double Sized", "", "Double Sized");
			WORD_L(IDC_X2_MESSAGE, "Double Sized", "", "Double Sized");
			WORD_L(IDC_TRANS_FPS, "Transparency", "", "Transparency");
			WORD_L(IDC_TRANS_MESSAGE, "Transparency", "", "Transparency");
			WORD_L(IDC_EFFECT_COLOR, "Effect Color", "", "Intro effect color");
			WORD_L(IDC_OPTION_SYSTEM, "System", "", "System");
			WORD_L(IDC_OPTION_FPS, "FPS", "", "FPS");
			WORD_L(IDC_OPTION_MESSAGE, "Message", "", "Message");
			WORD_L(IDC_OPTION_CANCEL, "Cancel", "", "&Cancel");
			WORD_L(IDC_OPTION_OK, "OK", "", "&OK");

			SendDlgItemMessage(hDlg, IDC_COLOR_FPS, TBM_SETRANGE, (WPARAM) (BOOL) TRUE, (LPARAM) MAKELONG(0, 3));
			SendDlgItemMessage(hDlg, IDC_COLOR_FPS, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) ((FPS_Style & 0x6) >> 1));
			SendDlgItemMessage(hDlg, IDC_COLOR_MESSAGE, TBM_SETRANGE, (WPARAM) (BOOL) TRUE, (LPARAM) MAKELONG(0, 3));
			SendDlgItemMessage(hDlg, IDC_COLOR_MESSAGE, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) ((Message_Style & 0x6) >> 1));
			SendDlgItemMessage(hDlg, IDC_COLOR_EFFECT, TBM_SETRANGE, (WPARAM) (BOOL) TRUE, (LPARAM) MAKELONG(0, 7));
			SendDlgItemMessage(hDlg, IDC_COLOR_EFFECT, TBM_SETPOS, (WPARAM) (BOOL) TRUE, (LPARAM) (LONG) Effect_Color);

			SendDlgItemMessage(hDlg, IDC_AUTOFIXCHECKSUM, BM_SETCHECK, (WPARAM) (Auto_Fix_CS)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_AUTOPAUSE, BM_SETCHECK, (WPARAM) (Auto_Pause)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_FASTBLUR, BM_SETCHECK, (WPARAM) (Fast_Blur)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_SHOWLED, BM_SETCHECK, (WPARAM) (Show_LED)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_ENABLE_FPS, BM_SETCHECK, (WPARAM) (Show_FPS)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_X2_FPS, BM_SETCHECK, (WPARAM) (FPS_Style & 0x10)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_TRANS_FPS, BM_SETCHECK, (WPARAM) (FPS_Style & 0x8)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_ENABLE_MESSAGE, BM_SETCHECK, (WPARAM) (Show_Message)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_X2_MESSAGE, BM_SETCHECK, (WPARAM) (Message_Style & 0x10)?BST_CHECKED:BST_UNCHECKED, 0);
			SendDlgItemMessage(hDlg, IDC_TRANS_MESSAGE, BM_SETCHECK, (WPARAM) (Message_Style & 0x8)?BST_CHECKED:BST_UNCHECKED, 0);

			return true;
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case IDC_OPTION_OK:
					unsigned int res;

					if (Full_Screen)
					{
						while (ShowCursor(true) < 0);
						while (ShowCursor(false) >= 0);
					}

					res = SendDlgItemMessage(hDlg, IDC_COLOR_FPS, TBM_GETPOS, 0, 0);
					FPS_Style = (FPS_Style & ~0x6) | ((res << 1) & 0x6);
					res = SendDlgItemMessage(hDlg, IDC_COLOR_MESSAGE, TBM_GETPOS, 0, 0);
					Message_Style = (Message_Style & 0xF9) | ((res << 1) & 0x6);
					Effect_Color = SendDlgItemMessage(hDlg, IDC_COLOR_EFFECT, TBM_GETPOS, 0, 0);

					Auto_Fix_CS = (SendDlgItemMessage(hDlg, IDC_AUTOFIXCHECKSUM, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;
					Auto_Pause = (SendDlgItemMessage(hDlg, IDC_AUTOPAUSE, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;
					Fast_Blur = (SendDlgItemMessage(hDlg, IDC_FASTBLUR, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;
					Show_LED = (SendDlgItemMessage(hDlg, IDC_SHOWLED, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;
					Show_FPS = (SendDlgItemMessage(hDlg, IDC_ENABLE_FPS, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;
					res = SendDlgItemMessage(hDlg, IDC_X2_FPS, BM_GETCHECK, 0, 0);
					FPS_Style = (FPS_Style & ~0x10) | ((res == BST_CHECKED)?0x10:0);
					res = SendDlgItemMessage(hDlg, IDC_TRANS_FPS, BM_GETCHECK, 0, 0);
					FPS_Style = (FPS_Style & ~0x8) | ((res == BST_CHECKED)?0x8:0);
					Show_Message = (SendDlgItemMessage(hDlg, IDC_ENABLE_MESSAGE, BM_GETCHECK, 0, 0) == BST_CHECKED)?1:0;
					res = SendDlgItemMessage(hDlg, IDC_X2_MESSAGE, BM_GETCHECK, 0, 0);
					Message_Style = (Message_Style & ~0x10) | ((res == BST_CHECKED)?0x10:0);
					res = SendDlgItemMessage(hDlg, IDC_TRANS_MESSAGE, BM_GETCHECK, 0, 0);
					Message_Style = (Message_Style & ~0x8) | ((res == BST_CHECKED)?0x8:0);

					Build_Main_Menu();
					
					EndDialog(hDlg, true);
					return true;
					break;

				case IDC_OPTION_CANCEL:
					if (Full_Screen)
					{
						while (ShowCursor(true) < 0);
						while (ShowCursor(false) >= 0);
					}

					EndDialog(hDlg, true);
					return true;
					break;

				case ID_HELP_HELP:
					if (Detect_Format(Manual_Path) != -1)		// can be used to detect if file exist
					{
						strcpy_s(Str_Tmp, Manual_Path);
						strcat_s(Str_Tmp, 1024, " helpmisc.html");
						system(Str_Tmp);
					}
					else
					{
						MessageBox(NULL, "You need to configure the Manual Path to have help", "info", MB_OK);
					}
					return true;
					break;
			}
			break;

		case WM_CLOSE:
			if (Full_Screen)
			{
				while (ShowCursor(true) < 0);
				while (ShowCursor(false) >= 0);
			}

			EndDialog(hDlg, true);
			return true;
			break;
	}

	return false;
}


INT_PTR CALLBACK ControllerProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	RECT r;
	RECT r2;
	int dx1, dy1, dx2, dy2;
    int i; 
	static HWND Tex0 = NULL;

	switch(uMsg)
	{
		case WM_INITDIALOG:
			if (Full_Screen)
			{
				while (ShowCursor(false) >= 0);
				while (ShowCursor(true) < 0);
			}

			GetWindowRect(HWnd, &r);
			dx1 = (r.right - r.left) / 2;
			dy1 = (r.bottom - r.top) / 2;

			GetWindowRect(hDlg, &r2);
			dx2 = (r2.right - r2.left) / 2;
			dy2 = (r2.bottom - r2.top) / 2;

			SetWindowPos(hDlg, NULL, r.left + (dx1 - dx2), r.top + (dy1 - dy2), NULL, NULL, SWP_NOSIZE | SWP_NOZORDER | SWP_SHOWWINDOW);

			Tex0 = GetDlgItem(hDlg, IDC_STATIC_TEXT0);

			if (!Init_Input(ghInstance, hDlg)) return false;

			WORD_L(IDC_JOYINFO1, "Controller info 1", "", "Player 1-B 1-C and 1-D are enabled only if a teamplayer is connected to port 1");
			WORD_L(IDC_JOYINFO2, "Controller info 2", "", "Player 2-B 2-C and 2-D are enabled only if a teamplayer is connected to port 2");
			WORD_L(IDC_JOYINFO3, "Controller info 3", "", "Only a few games support teamplayer (games which have 4 players support), so don't forget to use the \"load config\" and \"save config\" possibility :)");

			for(i = 0; i < 2; i++)
			{
				SendDlgItemMessage(hDlg, IDC_COMBO_PORT1 + i, CB_INSERTSTRING, (WPARAM) 0, (LONG) (LPTSTR) "teamplayer");
				SendDlgItemMessage(hDlg, IDC_COMBO_PORT1 + i, CB_INSERTSTRING, (WPARAM) 0, (LONG) (LPTSTR) "pad");
			}

			for(i = 0; i < 8; i++)
			{
				SendDlgItemMessage(hDlg, IDC_COMBO_PADP1 + i, CB_INSERTSTRING, (WPARAM) 0, (LONG) (LPTSTR) "6 buttons");
				SendDlgItemMessage(hDlg, IDC_COMBO_PADP1 + i, CB_INSERTSTRING, (WPARAM) 0, (LONG) (LPTSTR) "3 buttons");
			}

			SendDlgItemMessage(hDlg, IDC_COMBO_PORT1, CB_SETCURSEL, (WPARAM) ((Controller_1_Type >> 4) & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PORT2, CB_SETCURSEL, (WPARAM) ((Controller_2_Type >> 4) & 1), (LPARAM) 0);

			SendDlgItemMessage(hDlg, IDC_COMBO_PADP1, CB_SETCURSEL, (WPARAM) (Controller_1_Type & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PADP1B, CB_SETCURSEL, (WPARAM) (Controller_1B_Type & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PADP1C, CB_SETCURSEL, (WPARAM) (Controller_1C_Type & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PADP1D, CB_SETCURSEL, (WPARAM) (Controller_1D_Type & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PADP2, CB_SETCURSEL, (WPARAM) (Controller_2_Type & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PADP2B, CB_SETCURSEL, (WPARAM) (Controller_2B_Type & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PADP2C, CB_SETCURSEL, (WPARAM) (Controller_2C_Type & 1), (LPARAM) 0);
			SendDlgItemMessage(hDlg, IDC_COMBO_PADP2D, CB_SETCURSEL, (WPARAM) (Controller_2D_Type & 1), (LPARAM) 0);

			return true;
			break;

		case WM_COMMAND:
			switch(wParam)
			{
				case IDOK:
					Controller_1_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PORT1, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_1_Type <<= 4;

					Controller_1_Type |= (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_1B_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1B, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_1C_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1C, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_1D_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1D, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);

					
					Controller_2_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PORT2, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_2_Type <<= 4;

					Controller_2_Type |= (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_2B_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2B, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_2C_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2C, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);
					Controller_2D_Type = (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2D, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1);

					Make_IO_Table();
					End_Input();
					EndDialog(hDlg, true);
					return true;
					break;

				case IDC_BUTTON_SETKEYSP1:
					SetWindowText(Tex0, "SETTING KEYS P1");
					Setting_Keys(hDlg, 0, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(Tex0, "SETTING KEYS");
					return true;
					break;

				case IDC_BUTTON_SETKEYSP1B:
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS P1B");
					Setting_Keys(hDlg, 2, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1B, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS");
					return true;
					break;

				case IDC_BUTTON_SETKEYSP1C:
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS P1C");
					Setting_Keys(hDlg, 3, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1C, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS");
					return true;
					break;

				case IDC_BUTTON_SETKEYSP1D:
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS P1D");
					Setting_Keys(hDlg, 4, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP1D, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS");
					return true;
					break;

				case IDC_BUTTON_SETKEYSP2:
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS P2");
					Setting_Keys(hDlg, 1, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS");
					return true;
					break;

				case IDC_BUTTON_SETKEYSP2B:
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS P2B");
					Setting_Keys(hDlg, 5, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2B, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS");
					return true;
					break;

				case IDC_BUTTON_SETKEYSP2C:
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS P2C");
					Setting_Keys(hDlg, 6, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2C, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS");
					return true;
					break;

				case IDC_BUTTON_SETKEYSP2D:
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS P2D");
					Setting_Keys(hDlg, 7, (SendDlgItemMessage(hDlg, IDC_COMBO_PADP2D, CB_GETCURSEL, (WPARAM) 0, (LPARAM) 0) & 1));
					SetWindowText(GetDlgItem(hDlg, IDC_STATIC_TEXT0), "SETTING KEYS");
					return true;
					break;

				case ID_HELP_HELP:
					if (Detect_Format(Manual_Path) != -1)		// can be used to detect if file exist
					{
						strcpy_s(Str_Tmp, Manual_Path);
						strcat_s(Str_Tmp, 1024, " helpjoypads.html");
						system(Str_Tmp);
					}
					else
					{
						MessageBox(NULL, "You need to configure the Manual Path to have help", "info", MB_OK);
					}
					return true;
					break;
			}
			break;

		case WM_CLOSE:
			End_Input();
			EndDialog(hDlg, true);
			return true;
			break;
	}

	return false;
}

