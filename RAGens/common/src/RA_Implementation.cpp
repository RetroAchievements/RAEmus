#include "RA_Defs.h"
#include "RA_Implementation.h"
#include "RA_Interface.h"

#include "G_main.h"
#include "Rom.h"
#include "vdp_io.h"
#include "Gens.h"


//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
bool GameIsActive()
{
	return (Game!=NULL);
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	Paused = FALSE;
}

//	Perform whatever action is required to pause emulation.
void CausePause()
{
	Paused = TRUE;
}  

//	Perform whatever function in the case of needing to rebuild the menu.
void RebuildMenu()
{
	Build_Main_Menu(); 
}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle( char* sNameOut )
{
	if( Game != NULL )
		strcpy_s( sNameOut, 49, Game->Rom_Name_W );
}

void ResetEmulation()
{
	if (Genesis_Started)
	{
		Reset_Genesis();
	}
	else if (_32X_Started)
	{
		Reset_32X();
	}
	else if (SegaCD_Started)
	{
		Reset_SegaCD();
	}
}

void LoadROM( const char* sFullPath )
{
	Pre_Load_Rom( HWnd, const_cast<char*>( sFullPath ) );
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions( &GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM );
}
