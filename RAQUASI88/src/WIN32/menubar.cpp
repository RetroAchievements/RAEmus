/***********************************************************************
 * メニューバー処理
 ************************************************************************/

extern "C"
{
    #include "quasi88.h"
    #include "device.h"
    #include "event.h"

    #include "initval.h"
    #include "pc88main.h"   /* boot_basic, ...      */
    #include "memory.h"     /* use_pcg          */
    #include "soundbd.h"    /* sound_board          */
    #include "intr.h"       /* cpu_clock_mhz        */
    #include "keyboard.h"   /* mouse_mode           */
    #include "fdc.h"        /* fdc_wait         */
    #include "getconf.h"    /* config_save          */
    #include "screen.h"     /* SCREEN_INTERLACE_NO ...  */
    #include "emu.h"        /* cpu_timing, emu_reset()  */
    #include "menu.h"       /* menu_sound_restart()     */
    #include "drive.h"
    #include "snddrv.h"

    #include "resource.h"
}

#if USE_RETROACHIEVEMENTS
#include "retroachievements.h"
#endif


static  int menubar_active = TRUE;

static  T_RESET_CFG menubar_reset_cfg;

static  int menubar_images[NR_DRIVE];


/****************************************************************************
 * モード切り替え時の、メニューバーの処理再設定
 *  エミュモードとメニューモードで、メニューバーの内容を変更する
 *****************************************************************************/
static void menubar_item_setup(void);
static void menubar_item_sensitive(int sensitive);

void    menubar_setup(int active)
{
    if (active) {

    /*  メニューバーの内容を設定 */
    menubar_item_setup();

    /* 使えなくした項目を使えるようにする (エミュモード開始時) */
    menubar_item_sensitive(TRUE);

    menubar_active = TRUE;

    } else {

    /* ほとんどの項目を使えなくする (メニューモード開始時) */
    menubar_item_sensitive(FALSE);

    menubar_active = FALSE;
    }
}






/* メニューアイテムの文字列を変更 */
static void change_menuitem_label(UINT uItem, char *s)
{
    MENUITEMINFO menuInfo;

    memset(&menuInfo, 0, sizeof(menuInfo));
    menuInfo.cbSize = sizeof(menuInfo);

    menuInfo.fMask = MIIM_TYPE;
    menuInfo.fType = MFT_STRING;
    menuInfo.dwTypeData = s;
/*
    menuInfo.cch        = strlen(s);
*/

    SetMenuItemInfo(g_hMenu, uItem, FALSE, &menuInfo);
}


/* メニューアイテムの文字列を変更 … その２ … */
static void change_menuitem_label2(UINT uItem, char *s)
{
    MENUITEMINFO menuInfo;

    memset(&menuInfo, 0, sizeof(menuInfo));
    menuInfo.cbSize = sizeof(menuInfo);

    menuInfo.fMask = MIIM_TYPE;
    menuInfo.fType = MFT_STRING;
    menuInfo.dwTypeData = s;
/*
    menuInfo.cch        = strlen(s);
*/

    SetMenuItemInfo(GetSubMenu(g_hMenu, 2), uItem, TRUE, &menuInfo);
}


/* ファイル選択ダイアログを表示し、選択後に戻る */
static int select_file(int   for_read,
               const char *title,
               const char *filter,
               const char *def_ext,
               int  flag,
               char fullname[],
               int  n_fullname)
{
    OPENFILENAME ofn;
    char name[QUASI88_MAX_FILENAME];    /* パスなしファイル名 */
    int result;

    memset(fullname, 0, n_fullname);    /* パスありファイル名 */
    memset(name, 0, sizeof(name));
    memset(&ofn, 0, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);  /* 構造体サイズ           */
    ofn.hwndOwner   = g_hWnd;   /* ウインドウハンドル      */
    ofn.lpstrFilter = filter;   /* フィルター          */
    ofn.lpstrFile   = fullname; /* フルパス名バッファ      */
    ofn.nMaxFile    = n_fullname;   /*   そのサイズ            */
    ofn.lpstrFileTitle  = name;     /* ファイル名バッファ      */
    ofn.nMaxFileTitle   = sizeof(name); /*   そのサイズ            */
    ofn.lpstrTitle  = title;    /* ウインドウタイトル      */
    ofn.Flags       = flag;     /* 各種フラグ          */
    ofn.lpstrDefExt = def_ext;  /* 入力時に自動追加される拡張子   */

    if (for_read) {
    result = GetOpenFileName(&ofn);
    } else {
    result = GetSaveFileName(&ofn);
    }
    if (result) {
    if (ofn.Flags & OFN_READONLY) { return -1; }    /* ReadOnly で選択   */
    else                          { return  1; }    /* ReadWrite  で選択 */
    }
    return 0;                       /* ファイル選択せず  */
}






/* Reset メニューアイテムのラベルを更新する */
static void update_sys_reset(void)
{
    char buf[32];

    strcpy(buf, "Reset   [");

    switch (menubar_reset_cfg.boot_basic) {
    case BASIC_V2:      strcat(buf, "V2");      break;
    case BASIC_V1H:     strcat(buf, "V1H");     break;
    case BASIC_V1S:     strcat(buf, "V1S");     break;
    case BASIC_N:       strcat(buf, "N");       break;
    }

    strcat(buf, " : ");

    switch (menubar_reset_cfg.boot_clock_4mhz) {
    case CLOCK_4MHZ:        strcat(buf, "4MHz");        break;
    case CLOCK_8MHZ:        strcat(buf, "8MHz");        break;
    }

    strcat(buf, "]");

    change_menuitem_label(M_SYS_RESET, buf);
}



/* Drive メニューアイテムを生成・削除する */
static void update_drive(void)
{
    UINT uItem;
    char buf[64];
    int i;
    int drv, base, tag;
    int has_image = FALSE;

    /* 生成したメニューアイテムを削除する。*/

    for (drv = 0; drv < NR_DRIVE; drv ++) {
    base = (drv == DRIVE_1) ? M_DRV_DRV1_1 : M_DRV_DRV2_1;

    for (i = 0; i< menubar_images[drv]; i++) {
        uItem = i + base;
                      
        DeleteMenu(g_hMenu, uItem, MF_BYCOMMAND);
    }
    menubar_images[drv] = 0;
    }

    /* ドライブのイメージに応じて、メニューアイテムを生成する */

    for (drv = 0; drv < NR_DRIVE; drv ++) {
    base = (drv == DRIVE_1) ? M_DRV_DRV1_1  : M_DRV_DRV2_1;
    tag  = (drv == DRIVE_1) ? M_DRV_DRV1_NO : M_DRV_DRV2_NO;

    if (disk_image_exist(drv)) {

        /* イメージの数の分、生成する。*/
        for (i = 0; i < MIN(disk_image_num(drv), 9); i++) {
        uItem = i + base;

        sprintf(buf, "%d  ", i + 1);
        my_strncat(buf, drive[drv].image[i].name, sizeof(buf));

        InsertMenu(g_hMenu, tag, MF_BYCOMMAND | MFT_STRING,
               uItem, buf);
        }
        menubar_images[drv] = i;
        has_image = TRUE;

    } else {

        menubar_images[drv] = 0;
    }

    /* 選択中イメージの、ラジオボタンをチェックする */

    if (disk_image_exist(drv) == FALSE ||   /* ファイルなし or */
        drive_check_empty(drv)) {       /* 空を選択        */

        uItem = tag;                /*    → NO Disk */

    } else {
        i = disk_image_selected(drv);
        if (0 <= i && i <= 9) {         /* 1〜9番目選択 */
        uItem = i + base;           /*    → それだ */
        } else {                    /* 10番目〜     */
        uItem = 0;              /*    → なし   */
        }
    }
    CheckMenuRadioItem(g_hMenu,
               base,
               tag,
               uItem,
               MF_BYCOMMAND);
    }

    /* メニューの名前を変えたり、無効にしたり */

    for (drv = 0; drv < NR_DRIVE; drv ++) {
    const char *s;
    s = filename_get_disk_name(drv);

    if (s) {
        sprintf(buf, "Drive %d: ", drv + 1);
        my_strncat(buf, s, sizeof(buf));
    } else {
        sprintf(buf, "Drive %d:", drv + 1);
    }
    change_menuitem_label2(drv, buf);
    }

    change_menuitem_label(M_DRV_CHG, (has_image) ? "Change ..." : "Set ...");
    EnableMenuItem(g_hMenu, M_DRV_UNSET,
           MF_BYCOMMAND | ((has_image) ? MFS_ENABLED : MFS_GRAYED));

    DrawMenuBar(g_hWnd);
}

/* Tape Load メニューアイテムのラベルを変えたり使用不可にしたり */
static void update_misc_cload(void)
{
    UINT uItem;
    const char *s;
    char buf[64];

    s = filename_get_tape_name(CLOAD);

    /* テープありならファイル名を、なしならデフォルトのラベルを表示 */
    uItem = M_MISC_CLOAD_S;
    {
    if (s) { my_strncpy(buf, s, sizeof(buf)); }
    else   { strcpy(buf, "Set ...");          }
    change_menuitem_label(uItem, buf);
    }

    /* テープありなら、ラジオメニューをアクティブに */
    uItem = M_MISC_CLOAD_S;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | ((s) ? MFS_CHECKED : MFS_UNCHECKED));

    /* テープありなら unset を表示、なしなら隠す */
    uItem = M_MISC_CLOAD_U;
    EnableMenuItem(g_hMenu, uItem,
           MF_BYCOMMAND | ((s) ? MFS_ENABLED : MFS_GRAYED));

    DrawMenuBar(g_hWnd);
}

/* Tape Save メニューアイテムのラベルを変えたり使用不可にしたり */
static void update_misc_csave(void)
{
    UINT uItem;
    const char *s;
    char buf[64];

    s = filename_get_tape_name(CSAVE);

    /* テープありならファイル名を、なしならデフォルトのラベルを表示 */
    uItem = M_MISC_CSAVE_S;
    {
    if (s) { my_strncpy(buf, s, sizeof(buf)); }
    else   { strcpy(buf, "Set ...");          }
    change_menuitem_label(uItem, buf);
    }

    /* テープありなら、ラジオメニューをアクティブに */
    uItem = M_MISC_CSAVE_S;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | ((s) ? MFS_CHECKED : MFS_UNCHECKED));

    /* テープありなら unset を表示、なしなら隠す */
    uItem = M_MISC_CSAVE_U;
    EnableMenuItem(g_hMenu, uItem,
           MF_BYCOMMAND | ((s) ? MFS_ENABLED : MFS_GRAYED));

    DrawMenuBar(g_hWnd);
}

/* Sound Record メニューアイテムのチェックを変更する */
static void update_misc_record(void)
{
    UINT uItem;
    int i;

    i = xmame_wavout_opened();
    uItem = M_MISC_RECORD;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));
}




/*======================================================================
 * メニューバーの内容を再初期化
 *======================================================================*/
static void menubar_item_setup(void)
{
    UINT uItem;
    int i;
    MENUITEMINFO menuInfo;

    memset(&menuInfo, 0, sizeof(menuInfo));
    menuInfo.cbSize = sizeof(menuInfo);
    menuInfo.fMask = MIIM_STATE;
    menuInfo.fState = MFS_GRAYED;

    /* System -----------------------------------------------------------*/

    quasi88_get_reset_cfg(&menubar_reset_cfg);

    switch (menubar_reset_cfg.boot_basic) {
    case BASIC_V2:  uItem = M_SYS_MODE_V2;      break;
    case BASIC_V1H: uItem = M_SYS_MODE_V1H;     break;
    case BASIC_V1S: uItem = M_SYS_MODE_V1S;     break;
    case BASIC_N:   uItem = M_SYS_MODE_N;       break;
    }
    CheckMenuRadioItem(g_hMenu, M_SYS_MODE_V2, M_SYS_MODE_N, uItem,
               MF_BYCOMMAND);

    switch (menubar_reset_cfg.boot_clock_4mhz) {
    case CLOCK_4MHZ:    uItem = M_SYS_MODE_4MH;     break;
    case CLOCK_8MHZ:    uItem = M_SYS_MODE_8MH;     break;
    }
    CheckMenuRadioItem(g_hMenu, M_SYS_MODE_4MH, M_SYS_MODE_8MH, uItem,
               MF_BYCOMMAND);

    switch (menubar_reset_cfg.sound_board) {
    case SOUND_I:   uItem = M_SYS_MODE_SB;      break;
    case SOUND_II:  uItem = M_SYS_MODE_SB2;     break;
    }
    CheckMenuRadioItem(g_hMenu, M_SYS_MODE_SB, M_SYS_MODE_SB2, uItem,
               MF_BYCOMMAND);

    update_sys_reset();

    /* Setting ----------------------------------------------------------*/

    i = quasi88_cfg_now_wait_rate();                /* ＊＊＊＊ */
    switch (i) {
    case 25:        uItem = M_SET_SPD_25;       break;
    case 50:        uItem = M_SET_SPD_50;       break;
    case 100:       uItem = M_SET_SPD_100;      break;
    case 200:       uItem = M_SET_SPD_200;      break;
    case 400:       uItem = M_SET_SPD_400;      break;
    default:        uItem = 0;          break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_SPD_25, M_SET_SPD_400, uItem,
               MF_BYCOMMAND);
    /* uItem == 0 の時、 CheckMenuRadioItem() はエラーになる。
       結果、すべてチェックが外れる。… こんなんでいいのか？ */

    i = quasi88_cfg_now_no_wait();              /* ＊＊＊＊ */
    uItem = M_SET_SPD_MAX;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));

    i = cpu_timing;                     /* ＊＊＊＊ */
    switch (i) {
    case 0:     uItem = M_SET_SUB_SOME;     break;
    case 1:     uItem = M_SET_SUB_OFT;      break;
    case 2:     uItem = M_SET_SUB_ALL;      break;
    default:        uItem = 0;          break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_SUB_SOME, M_SET_SUB_ALL, uItem,
               MF_BYCOMMAND);

    i = fdc_wait;                       /* ＊＊＊＊ */
    uItem = M_SET_FDCWAIT;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));

    i = quasi88_cfg_now_frameskip_rate();           /* ＊＊＊＊ */
    switch (i) {
    case 1:     uItem = M_SET_REF_60;       break;
    case 2:     uItem = M_SET_REF_30;       break;
    case 3:     uItem = M_SET_REF_20;       break;
    case 4:     uItem = M_SET_REF_15;       break;
    default:        uItem = 0;          break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_REF_60, M_SET_REF_15, uItem,
               MF_BYCOMMAND);

    i = quasi88_cfg_now_interlace();                /* ＊＊＊＊ */
    switch (i) {
    case SCREEN_INTERLACE_NO:   uItem = M_SET_INT_NO;   break;
    case SCREEN_INTERLACE_SKIP: uItem = M_SET_INT_SKIP; break;
    case SCREEN_INTERLACE_YES:  uItem = M_SET_INT_YES;  break;
    default:            uItem = 0;      break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_INT_NO, M_SET_INT_YES, uItem,
               MF_BYCOMMAND);

    i = quasi88_cfg_now_size();                 /* ＊＊＊＊ */
    switch (i) {
    case SCREEN_SIZE_FULL:  uItem = M_SET_SIZ_FULL; break;
    case SCREEN_SIZE_HALF:  uItem = M_SET_SIZ_HALF; break;
#ifdef SUPPORT_DOUBLE
    case SCREEN_SIZE_DOUBLE: uItem = M_SET_SIZ_DOUBLE; break;
#endif
    default:            uItem = 0;      break;
    }

    CheckMenuRadioItem(g_hMenu, M_SET_SIZ_FULL, M_SET_SIZ_DOUBLE, uItem,
               MF_BYCOMMAND);

    i = quasi88_cfg_now_fullscreen();   /* ＊＊＊＊ */
    uItem = M_SET_FULLSCREEN;
    CheckMenuItem(g_hMenu, uItem,
        MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));

    i = use_pcg;                        /* ＊＊＊＊ */
    uItem = M_SET_PCG;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));

    i = mouse_mode;                     /* ＊＊＊＊ */
    switch (i) {
    case MOUSE_NONE:        uItem = M_SET_MO_NO;    break;
    case MOUSE_MOUSE:       uItem = M_SET_MO_MOUSE; break;
    case MOUSE_JOYMOUSE:    uItem = M_SET_MO_JOYMO; break;
    case MOUSE_JOYSTICK:    uItem = M_SET_MO_JOY;   break;
    default:            uItem = 0;      break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_MO_NO, M_SET_MO_JOY, uItem,
               MF_BYCOMMAND);

    i = cursor_key_mode;                    /* ＊＊＊＊ */
    switch (i) {
    case 0:     uItem = M_SET_CUR_DEF;      break;
    case 1:     uItem = M_SET_CUR_TEN;      break;
    default:        uItem = 0;          break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_CUR_DEF, M_SET_CUR_TEN, uItem,
               MF_BYCOMMAND);

    i = numlock_emu;                        /* ＊＊＊＊ */
    uItem = M_SET_NUMLOCK;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));

    i = romaji_input_mode;                  /* ＊＊＊＊ */
    uItem = M_SET_ROMAJI;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));

#ifdef  USE_SOUND
    if (xmame_has_sound()) {
#ifdef  USE_FMGEN
    i = xmame_cfg_get_use_fmgen();              /* ＊＊＊＊ */
    switch (i) {
    case 0:     uItem = M_SET_FM_MAME;      break;
    case 1:     uItem = M_SET_FM_FMGEN;     break;
    default:    uItem = 0;          break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_FM_MAME, M_SET_FM_FMGEN, uItem,
               MF_BYCOMMAND);
#endif

    i = xmame_cfg_get_sample_freq();            /* ＊＊＊＊ */
    switch (i) {
    case 48000: uItem = M_SET_FRQ_48;       break;
    case 44100: uItem = M_SET_FRQ_44;       break;
    case 22050: uItem = M_SET_FRQ_22;       break;
    case 11025: uItem = M_SET_FRQ_11;       break;
    default:    uItem = 0;          break;
    }
    CheckMenuRadioItem(g_hMenu, M_SET_FRQ_48, M_SET_FRQ_11, uItem,
               MF_BYCOMMAND);

    if (xmame_has_audiodevice()) {
        i = g_pcm_bufsize;                  /* ＊＊＊＊ */
        switch (i) {
        case 800:   uItem = M_SET_BUF_800;      break;
        case 400:   uItem = M_SET_BUF_400;      break;
        case 200:   uItem = M_SET_BUF_200;      break;
        case 100:   uItem = M_SET_BUF_100;      break;
        default:    uItem = 0;          break;
        }
        CheckMenuRadioItem(g_hMenu, M_SET_BUF_800, M_SET_BUF_100, uItem,
                   MF_BYCOMMAND);
    } else {
#ifdef  USE_FMGEN
        SetMenuItemInfo(GetSubMenu(g_hMenu, 1), 16, TRUE, &menuInfo);
#else
        SetMenuItemInfo(GetSubMenu(g_hMenu, 1), 15, TRUE, &menuInfo);
#endif
    }
    } else {
#ifdef  USE_FMGEN
    SetMenuItemInfo(GetSubMenu(g_hMenu, 1), 14, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 1), 15, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 1), 16, TRUE, &menuInfo);
#else
    /* メニュー位置がずれる・・・ */
    SetMenuItemInfo(GetSubMenu(g_hMenu, 1), 14, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 1), 15, TRUE, &menuInfo);
#endif
    }
#endif

    /* Drive ------------------------------------------------------------*/

    update_drive();

    /* Misc -------------------------------------------------------------*/

    if (xmame_has_sound()) {
    i = xmame_wavout_opened();
    uItem = M_MISC_RECORD;
    CheckMenuItem(g_hMenu, uItem,
              MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));
    } else {
    SetMenuItemInfo(GetSubMenu(g_hMenu, 3), 1, TRUE, &menuInfo);
    }

    update_misc_cload();

    update_misc_csave();

    i = quasi88_cfg_now_showstatus();
    uItem = M_MISC_STATUS;
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (i ? MFS_CHECKED : MFS_UNCHECKED));
}

/*======================================================================
 * メニューバー使用可能項目を設定
 *
 *  [System] [Setting] [Drive] [Misc] [Help] 
 *          0        1        2       3     4
 *           *        *       *
 *
 *      * が、不可にする対象
 *
 *  [System]
 *     Reset    0   *
 *    [Mode]    1   *
 *    ------    2
 *     V2       3   *
 *     V1H      4   *
 *     V1S      5   *
 *    ------    6
 *     Menu     7   *
 *    ------    8
 *     Save     9   *
 *     Exit     10
 *
 *      * が、不可にする対象
 *
 *  上のうち Reset などのようにメニューIDがあるものは、直接制御できるが
 *  ポップアップメニューはメニューIDが無いので、位置で制御しないとだめ。
 *  今回はすべて位置で制御しよう。    (なにか方法があるのでは…)
 *======================================================================*/
static void menubar_item_sensitive(int sensitive)
{
    MENUITEMINFO menuInfo;

    memset(&menuInfo, 0, sizeof(menuInfo));
    menuInfo.cbSize = sizeof(menuInfo);

    menuInfo.fMask = MIIM_STATE;
    menuInfo.fState = (sensitive) ? MFS_ENABLED : MFS_GRAYED;

    SetMenuItemInfo(g_hMenu,                 1, TRUE, &menuInfo);
    SetMenuItemInfo(g_hMenu,                 2, TRUE, &menuInfo);
    SetMenuItemInfo(g_hMenu,                 3, TRUE, &menuInfo);

    SetMenuItemInfo(GetSubMenu(g_hMenu, 0),  0, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 0),  1, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 0),  3, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 0),  4, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 0),  5, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 0),  7, TRUE, &menuInfo);
    SetMenuItemInfo(GetSubMenu(g_hMenu, 0),  9, TRUE, &menuInfo);

    DrawMenuBar(g_hWnd);
}


/***********************************************************************
 * メニューバーコールバック関数
 ************************************************************************/

static  void    f_sys_reset (void);
static  void    f_sys_basic (UINT uItem, int data);
static  void    f_sys_clock (UINT uItem, int data);
static  void    f_sys_sb    (UINT uItem, int data);
static  void    f_sys_menu  (void);
static  void    f_sys_save  (void);
static  void    f_sys_exit  (void);
static  void    f_set_speed (UINT uItem, int data);
static  void    f_set_nowait    (UINT uItem);
static  void    f_set_subcpu    (UINT uItem, int data);
static  void    f_set_fdcwait   (UINT uItem);
static  void    f_set_refresh   (UINT uItem, int data);
static  void    f_set_interlace (UINT uItem, int data);
static  void    f_set_size  (UINT uItem, int data);
static  void    f_set_fullscreen   (UINT uItem);
static  void    f_set_pcg   (UINT uItem);
static  void    f_set_mouse (UINT uItem, int data);
static  void    f_set_cursor    (UINT uItem, int data);
static  void    f_set_numlock   (UINT uItem);
static  void    f_set_romaji    (UINT uItem);
static  void    f_set_fm    (UINT uItem, int data);
static  void    f_set_frq   (UINT uItem, int data);
static  void    f_set_buf   (UINT uItem, int data);
static  void    f_drv_chg   (int data);
static  void    f_drv_drv1  (UINT uItem, int data);
static  void    f_drv_drv2  (UINT uItem, int data);
static  void    f_drv_unset (void);
static  void    f_misc_capture  (void);
static  void    f_misc_record   (UINT uItem);
static  void    f_misc_cload_s  (void);
static  void    f_misc_cload_u  (void);
static  void    f_misc_csave_s  (void);
static  void    f_misc_csave_u  (void);
static  void    f_misc_sload    (int data);
static  void    f_misc_ssave    (int data);
static  void    f_misc_status   (UINT uItem);
static  void    f_help_about    (void);



int menubar_event(int id)
{
    switch (id) {
    case M_SYS_RESET:       f_sys_reset();          break;
    case M_SYS_MENU:        f_sys_menu();           break;
    case M_SYS_SAVE:        f_sys_save();           break;
    case M_SYS_EXIT:        f_sys_exit();           break;

    case M_SYS_RESET_V2:    f_sys_basic(M_SYS_MODE_V2,  BASIC_V2);
                f_sys_reset();              break;
    case M_SYS_RESET_V1H:   f_sys_basic(M_SYS_MODE_V1H, BASIC_V1H);
                f_sys_reset();              break;
    case M_SYS_RESET_V1S:   f_sys_basic(M_SYS_MODE_V1S, BASIC_V1S);
                f_sys_reset();              break;

    case M_SYS_MODE_V2:     f_sys_basic(id, BASIC_V2);  break;
    case M_SYS_MODE_V1H:    f_sys_basic(id, BASIC_V1H); break;
    case M_SYS_MODE_V1S:    f_sys_basic(id, BASIC_V1S); break;
    case M_SYS_MODE_N:      f_sys_basic(id, BASIC_N);   break;

    case M_SYS_MODE_4MH:    f_sys_clock(id, CLOCK_4MHZ);    break;
    case M_SYS_MODE_8MH:    f_sys_clock(id, CLOCK_8MHZ);    break;

    case M_SYS_MODE_SB:     f_sys_sb(id, SOUND_I);      break;
    case M_SYS_MODE_SB2:    f_sys_sb(id, SOUND_II);     break;



    case M_SET_SPD_25:      f_set_speed(id, 25);        break;
    case M_SET_SPD_50:      f_set_speed(id, 50);        break;
    case M_SET_SPD_100:     f_set_speed(id, 100);       break;
    case M_SET_SPD_200:     f_set_speed(id, 200);       break;
    case M_SET_SPD_400:     f_set_speed(id, 400);       break;

    case M_SET_SPD_MAX:     f_set_nowait(id);       break;

    case M_SET_SUB_SOME:    f_set_subcpu(id, 0);        break;
    case M_SET_SUB_OFT:     f_set_subcpu(id, 1);        break;
    case M_SET_SUB_ALL:     f_set_subcpu(id, 2);        break;

    case M_SET_FDCWAIT:     f_set_fdcwait(id);      break;

    case M_SET_REF_60:      f_set_refresh(id, 1);       break;
    case M_SET_REF_30:      f_set_refresh(id, 2);       break;
    case M_SET_REF_20:      f_set_refresh(id, 3);       break;
    case M_SET_REF_15:      f_set_refresh(id, 4);       break;

    case M_SET_INT_NO:      f_set_interlace(id, SCREEN_INTERLACE_NO);   break;
    case M_SET_INT_SKIP:    f_set_interlace(id, SCREEN_INTERLACE_SKIP); break;
    case M_SET_INT_YES:     f_set_interlace(id, SCREEN_INTERLACE_YES);  break;

    case M_SET_SIZ_FULL:    f_set_size(id, SCREEN_SIZE_FULL);   break;
    case M_SET_SIZ_HALF:    f_set_size(id, SCREEN_SIZE_HALF);   break;
#ifdef SUPPORT_DOUBLE
    case M_SET_SIZ_DOUBLE:  f_set_size(id, SCREEN_SIZE_DOUBLE); break;
#endif

    case M_SET_FULLSCREEN:  f_set_fullscreen(id);   break;

    case M_SET_PCG:         f_set_pcg(id);          break;

    case M_SET_MO_NO:       f_set_mouse(id, MOUSE_NONE);        break;
    case M_SET_MO_MOUSE:    f_set_mouse(id, MOUSE_MOUSE);       break;
    case M_SET_MO_JOYMO:    f_set_mouse(id, MOUSE_JOYMOUSE);    break;
    case M_SET_MO_JOY:      f_set_mouse(id, MOUSE_JOYSTICK);    break;

    case M_SET_CUR_DEF:     f_set_cursor(id, 0);        break;
    case M_SET_CUR_TEN:     f_set_cursor(id, 1);        break;

    case M_SET_NUMLOCK:     f_set_numlock(id);      break;
    case M_SET_ROMAJI:      f_set_romaji(id);       break;

#ifdef  USE_SOUND
#ifdef  USE_FMGEN
    case M_SET_FM_MAME:     f_set_fm(id, 0);        break;
    case M_SET_FM_FMGEN:    f_set_fm(id, 1);        break;
#endif
    case M_SET_FRQ_48:      f_set_frq(id, 48000);       break;
    case M_SET_FRQ_44:      f_set_frq(id, 44100);       break;
    case M_SET_FRQ_22:      f_set_frq(id, 22050);       break;
    case M_SET_FRQ_11:      f_set_frq(id, 11025);       break;

    case M_SET_BUF_800:     f_set_buf(id, 800);     break;
    case M_SET_BUF_400:     f_set_buf(id, 400);     break;
    case M_SET_BUF_200:     f_set_buf(id, 200);     break;
    case M_SET_BUF_100:     f_set_buf(id, 100);     break;
#endif

    case M_DRV_DRV1_1:      f_drv_drv1(id, 0);      break;
    case M_DRV_DRV1_2:      f_drv_drv1(id, 1);      break;
    case M_DRV_DRV1_3:      f_drv_drv1(id, 2);      break;
    case M_DRV_DRV1_4:      f_drv_drv1(id, 3);      break;
    case M_DRV_DRV1_5:      f_drv_drv1(id, 4);      break;
    case M_DRV_DRV1_6:      f_drv_drv1(id, 5);      break;
    case M_DRV_DRV1_7:      f_drv_drv1(id, 6);      break;
    case M_DRV_DRV1_8:      f_drv_drv1(id, 7);      break;
    case M_DRV_DRV1_9:      f_drv_drv1(id, 8);      break;
    case M_DRV_DRV1_NO:     f_drv_drv1(id, -1);     break;
    case M_DRV_DRV1_CHG:    f_drv_chg(DRIVE_1);     break;

    case M_DRV_DRV2_1:      f_drv_drv2(id, 0);      break;
    case M_DRV_DRV2_2:      f_drv_drv2(id, 1);      break;
    case M_DRV_DRV2_3:      f_drv_drv2(id, 2);      break;
    case M_DRV_DRV2_4:      f_drv_drv2(id, 3);      break;
    case M_DRV_DRV2_5:      f_drv_drv2(id, 4);      break;
    case M_DRV_DRV2_6:      f_drv_drv2(id, 5);      break;
    case M_DRV_DRV2_7:      f_drv_drv2(id, 6);      break;
    case M_DRV_DRV2_8:      f_drv_drv2(id, 7);      break;
    case M_DRV_DRV2_9:      f_drv_drv2(id, 8);      break;
    case M_DRV_DRV2_NO:     f_drv_drv2(id, -1);     break;
    case M_DRV_DRV2_CHG:    f_drv_chg(DRIVE_2);     break;

    case M_DRV_CHG:     f_drv_chg(-1);          break;
    case M_DRV_UNSET:       f_drv_unset();          break;

    case M_MISC_CAPTURE:    f_misc_capture();       break;
    case M_MISC_RECORD:     f_misc_record(id);      break;

    case M_MISC_CLOAD_S:    f_misc_cload_s();       break;
    case M_MISC_CLOAD_U:    f_misc_cload_u();       break;

    case M_MISC_CSAVE_S:    f_misc_csave_s();       break;
    case M_MISC_CSAVE_U:    f_misc_csave_u();       break;

    case M_MISC_SLOAD_1:    f_misc_sload('1');      break;
    case M_MISC_SLOAD_2:    f_misc_sload('2');      break;
    case M_MISC_SLOAD_3:    f_misc_sload('3');      break;
    case M_MISC_SLOAD_4:    f_misc_sload('4');      break;
    case M_MISC_SLOAD_5:    f_misc_sload('5');      break;
    case M_MISC_SLOAD_6:    f_misc_sload('6');      break;
    case M_MISC_SLOAD_7:    f_misc_sload('7');      break;
    case M_MISC_SLOAD_8:    f_misc_sload('8');      break;
    case M_MISC_SLOAD_9:    f_misc_sload('9');      break;

    case M_MISC_SSAVE_1:    f_misc_ssave('1');      break;
    case M_MISC_SSAVE_2:    f_misc_ssave('2');      break;
    case M_MISC_SSAVE_3:    f_misc_ssave('3');      break;
    case M_MISC_SSAVE_4:    f_misc_ssave('4');      break;
    case M_MISC_SSAVE_5:    f_misc_ssave('5');      break;
    case M_MISC_SSAVE_6:    f_misc_ssave('6');      break;
    case M_MISC_SSAVE_7:    f_misc_ssave('7');      break;
    case M_MISC_SSAVE_8:    f_misc_ssave('8');      break;
    case M_MISC_SSAVE_9:    f_misc_ssave('9');      break;

    case M_MISC_STATUS:     f_misc_status(id);      break;

    case M_HELP_ABOUT:      f_help_about();         break;

    default:
#if USE_RETROACHIEVEMENTS
    return RA_HandleMenuEvent(id);
#else
    /* 未知のイベントは FALSE を返す */
    return FALSE;
#endif
    }

    return TRUE;
}

/*======================================================================
 * メニューバーサブ関数
 *======================================================================*/

/*----------------------------------------------------------------------
 * System メニュー
 *----------------------------------------------------------------------*/

static  void    f_sys_reset(void)
{
    if (menubar_reset_cfg.boot_clock_4mhz) {
    cpu_clock_mhz = CONST_4MHZ_CLOCK;
    } else {
    cpu_clock_mhz = CONST_8MHZ_CLOCK;
    }

    if (drive_check_empty(DRIVE_1)) {
    menubar_reset_cfg.boot_from_rom = TRUE;
    } else {
    menubar_reset_cfg.boot_from_rom = FALSE;
    }

    quasi88_reset(&menubar_reset_cfg);
}

static  void    f_sys_basic(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SYS_MODE_V2, M_SYS_MODE_N, uItem,
               MF_BYCOMMAND);
    {
    menubar_reset_cfg.boot_basic = (int)data;
    update_sys_reset();
    }
}

static  void    f_sys_clock(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SYS_MODE_4MH, M_SYS_MODE_8MH, uItem,
               MF_BYCOMMAND);
    {
    menubar_reset_cfg.boot_clock_4mhz = (int)data;
    update_sys_reset();
    }
}

static  void    f_sys_sb(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SYS_MODE_SB, M_SYS_MODE_SB2, uItem,
               MF_BYCOMMAND);
    {
    menubar_reset_cfg.sound_board = (int)data;
    update_sys_reset();
    }
}

static  void    f_sys_menu(void)
{
    quasi88_menu();
}

static  void    f_sys_save(void)
{
    config_save(NULL);
}

static  void    f_sys_exit(void)
{
    quasi88_quit();
}

/*----------------------------------------------------------------------
 * Setting メニュー
 *----------------------------------------------------------------------*/

static  void    f_set_speed(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    {
    quasi88_cfg_set_wait_rate((int)data);
    }

    if (quasi88_cfg_now_wait_rate() == data)
    {
        CheckMenuRadioItem(g_hMenu, M_SET_SPD_25, M_SET_SPD_400, uItem,
            MF_BYCOMMAND);
    }
}

static  void    f_set_nowait(UINT uItem)
{
    int active;
    UINT res;

    if (menubar_active == FALSE) { return; }

    res = GetMenuState(g_hMenu, uItem, MF_BYCOMMAND);
    active = (res & MFS_CHECKED) ? FALSE : TRUE;    /* 逆にする */
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));

    quasi88_cfg_set_no_wait(active);
}

static  void    f_set_subcpu(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_SUB_SOME, M_SET_SUB_ALL, uItem,
               MF_BYCOMMAND);
    {
    if (cpu_timing != (int)data) {
        cpu_timing = (int)data;
        emu_reset();
        /* 他に再初期化すべきものはないのか？ */
    }
    }
}

static  void    f_set_fdcwait(UINT uItem)
{
    int active;
    UINT res;

    if (menubar_active == FALSE) { return; }

    res = GetMenuState(g_hMenu, uItem, MF_BYCOMMAND);
    active = (res & MFS_CHECKED) ? FALSE : TRUE;    /* 逆にする */
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));

    fdc_wait = active;
}

static  void    f_set_refresh(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_REF_60, M_SET_REF_15, uItem,
               MF_BYCOMMAND);
    {
    quasi88_cfg_set_frameskip_rate((int)data);
    }
}

static  void    f_set_interlace(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_INT_NO, M_SET_INT_YES, uItem,
               MF_BYCOMMAND);
    {
    quasi88_cfg_set_interlace((int)data);
    }
}

static  void    f_set_size(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_SIZ_FULL, M_SET_SIZ_DOUBLE, uItem,
               MF_BYCOMMAND);
    {
    quasi88_cfg_set_size((int)data);
    }

    /*if (data > SCREEN_SIZE_FULL && quasi88_cfg_now_fullscreen()) {
        f_set_fullscreen(M_SET_FULLSCREEN);
    }*/
}

static  void    f_set_fullscreen(UINT uItem)
{
    int active;
    UINT res;

    if (menubar_active == FALSE) { return; }

    res = GetMenuState(g_hMenu, uItem, MF_BYCOMMAND);
    active = (res & MFS_CHECKED) ? FALSE : TRUE;    /* 逆にする */
    CheckMenuItem(g_hMenu, uItem,
        MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));

    quasi88_cfg_set_fullscreen(active);

    /*if (active && quasi88_cfg_now_size() > SCREEN_SIZE_FULL) {
        f_set_size(M_SET_SIZ_FULL, SCREEN_SIZE_FULL);
    }*/
}

static  void    f_set_pcg(UINT uItem)
{
    int active;
    UINT res;

    if (menubar_active == FALSE) { return; }

    res = GetMenuState(g_hMenu, uItem, MF_BYCOMMAND);
    active = (res & MFS_CHECKED) ? FALSE : TRUE;    /* 逆にする */
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));

    use_pcg = active;
}

static  void    f_set_mouse(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_MO_NO, M_SET_MO_JOY, uItem,
               MF_BYCOMMAND);
    {
    mouse_mode = (int)data;
    keyboard_switch();
    }
}

static  void    f_set_cursor(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_CUR_DEF, M_SET_CUR_TEN, uItem,
               MF_BYCOMMAND);
    {
    if ((int)data) {
        cursor_key_mode = 1;
    } else {
        cursor_key_mode = 0;
    }
    keyboard_switch();
    }
}

static  void    f_set_numlock(UINT uItem)
{
    int active;
    UINT res;

    if (menubar_active == FALSE) { return; }

    res = GetMenuState(g_hMenu, uItem, MF_BYCOMMAND);
    active = (res & MFS_CHECKED) ? FALSE : TRUE;    /* 逆にする */
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));

    quasi88_cfg_key_numlock(active);
}

static  void    f_set_romaji(UINT uItem)
{
    int active;
    UINT res;

    if (menubar_active == FALSE) { return; }

    res = GetMenuState(g_hMenu, uItem, MF_BYCOMMAND);
    active = (res & MFS_CHECKED) ? FALSE : TRUE;    /* 逆にする */
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));

    quasi88_cfg_key_romaji(active);
}

#ifdef  USE_SOUND
#ifdef  USE_FMGEN
static  void    f_set_fm(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_FM_MAME, M_SET_FM_FMGEN, uItem,
               MF_BYCOMMAND);
    {
    /* やっかい */
    if (((xmame_cfg_get_use_fmgen())          && ((int)data == FALSE)) ||
        ((xmame_cfg_get_use_fmgen() == FALSE) && ((int)data))) {

        xmame_cfg_set_use_fmgen((int)data);

        menu_sound_restart(TRUE);
        update_misc_record();
    }
    }
}
#endif

static  void    f_set_frq(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_FRQ_48, M_SET_FRQ_11, uItem,
               MF_BYCOMMAND);
    {
    /* やっかい */
    if (xmame_cfg_get_sample_freq() != (int)data) {
        if (8000 <= (int)data && (int)data <= 48000) {
        xmame_cfg_set_sample_freq((int)data);

        menu_sound_restart(TRUE);
        update_misc_record();
        }
    }
    }
}

static  void    f_set_buf(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_SET_BUF_800, M_SET_BUF_100, uItem,
               MF_BYCOMMAND);
    {
    /* やっかい */
    if (g_pcm_bufsize != (int)data) {
        if (10 <= (int)data && (int)data <= 1000) {
        g_pcm_bufsize = (int)data;

        menu_sound_restart(TRUE);
        update_misc_record();
        }
    }
    }
}
#endif

/*----------------------------------------------------------------------
 * Drive メニュー
 *----------------------------------------------------------------------*/

static  void    f_drv_chg(int data)
{
    char filename[QUASI88_MAX_FILENAME];    /* フルパスファイル名 */
    const char *headline;
    int result;

    if (menubar_active == FALSE) { return; }

    switch ((int)data) {
    case DRIVE_1:   headline = "Open Disk-Image-File (Drive 1:)";   break;
    case DRIVE_2:   headline = "Open Disk-Image-File (Drive 2:)";   break;
    default:        headline = "Open Disk-Image-File";      break;
    }   

    result = select_file(TRUE,
             headline,
             "d88(*.d88)\0*.d88\0All files(*.*)\0*.*\0\0",
             "d88",
             OFN_FILEMUSTEXIST, /* 既存ファイル限定 */
             filename,
             sizeof(filename));
    if (result) {
    int ok = FALSE;
    int ro = (result < 0) ? TRUE : FALSE;

    if ((data == DRIVE_1) || (data == DRIVE_2)) {

        ok = quasi88_disk_insert(data, filename, 0, ro);

    } else if (data < 0) {

        ok = quasi88_disk_insert_all(filename, ro);

    }

    /* すでにファイルを閉じているので、失敗してもメニューバー更新 */
    update_drive();
    }
}

static  void    f_drv_drv1(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_DRV_DRV1_1, M_DRV_DRV1_NO, uItem,
               MF_BYCOMMAND);
    {
    if ((int)data <  0) {
        quasi88_disk_image_empty(DRIVE_1);
    } else {
        quasi88_disk_image_select(DRIVE_1, (int)data);
    }
    }
}

static  void    f_drv_drv2(UINT uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    CheckMenuRadioItem(g_hMenu, M_DRV_DRV2_1, M_DRV_DRV2_NO, uItem,
               MF_BYCOMMAND);
    {
    if ((int)data <  0) {
        quasi88_disk_image_empty(DRIVE_2);
    } else {
        quasi88_disk_image_select(DRIVE_2, (int)data);
    }
    }
}

static  void    f_drv_unset(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_disk_eject_all();

    update_drive();
}

/*----------------------------------------------------------------------
 * Misc メニュー
 *----------------------------------------------------------------------*/

static  void    f_misc_capture(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_screen_snapshot();
}

static  void    f_misc_record(UINT uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = xmame_wavout_opened() ? FALSE : TRUE;  /* 逆にする */

    if (active == FALSE) {
    if (xmame_wavout_opened()) {
        quasi88_waveout(FALSE);
    }
    } else {
    if (xmame_wavout_opened() == FALSE) {
        if (quasi88_waveout(TRUE) == FALSE) {
        active = FALSE;
        }
    }
    }

    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));
}

static  void    f_misc_cload_s(void)
{
    char filename[QUASI88_MAX_FILENAME];    /* フルパスファイル名 */
    int result;

    if (menubar_active == FALSE) { return; }

    if (filename_get_tape(CLOAD)) { return; }   /* テープありなら戻る */

    result = select_file(TRUE,
             "Open Tape-Image-File for LOAD",
             "*.t88;*.cmt\0*.t88;*.cmt\0All files(*.*)\0*.*\0\0",
             "cmt",
             OFN_FILEMUSTEXIST |        /* 既存ファイル限定 */
                OFN_HIDEREADONLY,   /* 読取専用は隠す   */
             filename,
             sizeof(filename));

    if (result) {

    int ok = quasi88_load_tape_insert(filename);

    /* すでにファイルを閉じているので、失敗してもメニューバー更新 */
    update_misc_cload();
    }
}

static  void    f_misc_cload_u(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_load_tape_eject();

    update_misc_cload();
}

static  void    f_misc_csave_s(void)
{
    char filename[QUASI88_MAX_FILENAME];    /* フルパスファイル名 */
    int result;

    if (menubar_active == FALSE) { return; }

    if (filename_get_tape(CSAVE)) { return; }   /* テープありなら戻る */

    result = select_file(FALSE,
             "Open Tape-Image-File for SAVE (append)",
             "*.cmt\0*.cmt\0All files(*.*)\0*.*\0\0",
             "cmt",
             OFN_OVERWRITEPROMPT |      /* 上書き時は確認   */
                OFN_HIDEREADONLY,   /* 読取専用は隠す   */
             filename,
             sizeof(filename));

    if (result) {

    int ok = quasi88_save_tape_insert(filename);

    /* すでにファイルを閉じているので、失敗してもメニューバー更新 */
    update_misc_csave();
    }
}

static  void    f_misc_csave_u(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_save_tape_eject();

    update_misc_csave();
}

static  void    f_misc_sload(int data)
{
    if (menubar_active == FALSE) { return; }

    quasi88_stateload((int) data);

    /* 設定やファイル名が変更されたはずなので、メニューバーを全て更新 */
    menubar_setup(TRUE);
}

static  void    f_misc_ssave(int data)
{
    if (menubar_active == FALSE) { return; }

    quasi88_statesave((int) data);
}

static  void    f_misc_status(UINT uItem)
{
    int active;
    UINT res;

    if (menubar_active == FALSE) { return; }

    res = GetMenuState(g_hMenu, uItem, MF_BYCOMMAND);
    active = (res & MFS_CHECKED) ? FALSE : TRUE;    /* 逆にする */
    CheckMenuItem(g_hMenu, uItem,
          MF_BYCOMMAND | (active ? MFS_CHECKED : MFS_UNCHECKED));

    quasi88_cfg_set_showstatus(active);
}

/*----------------------------------------------------------------------
 * Help メニュー
 *----------------------------------------------------------------------*/

static  void    f_help_about (void)
{
    MessageBox(g_hWnd,
           Q_TITLE "  ver. " Q_VERSION "  <" Q_COMMENT ">"
           "\n  " Q_COPYRIGHT
#ifdef  USE_SOUND
           "\n"
           "\nMAME Sound-Engine included"
           "\n   " Q_MAME_COPYRIGHT
#ifdef  USE_FMGEN
           "\nFM Sound Generator (fmgen) included"
           "\n   " Q_FMGEN_COPYRIGHT
#endif
#endif
           ,
           "About " Q_TITLE, MB_OK);
}
