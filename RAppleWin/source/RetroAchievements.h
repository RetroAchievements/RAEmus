/******************************************************************************
*   RetroAchievements interface adapter                                       *
*   This file implements the RetroAchievements interface,                     *
*   as well as any helper function used to handle achievement functionality.  *
*******************************************************************************/

#if USE_RETROACHIEVEMENTS

#ifndef RETROACHIEVEMENTS_H
#define RETROACHIEVEMENTS_H

#include <RA_Interface.h>

typedef enum FileType
{
    FLOPPY_DISK,
    HARD_DISK
} FileType;

typedef struct FileInfo
{
    BYTE *data;
    unsigned long data_len;
    char name[1024];
    unsigned int title_id;
    FileType file_type;
} FileInfo;

#define FINFO_DEFAULT FileInfo { 0, 0, { 0 }, 0, FileType::FLOPPY_DISK };

// Save loaded media data
extern FileInfo loaded_floppy_disk;
extern FileInfo loaded_hard_disk;
extern FileInfo loading_file;
extern FileInfo *loaded_title;

extern bool confirmed_quitting;
extern bool is_initialized;

void reset_file_info(FileInfo *file);
void free_file_info(FileInfo *file);

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

void RA_InitSystem();
void RA_InitUI();
void RA_InitMemory();
int RA_PrepareLoadNewRom(const char *file_name, FileType file_type);
void RA_CommitLoadNewRom();
void RA_OnGameClose(int file_type);
void RA_ClearTitle();
void RA_ProcessReset();
int RA_HandleMenuEvent(int id);
void RA_RenderOverlayFrame(HDC hdc);
int RA_ConfirmQuit();

#endif /* RETROACHIEVEMENTS_H */

#endif
