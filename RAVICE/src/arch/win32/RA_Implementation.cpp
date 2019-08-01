#include "RA_Implementation.h"
#include "../../../RA_Integration/RA_Interface.h"
#include "../../../RA_Integration/RA_Resource.h"

//	Include any emulator-side headers, externs or functions here
//#include "../Common.h"
BOOL g_bLeaderboardsActive;
BOOL g_hardcoreModeActive;
BOOL g_snapshotsActive;




//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
BOOL GameIsActive()
{
	return true;
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	//FCEUI_SetEmulationPaused( false );
}

//	Perform whatever function in the case of needing to rebuild the menu.
void RebuildMenu()
{
	//// get main menu handle
	HMENU hMainMenu = CreateMenu();
	if(!hMainMenu) return;

	////	##RA embed RA
	HMENU hRA = CreatePopupMenu();


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
	
}

void LoadRom( char* path)
{
	//FCEUI_ResetNES();
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions( &GameIsActive, &CauseUnpause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadRom );
}

