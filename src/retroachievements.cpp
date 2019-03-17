#if USE_RETROACHIEVEMENTS
#include "retroachievements.h"
#include "BuildVer.h"

#include <windows.h>
#include "vm/scv/memory.h"

#define memory ((MEMORY *)(((VM*)(cur_emu->get_vm()))->get_memory(0)))
EMU *cur_emu = nullptr;
char loaded_file[_MAX_FNAME] = { 0 };

unsigned char VRAMReader(size_t nOffs)
{
    return memory->read_data8(0x2000 + nOffs);
}

void VRAMWriter(size_t nOffs, unsigned char nVal)
{
    memory->write_data8(0x2000 + nOffs, nVal);
}

unsigned char MainRAMReader(size_t nOffs)
{
    return memory->read_data8(0xFF80 + nOffs);
}

void MainRAMWriter(size_t nOffs, unsigned char nVal)
{
    memory->write_data8(0xFF80 + nOffs, nVal);
}

#if RA_ENABLE_SRAM // TODO: untested
unsigned char SRAMReader(size_t nOffs)
{
    return memory->get_sram()[nOffs];
}

void SRAMWriter(size_t nOffs, unsigned char nVal)
{
    memory->get_sram()[nOffs] = nVal;
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

void CauseUnpause()
{
    if (cur_emu != nullptr)
        cur_emu->run();
}

void CausePause()
{
    if (cur_emu != nullptr)
        cur_emu->suspend();
}

void RebuildMenu()
{
    if (cur_emu == nullptr)
        return;

    HWND hwnd = cur_emu->get_osd()->main_window_handle;

    // get main menu handle
    HMENU hMainMenu = GetMenu(hwnd);
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

    DrawMenuBar(hwnd);
}

void GetEstimatedGameTitle(char* sNameOut)
{
    const int ra_buffer_size = 64;

    if (cur_emu == nullptr || loaded_file == NULL)
        memset(sNameOut, 0, ra_buffer_size);
    else
        memcpy(sNameOut, loaded_file, ra_buffer_size);

    // Always null-terminate strings
    sNameOut[ra_buffer_size - 1] = '\0';
}

void ResetEmulation()
{
    if (cur_emu != nullptr)
        cur_emu->reset();
}

void LoadROM(const char* sFullPath)
{
    cur_emu->open_cart(0, sFullPath);
}

void RA_InitShared()
{
    RA_InstallSharedFunctions(NULL, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM);
}

static HDC main_hdc;
void RA_InitUI(EMU *new_emu)
{
    if (new_emu == nullptr)
        return;

    cur_emu = new_emu;

    HWND hwnd = cur_emu->get_osd()->main_window_handle;

    RA_Init(hwnd, RA_SCV, RASCV_VERSION_SHORT);
    RA_InitShared();
    RebuildMenu();
    RA_AttemptLogin(true);
    RebuildMenu();
    RA_InitMemory();

    if (main_hdc)
    {
        ReleaseDC(hwnd, main_hdc);
    }

    main_hdc = GetDC(hwnd);
}

void RA_InitMemory()
{
    if (cur_emu == nullptr)
        return;

    int bank_id = 0;

    RA_ClearMemoryBanks();
    RA_InstallMemoryBank(bank_id++, VRAMReader, VRAMWriter, 0x2000);
    RA_InstallMemoryBank(bank_id++, MainRAMReader, MainRAMWriter, 0x80);

#if RA_ENABLE_SRAM
    // Map bank-switched SRAM sequentially
    RA_InstallMemoryBank(bank_id++, SRAMReader, SRAMWriter, 0x2000);
#endif
}

void RA_OnGameOpen(const char *file_name)
{
    if (cur_emu == nullptr)
        return;

    FILE *f = fopen(file_name , "rb");

    if(!f)
        return;

    char basename[_MAX_FNAME];
    _splitpath(file_name, NULL, NULL, basename, NULL);
    strcpy(loaded_file, basename);

    fseek(f, 0, SEEK_END);
    const unsigned long file_size = (unsigned long)ftell(f);
    BYTE * const file_data = (BYTE *)malloc(file_size * sizeof(BYTE));
    fseek(f, 0, SEEK_SET);
    fread(file_data, sizeof(BYTE), file_size, f);

    fflush(f);
    fclose(f);

    int offset = 0;
    if (!memcmp(file_data, "SCV\x1A", 4))
    {
        offset = 16;
    }

    RA_OnLoadNewRom(file_data + offset, file_size - offset);
    RA_UpdateAppTitle(loaded_file);

    free(file_data);
}

void RA_OnGameClose()
{
    memset(loaded_file, 0, sizeof(loaded_file));
    RA_ClearTitle();
}

void RA_ClearTitle()
{
    RA_UpdateAppTitle("");
    RA_OnLoadNewRom(NULL, 0);
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

static unsigned long last_tick = timeGetTime(); // Last call time of RA_RenderOverlayFrame
void RA_RenderOverlayFrame(HDC hdc)
{
    if (cur_emu == nullptr)
        return;

    if (!hdc)
        hdc = main_hdc;

    float delta_time = (timeGetTime() - last_tick) / 1000.0f;

    int width = cur_emu->get_osd()->get_draw_screen_width();
    int height = cur_emu->get_osd()->get_draw_screen_height();

    RECT window_size = { 0, 0, width, height };

    ControllerInput input;
    /*uint32_t joy_status = cur_emu->get_osd()->get_joy_buffer()[0];
    input.m_bConfirmPressed = GetKeyState(VK_RETURN) & WM_KEYDOWN || joy_status & 0b10000;
    input.m_bCancelPressed = GetKeyState(VK_BACK) & WM_KEYDOWN || joy_status & 0b100000;
    input.m_bQuitPressed = GetKeyState(VK_ESCAPE) & WM_KEYDOWN || joy_status & 0b1000000;
    input.m_bLeftPressed = GetKeyState(VK_LEFT) & WM_KEYDOWN || joy_status & 0b100;
    input.m_bRightPressed = GetKeyState(VK_RIGHT) & WM_KEYDOWN || joy_status & 0b1000;
    input.m_bUpPressed = GetKeyState(VK_UP) & WM_KEYDOWN || joy_status & 0b1;
    input.m_bDownPressed = GetKeyState(VK_DOWN) & WM_KEYDOWN || joy_status & 0b10;*/

    // Emulator pause is not supported
    RA_UpdateRenderOverlay(hdc, &input, delta_time, &window_size, false, false);

    last_tick = timeGetTime();
}

#endif
