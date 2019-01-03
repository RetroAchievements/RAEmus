#if USE_RETROACHIEVEMENTS
#include "retroachievements.h"

#include "BuildVer.h"

extern "C"
{
    #include <windows.h>

    #include "device.h"
    #include "initval.h"
    #include "keyboard.h"
    #include "memory.h"
    #include "quasi88.h"
    #include "screen.h"
}

/****************************************************************************
 * 実績処理に使用するメモリの読み書き関数
 *****************************************************************************/
unsigned char ByteReader(byte *buf, size_t nOffs)
{
    return *(buf + nOffs);
}

void ByteWriter(byte *buf, size_t nOffs, unsigned char nVal)
{
    *(buf + nOffs) = nVal;
}

unsigned char MainRAMReader(size_t nOffs)
{
    return ByteReader(main_ram, nOffs);
}

void MainRAMWriter(size_t nOffs, unsigned char nVal)
{
    ByteWriter(main_ram, nOffs, nVal);
}

/* アドレス空間をできるだけ締めるため、有用性の低い高速RAM・VRAMを無効にする */
#define RA_ENABLE_VRAM 0
#if RA_ENABLE_VRAM
unsigned char HighSpeedRAMReader(size_t nOffs)
{
    return ByteReader(main_high_ram, nOffs);
}

void HighSpeedRAMWriter(size_t nOffs, unsigned char nVal)
{
    ByteWriter(main_high_ram, nOffs, nVal);
}

unsigned char MainVRAMReader(size_t nOffs)
{
    return ByteReader(main_vram[nOffs >> 14], nOffs % 0x4000);
}

void MainVRAMWriter(size_t nOffs, unsigned char nVal)
{
    ByteWriter(main_vram[nOffs >> 14], nOffs % 0x4000 , nVal);
}
#endif



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

static char loaded_title[_MAX_FNAME] = { NULL };
bool GameIsActive()
{
    return quasi88_is_exec();
}

void CauseUnpause()
{
    quasi88_exec();
}

void CausePause()
{
    quasi88_pause();
}

void RebuildMenu()
{
    // get main menu handle
    HMENU hMainMenu = g_hMenu;
    if (!hMainMenu) return;

    // get file menu index
    int index = GetMenuItemIndex(hMainMenu, "&RetroAchievements");
    if (index >= 0)
        DeleteMenu(hMainMenu, index, MF_BYPOSITION);

    index = GetMenuItemIndex(hMainMenu, "&Debug");
    if (index >= 0)
        DeleteMenu(hMainMenu, index, MF_BYPOSITION);

    // embed RA
    AppendMenu(hMainMenu, MF_POPUP | MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements"));

    DrawMenuBar(g_hWnd);
}

void GetEstimatedGameTitle(char* sNameOut)
{
    const int BUF_SIZE = 64;

    if (loaded_title[0] != NULL)
    {
        memcpy(sNameOut, loaded_title, BUF_SIZE);
        sNameOut[BUF_SIZE - 1] = '\0';
    }
    else
    {
        memset(sNameOut, 0, BUF_SIZE);
    }
}

void ResetEmulation()
{
    T_RESET_CFG cfg;
    quasi88_get_reset_cfg(&cfg);
    quasi88_reset(&cfg);
}

void LoadROM(const char* sFullPath)
{
    quasi88_disk_insert_all(sFullPath, TRUE);
}

void RA_InitShared()
{
    RA_InstallSharedFunctions(&GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM);
}

static HDC main_hdc;
void RA_InitUI()
{
    RA_Init(g_hWnd, RA_QUASI88, RAQUASI88_VERSION);
    RA_InitShared();
    RebuildMenu();
    RA_AttemptLogin(true);
    RebuildMenu();

    if (main_hdc)
    {
        ReleaseDC(g_hWnd, main_hdc);
    }

    main_hdc = GetDC(g_hWnd);
}

void RA_InitMemory()
{
    RA_ClearMemoryBanks();
    RA_InstallMemoryBank(0, MainRAMReader, MainRAMWriter, 0x10000);

#if RA_ENABLE_VRAM
    RA_InstallMemoryBank(1, HighSpeedRAMReader, HighSpeedRAMWriter, 0x1000);
    RA_InstallMemoryBank(2, MainVRAMReader, MainVRAMWriter, 0x4000 * 4);
#endif
}

void RA_OnGameClose()
{
    RA_ClearMemoryBanks();
    RA_SetGameTitle("");
    loaded_title[0] = '\0';
    RA_OnLoadNewRom(NULL, 0);
}

void RA_SetGameTitle(char *title)
{
    strcpy(loaded_title, title);
    RA_UpdateAppTitle(title);
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

static unsigned long last_tick = timeGetTime(); /* RA_RenderOverlayFrame の前コールの時刻 */
void RA_RenderOverlayFrame(HDC hdc)
{
    if (!hdc)
        hdc = main_hdc;

    float delta_time = (timeGetTime() - last_tick) / 1000.0f;

    int width = WIDTH, height = HEIGHT;

    switch (now_screen_size)
    {
    case SCREEN_SIZE_HALF:
        width >>= 1;
        height >>= 1;
        break;
#ifdef	SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE:
        /* どうやらWIDTH×HEIGHTのままでいい */
        break;
#endif
    default:
        break;
    }

    RECT window_size = { 0, 0, width, height };

    ControllerInput input;
    input.m_bConfirmPressed = IS_KEY88_PRESS(KEY88_RETURN);
    input.m_bCancelPressed = IS_KEY88_PRESS(KEY88_BS);
    input.m_bQuitPressed = IS_KEY88_PRESS(KEY88_ESC);
    input.m_bLeftPressed = IS_KEY88_PRESS(KEY88_LEFT);
    input.m_bRightPressed = IS_KEY88_PRESS(KEY88_RIGHT);
    input.m_bUpPressed = IS_KEY88_PRESS(KEY88_UP);
    input.m_bDownPressed = IS_KEY88_PRESS(KEY88_DOWN);

    RA_UpdateRenderOverlay(hdc, &input, delta_time, &window_size, use_fullscreen, (bool)quasi88_is_pause());

    last_tick = timeGetTime();
}

#endif
