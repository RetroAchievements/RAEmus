#if USE_RETROACHIEVEMENTS

#include <SDL_stdinc.h>
#include <windows.h>

#include "retroachievements.h"
#include "BuildVer.h"

#include "8912.h"
#include "disk.h"
#include "gui.h"
#include "gui_win.h"
#include "tape.h"

FileInfo loaded_tape = FINFO_DEFAULT;
FileInfo loaded_disk = FINFO_DEFAULT;
FileInfo loading_file = FINFO_DEFAULT;
FileInfo *loaded_title = 0;
bool should_activate = true;

bool confirmed_quitting = false;
bool is_initialized = false;
bool can_hotswap_tapes = true; // Allow tape hot-swapping until loaded

static HDC main_hdc;
static machine *active_machine;

void reset_file_info(FileInfo *file)
{
    file->data = 0;
    file->data_len = 0;
    file->name[0] = 0;
    file->title_id = 0;
    file->file_type = FileType::TAPE;
}

void free_file_info(FileInfo *file)
{
    if (file->data)
        free(file->data);

    reset_file_info(file);
}

/*****************************************************************************
 * Memory reader/writer for achievement processing                           *
 *****************************************************************************/

unsigned char MainRAMReader(size_t nOffs)
{
    if (!active_machine || active_machine->memsize < nOffs)
        return 0;

    return active_machine->mem[nOffs];
}

void MainRAMWriter(size_t nOffs, unsigned char nVal)
{
    if (!active_machine || active_machine->memsize < nOffs)
        return;

    *(active_machine->mem + nOffs) = nVal;
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
    return loaded_title != NULL;
}

void CauseUnpause()
{
    active_machine->emu_mode = EM_RUNNING;
}

void CausePause()
{
    active_machine->emu_mode = EM_PAUSED;
}

void RebuildMenu()
{
    // get main menu handle
    HMENU hMainMenu = GetMenu(g_SDL_Window);
    if (!hMainMenu) return;

    // get file menu index
    int index = GetMenuItemIndex(hMainMenu, "&RetroAchievements");
    if (index >= 0)
        DeleteMenu(hMainMenu, index, MF_BYPOSITION);

    // embed RA
    AppendMenu(hMainMenu, MF_POPUP | MF_STRING, (UINT_PTR)RA_CreatePopupMenu(), TEXT("&RetroAchievements"));

    DrawMenuBar(g_SDL_Window);
}

void GetEstimatedGameTitle(char* sNameOut)
{
    const int ra_buffer_size = 64;

    if (loading_file.data_len > 0)
    {
        // Return the file name being loaded
        memcpy(sNameOut, loading_file.name, ra_buffer_size);
    }
    else if (loaded_title != NULL && loaded_title->name[0] != NULL)
    {
        memcpy(sNameOut, loaded_title->name, ra_buffer_size);
    }
    else
    {
        memset(sNameOut, 0, ra_buffer_size);
    }

    // Always null-terminate strings
    sNameOut[ra_buffer_size - 1] = '\0';
}

void ResetEmulation()
{
    softresetoric(active_machine, NULL, 0);
}

void LoadROM(const char* sFullPath)
{
    // Reproduce the load behavior in main.c
    switch (detect_image_type((char *)sFullPath))
    {
        case IMG_ATMOS_MICRODISC:
        case IMG_ATMOS_JASMIN:
        case IMG_TELESTRAT_DISK:
        case IMG_PRAVETZ_DISK:
        case IMG_GUESS_MICRODISC:
            diskimage_load(active_machine, (char *)sFullPath, 0);
            switch (active_machine->drivetype)
            {
            case DRV_PRAVETZ:
                pravdiskboot(active_machine);
                break;

            case DRV_JASMIN:
                active_machine->auto_jasmin_reset = SDL_TRUE;
                break;
            }
            break;
        case IMG_TAPE:
            if (tape_load_tap(active_machine, (char*)sFullPath))
                queuekeys("CLOAD\"\"\x0d");
            break;
    }
}

void RA_InitShared()
{
    RA_InstallSharedFunctions(&GameIsActive, &CauseUnpause, &CausePause, &RebuildMenu, &GetEstimatedGameTitle, &ResetEmulation, &LoadROM);
}

void RA_InitSystem()
{
    if (!is_initialized)
    {
        RA_Init(g_SDL_Window, RA_Oricutron, RAORICUTRON_VERSION_SHORT);
        RA_InitShared();
        RA_AttemptLogin(true);

        is_initialized = true;
    }

    confirmed_quitting = false;
}

void RA_InitUI(machine *oric)
{
    active_machine = oric;

    RebuildMenu();
    RA_InitMemory();

    if (main_hdc)
        ReleaseDC(g_SDL_Window, main_hdc);

    main_hdc = GetDC(g_SDL_Window);
}

void RA_InitMemory()
{
    RA_ClearMemoryBanks();
    RA_InstallMemoryBank(0, MainRAMReader, MainRAMWriter, active_machine->memsize);
}

#define RA_RELOAD_MULTI_DISK FALSE /* When swapping disks, reload the achievement system
                                    even when the title is the same */
int RA_PrepareLoadNewRom(const char *file_name, FileType file_type)
{
    if (!file_name)
        return FALSE;

    FILE *f = fopen(file_name, "rb");

    if (!f)
        return FALSE;

    char basename[_MAX_FNAME];
    _splitpath(file_name, NULL, NULL, basename, NULL);
    strcpy(loading_file.name, basename);

    fseek(f, 0, SEEK_END);
    const unsigned long file_size = (unsigned long)ftell(f);
    loading_file.data_len = file_size;

    BYTE * const file_data = (BYTE *)malloc(file_size * sizeof(BYTE));
    loading_file.data = file_data;
    fseek(f, 0, SEEK_SET);
    fread(file_data, sizeof(BYTE), file_size, f);

    fflush(f);
    fclose(f);

    loading_file.title_id = RA_IdentifyRom(file_data, file_size);
    loading_file.file_type = file_type;

    if (loaded_title != NULL && loaded_title->data_len > 0)
    {
        if (loaded_title->title_id != loading_file.title_id || loaded_title->file_type != loading_file.file_type)
        {
            // Allow hot-swapping to unrecognized files for tape media, in order to support save/load systems
            if ((!can_hotswap_tapes && loading_file.title_id != 0) || loading_file.file_type != FileType::TAPE)
            {
                if (!RA_WarnDisableHardcore("load a new title without ejecting all images and hard-resetting the emulator"))
                {
                    free_file_info(&loading_file);
                    return FALSE; // Interrupt loading
                }
            }
        }
    }

#if !RA_RELOAD_MULTI_DISK
    should_activate = loaded_title != NULL &&
        loaded_title->title_id > 0 &&
        loaded_title->title_id == loading_file.title_id ?
        false :
        true;
#endif

    return TRUE;
}

void RA_CommitLoadNewRom()
{
    switch (loading_file.file_type)
    {
    case FileType::TAPE:
        free_file_info(&loaded_tape);
        loaded_tape = loading_file;
        loaded_title = &loaded_tape;
        break;
    case FileType::DISK:
        free_file_info(&loaded_disk);
        loaded_disk = loading_file;
        loaded_title = &loaded_disk;
        break;
    default:
        break;
    }

    RA_UpdateAppTitle(loading_file.name);

    if (should_activate)
    {
        // Initialize title data in the achievement system
        RA_ActivateGame(loading_file.title_id);
    }

    // Clear loading data
    reset_file_info(&loading_file);
}

void RA_OnGameClose(int file_type)
{
    if (loaded_title != NULL && loaded_title->file_type == file_type)
        loaded_title = NULL;

    switch (file_type)
    {
    case FileType::TAPE:
        free_file_info(&loaded_tape);
        if (loaded_disk.data_len > 0 && !RA_HardcoreModeIsActive())
        {
            loaded_title = &loaded_disk;
            RA_UpdateAppTitle(loaded_title->name);
            RA_ActivateGame(loaded_title->title_id);
        }
        break;
    case FileType::DISK:
        free_file_info(&loaded_disk);
        if (loaded_tape.data_len > 0 && !RA_HardcoreModeIsActive())
        {
            loaded_title = &loaded_tape;
            RA_UpdateAppTitle(loaded_title->name);
            RA_ActivateGame(loaded_title->title_id);
        }
        break;
    default:
        break;
    }

    if (loaded_title == NULL && loading_file.data_len == 0)
    {
        RA_UpdateAppTitle("");
        RA_OnLoadNewRom(NULL, 0);
    }
}

void RA_ProcessReset()
{
    if (RA_HardcoreModeIsActive())
    {
        if (loaded_tape.data_len > 0 && loaded_disk.data_len > 0)
        {
            if (loaded_title != NULL)
            {
                switch (loaded_title->file_type)
                {
                case FileType::TAPE:
                    disk_eject(active_machine, 0);
                    break;
                case FileType::DISK:
                    tape_eject(active_machine);
                    break;
                default:
                    // Prioritize tapes
                    disk_eject(active_machine, 0);
                    break;
                }
            }
        }
    }

    // Reset any softcore features here

    if (loaded_title == NULL)
    {
        if (loaded_tape.data_len > 0)
            loaded_title = &loaded_tape;
        else if (loaded_disk.data_len > 0)
            loaded_title = &loaded_disk;

        if (loaded_title != NULL)
        {
            RA_UpdateAppTitle(loaded_title->name);
            RA_ActivateGame(loaded_title->title_id);
        }
    }

    RA_ToggleTapeHotSwapping(true);

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
    if (!hdc)
        hdc = main_hdc;

    float delta_time = (timeGetTime() - last_tick) / 1000.0f;

    int width = 640, height = 480; // no scaling options

    RECT window_size = { 0, 0, width, height };

    ControllerInput input;
    input.m_bConfirmPressed = GetKeyState(VK_RETURN) & WM_KEYDOWN;
    input.m_bCancelPressed = GetKeyState(VK_BACK) & WM_KEYDOWN;
    input.m_bQuitPressed = GetKeyState(VK_ESCAPE) & WM_KEYDOWN;
    input.m_bLeftPressed = GetKeyState(VK_LEFT) & WM_KEYDOWN;
    input.m_bRightPressed = GetKeyState(VK_RIGHT) & WM_KEYDOWN;
    input.m_bUpPressed = GetKeyState(VK_UP) & WM_KEYDOWN;
    input.m_bDownPressed = GetKeyState(VK_DOWN) & WM_KEYDOWN;
    
    RA_UpdateRenderOverlay(hdc, &input, delta_time, &window_size, fullscreen, active_machine->emu_mode == EM_PAUSED);

    last_tick = timeGetTime();
}

int RA_ConfirmQuit()
{
    if (!confirmed_quitting)
        confirmed_quitting = RA_ConfirmLoadNewRom(true);

    return confirmed_quitting;
}

void RA_ToggleTapeHotSwapping(int enabled)
{
    can_hotswap_tapes = enabled;
}

#endif
