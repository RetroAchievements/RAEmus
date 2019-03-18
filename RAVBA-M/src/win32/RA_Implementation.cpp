#include "stdafx.h"

#include "RA_Interface.h"

#include "VBA.h"
#include "../gb/gbGlobals.h"
#include "../gba/Globals.h"

#include "../win32/Disassemble.h"
#include "../win32/GBDisassemble.h"
#include "../win32/MemoryViewerDlg.h"
#include "../win32/GBMemoryViewerDlg.h"
#include "../win32/MapView.h"
#include "../win32/GBMapView.h"

//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
bool GameIsActive()
{ 
	return( emulating == TRUE ); 
}

//	Perform whatever action is required to unpause emulation.
void CauseUnpause()
{
	theApp.paused = false;
	theApp.wasPaused = true;
}

//	Perform whatever action is required to pause emulation.
void CausePause()
{
	theApp.paused = true;
}	

//	Perform whatever function in the case of needing to rebuild the menu.
void RebuildMenu()
{
	theApp.updateMenuBar();
}

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
void GetEstimatedGameTitle( char* sNameOut )
{
	if( theApp.cartridgeType == IMAGE_GB )
	{
		strncpy_s(sNameOut, 16, (const char *)&gbRom[0x134], 15);
		sNameOut[15] = 0;
	}
	else if( theApp.cartridgeType == IMAGE_GBA )
	{			
		strncpy_s(sNameOut, 16, (const char *)&rom[0xa0], 12);
		sNameOut[12] = 0;
	}
}

static BOOL CALLBACK CloseDebugWindows(HWND hWnd, LPARAM lParam)
{
    CWnd *pWnd = CWnd::FromHandle(hWnd);
    if (dynamic_cast<Disassemble*>(pWnd) != NULL)
        reinterpret_cast<CDialog*>(pWnd)->EndDialog(IDCANCEL);
    else if (dynamic_cast<GBDisassemble*>(pWnd) != NULL)
        reinterpret_cast<CDialog*>(pWnd)->EndDialog(IDCANCEL);
    else if (dynamic_cast<MemoryViewerDlg*>(pWnd) != NULL)
        reinterpret_cast<CDialog*>(pWnd)->EndDialog(IDCANCEL);
    else if (dynamic_cast<GBMemoryViewerDlg*>(pWnd) != NULL)
        reinterpret_cast<CDialog*>(pWnd)->EndDialog(IDCANCEL);
    else if (dynamic_cast<MapView*>(pWnd) != NULL)
        reinterpret_cast<CDialog*>(pWnd)->EndDialog(IDCANCEL);
    else if (dynamic_cast<GBMapView*>(pWnd) != NULL)
        reinterpret_cast<CDialog*>(pWnd)->EndDialog(IDCANCEL);

    return TRUE;
}

void ResetEmulation()
{
	if( theApp.emulator.emuReset != NULL )
		theApp.emulator.emuReset();

    theApp.updateThrottle(0);
    theApp.autoFrameSkip = true;
    theApp.throttle = false;
    frameSkip = 0;
    systemFrameSkip = 0;

    DWORD processid;
    DWORD threadid = GetWindowThreadProcessId(theApp.m_pMainWnd->m_hWnd, &processid);
    EnumThreadWindows(threadid, &CloseDebugWindows, 0);
}

void LoadROM( const char* sFullPath )
{
	theApp.szFile = sFullPath;
	theApp.Run();
}

//	Installs these shared functions into the DLL
void RA_InitShared()
{
	RA_InstallSharedFunctions( &GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM );
}
