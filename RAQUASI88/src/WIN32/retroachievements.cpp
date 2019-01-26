#if USE_RETROACHIEVEMENTS
#include "retroachievements.h"

#include "BuildVer.h"

extern "C"
{
    #include <windows.h>

    #include "device.h"
    #include "file-op.h"
    #include "initval.h"
    #include "keyboard.h"
    #include "memory.h"
    #include "quasi88.h"
    #include "screen.h"
}

FileInfo loaded_disk = FINFO_DEFAULT;
FileInfo loaded_tape = FINFO_DEFAULT;
FileInfo loading_file = FINFO_DEFAULT;
FileInfo *loaded_title = 0;
bool should_activate = true;

void reset_file_info(FileInfo *file)
{
    file->data = 0;
    file->data_len = 0;
    file->name[0] = 0;
    file->title_id = 0;
    file->file_type = 0;
}

void free_file_info(FileInfo *file)
{
    if (file->data)
        free(file->data);

    reset_file_info(file);
}

/****************************************************************************
 * 実績処理に使用するメモリの読み書き関数
 *****************************************************************************/
#ifndef RA_ENABLE_TVRAM
#define RA_ENABLE_TVRAM 1 /* プログラムデータは高速RAMに保存されることがあるため、有効にする */
#endif

#ifndef RA_ENABLE_GVRAM
#define RA_ENABLE_GVRAM 0 /* アドレス空間をできるだけ締めるため、有用性の低いグラフィックVRAMを無効にする */
#endif

#ifndef RA_ENABLE_EXTRAM
#define RA_ENABLE_EXTRAM 0 /* 拡張RAMの有無は期待できないため、とにかく無効にする */
#endif

unsigned char ByteReader(byte *buf, size_t nOffs)
{
    return *(buf + nOffs);
}

void ByteWriter(byte *buf, size_t nOffs, unsigned char nVal)
{
    *(buf + nOffs) = nVal;
}

unsigned char DummyReader(size_t nOffs)
{
    return 0;
}

void DummyWriter(size_t nOffs, unsigned char nVal)
{
    return;
}

unsigned char MainRAMReader(size_t nOffs)
{
    return ByteReader(main_ram, nOffs);
}

void MainRAMWriter(size_t nOffs, unsigned char nVal)
{
    ByteWriter(main_ram, nOffs, nVal);
}

#if RA_ENABLE_TVRAM
unsigned char TVRAMReader(size_t nOffs)
{
    return ByteReader(main_high_ram, nOffs);
}

void TVRAMWriter(size_t nOffs, unsigned char nVal)
{
    ByteWriter(main_high_ram, nOffs, nVal);
}
#endif

#if RA_ENABLE_GVRAM
unsigned char GVRAMReader(size_t nOffs)
{
    return ByteReader(main_vram[nOffs >> 14], nOffs % 0x4000);
}

void GVRAMWriter(size_t nOffs, unsigned char nVal)
{
    ByteWriter(main_vram[nOffs >> 14], nOffs % 0x4000, nVal);
}
#endif

#if RA_ENABLE_EXTRAM
unsigned char ExtRAMReader(size_t nOffs)
{
    return ByteReader((byte *)ext_ram, nOffs);
}

void ExtRAMWriter(size_t nOffs, unsigned char nVal)
{
    ByteWriter((byte *)ext_ram, nOffs, nVal);
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

bool GameIsActive()
{
    return loaded_title != NULL;
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
    const int ra_buffer_size = 64;

    if (loading_file.data_len > 0)
    {
        // ロード中のファイル名を返す
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

    // 文字列を必ず NULL で終わらせる
    sNameOut[ra_buffer_size - 1] = '\0';
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
    RA_InitMemory();

    if (main_hdc)
    {
        ReleaseDC(g_hWnd, main_hdc);
    }

    main_hdc = GetDC(g_hWnd);
}

void RA_InitMemory()
{
    int bank_id = 0;

    RA_ClearMemoryBanks();
    RA_InstallMemoryBank(bank_id++, MainRAMReader, MainRAMWriter, 0x10000);

#if RA_ENABLE_TVRAM
    RA_InstallMemoryBank(bank_id++, TVRAMReader, TVRAMWriter, 0x1000);
#elif RA_ENABLE_GVRAM || RA_ENABLE_EXTRAM
    RA_InstallMemoryBank(bank_id++, DummyReader, DummyWriter, 0x1000);
#endif

#if RA_ENABLE_GVRAM
    RA_InstallMemoryBank(bank_id++, GVRAMReader, GVRAMWriter, 0x4000 * 4);
#elif RA_ENABLE_EXTRAM
    RA_InstallMemoryBank(bank_id++, DummyReader, DummyWriter, 0x4000 * 4);
#endif

    /* 注意：RA_ENABLE_EXTRAM をセットする場合は、
    use_extram がいつでも変わることができるため、再初期化の管理が必要 */
#if RA_ENABLE_EXTRAM
    RA_InstallMemoryBank(bank_id++, ExtRAMReader, ExtRAMWriter, 0x8000 * 4 * use_extram);
#endif
}

#define RA_RELOAD_MULTI_DISK FALSE /* ディスクを切り替えるときは同じタイトルが検出されても、
                                     実績システムを再初期化する */
int RA_PrepareLoadNewRom(const char *file_name, int file_type)
{
    FILE *f = fopen(file_name, "rb");

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
            if (!RA_WarnDisableHardcore("load a new title without ejecting all images and resetting the emulator"))
            {
                free_file_info(&loading_file);
                return FALSE; /* 読み込みを中止する */
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
    case FTYPE_DISK:
        free_file_info(&loaded_disk);
        loaded_disk = loading_file;
        loaded_title = &loaded_disk;
        break;
    case FTYPE_TAPE_LOAD:
        free_file_info(&loaded_tape);
        loaded_tape = loading_file;
        loaded_title = &loaded_tape;
        break;
    default:
        break;
    }

    RA_UpdateAppTitle(loading_file.name);

    if (should_activate)
    {
        /* 実績システムのイメージデータを初期化する */
        RA_ActivateGame(loading_file.title_id);
        should_activate = true;
    }

    /* ロード中のデータをクリアする */
    reset_file_info(&loading_file);
}

void RA_OnGameClose(int file_type)
{
    if (loaded_title != NULL && loaded_title->file_type == file_type)
        loaded_title = NULL;

    switch (file_type)
    {
    case FTYPE_DISK:
        free_file_info(&loaded_disk);
        if (loaded_tape.data_len > 0 && !RA_HardcoreModeIsActive())
        {
            loaded_title = &loaded_tape;
            RA_UpdateAppTitle(loaded_title->name);
            RA_ActivateGame(loaded_title->title_id);
        }
        break;
    case FTYPE_TAPE_LOAD:
        free_file_info(&loaded_tape);
        if (loaded_disk.data_len > 0 && !RA_HardcoreModeIsActive())
        {
            loaded_title = &loaded_disk;
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
#ifdef  SUPPORT_DOUBLE
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
