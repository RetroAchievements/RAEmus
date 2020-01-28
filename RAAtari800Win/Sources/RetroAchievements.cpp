
#include "StdAfx.h"
#include <stdio.h>
#include <assert.h>
#include <windows.h>

#if USE_RETROACHIEVEMENTS


#include "globals.h"
#include "Atari800Win.h"
#include "memory.h"
#include "RetroAchievements.h"

#endif

bool should_activate = true;
bool g_bFrameActive = false;
bool confirmed_quitting = false;
bool is_initialized = false;
HWND hRAMainWnd = NULL;


/*****************************************************************************
 * Memory readers/writers for achievement processing                         *
 *****************************************************************************/

#ifndef RA_ENABLE_AUXRAM
#define RA_ENABLE_AUXRAM 1 // Enable auxiliary RAM by default
#endif

unsigned char MainRAMReader(size_t nOffs)
{
    
    return MEMORY_mem[(WORD)nOffs];
}

void MainRAMWriter(size_t nOffs, unsigned char nVal)
{
    
    MEMORY_mem[(WORD)nOffs] = nVal;
}




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

    // not found
    return -1;
}

bool GameIsActive()
{
   
	return is_initialized;

}

void CauseUnpause()
{
    
}

void CausePause()
{
   
}

void RebuildMenu()
{
    // get main menu handle
    HMENU hMainMenu = GetMenu(hRAMainWnd);
    if (!hMainMenu) return;

    // get file menu index
    int index = GetMenuItemIndex(hMainMenu, "&RetroAchievements");
    if (index >= 0)
        DeleteMenu(hMainMenu, index, MF_BYPOSITION);

    // embed RA
    AppendMenu(hMainMenu, MF_POPUP | MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements"));

    DrawMenuBar(hRAMainWnd);
}

void GetEstimatedGameTitle(char* sNameOut)
{
    
}

void ResetEmulation()
{
    
}

void LoadROM(const char* sFullPath)
{
    // Assume that the image is a floppy disk
    
}

void RA_InitShared()
{
    RA_InstallSharedFunctions(&GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM);
}

void RA_InitSystem(HWND hwnd)
{

	hRAMainWnd = GetLastActivePopup(hwnd);

    if (is_initialized)
    {
        RA_UpdateHWnd(hRAMainWnd);
    }
    else
    {
        RA_Init(hRAMainWnd, RA_Libretro, "0.10");
        RA_InitShared();
        RA_AttemptLogin(true);
        is_initialized = true;
    }

    confirmed_quitting = false;
}

void RA_InitUI()
{
    RebuildMenu();
    RA_InitMemory();
}

void RA_InitMemory()
{
    int bank_id = 0;

    RA_ClearMemoryBanks();

    RA_InstallMemoryBank(bank_id++, MainRAMReader, MainRAMWriter, 0x10000);


}

int RA_PrepareLoadNewRom(const char *file_name, FileType file_type)
{
    if (!file_name)
        return FALSE;

    char file_extension[_MAX_EXT];
    _splitpath(file_name, NULL, NULL, NULL, file_extension);

    if (!strcmp(_strupr(file_extension), ".ZIP") ||
        !strcmp(_strupr(file_extension), ".GZ"))
    {
        return false;
    }

    FILE *f = fopen(file_name, "rb");

    if (!f)
        return FALSE;

    char basename[_MAX_FNAME];
    _splitpath(file_name, NULL, NULL, basename, NULL);


    fseek(f, 0, SEEK_END);
    const unsigned long file_size = (unsigned long)ftell(f);


    BYTE * const file_data = (BYTE *)malloc(file_size * sizeof(BYTE));
    if (!file_data)
        return FALSE;

  
    fseek(f, 0, SEEK_SET);
    fread(file_data, sizeof(BYTE), file_size, f);

    fflush(f);
    fclose(f);



    return TRUE;
}

void RA_CommitLoadNewRom()
{
    

    RA_UpdateAppTitle("");

    if (should_activate)
    {
        // Initialize title data in the achievement system
        RA_ActivateGame(0);
        should_activate = false;
    }


}

void RA_OnGameClose(int file_type)
{
  
   RA_ClearTitle();
    
}

void RA_ClearTitle()
{
    RA_UpdateAppTitle("");
    RA_OnLoadNewRom(NULL, 0);
    should_activate = true;
}

void RA_ProcessReset()
{


    RA_OnReset();
}

int RA_HandleMenuEvent(int id)
{
    if (LOWORD(id) >= IDM_RA_MENUSTART &&
        LOWORD(id) < IDM_RA_MENUEND)
    {
        RA_InvokeDialog(LOWORD(id));
        return TRUE;
    }

    return FALSE;
}

static unsigned long last_tick = timeGetTime(); // Last call time of RA_RenderOverlayFrame()
void RA_RenderOverlayFrame(HDC hdc)
{
    float delta_time = (timeGetTime() - last_tick) / 1000.0f;
    int width = 640, height = 480;
    RECT window_size = { 0, 0, width, height };


    ControllerInput input = {};

    if (g_bFrameActive) // Do not process input while out of focus
    {
        input.m_bConfirmPressed = GetKeyState(VK_RETURN) & WM_KEYDOWN;
        input.m_bCancelPressed = GetKeyState(VK_BACK) & WM_KEYDOWN;
        input.m_bQuitPressed = GetKeyState(VK_ESCAPE) & WM_KEYDOWN;
        input.m_bLeftPressed = GetKeyState(VK_LEFT) & WM_KEYDOWN;
        input.m_bRightPressed = GetKeyState(VK_RIGHT) & WM_KEYDOWN;
        input.m_bUpPressed = GetKeyState(VK_UP) & WM_KEYDOWN;
        input.m_bDownPressed = GetKeyState(VK_DOWN) & WM_KEYDOWN;
    }

    RA_UpdateRenderOverlay(hdc, &input, delta_time, &window_size, false, g_ulAtariState == ATARI_PAUSED);

    last_tick = timeGetTime();
}

int RA_ConfirmQuit()
{
    if (!confirmed_quitting)
        confirmed_quitting = RA_ConfirmLoadNewRom(true);

    return confirmed_quitting;
}


