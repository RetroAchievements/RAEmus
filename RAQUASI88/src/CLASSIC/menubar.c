/***********************************************************************
 * メニューバー処理
 ************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "device.h"
#include "event.h"

#include "initval.h"
#include "pc88main.h"		/* boot_basic, ...		*/
#include "memory.h"		/* use_pcg			*/
#include "soundbd.h"		/* sound_board			*/
#include "intr.h"		/* cpu_clock_mhz		*/
#include "keyboard.h"		/* mouse_mode			*/
#include "fdc.h"		/* fdc_wait			*/
#include "getconf.h"		/* config_save			*/
#include "screen.h"		/* SCREEN_INTERLACE_NO ...	*/
#include "emu.h"		/* cpu_timing, emu_reset()	*/
#include "menu.h"		/* menu_sound_restart()		*/
#include "drive.h"
#include "snddrv.h"


/*---------------------------------------------------------------------------*/
/* for About Box */
#define ABOUT_DIALOG		128
#define ABOUT_OK_BUTTON		1
#define ABOUT_TEXT		2

/* for error dialog */
#define ERROR_DIALOG		129
#define ERROR_OK_BUTTON		1
#define ERROR_TEXT		2


/***********************************************************************
 *
 ************************************************************************/
void	mac_error_dialog(const char *message)
{
    Str255 ptext;
    DialogItemType itemType;
    Handle Item;
    Rect box;
    DialogPtr theDialog;
    short theItem;

    ptext[0] = strlen(message);
    memcpy(ptext+1, message, ptext[0]);

    theDialog = GetNewDialog(ERROR_DIALOG, NULL, (WindowRef)-1L);	
    SetDialogDefaultItem(theDialog, ERROR_OK_BUTTON);
    GetDialogItem(theDialog, ERROR_TEXT, &itemType, &Item, &box);
    SetDialogItemText(Item, ptext);
    do {
	ModalDialog(NULL, &theItem);
    } while (theItem != ERROR_OK_BUTTON);
    DisposeDialog(theDialog);
}



/***********************************************************************
 *
 ************************************************************************/
static	int	menubar_active = TRUE;

static	T_RESET_CFG	menubar_reset_cfg;

/*---------------------------------------------------------------------------*/
#define	M_APPLE			128
#define	M_APPLE_ABOUT			1
/*---------------------------------------------------------------------------*/
#define	M_SYSTEM		129

#define	M_SYS_RESET			1

#define	M_SYS_MODE			133
#define	M_SYS_MODE_V2				1
#define	M_SYS_MODE_V1H				2
#define	M_SYS_MODE_V1S				3
#define	M_SYS_MODE_N				4
#define	M_SYS_MODE_4MH				6
#define	M_SYS_MODE_8MH				7
#define	M_SYS_MODE_SB				9
#define	M_SYS_MODE_SB2				10

#define	M_SYS_RESET_V2			4
#define	M_SYS_RESET_V1H			5
#define	M_SYS_RESET_V1S			6

#define	M_SYS_MENU			8

#define	M_SYS_SAVE			10

#define	M_SYS_EXIT			11

/*---------------------------------------------------------------------------*/
#define M_SET			130

#define	M_SET_SPD			134
#define	M_SET_SPD_25				1
#define	M_SET_SPD_50				2
#define	M_SET_SPD_100				3
#define	M_SET_SPD_200				4
#define	M_SET_SPD_400				5
#define	M_SET_SPD_MAX				7

#define	M_SET_SUB			135
#define	M_SET_SUB_SOME				1
#define	M_SET_SUB_OFT				2
#define	M_SET_SUB_ALL				3

#define	M_SET_FDCWAIT			3

#define	M_SET_REF			136
#define	M_SET_REF_60				1
#define	M_SET_REF_30				2
#define	M_SET_REF_20				3
#define	M_SET_REF_15				4

#define	M_SET_INT			137
#define	M_SET_INT_NO				1
#define	M_SET_INT_SKIP				2
#define	M_SET_INT_YES				3

#define	M_SET_SIZ			138
#define	M_SET_SIZ_FULL				1
#define	M_SET_SIZ_HALF				2

#define	M_SET_FUL			8
#define	M_SET_PCG			9

#define	M_SET_MO			139
#define	M_SET_MO_NO				1
#define	M_SET_MO_MOUSE				2
#define	M_SET_MO_JOYMO				3
#define	M_SET_MO_JOY				4

#define	M_SET_CUR			140
#define	M_SET_CUR_DEF				1
#define	M_SET_CUR_TEN				2

#define	M_SET_NUMLOCK			13

#define	M_SET_ROMAJI			14

#define	M_SET_FM			141
#define	M_SET_FM_MAME				1
#define	M_SET_FM_FMGEN				2

#define	M_SET_FRQ			142
#define	M_SET_FRQ_44				1
#define	M_SET_FRQ_22				2
#define	M_SET_FRQ_11				3
#define	M_SET_FRQ_08				4

/*---------------------------------------------------------------------------*/
#define M_DRV			131

#define	M_DRV_DRV1			143
#define	M_DRV_DRV1_1				1
#define	M_DRV_DRV1_2				2
#define	M_DRV_DRV1_3				3
#define	M_DRV_DRV1_4				4
#define	M_DRV_DRV1_5				5
#define	M_DRV_DRV1_6				6
#define	M_DRV_DRV1_7				7
#define	M_DRV_DRV1_8				8
#define	M_DRV_DRV1_9				9
#define	M_DRV_DRV1_NO				10
#define	M_DRV_DRV1_CHG				12

#define	M_DRV_DRV2			144
#define	M_DRV_DRV2_1				1
#define	M_DRV_DRV2_2				2
#define	M_DRV_DRV2_3				3
#define	M_DRV_DRV2_4				4
#define	M_DRV_DRV2_5				5
#define	M_DRV_DRV2_6				6
#define	M_DRV_DRV2_7				7
#define	M_DRV_DRV2_8				8
#define	M_DRV_DRV2_9				9
#define	M_DRV_DRV2_NO				10
#define	M_DRV_DRV2_CHG				12

#define	M_DRV_CHG			4
#define	M_DRV_UNSET			5

/*---------------------------------------------------------------------------*/
#define M_MISC			132

#define	M_MISC_CAPTURE			1

#define	M_MISC_RECORD			2

#define	M_MISC_CLOAD			145
#define	M_MISC_CLOAD_S				1
#define	M_MISC_CLOAD_U				2

#define	M_MISC_CSAVE			146
#define	M_MISC_CSAVE_S				1
#define	M_MISC_CSAVE_U				2

#define	M_MISC_SLOAD			147
#define	M_MISC_SLOAD_1				1
#define	M_MISC_SLOAD_2				2
#define	M_MISC_SLOAD_3				3
#define	M_MISC_SLOAD_4				4
#define	M_MISC_SLOAD_5				5
#define	M_MISC_SLOAD_6				6
#define	M_MISC_SLOAD_7				7
#define	M_MISC_SLOAD_8				8
#define	M_MISC_SLOAD_9				9

#define	M_MISC_SSAVE			148
#define	M_MISC_SSAVE_1				1
#define	M_MISC_SSAVE_2				2
#define	M_MISC_SSAVE_3				3
#define	M_MISC_SSAVE_4				4
#define	M_MISC_SSAVE_5				5
#define	M_MISC_SSAVE_6				6
#define	M_MISC_SSAVE_7				7
#define	M_MISC_SSAVE_8				8
#define	M_MISC_SSAVE_9				9

#define	M_MISC_STATUS			9

/***********************************************************************
 * メニューバー生成
 ************************************************************************/
Boolean mac_create_menubar(void)
{
    static const int mtable[] = {
	M_SYS_MODE, 
	M_SET_SPD,
	M_SET_SUB,
	M_SET_REF,
	M_SET_INT,
	M_SET_SIZ,
	M_SET_MO,
	M_SET_CUR,
	M_SET_FM,
	M_SET_FRQ,
	M_DRV_DRV1,
	M_DRV_DRV2,
	M_MISC_CLOAD,
	M_MISC_CSAVE,
	M_MISC_SLOAD,
	M_MISC_SSAVE,
    };
    int i;

    Handle menuBar;
    MenuHandle mh;

    menuBar = GetNewMBar(128);
    if (! menuBar) return false;
    SetMenuBar(menuBar);
    DisposeHandle(menuBar);
    AppendResMenu(GetMenuHandle(128), 'DRVR');

    for (i=0; i<COUNTOF(mtable); i++) {
	mh = GetMenu(mtable[i]);
	InsertMenu(mh, -1);
    }

    DrawMenuBar();

    return true;
}


/****************************************************************************
 * モード切り替え時の、メニューバーの処理再設定
 *	エミュモードとメニューモードで、メニューバーの内容を変更する
 *****************************************************************************/
static void menubar_item_setup(void);
static void menubar_item_sensitive(int sensitive);

void	menubar_setup(int active)
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






/* ラジオメニューの一つをチェックする */
static void check_radio_item(int subMenu, int start, int end, int uItem)
{
    int i;

    for (i = start; i <= end; i++) {
	CheckItem(GetMenuRef(subMenu), i, (i == uItem));
    }
}


/* メニューアイテムの文字列を変更 */
static void change_menuitem_label(int subMenu, int uItem, char *s)
{
    unsigned char str[256];
    size_t len;

    len = strlen(s);
    if (len > 255) len = 255;

    memcpy((char *) &str[1], s, len);
    str[0] = len;

    SetMenuItemText(GetMenuRef(subMenu), uItem, str);
}


/* ファイルを開く ダイアログ */
static Boolean openGetFile(FSSpec *file)
{
    StandardFileReply reply;

    StandardGetFile(NULL, -1, NULL, &reply); 
    if (reply.sfGood && !reply.sfIsFolder) {
	*file = reply.sfFile;
	return true;
    } else {
	return false;
    }
}


/* ファイルを保存ダイアログ */
static Boolean openPutFile(FSSpec *file)
{
    StandardFileReply reply;

    StandardPutFile((const unsigned char *)"",
		    (const unsigned char *)"", &reply); 
    if (reply.sfGood && !reply.sfIsFolder) {
	*file = reply.sfFile;
	return true;
    } else {
	return false;
    }
}


/* ダイアログで選択したファイル名を取得 */
static char *get_filename(FSSpec *file)
{
    OSErr  err;
    FSSpec spec;
    UInt8  nullstr[2] = { 0, 0 };
    char   tmp[257];

    static char filename[257];

    if (file == NULL) return NULL;


    spec = *file;

    memcpy(filename, &spec.name[1], spec.name[0]);
    filename[ spec.name[0] ] = '\0';

    do {
	err = FSMakeFSSpec(spec.vRefNum, spec.parID, nullstr, &spec);
	if (noErr == err) {
	    strcpy(tmp, filename);
	    memcpy(filename, &spec.name[1], spec.name[0]);
	    filename[ spec.name[0] ] = '\0';
	    strcat(filename, ":");
	    strcat(filename, tmp);
	}
    } while (spec.parID != 1 && noErr == err);

    if (err == noErr) return filename;
    else              return NULL;
}






/* Reset メニューアイテムのラベルを更新する */
static void update_sys_reset(void)
{
    char buf[32];

    strcpy(buf, "Reset   [");

    switch (menubar_reset_cfg.boot_basic) {
    case BASIC_V2:		strcat(buf, "V2");		break;
    case BASIC_V1H:		strcat(buf, "V1H");		break;
    case BASIC_V1S:		strcat(buf, "V1S");		break;
    case BASIC_N:		strcat(buf, "N");		break;
    }

    strcat(buf, " : ");

    switch (menubar_reset_cfg.boot_clock_4mhz) {
    case CLOCK_4MHZ:		strcat(buf, "4MHz");		break;
    case CLOCK_8MHZ:		strcat(buf, "8MHz");		break;
    }

    strcat(buf, "]");

    change_menuitem_label(M_SYSTEM, M_SYS_RESET, buf);
}



/* Drive メニューアイテムを表示する・隠す */
static void update_drive(void)
{
    int uItem;
    char buf[64];
    int i;
    int drv, base;
    int has_image = FALSE;

    for (drv = 0; drv < NR_DRIVE; drv ++) {
	base = (drv == DRIVE_1) ? M_DRV_DRV1 : M_DRV_DRV2;

	if (disk_image_exist(drv)) {

	    /* イメージの数の分、メニューアイテムを表示する。
	       ラベルは、イメージ名にセットし直す。           */
	    for (i = 0; i < MIN(disk_image_num(drv), 9); i++) {
		uItem = i + 1;

		sprintf(buf, "%d  ", i + 1);
		my_strncat(buf, drive[drv].image[i].name, sizeof(buf));

		change_menuitem_label(base, uItem, buf);
		EnableItem(GetMenuRef(base), uItem);
	    }
	    for (   ; i<9; i++) {
		uItem = i + 1;
		sprintf(buf, "%d  nonexistant", i + 1);
		change_menuitem_label(base, uItem, buf);
		DisableItem(GetMenuRef(base), uItem);
	    }
	    has_image = TRUE;

	} else {

	    /* ディスクがない場合 … 本当は消したいんだけど、どうするの? */
	    for (i=0; i<9; i++) {
		uItem = i + 1;
		sprintf(buf, "%d  nonexistant", i + 1);
		change_menuitem_label(base, uItem, buf);
		DisableItem(GetMenuRef(base), uItem);
	    }
	}

	/* 選択中イメージの、ラジオメニューをアクティブにする */

	if (disk_image_exist(drv) == FALSE ||	/* ファイルなし or */
	    drive_check_empty(drv)) {		/* 空を選択        */

	    uItem = 10;					/*    → NO Disk */

	} else {
	    i = disk_image_selected(drv);
	    if (0 <= i && i <= 9) {			/* 1〜9番目選択 */
		uItem = i + 1;				/*    → それだ */
	    } else {					/* 10番目〜     */
		uItem = 0;				/*    → なし   */
	    }
	}
	check_radio_item(base, 1, 10, uItem);
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
	change_menuitem_label(M_DRV, drv + 1, buf);
    }

    if (has_image) {
	change_menuitem_label(M_DRV, M_DRV_CHG, "Change ...");
	EnableItem(GetMenuRef(M_DRV), M_DRV_UNSET);
    } else {
	change_menuitem_label(M_DRV, M_DRV_CHG, "Set ...");
	DisableItem(GetMenuRef(M_DRV), M_DRV_UNSET);
    }
}

/* Tape Load メニューアイテムのラベルを変えたり使用不可にしたり */
static void update_misc_cload(void)
{
    int uItem;
    const char *s;
    char buf[64];

    s = filename_get_tape_name(CLOAD);

    /* テープありならファイル名を、なしならデフォルトのラベルを表示 */
    uItem = M_MISC_CLOAD_S;
    {
	if (s) { my_strncpy(buf, s, sizeof(buf)); }
	else   { strcpy(buf, "Set ...");          }
	change_menuitem_label(M_MISC_CLOAD, uItem, buf);
    }

    /* テープありなら、ラジオメニューをアクティブに */
    uItem = M_MISC_CLOAD_S;
    CheckItem(GetMenuRef(M_MISC_CLOAD), uItem, (s) ? true : false);

    /* テープありなら unset を表示、なしなら隠す */
    uItem = M_MISC_CLOAD_U;
    if (s) { EnableItem(GetMenuRef(M_MISC_CLOAD), uItem); }
    else   { DisableItem(GetMenuRef(M_MISC_CLOAD), uItem); }
}

/* Tape Save メニューアイテムのラベルを変えたり使用不可にしたり */
static void update_misc_csave(void)
{
    int uItem;
    const char *s;
    char buf[64];

    s = filename_get_tape_name(CSAVE);

    /* テープありならファイル名を、なしならデフォルトのラベルを表示 */
    uItem = M_MISC_CSAVE_S;
    {
	if (s) { my_strncpy(buf, s, sizeof(buf)); }
	else   { strcpy(buf, "Set ...");          }
	change_menuitem_label(M_MISC_CSAVE, uItem, buf);
    }

    /* テープありなら、ラジオメニューをアクティブに */
    uItem = M_MISC_CSAVE_S;
    CheckItem(GetMenuRef(M_MISC_CSAVE), uItem, (s) ? true : false);

    /* テープありなら unset を表示、なしなら隠す */
    uItem = M_MISC_CSAVE_U;
    if (s) { EnableItem(GetMenuRef(M_MISC_CSAVE), uItem); }
    else   { DisableItem(GetMenuRef(M_MISC_CSAVE), uItem); }
}

/* Sound Record メニューアイテムのチェックを変更する */
static void update_misc_record(void)
{
    int uItem;
    int i;

    i = xmame_wavout_opened();
    uItem = M_MISC_RECORD;
    CheckItem(GetMenuRef(M_MISC), uItem, (i ? true : false));
}




/*======================================================================
 * メニューバーの内容を再初期化
 *======================================================================*/
static void menubar_item_setup(void)
{
    int uItem;
    int i;

    /* System -----------------------------------------------------------*/

    quasi88_get_reset_cfg(&menubar_reset_cfg);

    switch (menubar_reset_cfg.boot_basic) {
    case BASIC_V2:	uItem = M_SYS_MODE_V2;		break;
    case BASIC_V1H:	uItem = M_SYS_MODE_V1H;		break;
    case BASIC_V1S:	uItem = M_SYS_MODE_V1S;		break;
    case BASIC_N:	uItem = M_SYS_MODE_N;		break;
    }
    check_radio_item(M_SYS_MODE, M_SYS_MODE_V2, M_SYS_MODE_N, uItem);

    switch (menubar_reset_cfg.boot_clock_4mhz) {
    case CLOCK_4MHZ:	uItem = M_SYS_MODE_4MH;		break;
    case CLOCK_8MHZ:	uItem = M_SYS_MODE_8MH;		break;
    }
    check_radio_item(M_SYS_MODE, M_SYS_MODE_4MH, M_SYS_MODE_8MH, uItem);

    switch (menubar_reset_cfg.sound_board) {
    case SOUND_I:	uItem = M_SYS_MODE_SB;		break;
    case SOUND_II:	uItem = M_SYS_MODE_SB2;		break;
    }
    check_radio_item(M_SYS_MODE, M_SYS_MODE_SB, M_SYS_MODE_SB2, uItem);

    update_sys_reset();

    /* Setting ----------------------------------------------------------*/

    i = quasi88_cfg_now_wait_rate();				/* ＊＊＊＊ */
    switch (i) {
    case 25:		uItem = M_SET_SPD_25;		break;
    case 50:		uItem = M_SET_SPD_50;		break;
    case 100:		uItem = M_SET_SPD_100;		break;
    case 200:		uItem = M_SET_SPD_200;		break;
    case 400:		uItem = M_SET_SPD_400;		break;
    default:		uItem = 0;			break;
    }
    check_radio_item(M_SET_SPD, M_SET_SPD_25, M_SET_SPD_400, uItem);

    i = quasi88_cfg_now_no_wait();				/* ＊＊＊＊ */
    uItem = M_SET_SPD_MAX;
    CheckItem(GetMenuRef(M_SET_SPD), uItem, (i ? true : false));

    i = cpu_timing;						/* ＊＊＊＊ */
    switch (i) {
    case 0:		uItem = M_SET_SUB_SOME;		break;
    case 1:		uItem = M_SET_SUB_OFT;		break;
    case 2:		uItem = M_SET_SUB_ALL;		break;
    default:		uItem = 0;			break;
    }
    check_radio_item(M_SET_SUB, M_SET_SUB_SOME, M_SET_SUB_ALL, uItem);

    i = fdc_wait;						/* ＊＊＊＊ */
    uItem = M_SET_FDCWAIT;
    CheckItem(GetMenuRef(M_SET), uItem, (i ? true : false));

    i = quasi88_cfg_now_frameskip_rate();			/* ＊＊＊＊ */
    switch (i) {
    case 1:		uItem = M_SET_REF_60;		break;
    case 2:		uItem = M_SET_REF_30;		break;
    case 3:		uItem = M_SET_REF_20;		break;
    case 4:		uItem = M_SET_REF_15;		break;
    default:		uItem = 0;			break;
    }
    check_radio_item(M_SET_REF, M_SET_REF_60, M_SET_REF_15, uItem);

    i = quasi88_cfg_now_interlace();				/* ＊＊＊＊ */
    switch (i) {
    case SCREEN_INTERLACE_NO:	uItem = M_SET_INT_NO;	break;
    case SCREEN_INTERLACE_SKIP:	uItem = M_SET_INT_SKIP;	break;
    case SCREEN_INTERLACE_YES:	uItem = M_SET_INT_YES;	break;
    default:			uItem = 0;		break;
    }
    check_radio_item(M_SET_INT, M_SET_INT_NO, M_SET_INT_YES, uItem);

    i = quasi88_cfg_now_size();					/* ＊＊＊＊ */
    switch (i) {
    case SCREEN_SIZE_FULL:	uItem = M_SET_SIZ_FULL;	break;
    case SCREEN_SIZE_HALF:	uItem = M_SET_SIZ_HALF;	break;
    default:			uItem = 0;		break;
    }
    check_radio_item(M_SET_SIZ, M_SET_SIZ_FULL, M_SET_SIZ_HALF, uItem);

    i = quasi88_cfg_now_fullscreen();				/* ＊＊＊＊ */
    uItem = M_SET_FUL;
    CheckItem(GetMenuRef(M_SET), uItem, (i ? true : false));

    i = use_pcg;						/* ＊＊＊＊ */
    uItem = M_SET_PCG;
    CheckItem(GetMenuRef(M_SET), uItem, (i ? true : false));

    i = mouse_mode;						/* ＊＊＊＊ */
    switch (i) {
    case MOUSE_NONE:		uItem = M_SET_MO_NO;	break;
    case MOUSE_MOUSE:		uItem = M_SET_MO_MOUSE;	break;
    case MOUSE_JOYMOUSE:	uItem = M_SET_MO_JOYMO;	break;
    case MOUSE_JOYSTICK:	uItem = M_SET_MO_JOY;	break;
    default:			uItem = 0;		break;
    }
    check_radio_item(M_SET_MO, M_SET_MO_NO, M_SET_MO_JOY, uItem);

    i = cursor_key_mode;					/* ＊＊＊＊ */
    switch (i) {
    case 0:		uItem = M_SET_CUR_DEF;		break;
    case 1:		uItem = M_SET_CUR_TEN;		break;
    default:		uItem = 0;			break;
    }
    check_radio_item(M_SET_CUR, M_SET_CUR_DEF, M_SET_CUR_TEN, uItem);

    i = numlock_emu;						/* ＊＊＊＊ */
    uItem = M_SET_NUMLOCK;
    CheckItem(GetMenuRef(M_SET), uItem, (i ? true : false));

    i = romaji_input_mode;					/* ＊＊＊＊ */
    uItem = M_SET_ROMAJI;
    CheckItem(GetMenuRef(M_SET), uItem, (i ? true : false));

#ifdef	USE_SOUND
    if (xmame_has_sound()) {
#ifdef	USE_FMGEN
	i = xmame_cfg_get_use_fmgen();				/* ＊＊＊＊ */
	switch (i) {
	case 0:		uItem = M_SET_FM_MAME;		break;
	case 1:		uItem = M_SET_FM_FMGEN;		break;
	default:	uItem = 0;			break;
	}
	check_radio_item(M_SET_FM, M_SET_FM_MAME, M_SET_FM_FMGEN, uItem);
#else
	DisableItem(GetMenuRef(M_SET), 16);
#endif

	i = xmame_cfg_get_sample_freq();			/* ＊＊＊＊ */
	switch (i) {
	case 44100:	uItem = M_SET_FRQ_44;		break;
	case 22050:	uItem = M_SET_FRQ_22;		break;
	case 11025:	uItem = M_SET_FRQ_11;		break;
	case  8000:	uItem = M_SET_FRQ_08;		break;
	default:	uItem = 0;			break;
	}
	check_radio_item(M_SET_FRQ, M_SET_FRQ_44, M_SET_FRQ_08, uItem);

    } else
#endif
    {
	DisableItem(GetMenuRef(M_SET), 16);
	DisableItem(GetMenuRef(M_SET), 17);
    }

    /* Drive ------------------------------------------------------------*/

    update_drive();

    /* Misc -------------------------------------------------------------*/

    if (xmame_has_sound()) {
	i = xmame_wavout_opened();
	uItem = M_MISC_RECORD;
	CheckItem(GetMenuRef(M_MISC), uItem, (i ? true : false));
    } else {
	uItem = M_MISC_RECORD;
	CheckItem(GetMenuRef(M_MISC), uItem, false);
	DisableItem(GetMenuRef(M_MISC), uItem);
    }

    update_misc_cload();

    update_misc_csave();

    i = quasi88_cfg_now_showstatus();
    uItem = M_MISC_STATUS;
    CheckItem(GetMenuRef(M_MISC), uItem, (i ? true : false));
}

/*======================================================================
 * メニューバー使用可能項目を設定
 *======================================================================*/
static void menubar_item_sensitive(int sensitive)
{
    if (sensitive) {

	EnableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET);
	EnableItem(GetMenuRef(M_SYSTEM), 2);
	EnableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET_V2);
	EnableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET_V1H);
	EnableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET_V1S);
	EnableItem(GetMenuRef(M_SYSTEM), M_SYS_MENU);
	EnableItem(GetMenuRef(M_SYSTEM), M_SYS_SAVE);

	EnableItem(GetMenuRef(M_SET), 0);
	EnableItem(GetMenuRef(M_DRV), 0);
	EnableItem(GetMenuRef(M_MISC), 0);

    } else {

	DisableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET);
	DisableItem(GetMenuRef(M_SYSTEM), 2);
	DisableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET_V2);
	DisableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET_V1H);
	DisableItem(GetMenuRef(M_SYSTEM), M_SYS_RESET_V1S);
	DisableItem(GetMenuRef(M_SYSTEM), M_SYS_MENU);
	DisableItem(GetMenuRef(M_SYSTEM), M_SYS_SAVE);

	DisableItem(GetMenuRef(M_SET), 0);
	DisableItem(GetMenuRef(M_DRV), 0);
	DisableItem(GetMenuRef(M_MISC), 0);
    }
}


/***********************************************************************
 * メニューバーコールバック関数
 ************************************************************************/

static	void	aboutBox(void);

static	void	f_sys_reset	(void);
static	void	f_sys_basic	(int uItem, int data);
static	void	f_sys_clock	(int uItem, int data);
static	void	f_sys_sb	(int uItem, int data);
static	void	f_sys_menu	(void);
static	void	f_sys_save	(void);
static	void	f_sys_exit	(void);
static	void	f_set_speed	(int uItem, int data);
static	void	f_set_nowait	(int uItem);
static	void	f_set_subcpu	(int uItem, int data);
static	void	f_set_fdcwait	(int uItem);
static	void	f_set_refresh	(int uItem, int data);
static	void	f_set_interlace	(int uItem, int data);
static	void	f_set_size	(int uItem, int data);
static	void	f_set_full	(int uItem);
static	void	f_set_pcg	(int uItem);
static	void	f_set_mouse	(int uItem, int data);
static	void	f_set_cursor	(int uItem, int data);
static	void	f_set_numlock	(int uItem);
static	void	f_set_romaji	(int uItem);
static	void	f_set_fm	(int uItem, int data);
static	void	f_set_frq	(int uItem, int data);
static	void	f_set_buf	(int uItem, int data);
static	void	f_drv_chg	(int data);
static	void	f_drv_drv1	(int uItem, int data);
static	void	f_drv_drv2	(int uItem, int data);
static	void	f_drv_unset	(void);
static	void	f_misc_capture	(void);
static	void	f_misc_record	(int uItem);
static	void	f_misc_cload_s	(void);
static	void	f_misc_cload_u	(void);
static	void	f_misc_csave_s	(void);
static	void	f_misc_csave_u	(void);
static	void	f_misc_sload	(int data);
static	void	f_misc_ssave	(int data);
static	void	f_misc_status	(int uItem);



void doMenuCommand(long	menuResult)
{
    short menuID;
    short menuItem;

    menuID = HiWord(menuResult);
    menuItem = LoWord(menuResult);

    switch (menuID) {

    case M_APPLE:
	switch (menuItem) {
	case M_APPLE_ABOUT:
	    aboutBox();
	    break;
	default:
	    {
		Str255 deskName;
		GetMenuItemText(GetMenuHandle(128),menuItem,deskName);
		OpenDeskAcc(deskName);
	    }
	    break;
	}
	break;

    case M_SYSTEM:
	switch (menuItem) {
	case M_SYS_RESET:	f_sys_reset();			break;
	case M_SYS_MENU:	f_sys_menu();			break;
	case M_SYS_SAVE:	f_sys_save();			break;
	case M_SYS_EXIT:	f_sys_exit();			break;

	case M_SYS_RESET_V2:	f_sys_basic(M_SYS_MODE_V2,  BASIC_V2);
				f_sys_reset();				break;
	case M_SYS_RESET_V1H:	f_sys_basic(M_SYS_MODE_V1H, BASIC_V1H);
				f_sys_reset();				break;
	case M_SYS_RESET_V1S:	f_sys_basic(M_SYS_MODE_V1S, BASIC_V1S);
				f_sys_reset();				break;
	}
	break;

    case M_SYS_MODE:
	switch (menuItem) {
	case M_SYS_MODE_V2:	f_sys_basic(menuItem, BASIC_V2);	break;
	case M_SYS_MODE_V1H:	f_sys_basic(menuItem, BASIC_V1H);	break;
	case M_SYS_MODE_V1S:	f_sys_basic(menuItem, BASIC_V1S);	break;
	case M_SYS_MODE_N:	f_sys_basic(menuItem, BASIC_N);		break;

	case M_SYS_MODE_4MH:	f_sys_clock(menuItem, CLOCK_4MHZ);	break;
	case M_SYS_MODE_8MH:	f_sys_clock(menuItem, CLOCK_8MHZ);	break;

	case M_SYS_MODE_SB:	f_sys_sb(menuItem, SOUND_I);		break;
	case M_SYS_MODE_SB2:	f_sys_sb(menuItem, SOUND_II);		break;
	}
	break;

    case M_SET:
	switch (menuItem) {
	case M_SET_FDCWAIT:	f_set_fdcwait(menuItem);	break;
	case M_SET_FUL:		f_set_full(menuItem);		break;
	case M_SET_PCG:		f_set_pcg(menuItem);		break;
	case M_SET_NUMLOCK:	f_set_numlock(menuItem);	break;
	case M_SET_ROMAJI:	f_set_romaji(menuItem);		break;
	}
	break;

    case M_SET_SPD:
	switch (menuItem) {
	case M_SET_SPD_25:	f_set_speed(menuItem, 25);	break;
	case M_SET_SPD_50:	f_set_speed(menuItem, 50);	break;
	case M_SET_SPD_100:	f_set_speed(menuItem, 100);	break;
	case M_SET_SPD_200:	f_set_speed(menuItem, 200);	break;
	case M_SET_SPD_400:	f_set_speed(menuItem, 400);	break;

	case M_SET_SPD_MAX:	f_set_nowait(menuItem);		break;
	}
	break;

    case M_SET_SUB:
	switch (menuItem) {
	case M_SET_SUB_SOME:	f_set_subcpu(menuItem, 0);	break;
	case M_SET_SUB_OFT:	f_set_subcpu(menuItem, 1);	break;
	case M_SET_SUB_ALL:	f_set_subcpu(menuItem, 2);	break;
	}
	break;

    case M_SET_REF:
	switch (menuItem) {
	case M_SET_REF_60:	f_set_refresh(menuItem, 1);	break;
	case M_SET_REF_30:	f_set_refresh(menuItem, 2);	break;
	case M_SET_REF_20:	f_set_refresh(menuItem, 3);	break;
	case M_SET_REF_15:	f_set_refresh(menuItem, 4);	break;
	}
	break;

    case M_SET_INT:
	switch (menuItem) {
	case M_SET_INT_NO:   f_set_interlace(menuItem, SCREEN_INTERLACE_NO);
	    break;
	case M_SET_INT_SKIP: f_set_interlace(menuItem, SCREEN_INTERLACE_SKIP);
	    break;
	case M_SET_INT_YES:  f_set_interlace(menuItem, SCREEN_INTERLACE_YES);
	    break;
	}
	break;

    case M_SET_SIZ:
	switch (menuItem) {
	case M_SET_SIZ_FULL:	f_set_size(menuItem, SCREEN_SIZE_FULL);	break;
	case M_SET_SIZ_HALF:	f_set_size(menuItem, SCREEN_SIZE_HALF);	break;
	}
	break;

    case M_SET_MO:
	switch (menuItem) {
	case M_SET_MO_NO:	f_set_mouse(menuItem, MOUSE_NONE);	break;
	case M_SET_MO_MOUSE:	f_set_mouse(menuItem, MOUSE_MOUSE);	break;
	case M_SET_MO_JOYMO:	f_set_mouse(menuItem, MOUSE_JOYMOUSE);	break;
	case M_SET_MO_JOY:	f_set_mouse(menuItem, MOUSE_JOYSTICK);	break;
	}
	break;

    case M_SET_CUR:
	switch (menuItem) {
	case M_SET_CUR_DEF:	f_set_cursor(menuItem, 0);	break;
	case M_SET_CUR_TEN:	f_set_cursor(menuItem, 1);	break;
	}
	break;

    case M_SET_FM:
	switch (menuItem) {
	case M_SET_FM_MAME:	f_set_fm(menuItem, 0);		break;
	case M_SET_FM_FMGEN:	f_set_fm(menuItem, 1);		break;
	}
	break;

    case M_SET_FRQ:
	switch (menuItem) {
	case M_SET_FRQ_44:	f_set_frq(menuItem, 44100);	break;
	case M_SET_FRQ_22:	f_set_frq(menuItem, 22050);	break;
	case M_SET_FRQ_11:	f_set_frq(menuItem, 11025);	break;
	case M_SET_FRQ_08:	f_set_frq(menuItem,  8000);	break;
	}
	break;

    case M_DRV:
	switch (menuItem) {
	case M_DRV_CHG:		f_drv_chg(-1);			break;
	case M_DRV_UNSET:	f_drv_unset();			break;
	}
	break;

    case M_DRV_DRV1:
	switch (menuItem) {
	case M_DRV_DRV1_1:	f_drv_drv1(menuItem, 0);	break;
	case M_DRV_DRV1_2:	f_drv_drv1(menuItem, 1);	break;
	case M_DRV_DRV1_3:	f_drv_drv1(menuItem, 2);	break;
	case M_DRV_DRV1_4:	f_drv_drv1(menuItem, 3);	break;
	case M_DRV_DRV1_5:	f_drv_drv1(menuItem, 4);	break;
	case M_DRV_DRV1_6:	f_drv_drv1(menuItem, 5);	break;
	case M_DRV_DRV1_7:	f_drv_drv1(menuItem, 6);	break;
	case M_DRV_DRV1_8:	f_drv_drv1(menuItem, 7);	break;
	case M_DRV_DRV1_9:	f_drv_drv1(menuItem, 8);	break;
	case M_DRV_DRV1_NO:	f_drv_drv1(menuItem, -1);	break;
	case M_DRV_DRV1_CHG:	f_drv_chg(DRIVE_1);		break;
	}
	break;

    case M_DRV_DRV2:
	switch (menuItem) {
	case M_DRV_DRV2_1:	f_drv_drv2(menuItem, 0);	break;
	case M_DRV_DRV2_2:	f_drv_drv2(menuItem, 1);	break;
	case M_DRV_DRV2_3:	f_drv_drv2(menuItem, 2);	break;
	case M_DRV_DRV2_4:	f_drv_drv2(menuItem, 3);	break;
	case M_DRV_DRV2_5:	f_drv_drv2(menuItem, 4);	break;
	case M_DRV_DRV2_6:	f_drv_drv2(menuItem, 5);	break;
	case M_DRV_DRV2_7:	f_drv_drv2(menuItem, 6);	break;
	case M_DRV_DRV2_8:	f_drv_drv2(menuItem, 7);	break;
	case M_DRV_DRV2_9:	f_drv_drv2(menuItem, 8);	break;
	case M_DRV_DRV2_NO:	f_drv_drv2(menuItem, -1);	break;
	case M_DRV_DRV2_CHG:	f_drv_chg(DRIVE_2);		break;
	}
	break;

    case M_MISC:
	switch (menuItem) {
	case M_MISC_CAPTURE:	f_misc_capture();		break;
	case M_MISC_RECORD:	f_misc_record(menuItem);	break;
	case M_MISC_STATUS:	f_misc_status(menuItem);	break;
	}
	break;

    case M_MISC_CLOAD:
	switch (menuItem) {
	case M_MISC_CLOAD_S:	f_misc_cload_s();		break;
	case M_MISC_CLOAD_U:	f_misc_cload_u();		break;
	}
	break;

    case M_MISC_CSAVE:
	switch (menuItem) {
	case M_MISC_CSAVE_S:	f_misc_csave_s();		break;
	case M_MISC_CSAVE_U:	f_misc_csave_u();		break;
	}
	break;

    case M_MISC_SLOAD:
	switch (menuItem) {
	case M_MISC_SLOAD_1:	f_misc_sload('1');		break;
	case M_MISC_SLOAD_2:	f_misc_sload('2');		break;
	case M_MISC_SLOAD_3:	f_misc_sload('3');		break;
	case M_MISC_SLOAD_4:	f_misc_sload('4');		break;
	case M_MISC_SLOAD_5:	f_misc_sload('5');		break;
	case M_MISC_SLOAD_6:	f_misc_sload('6');		break;
	case M_MISC_SLOAD_7:	f_misc_sload('7');		break;
	case M_MISC_SLOAD_8:	f_misc_sload('8');		break;
	case M_MISC_SLOAD_9:	f_misc_sload('9');		break;
	}
	break;

    case M_MISC_SSAVE:
	switch (menuItem) {
	case M_MISC_SSAVE_1:	f_misc_ssave('1');		break;
	case M_MISC_SSAVE_2:	f_misc_ssave('2');		break;
	case M_MISC_SSAVE_3:	f_misc_ssave('3');		break;
	case M_MISC_SSAVE_4:	f_misc_ssave('4');		break;
	case M_MISC_SSAVE_5:	f_misc_ssave('5');		break;
	case M_MISC_SSAVE_6:	f_misc_ssave('6');		break;
	case M_MISC_SSAVE_7:	f_misc_ssave('7');		break;
	case M_MISC_SSAVE_8:	f_misc_ssave('8');		break;
	case M_MISC_SSAVE_9:	f_misc_ssave('9');		break;
	}
	break;

    }
    HiliteMenu(0);
}

/*======================================================================
 * メニューバーサブ関数
 *======================================================================*/

/*----------------------------------------------------------------------
 * System メニュー
 *----------------------------------------------------------------------*/

static	void	f_sys_reset(void)
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

static	void	f_sys_basic(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SYS_MODE, M_SYS_MODE_V2, M_SYS_MODE_N, uItem);
    {
	menubar_reset_cfg.boot_basic = (int)data;
	update_sys_reset();
    }
}

static	void	f_sys_clock(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SYS_MODE, M_SYS_MODE_4MH, M_SYS_MODE_8MH, uItem);
    {
	menubar_reset_cfg.boot_clock_4mhz = (int)data;
	update_sys_reset();
    }
}

static	void	f_sys_sb(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SYS_MODE, M_SYS_MODE_SB, M_SYS_MODE_SB2, uItem);
    {
	menubar_reset_cfg.sound_board = (int)data;
	update_sys_reset();
    }
}

static	void	f_sys_menu(void)
{
    quasi88_menu();
}

static	void	f_sys_save(void)
{
    config_save(NULL);
}

static	void	f_sys_exit(void)
{
    quasi88_quit();
}

/*----------------------------------------------------------------------
 * Setting メニュー
 *----------------------------------------------------------------------*/

static	void	f_set_speed(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_SPD, M_SET_SPD_25, M_SET_SPD_400, uItem);
    {
	quasi88_cfg_set_wait_rate((int)data);
    }
}

static	void	f_set_nowait(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = quasi88_cfg_now_no_wait() ? FALSE : TRUE; 	/* 逆にする */
    CheckItem(GetMenuRef(M_SET_SPD), uItem, (active ? true : false));

    quasi88_cfg_set_no_wait(active);
}

static	void	f_set_subcpu(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_SUB, M_SET_SUB_SOME, M_SET_SUB_ALL, uItem);
    {
	if (cpu_timing != (int)data) {
	    cpu_timing = (int)data;
	    emu_reset();
	    /* 他に再初期化すべきものはないのか？ */
	}
    }
}

static	void	f_set_fdcwait(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = fdc_wait ? FALSE : TRUE; 			/* 逆にする */
    CheckItem(GetMenuRef(M_SET), uItem, (active ? true : false));

    fdc_wait = active;
}

static	void	f_set_refresh(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_REF, M_SET_REF_60, M_SET_REF_15, uItem);
    {
	quasi88_cfg_set_frameskip_rate((int)data);
    }
}

static	void	f_set_interlace(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_INT, M_SET_INT_NO, M_SET_INT_YES, uItem);
    {
	quasi88_cfg_set_interlace((int)data);
    }
}

static	void	f_set_size(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_SIZ, M_SET_SIZ_FULL, M_SET_SIZ_HALF, uItem);
    {
	quasi88_cfg_set_size((int)data);
    }
}

static	void	f_set_full(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = quasi88_cfg_now_fullscreen() ? FALSE : TRUE; 	/* 逆にする */
    CheckItem(GetMenuRef(M_SET), uItem, (active ? true : false));

    quasi88_cfg_set_fullscreen(active);
}

static	void	f_set_pcg(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = use_pcg ? FALSE : TRUE; 				/* 逆にする */
    CheckItem(GetMenuRef(M_SET), uItem, (active ? true : false));

    use_pcg = active;
}

static	void	f_set_mouse(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_MO, M_SET_MO_NO, M_SET_MO_JOY, uItem);
    {
	mouse_mode = (int)data;
	keyboard_switch();
    }
}

static	void	f_set_cursor(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_CUR, M_SET_CUR_DEF, M_SET_CUR_TEN, uItem);
    {
	if ((int)data) {
	    cursor_key_mode = 1;
	} else {
	    cursor_key_mode = 0;
	}
	keyboard_switch();
    }
}

static	void	f_set_numlock(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = numlock_emu ? FALSE : TRUE; 		/* 逆にする */
    CheckItem(GetMenuRef(M_SET), uItem, (active ? true : false));

    quasi88_cfg_key_numlock(active);
}

static	void	f_set_romaji(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = romaji_input_mode ? FALSE : TRUE; 		/* 逆にする */
    CheckItem(GetMenuRef(M_SET), uItem, (active ? true : false));

    quasi88_cfg_key_romaji(active);
}

static	void	f_set_fm(int uItem, int data)
{
#ifdef	USE_SOUND
#ifdef	USE_FMGEN
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_FM, M_SET_FM_MAME, M_SET_FM_FMGEN, uItem);
    {
	//やっかい
	if (((xmame_cfg_get_use_fmgen())          && ((int)data == FALSE)) ||
	    ((xmame_cfg_get_use_fmgen() == FALSE) && ((int)data))) {

	    xmame_cfg_set_use_fmgen((int)data);

	    menu_sound_restart(TRUE);
	    update_misc_record();
	}
    }
#endif
#endif
}

static	void	f_set_frq(int uItem, int data)
{
#ifdef	USE_SOUND
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_SET_FRQ, M_SET_FRQ_44, M_SET_FRQ_08, uItem);
    {
	//やっかい
	if (xmame_cfg_get_sample_freq() != (int)data) {
	    if (8000 <= (int)data && (int)data <= 48000) {
		xmame_cfg_set_sample_freq((int)data);

		menu_sound_restart(TRUE);
		update_misc_record();
	    }
	}
    }
#endif
}

/*----------------------------------------------------------------------
 * Drive メニュー
 *----------------------------------------------------------------------*/

static	void	f_drv_chg(int data)
{
    FSSpec file;
    char *filename;				/* フルパスファイル名 */
    const char *headline;

    if (menubar_active == FALSE) { return; }

    switch ((int)data) {
    case DRIVE_1:	headline = "Open Disk-Image-File (Drive 1:)";	break;
    case DRIVE_2:	headline = "Open Disk-Image-File (Drive 2:)";	break;
    default:		headline = "Open Disk-Image-File";		break;
    }	

    /* headline の出番がない… */

    if (openGetFile(&file) &&
	(filename = get_filename(&file))) {

	int ok = FALSE;
	int ro = FALSE;

	if ((data == DRIVE_1) || (data == DRIVE_2)) {

	    ok = quasi88_disk_insert(data, filename, 0, ro);

	} else if (data < 0) {

	    ok = quasi88_disk_insert_all(filename, ro);

	}

	/* すでにファイルを閉じているので、失敗してもメニューバー更新 */
	update_drive();
    }

    /* ReadOnly属性のファイルを開くと、以降 Drive 1: のメニューが */
    /* 無効になる? どうすればいい ? */
}

static	void	f_drv_drv1(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_DRV_DRV1, M_DRV_DRV1_1, M_DRV_DRV1_NO, uItem);
    {
	if ((int)data <  0) {
	    quasi88_disk_image_empty(DRIVE_1);
	} else {
	    quasi88_disk_image_select(DRIVE_1, (int)data);
	}
    }
}

static	void	f_drv_drv2(int uItem, int data)
{
    if (menubar_active == FALSE) { return; }

    check_radio_item(M_DRV_DRV2, M_DRV_DRV2_1, M_DRV_DRV2_NO, uItem);
    {
	if ((int)data <  0) {
	    quasi88_disk_image_empty(DRIVE_2);
	} else {
	    quasi88_disk_image_select(DRIVE_2, (int)data);
	}
    }
}

static	void	f_drv_unset(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_disk_eject_all();

    update_drive();
}

/*----------------------------------------------------------------------
 * Misc メニュー
 *----------------------------------------------------------------------*/

static	void	f_misc_capture(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_screen_snapshot();
}

static	void	f_misc_record(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = xmame_wavout_opened() ? FALSE : TRUE; 	/* 逆にする */

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

    CheckItem(GetMenuRef(M_MISC), uItem, (active ? true : false));
}

static	void	f_misc_cload_s(void)
{
    FSSpec file;
    char *filename;				/* フルパスファイル名 */
    const char *headline = "Open Tape-Image-File for LOAD";

    if (menubar_active == FALSE) { return; }

    if (filename_get_tape(CLOAD)) { return; }	/* テープありなら戻る */

    /* headline の出番がない… */

    if (openGetFile(&file) &&
	(filename = get_filename(&file))) {

	int ok = quasi88_load_tape_insert(filename);

	/* すでにファイルを閉じているので、失敗してもメニューバー更新 */
	update_misc_cload();
    }
}

static	void	f_misc_cload_u(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_load_tape_eject();

    update_misc_cload();
}

static	void	f_misc_csave_s(void)
{
    FSSpec file;
    char *filename;				/* フルパスファイル名 */
    const char *headline = "Open Tape-Image-File for SAVE (append)";

    if (menubar_active == FALSE) { return; }

    if (filename_get_tape(CSAVE)) { return; }	/* テープありなら戻る */

    /* headline の出番がない… */

    if (openPutFile(&file) &&
	(filename = get_filename(&file))) {

	int ok = quasi88_save_tape_insert(filename);

	/* すでにファイルを閉じているので、失敗してもメニューバー更新 */
	update_misc_csave();
    }
}

static	void	f_misc_csave_u(void)
{
    if (menubar_active == FALSE) { return; }

    quasi88_save_tape_eject();

    update_misc_csave();
}

static	void	f_misc_sload(int data)
{
    if (menubar_active == FALSE) { return; }

    quasi88_stateload((int) data);

    /* 設定やファイル名が変更されたはずなので、メニューバーを全て更新 */
    menubar_setup(TRUE);
}

static	void	f_misc_ssave(int data)
{
    if (menubar_active == FALSE) { return; }

    quasi88_statesave((int) data);
}

static	void	f_misc_status(int uItem)
{
    int active;

    if (menubar_active == FALSE) { return; }

    active = quasi88_cfg_now_showstatus() ? FALSE : TRUE; 	/* 逆にする */
    CheckItem(GetMenuRef(M_MISC), uItem, (active ? true : false));

    quasi88_cfg_set_showstatus(active);
}

/*----------------------------------------------------------------------
 * Apple メニュー
 *----------------------------------------------------------------------*/

static void aboutBox(void)
{
    Str255 ptext;
    DialogItemType itemType;
    Handle Item;
    Rect box;
    DialogPtr theDialog;
    short theItem;

    const char *message = Q_TITLE "  ver. " Q_VERSION  "\n    <" Q_COMMENT ">";

    ptext[0] = strlen(message);
    memcpy(ptext+1, message, ptext[0]);

    theDialog = GetNewDialog(ABOUT_DIALOG, NULL, (WindowRef)-1L);	
    SetDialogDefaultItem(theDialog, ABOUT_OK_BUTTON);
    GetDialogItem(theDialog, ABOUT_TEXT, &itemType, &Item, &box);
    SetDialogItemText(Item, ptext);
    do {
	ModalDialog(NULL, &theItem);
    } while (theItem != ABOUT_OK_BUTTON);
    DisposeDialog(theDialog);
}
