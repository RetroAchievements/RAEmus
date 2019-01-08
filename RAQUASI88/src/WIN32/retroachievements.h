/*****************************************************************************/
/*	RetroAchievements実績システムの管理								     	 */
/*									     									 */
/*	RetroAchievementsのインタフェースを実施するためのファイル 					 */
/*									    									 */
/*****************************************************************************/

#if USE_RETROACHIEVEMENTS

#ifndef RETROACHIEVEMENTS_H
#define RETROACHIEVEMENTS_H

#include <RA_Interface.h>

typedef struct FileInfo
{
    BYTE *data;
    unsigned long data_len;
    char name[1024];
    unsigned int title_id;
    /* FTYPE_DISK 又は FTYPE_TAPE_LOAD を保存し、
    リセット時に Softcore から Harcore へ移行する場合は
    相当しない方からイメージファイルを取り外す */
    int file_type;
} FileInfo;

#define FINFO_DEFAULT FileInfo { 0, 0, { 0 }, 0, 0 };

/* 挿入時のディスク・テープイメージのデータを保存する */
extern FileInfo loaded_disk;
extern FileInfo loaded_tape;
extern FileInfo loading_file;
extern FileInfo *loaded_title;

void reset_file_info(FileInfo *file);
void free_file_info(FileInfo *file);

//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
extern bool GameIsActive();

//	Perform whatever action is required to unpause emulation.
extern void CauseUnpause();

//	Perform whatever action is required to pause emulation.
extern void CausePause();

//	Perform whatever function in the case of needing to rebuild the menu.
extern void RebuildMenu();

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
extern void GetEstimatedGameTitle(char* sNameOut);

extern void ResetEmulation();

//	Called BY the toolset to tell the emulator to load a particular ROM.
extern void LoadROM(char* sFullPath);

//	Installs these shared functions into the DLL
extern void RA_InitShared();

void RA_InitUI();
void RA_InitMemory();
int RA_PrepareLoadNewRom(const char *file_name, int file_type);
void RA_CommitLoadNewRom();
void RA_OnGameClose(int file_type);
int RA_HandleMenuEvent(int id);
void RA_RenderOverlayFrame(HDC hdc);

#endif /* RETROACHIEVEMENTS_H */

#endif
