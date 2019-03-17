/******************************************************************************
*   RetroAchievements interface adapter                                       *
*   This file implements the RetroAchievements interface,                     *
*   as well as any helper function used to handle achievement functionality.  *
*******************************************************************************/

#if USE_RETROACHIEVEMENTS

#ifndef RETROACHIEVEMENTS_H
#define RETROACHIEVEMENTS_H

#include <RA_Interface.h>

#include "emu.h"

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

void RA_InitUI(EMU *new_emu);
void RA_InitMemory();
void RA_OnGameOpen(const char *file_name);
void RA_OnGameClose();
void RA_ClearTitle();
int RA_HandleMenuEvent(int id);
void RA_RenderOverlayFrame(HDC hdc);

#endif /* RETROACHIEVEMENTS_H */

#endif
