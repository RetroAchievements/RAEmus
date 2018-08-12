//	Include any emulator-side headers, externs or functions here
#include <stdafx.h>
#include <Windows.h>
#include "RA_Implementation.h"
#include "RA_Interface.h"

//#include "Project64\UserInterface\Settings\SettingsPage.h"

// returns -1 if not found
int GetMenuItemIndex(HMENU hMenu, const char* ItemName)
{
	int index = 0;
	char buf[256];

	while (index < GetMenuItemCount(hMenu))
	{
		if (GetMenuString(hMenu, index, buf, sizeof(buf) - 1, MF_BYPOSITION))
		{
			if (!strcmp(ItemName, buf))
				return index;
		}
		index++;
	}
	return -1;
}


//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
bool GameIsActive()
{
	return true;
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	g_BaseSystem->ExternalEvent( SysEvent_ResumeCPU_FromMenu );
}

//	Perform whatever action is required to pause emulation.
void CausePause()
{
	g_BaseSystem->ExternalEvent( SysEvent_PauseCPU_FromMenu );
}

//	Perform whatever function in the case of needing to rebuild the menu.
void RebuildMenu()
{
	// get main menu handle
	HMENU hMainMenu = GetMenu( hMainWindow );
	if(!hMainMenu) return;

	// get file menu index
	int index = GetMenuItemIndex(hMainMenu, "&RetroAchievements");
	if(index >= 0)
		DeleteMenu( hMainMenu, index, MF_BYPOSITION );

	//	##RA embed menu
	AppendMenu( hMainMenu, MF_POPUP|MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements") );

	DrawMenuBar( hMainWindow );
}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle( char* sNameOut )
{
	//if( emu && emu->get_NES_ROM() )
	//	strcpy_s( sNameOut, 49, emu->get_NES_ROM()->GetRomName() );
}

void ResetEmulation()
{
	if (g_Settings->LoadBool(GameRunning_CPU_Running) || g_Settings->LoadBool(GameRunning_CPU_Paused))
		g_BaseSystem->ExternalEvent(SysEvent_ResetCPU_Hard);
}

void LoadROM( const char* sFullPath )
{
	//FCEUI_LoadGame( sFullPath, 0 );
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions( &GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM );
}

