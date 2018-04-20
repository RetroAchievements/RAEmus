//	RetroAchievements Integration files:
//	Example implementation file: copy .h/.cpp to your emulator and modify functionality.
//	www.retroachievements.org

//	Return whether a game has been loaded. Should return FALSE if
//	 no ROM is loaded, or a ROM has been unloaded.
extern bool GameIsActive();

//	Perform whatever action is required to unpause emulation.
extern void CauseUnpause();

//	Perform whatever action is required to Pause emulation.
extern void CausePause();

//	Perform whatever function in the case of needing to rebuild the menu.
extern void RebuildMenu();

//	sNameOut points to a 64 character buffer.
//	sNameOut should have copied into it the estimated game title 
//	 for the ROM, if one can be inferred from the ROM.
extern void GetEstimatedGameTitle(char* sNameOut);

extern void ResetEmulation();

//	Called BY the toolset to tell the emulator to load a particular ROM.
extern void LoadROMFromEmu(const char* sFullPath);

//	Installs these shared functions into the DLL
extern void RA_InitShared();



//extras
//Purpose of these functions is to make the original Meka code as unaware of RA
//and RAMeka's needs as possible, and keep all the implenetation code as much
//as possible in one place, rather than scattered ad hoc all over the original Meka code

//Idea is that Meka knows as little about RAMeka as possible 

//-----------------------------------------------------------------------
//                Emulator RA Integration Wrapper Functions             | 
//-----------------------------------------------------------------------

//Most of the other emulators define their version in their MakeBuildVer.bat scripts. 
//But we're using NuGet/Allegro/VS2015 so nothing here is "straightforward"
#define RAMEKA_VERSION  "0.021"

#include <windows.h> //for WPARAM (kind of wasteful really)
#include "RA_Interface.h"

//See: RA_MemManager.h _RAMByteReadFn _RAMByteWriteFn
unsigned char	RAMeka_RAMByteReadFn (unsigned int Offset);
void			RAMeka_RAMByteWriteFn(unsigned int Offset, unsigned int nVal);

void RAMeka_Stash_Meka_CurrentDirectory();
void RAMeka_Restore_Meka_CurrentDirectory();
void RAMeka_Stash_RA_RootDirectory();
void RAMeka_Restore_RA_RootDirectory();

void RAMeka_RA_Setup();
void RAMeka_RA_Shutdown();
void RAMeka_RA_InvokeDialog(WPARAM wParam);

void RAMeka_RA_SetPaused(bool bIsPaused);
void RAMeka_RA_OnSaveStateLoad(char* filename);
void RAMeka_RA_OnSaveStateSave(char* filename);

void RAMeka_RA_MountMasterSystemROM();
void RAMeka_RA_MountROM( ConsoleID consoleID );


void RAMeka_MakePlaceholderRAMenu();
void RAMeka_InstallRA();

void RAMeka_ValidateHardcoreMode();

void RAMeka_RA_AchievementsFrameCheck();


enum RAMeka_Softcore_Feature {
	SCF_MEMORY_EDITOR,
	SCF_DEBUGGER,
	SCF_CHEAT_FINDER,
	SCF_SAVE_LOAD,
//	SCF_LOAD,
//	SCF_SAVE,
	SCF_UNKNOWN
};

bool RAMeka_HardcoreIsActiveCheck(RAMeka_Softcore_Feature current_feature);
bool RAMeka_HardcoreDeactivateConfirm(RAMeka_Softcore_Feature current_feature);


void RAMeka_Config_Load_Line(char *var, char *value);
//void RAMeka_Configs_Save(void *Write_Line_func, void *Write_Int_func, void *Write_Str_func);

//Goal here is to make the Meka code as unaware of RAMeka's needs as possible
//Required functions declared static in config.c so no other option here but to pass function
//pointers if we (I) want a minimal insert in the original meka code
typedef void(*CFG_Write_Line_Func)(const char* fmt, ...);
typedef void(*CFG_Write_Int_Func)(const char *name, int value);
typedef void(*CFG_Write_Str_Func)(const char *name, const char *str);
void RAMeka_Configs_Save(CFG_Write_Line_Func , CFG_Write_Int_Func, CFG_Write_Str_Func);  //forward declarations don't require variable names

//-----------------------------------------------------------------------
//                RA Overlay Video Functions                            | 
//-----------------------------------------------------------------------

//#RA overlay code
enum {
	OVERLAY_RENDER_ALLEGRO = 0,
	OVERLAY_RENDER_WIN_LAYER = 1
};

extern int overlay_render_method;
extern bool disable_RA_overlay;
extern int overlay_frame_skip;
extern bool overlay_alternate_render_blit;
extern int overlay_bg_splits;
extern int overlay_bg_splits;

void RAMeka_Overlay_ClearBackbuffer();

#define BYTES_PER_PIXEL(bpp)     (((int)(bpp) + 7) / 8)

LRESULT CALLBACK RAMeka_RAWndProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);

void RAMeka_Overlay_RenderAchievementOverlays();
void RAMeka_Overlay_RenderAchievementOverlays_WIN_LAYER();
void RAMeka_Overlay_RenderAchievementOverlays_ALLEGRO_OVERLAY();
void RAMeka_Overlay_UpdateOverlay(HDC hdc, RECT rect);

