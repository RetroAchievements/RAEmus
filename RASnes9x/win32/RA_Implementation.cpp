#include "RA_Implementation.h"
#include "RA_Interface.h"

#include "wsnes9x.h"
#include "snes9x.h"
#include "port.h"
#include "memmap.h"
#include "controls.h"

//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
bool GameIsActive()
{
	return TRUE;
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	Settings.Paused = false;
	Settings.FrameAdvance = false;
	GUI.FrameAdvanceJustPressed = 0;
} 

//	Perform whatever function in the case of needing to rebuild the menu.
void _RebuildMenu()
{
	RebuildMenu();
}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle( char* sNameOut )
{
	sprintf_s( sNameOut, 23, Memory.ROMName ? &Memory.ROMName[0] : "" );
	sNameOut[23] = '\0';
}

void ResetEmulation()
{
	if( !Settings.StopEmulation )
		S9xReset();
}

extern bool8 S9xLoadROMImage( const TCHAR* string );
void LoadROMFromEmu( const char* sFullPath )
{
#ifdef UNICODE
	S9xLoadROMImage( Widen( sFullPath ).c_str() );
#else
	S9xLoadROMImage( sFullPath );
#endif
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions( &GameIsActive, &CauseUnpause, &_RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROMFromEmu );
}
