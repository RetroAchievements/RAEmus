/******************************************************************************
*   RetroAchievements interface adapter                                       *
*   This file implements the RetroAchievements interface,                     *
*   as well as any helper function used to handle achievement functionality.  *
*******************************************************************************/

#if USE_RETROACHIEVEMENTS

#ifndef RETROACHIEVEMENTS_H
#define RETROACHIEVEMENTS_H


#include "../../RA_Integration/src/RA_Interface.h"
#include "../../RA_Integration/src/RA_Resource.h"

typedef enum FileType
{
    FLOPPY_DISK,
    HARD_DISK
} FileType;


extern HWND hRAMainWnd = NULL;

extern bool confirmed_quitting;
extern bool is_initialized;


//  Return whether a game has been loaded. Should return FALSE if
//   no ROM is loaded, or a ROM has been unloaded.
extern bool GameIsActive();

//  Perform whatever action is required to unpause emulation.
extern void CauseUnpause();

//  Perform whatever action is required to pause emulation.
extern void CausePause();

//  Perform whatever function in the case of needing to rebuild the menu.
extern void RebuildMenu();

//  sNameOut points to a 64 character buffer.
//  sNameOut should have copied into it the estimated game title 
//   for the ROM, if one can be inferred from the ROM.
extern void GetEstimatedGameTitle(char* sNameOut);

extern void ResetEmulation();

//  Called BY the toolset to tell the emulator to load a particular ROM.
extern void LoadROM(char* sFullPath);

//  Installs these shared functions into the DLL
extern void RA_InitShared();

extern void RA_InitSystem(HWND hwnd);
extern void RA_InitUI();
extern void RA_InitMemory();
extern int RA_PrepareLoadNewRom(const char *file_name, FileType file_type);
extern void RA_CommitLoadNewRom();
extern void RA_OnGameClose(int file_type);
extern void RA_ClearTitle();
extern void RA_ProcessReset();
extern int RA_HandleMenuEvent(int id);
extern void RA_RenderOverlayFrame(HDC hdc);
extern int RA_ConfirmQuit();

#endif /* RETROACHIEVEMENTS_H */

#endif
