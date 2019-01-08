/************************************************************************/
/*									*/
/* メニューモード							*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "initval.h"
#include "menu.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "screen.h"

#include "emu.h"
#include "drive.h"
#include "image.h"
#include "status.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "file-op.h"
#include "suspend.h"
#include "snapshot.h"
#include "fdc.h"
#include "soundbd.h"
#include "getconf.h"

#include "event.h"
#include "q8tk.h"


/* メニューの言語           */
#if LANG_EN
int	menu_lang = MENU_ENGLISH;
#else
int	menu_lang = MENU_JAPAN;
#endif

int	menu_readonly	= FALSE;		/* ディスク選択ダイアログの */
						/* 初期状態は ReadOnly ?    */
int	menu_swapdrv	= FALSE;		/* ドライブの表示順序       */



/* コールバック関数の引数 (Q8tkWidget*, void*) が未使用の場合、
 * ワーニングが出て鬱陶しいので、 gcc に頼んで許してもらう。 */
#if defined(__GNUC__)
#define	UNUSED_WIDGET	__attribute__((__unused__)) Q8tkWidget *dummy_0
#define	UNUSED_PARM	__attribute__((__unused__)) void *dummy_1
#else
#define	UNUSED_WIDGET	Q8tkWidget *dummy_0
#define	UNUSED_PARM	void *dummy_1
#endif



/*--------------------------------------------------------------*/
/* メニューでの表示メッセージは全て、このファイルの中に		*/
/*--------------------------------------------------------------*/
#include "message.h"




/****************************************************************/
/* ワーク							*/
/****************************************************************/

static	int	menu_last_page = 0;	/* 前回時のメニュータグを記憶 */

static	int	menu_boot_clock_async;	/* リセット時にクロック指定同期する? */

static	T_RESET_CFG	reset_req;	/* リセット時に要求する設定を、保存 */

/* メニュー終了時に、サウンドの環境が変わってないか確認のため、初期値保存 */
#define	NR_SD_CFG_LOCAL	(5)
typedef union {
    int		i;
    float	f;
} SD_CFG_LOCAL_VAL;

static struct {

    int		sound_board;
    int		use_fmgen;
    int		sample_freq;
    int		use_samples;

    /* 以下、システム依存の設定 */
    int		local_cnt;
    struct {
	T_SNDDRV_CONFIG		*info;
	SD_CFG_LOCAL_VAL	val;
    } local[ NR_SD_CFG_LOCAL ];

} sd_cfg_init, sd_cfg_now;
static	void	sd_cfg_save(void);
static	int	sd_cfg_has_changed(void);


static	int	cpu_timing_save;	/* メニュー開始時の -cpu 値 記憶 */

					/* 起動デバイスの制御に必要 */
static	Q8tkWidget	*widget_reset_boot;
static	Q8tkWidget	*widget_dipsw_b_boot_disk;
static	Q8tkWidget	*widget_dipsw_b_boot_rom;

static	Q8tkWidget	*menu_accel;	/* メインメニューのキー定義 */

/* メニュー下部のリセットと、リセットタグのリセットを連動させたいので、
   片方が選択されたら、反対のも選択されるように、ウィジットを記憶         */
static	Q8tkWidget	*widget_reset_basic[2][4];
static	Q8tkWidget	*widget_reset_clock[2][2];



/*===========================================================================
 * ファイル操作エラーメッセージのダイアログ処理
 *===========================================================================*/
static	void	cb_file_error_dialog_ok(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	void	start_file_error_dialog(int drv, int result)
{
    char wk[128];
    const t_menulabel *l = (drv<0) ? data_err_file : data_err_drive;

    if (result == ERR_NO) return;
    if (drv < 0) sprintf(wk, GET_LABEL(l, result));
    else         sprintf(wk, GET_LABEL(l, result), drv+1);

    dialog_create();
    {
	dialog_set_title(wk);
	dialog_set_separator();
	dialog_set_button(GET_LABEL(l, ERR_NO),
			  cb_file_error_dialog_ok, NULL);
    }
    dialog_start();
}

/*===========================================================================
 * ディスク挿入 & 排出
 *===========================================================================*/
static void sub_misc_suspend_update(void);
static void sub_misc_snapshot_update(void);
static void sub_misc_waveout_update(void);

/*===========================================================================
 *
 *	メインページ	リセット
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						 /* BASICモード切り替え */
static	int	get_reset_basic(void)
{
    return reset_req.boot_basic;
}
static	void	cb_reset_basic(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_basic != (int)p) {
	reset_req.boot_basic = (int)p;

	q8tk_toggle_button_set_state(widget_reset_basic[ 1 ][ (int)p ], TRUE);
    }
}


static	Q8tkWidget	*menu_reset_basic(void)
{
    Q8tkWidget *box;
    Q8List     *list;

    box = PACK_VBOX(NULL);
    {
	list = PACK_RADIO_BUTTONS(box,
				  data_reset_basic, COUNTOF(data_reset_basic),
				  get_reset_basic(), cb_reset_basic);

	/* リストを手繰って、全ウィジットを取得 */
	widget_reset_basic[0][BASIC_V2 ] = list->data;	list = list->next;
	widget_reset_basic[0][BASIC_V1H] = list->data;	list = list->next;
	widget_reset_basic[0][BASIC_V1S] = list->data;	list = list->next;
	widget_reset_basic[0][BASIC_N  ] = list->data;
    }

    return box;
}

/*----------------------------------------------------------------------*/
						       /* CLOCK切り替え */
static	int	get_reset_clock(void)
{
    return reset_req.boot_clock_4mhz;
}
static	void	cb_reset_clock(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_clock_4mhz != (int)p) {
	reset_req.boot_clock_4mhz = (int)p;

	q8tk_toggle_button_set_state(widget_reset_clock[ 1 ][ (int)p ], TRUE);
    }
}
static	int	get_reset_clock_async(void)
{
    return menu_boot_clock_async;
}
static	void	cb_reset_clock_async(Q8tkWidget *widget, UNUSED_PARM)
{
    int async = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    menu_boot_clock_async = async;
}


static	Q8tkWidget	*menu_reset_clock(void)
{
    Q8tkWidget *box, *box2;
    Q8List     *list;

    box = PACK_VBOX(NULL);
    {
	list = PACK_RADIO_BUTTONS(box,
				  data_reset_clock, COUNTOF(data_reset_clock),
				  get_reset_clock(), cb_reset_clock);

	/* リストを手繰って、全ウィジットを取得 */
	widget_reset_clock[0][CLOCK_4MHZ] = list->data;	list = list->next;
	widget_reset_clock[0][CLOCK_8MHZ] = list->data;

	PACK_LABEL(box, "");			/* 空行 */

	box2 = PACK_HBOX(box);
	{
	    PACK_LABEL(box2, "  ");		/* インデント */
	    PACK_CHECK_BUTTON(box2,
			      GET_LABEL(data_reset_clock_async, 0),
			      get_reset_clock_async(),
			      cb_reset_clock_async, NULL);
	}
    }

    return box;
}

/*----------------------------------------------------------------------*/
					      /* サウンドボード切り替え */
static	int	get_reset_sound(void)
{
    return reset_req.sound_board;
}
static	void	cb_reset_sound(UNUSED_WIDGET, void *p)
{
    reset_req.sound_board = (int)p;
}


static	Q8tkWidget	*menu_reset_sound(void)
{
    Q8tkWidget *box;

    box = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(box,
			   data_reset_sound, COUNTOF(data_reset_sound),
			   get_reset_sound(), cb_reset_sound);
	PACK_LABEL(box, "");			/* 空行 */
	PACK_LABEL(box, "");			/* 空行 */
    }

    return box;
}

/*----------------------------------------------------------------------*/
								/* 起動 */
static	void	set_reset_dipsw_boot(void)
{
    const t_menulabel *l = data_reset_boot;

    if (widget_reset_boot) {
	q8tk_label_set(widget_reset_boot,
		       (reset_req.boot_from_rom ? GET_LABEL(l, 1)
						: GET_LABEL(l, 0)));
    }
}


static	Q8tkWidget	*menu_reset_boot(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	widget_reset_boot = PACK_LABEL(vbox, "");
	set_reset_dipsw_boot();
    }
    return vbox;
}

/*----------------------------------------------------------------------*/
								/* 詳細 */
static	Q8tkWidget	*reset_detail_widget;
static	int		reset_detail_hide;
static	Q8tkWidget	*reset_detail_button;
static	void	cb_reset_detail(UNUSED_WIDGET, UNUSED_PARM)
{
    reset_detail_hide ^= 1;
  
    if (reset_detail_hide) {
	q8tk_widget_hide(reset_detail_widget);
    } else {
	q8tk_widget_show(reset_detail_widget);
    }
    q8tk_label_set(reset_detail_button->child,
		   GET_LABEL(data_reset_detail, reset_detail_hide));
}


static	Q8tkWidget	*menu_reset_detail(void)
{
    Q8tkWidget *box;

    box = PACK_VBOX(NULL);
    {
	PACK_LABEL(box, "");

	reset_detail_hide = 0;
	reset_detail_button = PACK_BUTTON(box,
					  "",
					  cb_reset_detail, NULL);
	cb_reset_detail(NULL, NULL);

	PACK_LABEL(box, "");
    }

    return box;
}

/*----------------------------------------------------------------------*/
						/* ディップスイッチ設定 */
static	void	dipsw_create(void);
static	void	dipsw_start(void);
static	void	dipsw_finish(void);

static	void	cb_reset_dipsw(UNUSED_WIDGET, UNUSED_PARM)
{
    dipsw_start();
}


static	Q8tkWidget	*menu_reset_dipsw(void)
{
    Q8tkWidget *button;

    button = PACK_BUTTON(NULL,
			 GET_LABEL(data_reset, DATA_RESET_DIPSW_BTN),
			 cb_reset_dipsw, NULL);
    q8tk_misc_set_placement(button, Q8TK_PLACEMENT_X_CENTER, 0);

    return button;
}

/*----------------------------------------------------------------------*/
						  /* バージョン切り替え */
static	int	get_reset_version(void)
{
    return reset_req.set_version;
}
static	void	cb_reset_version(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_reset_version;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_reset_version); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    reset_req.set_version = p->val;
	    break;
	}
    }
}


static	Q8tkWidget	*menu_reset_version(void)
{
    Q8tkWidget *box, *combo;
    char wk[4];

    wk[0] = get_reset_version();
    wk[1] = '\0';

    box = PACK_VBOX(NULL);
    {
	combo = PACK_COMBO(box,
			   data_reset_version, COUNTOF(data_reset_version),
			   get_reset_version(), wk, 8,
			   cb_reset_version, NULL,
			   NULL, NULL);
	PACK_LABEL(box, "");			/* 空行 */
    }

    return box;
}

/*----------------------------------------------------------------------*/
						  /* 拡張メモリ切り替え */
static	int	get_reset_extram(void)
{
    return reset_req.use_extram;
}
static	void	cb_reset_extram(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_reset_extram;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_reset_extram); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    reset_req.use_extram = p->val;
	    break;
	}
    }
}


static	Q8tkWidget	*menu_reset_extram(void)
{
    Q8tkWidget *box, *combo;
    char wk[16];

    sprintf(wk, "  %5dKB", use_extram * 128);

    box = PACK_VBOX(NULL);
    {
	combo = PACK_COMBO(box,
			   data_reset_extram, COUNTOF(data_reset_extram),
			   get_reset_extram(), wk, 0,
			   cb_reset_extram, NULL,
			   NULL, NULL);
	PACK_LABEL(box, "");			/* 空行 */
    }

    return box;
}

/*----------------------------------------------------------------------*/
						     /* 辞書ROM切り替え */
static	int	get_reset_jisho(void)
{
    return reset_req.use_jisho_rom;
}
static	void	cb_reset_jisho(UNUSED_WIDGET, void *p)
{
    reset_req.use_jisho_rom = (int)p;
}


static	Q8tkWidget	*menu_reset_jisho(void)
{
    Q8tkWidget *box;

    box = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(box,
			   data_reset_jisho, COUNTOF(data_reset_jisho),
			   get_reset_jisho(), cb_reset_jisho);
    }

    return box;
}

/*----------------------------------------------------------------------*/
						   /* 現在のBASICモード */
static	Q8tkWidget	*menu_reset_current(void)
{
    static const char *type[] = {
	"PC-8801",
	"PC-8801",
	"PC-8801",
	"PC-8801mkII",
	"PC-8801mkIISR",
	"PC-8801mkIITR/FR/MR",
	"PC-8801mkIITR/FR/MR",
	"PC-8801mkIITR/FR/MR",
	"PC-8801FH/MH",
	"PC-8801FA/MA/FE/MA2/FE2/MC",
    };
    static const char *basic[] = { " N ", "V1S", "V1H", " V2", };
    static const char *clock[] = { "8MHz", "4MHz", };
    const char *t = "";
    const char *b = "";
    const char *c = "";
    int i;
    char wk[80], ext[40];

    i = (ROM_VERSION & 0xff) - '0';
    if (0 <= i && i< COUNTOF(type)) t = type[ i ];

    i = get_reset_basic();
    if (0 <= i && i< COUNTOF(basic)) b = basic[ i ];

    i = get_reset_clock();
    if (0 <= i && i< COUNTOF(clock)) c = clock[ i ];

    ext[0] = 0;
    {
	if (sound_port) {
	    if (ext[0] == 0) strcat(ext, "(");
	    else             strcat(ext, ", ");
	    if (sound_board == SOUND_I) strcat(ext, "OPN");
	    else                        strcat(ext, "OPNA");
	}

	if (use_extram) {
	    if (ext[0] == 0) strcat(ext, "(");
	    else             strcat(ext, ", ");
	    sprintf(wk, "%dKB", use_extram * 128);
	    strcat(ext, wk);
	    strcat(ext, GET_LABEL(data_reset_current,0));/* ExtRAM*/
	}

	if (use_jisho_rom) {
	    if (ext[0] == 0) strcat(ext, "(");
	    else             strcat(ext, ", ");
	    strcat(ext, GET_LABEL(data_reset_current,1));/*DictROM*/
	}
    }
    if (ext[0]) strcat(ext, ")");


    sprintf(wk, " %-30s  %4s  %4s  %30s ",
	    t, b, c, ext);

    return PACK_LABEL(NULL, wk);
}

/*----------------------------------------------------------------------*/
							    /* リセット */
static	void	cb_reset_now(UNUSED_WIDGET, UNUSED_PARM)
{
    /* CLOCK設定と、CPUクロックを同期させる */
    if (menu_boot_clock_async == FALSE) {
	cpu_clock_mhz = reset_req.boot_clock_4mhz ? CONST_4MHZ_CLOCK
						  : CONST_8MHZ_CLOCK;
    }

    /* reset_req の設定に基づき、リセット → 実行 */
    quasi88_reset(&reset_req);

    quasi88_exec();		/* ← q8tk_main_quit() 呼出済み */

#if 0
    printf("boot_dipsw      %04x\n",boot_dipsw   );
    printf("boot_from_rom   %d\n",boot_from_rom  );
    printf("boot_basic      %d\n",boot_basic     );
    printf("boot_clock_4mhz %d\n",boot_clock_4mhz);
    printf("ROM_VERSION     %c\n",ROM_VERSION    );
    printf("baudrate_sw     %d\n",baudrate_sw    );
#endif
}

/*======================================================================*/

static	Q8tkWidget	*menu_reset(void)
{
    Q8tkWidget *hbox, *vbox;
    Q8tkWidget *w, *f;
    const t_menulabel *l = data_reset;

    dipsw_create();		/* ディップスイッチウインドウ生成 */

    vbox = PACK_VBOX(NULL);
    {
	f = PACK_FRAME(vbox, "", menu_reset_current());
	q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_ETCHED_OUT);

	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_RESET_BASIC), menu_reset_basic());

	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_RESET_CLOCK), menu_reset_clock());

	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_RESET_SOUND), menu_reset_sound());

	    w = PACK_FRAME(hbox,
			   GET_LABEL(l, DATA_RESET_BOOT), menu_reset_boot());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_ETCHED_IN);
	}

	PACK_LABEL(vbox, GET_LABEL(l, DATA_RESET_NOTICE));

	hbox = PACK_HBOX(vbox);
	{
	    reset_detail_widget = PACK_HBOX(NULL);

	    q8tk_box_pack_start(hbox, menu_reset_detail());
	    q8tk_box_pack_start(hbox, reset_detail_widget);
	    {
		PACK_VSEP(reset_detail_widget);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_DIPSW), menu_reset_dipsw());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_CENTER);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_VERSION), menu_reset_version());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_BOTTOM);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_EXTRAM), menu_reset_extram());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_BOTTOM);

		f = PACK_FRAME(reset_detail_widget,
		       GET_LABEL(l, DATA_RESET_JISHO), menu_reset_jisho());

		q8tk_misc_set_placement(f, 0, Q8TK_PLACEMENT_Y_BOTTOM);
	    }

	}

	hbox = PACK_HBOX(vbox);
	{
	    w = PACK_LABEL(hbox, GET_LABEL(l, DATA_RESET_INFO));
	    q8tk_misc_set_placement(w, 0, Q8TK_PLACEMENT_Y_CENTER);

	    w = PACK_BUTTON(hbox,
			    GET_LABEL(data_reset, DATA_RESET_NOW),
			    cb_reset_now, NULL);
	}
	q8tk_misc_set_placement(hbox, Q8TK_PLACEMENT_X_RIGHT, 0);
    }

    return vbox;
}



/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	サブウインドウ	DIPSW
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static	Q8tkWidget	*dipsw_window;
static	Q8tkWidget	*dipsw[4];
static	Q8tkWidget	*dipsw_accel;

enum {
    DIPSW_WIN,
    DIPSW_FRAME,
    DIPSW_VBOX,
    DIPSW_QUIT
};

/*----------------------------------------------------------------------*/
					    /* ディップスイッチ切り替え */
static	int	get_dipsw_b(int p)
{
    int shift = data_dipsw_b[p].val;

    return ((p<<1) | ((reset_req.boot_dipsw >> shift) & 1));
}
static	void	cb_dipsw_b(UNUSED_WIDGET, void *p)
{
    int shift = data_dipsw_b[ (int)p >> 1 ].val;
    int on    = (int)p & 1;

    if (on) reset_req.boot_dipsw |=  (1 << shift);
    else    reset_req.boot_dipsw &= ~(1 << shift);
}
static	int	get_dipsw_b2(void)
{
    return (reset_req.boot_from_rom ? TRUE : FALSE);
}
static	void	cb_dipsw_b2(UNUSED_WIDGET, void *p)
{
    if ((int)p) reset_req.boot_from_rom = TRUE;
    else        reset_req.boot_from_rom = FALSE;

    set_reset_dipsw_boot();
}


static	Q8tkWidget	*menu_dipsw_b(void)
{
    int i;
    Q8tkWidget *vbox, *hbox;
    Q8tkWidget *b = NULL;
    const t_dipsw *pd;
    const t_menudata *p;


    vbox = PACK_VBOX(NULL);
    {
	pd = data_dipsw_b;
	for (i=0; i<COUNTOF(data_dipsw_b); i++, pd++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(pd, 0));

		PACK_RADIO_BUTTONS(hbox,
				   pd->p, 2,
				   get_dipsw_b(i), cb_dipsw_b);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    pd = data_dipsw_b2;
	    p  = pd->p;

	    PACK_LABEL(hbox, GET_LABEL(pd, 0));

	    for (i=0; i<2; i++, p++) {
		b = PACK_RADIO_BUTTON(hbox,
				      b,
				      GET_LABEL(p, 0), 
				      (get_dipsw_b2() == p->val) ?TRUE :FALSE,
				      cb_dipsw_b2, (void *)(p->val));

		if (i == 0) widget_dipsw_b_boot_disk = b;  /*これらのボタンは*/
		else        widget_dipsw_b_boot_rom  = b;  /*覚えておく      */
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
				   /* ディップスイッチ切り替え(RS-232C) */
static	int	get_dipsw_r(int p)
{
    int shift = data_dipsw_r[p].val;

    return ((p<<1) | ((reset_req.boot_dipsw >> shift) & 1));
}
static	void	cb_dipsw_r(UNUSED_WIDGET, void *p)
{
    int shift = data_dipsw_r[ (int)p >> 1 ].val;
    int on    = (int)p & 1;

    if (on) reset_req.boot_dipsw |=  (1 << shift);
    else    reset_req.boot_dipsw &= ~(1 << shift);
}
static	int	get_dipsw_r_baudrate(void)
{
    return reset_req.baudrate_sw;
}
static	void	cb_dipsw_r_baudrate(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_dipsw_r_baudrate;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_dipsw_r_baudrate); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    reset_req.baudrate_sw = p->val;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_dipsw_r(void)
{
    int i;
    Q8tkWidget *vbox, *hbox;
    const t_dipsw *pd;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, GET_LABEL(data_dipsw_r2, 0));

	    PACK_COMBO(hbox,
		       data_dipsw_r_baudrate,
		       COUNTOF(data_dipsw_r_baudrate),
		       get_dipsw_r_baudrate(), NULL, 8,
		       cb_dipsw_r_baudrate, NULL,
		       NULL, NULL);
	}

	pd = data_dipsw_r;
	for (i=0; i<COUNTOF(data_dipsw_r); i++, pd++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(data_dipsw_r, i));

		PACK_RADIO_BUTTONS(hbox,
				   pd->p, 2,
				   get_dipsw_r(i), cb_dipsw_r);
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/

static	void	dipsw_create(void)
{
    Q8tkWidget *vbox;
    const t_menulabel *l = data_dipsw;

    vbox = PACK_VBOX(NULL);
    {
	PACK_FRAME(vbox, GET_LABEL(l, DATA_DIPSW_B), menu_dipsw_b());

	PACK_FRAME(vbox, GET_LABEL(l, DATA_DIPSW_R), menu_dipsw_r());
    }

    dipsw_window = vbox;
}

static	void	cb_reset_dipsw_end(UNUSED_WIDGET, UNUSED_PARM)
{
    dipsw_finish();
}

static	void	dipsw_start(void)
{
    Q8tkWidget *w, *f, *x, *b;
    const t_menulabel *l = data_reset;

    {						/* メインとなるウインドウ */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	dipsw_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(dipsw_accel, w);
    }
    {						/* に、フレームを乗せて */
	f = q8tk_frame_new(GET_LABEL(l, DATA_RESET_DIPSW_SET));
	q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_OUT);
	q8tk_container_add(w, f);
	q8tk_widget_show(f);
    }
    {						/* それにボックスを乗せる */
	x = q8tk_vbox_new();
	q8tk_container_add(f, x);
	q8tk_widget_show(x);
							/* ボックスには     */
	{						/* DIPSWメニュー と */
	    q8tk_box_pack_start(x, dipsw_window);
	}
	{						/* 終了ボタンを配置 */
	    b = PACK_BUTTON(x,
			    GET_LABEL(l, DATA_RESET_DIPSW_QUIT),
			    cb_reset_dipsw_end, NULL);

	    q8tk_accel_group_add(dipsw_accel, Q8TK_KEY_ESC, b, "clicked");
	}
    }

    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b);


    dipsw[ DIPSW_WIN   ] = w;	/* ダイアログを閉じたときに備えて */
    dipsw[ DIPSW_FRAME ] = f;	/* ウィジットを覚えておきます     */
    dipsw[ DIPSW_VBOX  ] = x;
    dipsw[ DIPSW_QUIT  ] = b;
}

/* ディップスイッチ設定ウインドウの消去 */

static	void	dipsw_finish(void)
{
    q8tk_widget_destroy(dipsw[ DIPSW_QUIT ]);
    q8tk_widget_destroy(dipsw[ DIPSW_VBOX ]);
    q8tk_widget_destroy(dipsw[ DIPSW_FRAME ]);

    q8tk_grab_remove(dipsw[ DIPSW_WIN ]);
    q8tk_widget_destroy(dipsw[ DIPSW_WIN ]);
    q8tk_widget_destroy(dipsw_accel);
}





/*===========================================================================
 *
 *	メインページ	CPU設定
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						     /* CPU処理切り替え */
static	int	get_cpu_cpu(void)
{
    return cpu_timing;
}
static	void	cb_cpu_cpu(UNUSED_WIDGET, void *p)
{
    cpu_timing = (int)p;
}


static	Q8tkWidget	*menu_cpu_cpu(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(vbox,
			   data_cpu_cpu, COUNTOF(data_cpu_cpu),
			   get_cpu_cpu(), cb_cpu_cpu);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
								/* 説明 */
static	Q8tkWidget	*help_widget[5];
static	Q8tkWidget	*help_string[40];
static	int		help_string_cnt;	
static	Q8tkWidget	*help_accel;

enum {
    HELP_WIN,
    HELP_VBOX,
    HELP_SWIN,
    HELP_BOARD,
    HELP_EXIT
};

static	void	help_finish(void);
static	void	cb_cpu_help_end(UNUSED_WIDGET, UNUSED_PARM)
{
    help_finish();
}

static	void	cb_cpu_help(UNUSED_WIDGET, UNUSED_PARM)
{
    Q8tkWidget *w, *swin, *x, *b, *z;

    {						/* メインとなるウインドウ */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	help_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(help_accel, w);
    }
    {						/* それにボックスを乗せる */
	x = q8tk_vbox_new();
	q8tk_container_add(w, x);
	q8tk_widget_show(x);
							/* ボックスには     */
	{						/* SCRLウインドウと */
	    swin  = q8tk_scrolled_window_new(NULL, NULL);
	    q8tk_widget_show(swin);
	    q8tk_scrolled_window_set_policy(swin, Q8TK_POLICY_NEVER,
						  Q8TK_POLICY_AUTOMATIC);
	    q8tk_misc_set_size(swin, 71, 20);
	    q8tk_box_pack_start(x, swin);
	}
	{						/* 終了ボタンを配置 */
	    b = PACK_BUTTON(x,
			    " O K ",
			    cb_cpu_help_end, NULL);
	    q8tk_misc_set_placement(b, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);

	    q8tk_accel_group_add(help_accel, Q8TK_KEY_ESC, b, "clicked");
	}
    }

    {							/* SCRLウインドウに */
	int i;
	const char **s = (menu_lang == MENU_JAPAN) ? help_jp : help_en;
	z = q8tk_vbox_new();				/* VBOXを作って     */
	q8tk_container_add(swin, z);
	q8tk_widget_show(z);

	for (i=0; i<COUNTOF(help_string); i++) {	/* 説明ラベルを配置 */
	    if (s[i] == NULL) break;
	    help_string[i] = q8tk_label_new(s[i]);
	    q8tk_widget_show(help_string[i]);
	    q8tk_box_pack_start(z, help_string[i]);
	}
	help_string_cnt = i;
    }

    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b);


    help_widget[ HELP_WIN   ] = w;	/* ダイアログを閉じたときに備えて */
    help_widget[ HELP_VBOX  ] = x;	/* ウィジットを覚えておきます     */
    help_widget[ HELP_SWIN  ] = swin;
    help_widget[ HELP_BOARD ] = z;
    help_widget[ HELP_EXIT  ] = b;
}

/* 説明ウインドウの消去 */

static	void	help_finish(void)
{
    int i;
    for (i=0; i<help_string_cnt; i++)
	q8tk_widget_destroy(help_string[ i ]);

    q8tk_widget_destroy(help_widget[ HELP_EXIT  ]);
    q8tk_widget_destroy(help_widget[ HELP_BOARD ]);
    q8tk_widget_destroy(help_widget[ HELP_SWIN  ]);
    q8tk_widget_destroy(help_widget[ HELP_VBOX  ]);

    q8tk_grab_remove(help_widget[ HELP_WIN ]);
    q8tk_widget_destroy(help_widget[ HELP_WIN ]);
    q8tk_widget_destroy(help_accel);
}



static	Q8tkWidget	*menu_cpu_help(void)
{
    Q8tkWidget *button;
    const t_menulabel *l = data_cpu;

    button = PACK_BUTTON(NULL,
			 GET_LABEL(l, DATA_CPU_HELP),
			 cb_cpu_help, NULL);
    q8tk_misc_set_placement(button, Q8TK_PLACEMENT_X_CENTER,
				    Q8TK_PLACEMENT_Y_CENTER);
    return button;
}

/*----------------------------------------------------------------------*/
						 /* CPUクロック切り替え */
static	double	get_cpu_clock(void)
{
    return cpu_clock_mhz;
}
static	void	cb_cpu_clock(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_cpu_clock_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    double val = 0;
    int fit = FALSE;
#if USE_RETROACHIEVEMENTS
    double min_val = CONST_4MHZ_CLOCK;
#else
    double min_val = 0.1;
#endif
    double max_val = CONST_4MHZ_CLOCK * 250;

    /* COMBO BOX から ENTRY に一致するものを探す */
    for (i=0; i<COUNTOF(data_cpu_clock_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = (double)p->val / 1000000.0;
	    fit = TRUE;					/* 一致した値を適用 */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX に該当がない場合 */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtod(buf, &conv_end);

	if (((int)mode == 0) &&				/* 空白 + ENTER や   */
	    (strlen(buf) == 0 || val == 0.0)) {		/*   0  + ENTER 時は */
							/* デフォルト値を適用*/
	    val = boot_clock_4mhz ? CONST_4MHZ_CLOCK
				  : CONST_8MHZ_CLOCK;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* 数字変換失敗なら */
	    fit = FALSE;				/* その値は使えない */

	} else {					/* 数字変換成功なら */
	    fit = TRUE;					/* その値を適用する */
	}
    }

    if (fit) {				/* 適用した値が有効範囲なら、セット */
	if (min_val <= val && val < max_val) {
	    cpu_clock_mhz = val;
	}
    else if (val < min_val) {
        cpu_clock_mhz = min_val;
    }
    else /* if (val > max_val) */ {
        cpu_clock_mhz = max_val;
    }

    interval_work_init_all();
    }

    if ((int)mode == 0) {		/* COMBO ないし ENTER時は、値を再表示*/
	sprintf(buf, "%8.4f", get_cpu_clock());
	q8tk_combo_set_text(widget, buf);
    }
}


static	Q8tkWidget	*menu_cpu_clock(void)
{
    Q8tkWidget *vbox, *hbox;
    const t_menulabel *p = data_cpu_clock;
    char buf[16];

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_CLOCK_CLOCK));

	    sprintf(buf, "%8.4f", get_cpu_clock());
	    PACK_COMBO(hbox,
		       data_cpu_clock_combo, COUNTOF(data_cpu_clock_combo),
		       (int) get_cpu_clock(), buf, 9,
		       cb_cpu_clock, (void *)0,
		       cb_cpu_clock, (void *)1);

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_CLOCK_MHZ));

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_CLOCK_INFO));
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
							/* ウエイト変更 */
static	int	get_cpu_nowait(void)
{
    return no_wait;
}
static	void	cb_cpu_nowait(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    no_wait = key;
}

static	int	get_cpu_wait(void)
{
    return wait_rate;
}
static	void	cb_cpu_wait(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_cpu_wait_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    int val = 0;
    int fit = FALSE;
#if USE_RETROACHIEVEMENTS
    int min_val = 100;
#else
    int min_val = 5;
#endif
    int max_val = 5000;

    /* COMBO BOX から ENTRY に一致するものを探す */
    for (i=0; i<COUNTOF(data_cpu_wait_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = p->val;
	    fit = TRUE;					/* 一致した値を適用 */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX に該当がない場合 */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtoul(buf, &conv_end, 10);

	if (((int)mode == 0) &&				/* 空白 + ENTER や   */
	    (strlen(buf) == 0 || val == 0)) {		/*   0  + ENTER 時は */
							/* デフォルト値を適用*/
	    val = 100;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* 数字変換失敗なら */
	    fit = FALSE;				/* その値は使えない */

	} else {					/* 数字変換成功なら */
	    fit = TRUE;					/* その値を適用する */
	}
    }

    if (fit) {				/* 適用した値が有効範囲なら、セット */
	if (min_val <= val && val <= max_val) {
	    wait_rate = val;
	}
    else if (val < min_val) {
        wait_rate = min_val;
    }
    else /* if (val > max_val) */ {
        wait_rate = max_val;
    }
    }

    if ((int)mode == 0) {		/* COMBO ないし ENTER時は、値を再表示*/
	sprintf(buf, "%4d", get_cpu_wait());
	q8tk_combo_set_text(widget, buf);
    }
}


static	Q8tkWidget	*menu_cpu_wait(void)
{
    Q8tkWidget *vbox, *hbox, *button;
    const t_menulabel *p = data_cpu_wait;
    char buf[16];

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_WAIT_RATE));

	    sprintf(buf, "%4d", get_cpu_wait());
	    PACK_COMBO(hbox,
		       data_cpu_wait_combo, COUNTOF(data_cpu_wait_combo),
		       get_cpu_wait(), buf, 5,
		       cb_cpu_wait, (void *)0,
		       cb_cpu_wait, (void *)1);

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_WAIT_PERCENT));

	    PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_WAIT_INFO));
	}

	button = PACK_CHECK_BUTTON(vbox,
				   GET_LABEL(p, DATA_CPU_WAIT_NOWAIT),
				   get_cpu_nowait(),
				   cb_cpu_nowait, NULL);
	q8tk_misc_set_placement(button, Q8TK_PLACEMENT_X_RIGHT,
					Q8TK_PLACEMENT_Y_CENTER);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
							    /* ブースト */
static	int	get_cpu_boost(void)
{
    return boost;
}
static	void	cb_cpu_boost(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_cpu_boost_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    int val = 0;
    int fit = FALSE;

    /* COMBO BOX から ENTRY に一致するものを探す */
    for (i=0; i<COUNTOF(data_cpu_boost_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = p->val;
	    fit = TRUE;					/* 一致した値を適用 */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX に該当がない場合 */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtoul(buf, &conv_end, 10);

	if (((int)mode == 0) &&				/* 空白 + ENTER や   */
	    (strlen(buf) == 0 || val == 0)) {		/*   0  + ENTER 時は */
							/* デフォルト値を適用*/
	    val = 1;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* 数字変換失敗なら */
	    fit = FALSE;				/* その値は使えない */

	} else {					/* 数字変換成功なら */
	    fit = TRUE;					/* その値を適用する */
	}
    }

    if (fit) {				/* 適用した値が有効範囲なら、セット */
	if (1 <= val && val <= 100) {
	    if (boost != val) {
		boost_change(val);
	    }
	}
    }

    if ((int)mode == 0) {		/* COMBO ないし ENTER時は、値を再表示*/
	sprintf(buf, "%4d", get_cpu_boost());
	q8tk_combo_set_text(widget, buf);
    }
}

static	Q8tkWidget	*menu_cpu_boost(void)
{
    Q8tkWidget *hbox;
    char buf[8];
    const t_menulabel *p = data_cpu_boost;

    hbox = PACK_HBOX(NULL);
    {
	PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_BOOST_MAGNIFY));

	sprintf(buf, "%4d", get_cpu_boost());
	PACK_COMBO(hbox,
		   data_cpu_boost_combo, COUNTOF(data_cpu_boost_combo),
		   get_cpu_boost(), buf, 5,
		   cb_cpu_boost, (void*)0,
		   cb_cpu_boost, (void*)1);

	PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_BOOST_UNIT));

	PACK_LABEL(hbox, GET_LABEL(p, DATA_CPU_BOOST_INFO));
    }

    return hbox;
}

/*----------------------------------------------------------------------*/
						      /* 各種設定の変更 */
static	int	get_cpu_misc(int type)
{
    switch (type) {
    case DATA_CPU_MISC_FDCWAIT:
	return (fdc_wait == 0) ? FALSE : TRUE;

    case DATA_CPU_MISC_HSBASIC:
	return highspeed_mode;

    case DATA_CPU_MISC_MEMWAIT:
	return memory_wait;

    case DATA_CPU_MISC_CMDSING:
	return use_cmdsing;
    }
    return FALSE;
}
static	void	cb_cpu_misc(Q8tkWidget *widget, void *p)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    switch ((int)p) {
    case DATA_CPU_MISC_FDCWAIT:
	fdc_wait = (key) ? 1 : 0;
	return;

    case DATA_CPU_MISC_HSBASIC:
	highspeed_mode = (key) ? TRUE : FALSE;
	return;

    case DATA_CPU_MISC_MEMWAIT:
	memory_wait = (key) ? TRUE : FALSE;
	return;

    case DATA_CPU_MISC_CMDSING:
	use_cmdsing = (key) ? TRUE : FALSE;
#ifdef	USE_SOUND
	xmame_dev_beep_cmd_sing((byte) use_cmdsing);
#endif
	return;
    }
}


static	Q8tkWidget	*menu_cpu_misc(void)
{
    int i;
    Q8tkWidget *vbox, *l;
    const t_menudata *p = data_cpu_misc;

    vbox = PACK_VBOX(NULL);
    {
	for (i=0; i<COUNTOF(data_cpu_misc); i++, p++) {
	    if (p->val >= 0) {
		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(p, 0),
				  get_cpu_misc(p->val),
				  cb_cpu_misc, (void *)(p->val));
	    } else {
		l = PACK_LABEL(vbox, GET_LABEL(p, 0));
		q8tk_misc_set_placement(l, Q8TK_PLACEMENT_X_RIGHT, 0);
	    }
	}
    }

    return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_cpu(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2;
    Q8tkWidget *f;
    const t_menulabel *l = data_cpu;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox, GET_LABEL(l, DATA_CPU_CPU), menu_cpu_cpu());

	    f = PACK_FRAME(hbox, "              ", menu_cpu_help());
	    q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_NONE);
	}

	hbox = PACK_HBOX(vbox);
	{
	    vbox2 = PACK_VBOX(hbox);
	    {
		PACK_FRAME(vbox2,
			   GET_LABEL(l, DATA_CPU_CLOCK), menu_cpu_clock());

		PACK_FRAME(vbox2,
			   GET_LABEL(l, DATA_CPU_WAIT), menu_cpu_wait());

		PACK_FRAME(vbox2,
			   GET_LABEL(l, DATA_CPU_BOOST), menu_cpu_boost());
	    }

	    f = PACK_FRAME(hbox, "", menu_cpu_misc());
	    q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_NONE);
	}
    }

    return vbox;
}








/*===========================================================================
 *
 *	メインページ	画面
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						  /* フレームレート変更 */
static	int	get_graph_frate(void)
{
    return quasi88_cfg_now_frameskip_rate();
}
static	void	cb_graph_frate(Q8tkWidget *widget, void *label)
{
    int i;
    const t_menudata *p = data_graph_frate;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char  str[32];

    for (i=0; i<COUNTOF(data_graph_frate); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    sprintf(str, " fps (-frameskip %d)", p->val);
	    q8tk_label_set((Q8tkWidget*)label, str);

	    quasi88_cfg_set_frameskip_rate(p->val);
	    return;
	}
    }
}
						/* thanks floi ! */
static	int	get_graph_autoskip(void)
{
    return use_auto_skip;
}
static	void	cb_graph_autoskip(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    use_auto_skip = key;
}


static	Q8tkWidget	*menu_graph_frate(void)
{
    Q8tkWidget *vbox, *hbox, *combo, *label;
    char wk[32];

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    label = q8tk_label_new(" fps");
	    {
		sprintf(wk, "%6.3f", 60.0f / get_graph_frate());
		combo = PACK_COMBO(hbox,
				   data_graph_frate, COUNTOF(data_graph_frate),
				   get_graph_frate(), wk, 6,
				   cb_graph_frate, label,
				   NULL, NULL);
	    }
	    {
		q8tk_box_pack_start(hbox, label);
		q8tk_widget_show(label);
		cb_graph_frate(combo, (void*)label);
	    }
	}

	PACK_LABEL(vbox, "");			/* 空行 */
							/* thanks floi ! */
	PACK_CHECK_BUTTON(vbox,
			  GET_LABEL(data_graph_autoskip, 0),
			  get_graph_autoskip(),
			  cb_graph_autoskip, NULL);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						  /* 画面サイズ切り替え */
static	int	get_graph_resize(void)
{
    return quasi88_cfg_now_size();
}
static	void	cb_graph_resize(UNUSED_WIDGET, void *p)
{
    int new_size = (int)p;

    quasi88_cfg_set_size(new_size);
}
static	int	get_graph_fullscreen(void)
{
    return use_fullscreen;
}
static	void	cb_graph_fullscreen(Q8tkWidget *widget, UNUSED_PARM)
{
    int on = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    if (quasi88_cfg_can_fullscreen()) {
	quasi88_cfg_set_fullscreen(on);
	
	/* Q8TK カーソル有無設定 (全画面切替時に呼ぶ必要あり) */
	q8tk_set_cursor(now_swcursor);
    }
}


static	Q8tkWidget	*menu_graph_resize(void)
{
    Q8tkWidget *vbox;
    int i = COUNTOF(data_graph_resize);
    int j = quasi88_cfg_max_size() - quasi88_cfg_min_size() + 1;

    vbox = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(PACK_VBOX(vbox),
			   &data_graph_resize[quasi88_cfg_min_size()],
			   MIN(i, j),
			   get_graph_resize(), cb_graph_resize);

	if (quasi88_cfg_can_fullscreen()) {

	    PACK_LABEL(vbox, "");		/* 空行 */

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_graph_fullscreen, 0),
			      get_graph_fullscreen(),
			      cb_graph_fullscreen, NULL);
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						      /* 各種設定の変更 */
static	int	get_graph_misc(int type)
{
    switch (type) {
    case DATA_GRAPH_MISC_15K:
	return (monitor_15k == 0x02) ? TRUE: FALSE;

    case DATA_GRAPH_MISC_DIGITAL:
	return (monitor_analog == FALSE) ? TRUE : FALSE;

    case DATA_GRAPH_MISC_NOINTERP:
	return (quasi88_cfg_now_interp() == FALSE) ? TRUE : FALSE;
    }
    return FALSE;
}
static	void	cb_graph_misc(Q8tkWidget *widget, void *p)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    switch ((int)p) {
    case DATA_GRAPH_MISC_15K:
	monitor_15k = (key) ? 0x02 : 0x00;
	return;

    case DATA_GRAPH_MISC_DIGITAL:
	monitor_analog = (key) ? FALSE : TRUE;
	return;

    case DATA_GRAPH_MISC_NOINTERP:
	if (quasi88_cfg_can_interp()) {
	    quasi88_cfg_set_interp((key) ? FALSE : TRUE);
	}
	return;
    }
}
static	int	get_graph_misc2(void)
{
    return quasi88_cfg_now_interlace();
}
static	void	cb_graph_misc2(UNUSED_WIDGET, void *p)
{
    quasi88_cfg_set_interlace((int)p);
}


static	Q8tkWidget	*menu_graph_misc(void)
{
    Q8tkWidget *vbox;
    const t_menudata *p = data_graph_misc;
    int i       = COUNTOF(data_graph_misc);

    if (quasi88_cfg_can_interp() == FALSE) {
	i --;
    }

    vbox = PACK_VBOX(NULL);
    {
	PACK_CHECK_BUTTONS(vbox,
			   p, i,
			   get_graph_misc, cb_graph_misc);

	PACK_LABEL(vbox, "");

	PACK_RADIO_BUTTONS(vbox,
			   data_graph_misc2, COUNTOF(data_graph_misc2),
			   get_graph_misc2(), cb_graph_misc2);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						 /* PCG有無 ＆ フォント */
static	Q8tkWidget	*graph_font_widget;
static	int	get_graph_pcg(void)
{
    return use_pcg;
}
static	void	cb_graph_pcg(Q8tkWidget *widget, void *p)
{
    if (widget) {
	use_pcg = (int)p;
	memory_set_font();
    }

    if (use_pcg) { q8tk_widget_set_sensitive(graph_font_widget, FALSE); }
    else         { q8tk_widget_set_sensitive(graph_font_widget, TRUE);  }
}

static	int	get_graph_font(void)
{
    return font_type;
}
static	void	cb_graph_font(UNUSED_WIDGET, void *p)
{
    font_type = (int)p;
    memory_set_font();
}

static	Q8tkWidget	*menu_graph_pcg(void)
{
    Q8tkWidget *vbox, *b;
    const t_menulabel *l = data_graph;
    t_menudata data_graph_font[3];


    /* フォント選択ウィジット生成 (PCG有無により、insensitive になる) */
    {
	data_graph_font[0] = data_graph_font1[ (font_loaded & 1) ? 1 : 0 ];
	data_graph_font[1] = data_graph_font2[ (font_loaded & 2) ? 1 : 0 ];
	data_graph_font[2] = data_graph_font3[ (font_loaded & 4) ? 1 : 0 ];

	b = PACK_VBOX(NULL);
	{
	    PACK_RADIO_BUTTONS(b,
			       data_graph_font, COUNTOF(data_graph_font),
			       get_graph_font(), cb_graph_font);
	}
	graph_font_widget = PACK_FRAME(NULL,
				       GET_LABEL(l, DATA_GRAPH_FONT), b);
    }

    /* PCG有無ウィジットと、フォント選択ウィジットを並べる */
    vbox = PACK_VBOX(NULL);
    {
	{
	    b = PACK_HBOX(NULL);
	    {
		PACK_RADIO_BUTTONS(b,
				   data_graph_pcg, COUNTOF(data_graph_pcg),
				   get_graph_pcg(), cb_graph_pcg);
	    }
	    PACK_FRAME(vbox, GET_LABEL(l, DATA_GRAPH_PCG), b);
	}

	q8tk_box_pack_start(vbox, graph_font_widget);
    }

    return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_graph(void)
{
    Q8tkWidget *vbox, *hbox;
    Q8tkWidget *w;
    const t_menulabel *l = data_graph;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_GRAPH_FRATE), menu_graph_frate());

	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_GRAPH_RESIZE), menu_graph_resize());
	}

	hbox = PACK_HBOX(vbox);
	{
	    w = PACK_FRAME(hbox, "", menu_graph_misc());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_NONE);

	    w = PACK_FRAME(hbox, "", menu_graph_pcg());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_NONE);
	}
    }

    return vbox;
}








/*===========================================================================
 *
 *	メインページ	音量
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
						      /* ボリューム変更 */
#ifdef	USE_SOUND
static	int	get_volume(int type)
{
    switch (type) {
    case VOL_TOTAL:  return  xmame_cfg_get_mastervolume();
    case VOL_FM:     return  xmame_cfg_get_mixer_volume(XMAME_MIXER_FM);
    case VOL_PSG:    return  xmame_cfg_get_mixer_volume(XMAME_MIXER_PSG);
    case VOL_BEEP:   return  xmame_cfg_get_mixer_volume(XMAME_MIXER_BEEP);
    case VOL_RHYTHM: return  xmame_cfg_get_mixer_volume(XMAME_MIXER_RHYTHM);
    case VOL_ADPCM:  return  xmame_cfg_get_mixer_volume(XMAME_MIXER_ADPCM);
    case VOL_FMGEN:  return  xmame_cfg_get_mixer_volume(XMAME_MIXER_FMGEN);
    case VOL_SAMPLE: return  xmame_cfg_get_mixer_volume(XMAME_MIXER_SAMPLE);
    }
    return 0;
}
static	void	cb_volume(Q8tkWidget *widget, void *p)
{
    int vol = Q8TK_ADJUSTMENT(widget)->value;

    switch ((int)p) {
    case VOL_TOTAL:  xmame_cfg_set_mastervolume(vol);			 break;
    case VOL_FM:     xmame_cfg_set_mixer_volume(XMAME_MIXER_FM, vol);	 break;
    case VOL_PSG:    xmame_cfg_set_mixer_volume(XMAME_MIXER_PSG, vol);	 break;
    case VOL_BEEP:   xmame_cfg_set_mixer_volume(XMAME_MIXER_BEEP, vol);  break;
    case VOL_RHYTHM: xmame_cfg_set_mixer_volume(XMAME_MIXER_RHYTHM, vol);break;
    case VOL_ADPCM:  xmame_cfg_set_mixer_volume(XMAME_MIXER_ADPCM, vol); break;
    case VOL_FMGEN:  xmame_cfg_set_mixer_volume(XMAME_MIXER_FMGEN, vol); break;
    case VOL_SAMPLE: xmame_cfg_set_mixer_volume(XMAME_MIXER_SAMPLE, vol);break;
    }
}


static	Q8tkWidget	*menu_volume_unit(const t_volume *p, int count)
{
    int i;
    Q8tkWidget *vbox, *hbox;

    vbox = PACK_VBOX(NULL);
    {
	for (i=0; i<count; i++, p++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(p, 0));

		PACK_HSCALE(hbox,
			    p,
			    get_volume(p->val),
			    cb_volume, (void*)(p->val));
	    }
	}
    }

    return vbox;
}


static	Q8tkWidget	*menu_volume_total(void)
{
    return menu_volume_unit(data_volume_total, COUNTOF(data_volume_total));
}
static	Q8tkWidget	*menu_volume_level(void)
{
    return menu_volume_unit(data_volume_level, COUNTOF(data_volume_level));
}
static	Q8tkWidget	*menu_volume_rhythm(void)
{
    return menu_volume_unit(data_volume_rhythm, COUNTOF(data_volume_rhythm));
}
static	Q8tkWidget	*menu_volume_fmgen(void)
{
    return menu_volume_unit(data_volume_fmgen, COUNTOF(data_volume_fmgen));
}
static	Q8tkWidget	*menu_volume_sample(void)
{
    return menu_volume_unit(data_volume_sample, COUNTOF(data_volume_sample));
}
#endif
/*----------------------------------------------------------------------*/
					    /* サウンドなし時メッセージ */

static	Q8tkWidget	*menu_volume_no_available(void)
{
    int type;
    Q8tkWidget *l;

#ifdef	USE_SOUND
    type = 2;
#else
    type = 0;
#endif

    if (sound_board == SOUND_II) {
	type |= 1;
    }

    l = q8tk_label_new(GET_LABEL(data_volume_no, type));

    q8tk_widget_show(l);

    return l;
}

/*----------------------------------------------------------------------*/
					    /* サウンドドライバ種別表示 */
#ifdef	USE_SOUND
static	Q8tkWidget	*menu_volume_type(void)
{
    int type;
    Q8tkWidget *l;

#ifdef	USE_FMGEN
    if (xmame_cfg_get_use_fmgen()) {
	type = 2;
    } else
#endif
    {
	type = 0;
    }

    if (sound_board == SOUND_II) {
	type |= 1;
    }

    l = q8tk_label_new(GET_LABEL(data_volume_type, type));

    q8tk_widget_show(l);

    return l;
}
#endif

/*----------------------------------------------------------------------*/
							/* サウンド詳細 */
#ifdef	USE_SOUND
static	void	audio_create(void);
static	void	audio_start(void);
static	void	audio_finish(void);

static	void	cb_volume_audio(UNUSED_WIDGET, UNUSED_PARM)
{
    audio_start();
}

static	Q8tkWidget	*menu_volume_audio(void)
{
    Q8tkWidget *hbox;

    hbox = PACK_HBOX(NULL);
    {
	PACK_LABEL(hbox, " ");			/* 空行 */
	PACK_BUTTON(hbox,
		    GET_LABEL(data_volume, DATA_VOLUME_AUDIO),
		    cb_volume_audio, NULL);
	PACK_LABEL(hbox, " ");			/* 空行 */
    }
    return hbox;
}
#endif

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_volume(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2;
    Q8tkWidget *w;
    const t_menulabel *l = data_volume;

    if (xmame_has_sound() == FALSE) {

	w = PACK_FRAME(NULL, "", menu_volume_no_available());
	q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_ETCHED_OUT);

	return w;
    }


#ifdef	USE_SOUND
    audio_create();			/* サウンド詳細ウインドウ生成 */

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(NULL);
	{
	    w = PACK_FRAME(hbox, "", menu_volume_type());
	    q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_ETCHED_OUT);

	    PACK_LABEL(hbox, "  ");

	    PACK_BUTTON(hbox, GET_LABEL(data_volume, DATA_VOLUME_AUDIO),
			cb_volume_audio, NULL);
	}
	q8tk_box_pack_start(vbox, hbox);

	if (xmame_has_mastervolume()) {
	    PACK_FRAME(vbox,
		       GET_LABEL(l, DATA_VOLUME_TOTAL), menu_volume_total());
	}

	vbox2 = PACK_VBOX(NULL);
	{
#ifdef	USE_FMGEN
	    if (xmame_cfg_get_use_fmgen()) {
		w = menu_volume_fmgen();
	    }
	    else
#endif
	    {
		w = menu_volume_level();
	    }
	    q8tk_box_pack_start(vbox2, w);
	    
	    if (xmame_cfg_get_use_samples()) {
		q8tk_box_pack_start(vbox2, menu_volume_sample());
	    }
	}
	PACK_FRAME(vbox, GET_LABEL(l, DATA_VOLUME_LEVEL), vbox2);

#ifdef	USE_FMGEN
	if (xmame_cfg_get_use_fmgen()) {
	    ;
	}
	else
#endif
	if (sound_board == SOUND_II) {
	    PACK_FRAME(vbox,
		       GET_LABEL(l, DATA_VOLUME_DEPEND), menu_volume_rhythm());
	}

	if (xmame_has_audiodevice() == FALSE) {
	    PACK_LABEL(vbox, "");
	    PACK_LABEL(vbox, GET_LABEL(data_volume_audiodevice_stop, 0));
	}
    }

    return vbox;
#endif
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	サブウインドウ	AUDIO
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */
#ifdef	USE_SOUND

static	Q8tkWidget	*audio_window;
static	Q8tkWidget	*audio[6];
static	Q8tkWidget	*audio_accel;

enum {
    AUDIO_WIN,
    AUDIO_FRAME,
    AUDIO_VBOX,
    AUDIO_HBOX,
    AUDIO_BUTTON,
    AUDIO_LABEL
};

/*----------------------------------------------------------------------*/
						      /* FM音源ジェネレータ */
#ifdef	USE_FMGEN
static	int	get_volume_audio_fmgen(void)
{
    return sd_cfg_now.use_fmgen;
}
static	void	cb_volume_audio_fmgen(UNUSED_WIDGET, void *p)
{
    sd_cfg_now.use_fmgen = xmame_cfg_set_use_fmgen((int)p);
}


static	Q8tkWidget	*volume_audio_fmgen(void)
{
    Q8tkWidget *box;
    const t_menulabel *l = data_volume_audio;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, GET_LABEL(l, DATA_VOLUME_AUDIO_FMGEN));

	PACK_RADIO_BUTTONS(box,
		    data_volume_audio_fmgen, COUNTOF(data_volume_audio_fmgen),
		    get_volume_audio_fmgen(), cb_volume_audio_fmgen);
    }

    return box;
}
#endif

/*----------------------------------------------------------------------*/
						      /* サンプル周波数 */
static	int	get_volume_audio_freq(void)
{
    return sd_cfg_now.sample_freq;
}
static	void	cb_volume_audio_freq(Q8tkWidget *widget, void *mode)
{
    int i;
    const t_menudata *p = data_volume_audio_freq_combo;
    const char       *combo_str = q8tk_combo_get_text(widget);
    char buf[16], *conv_end;
    int val = 0;
    int fit = FALSE;

    /* COMBO BOX から ENTRY に一致するものを探す */
    for (i=0; i<COUNTOF(data_volume_audio_freq_combo); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    val = p->val;
	    fit = TRUE;					/* 一致した値を適用 */
	    break;
	}
    }

    if (fit == FALSE) {			/* COMBO BOX に該当がない場合 */
	strncpy(buf, combo_str, 15);
	buf[15] = '\0';

	val = strtoul(buf, &conv_end, 10);

	if (((int)mode == 0) &&				/* 空白 + ENTER や   */
	    (strlen(buf) == 0 || val == 0)) {		/*   0  + ENTER 時は */
							/* デフォルト値を適用*/
	    val = 44100;
	    fit = TRUE;

	} else if (*conv_end != '\0') {			/* 数字変換失敗なら */
	    fit = FALSE;				/* その値は使えない */

	} else {					/* 数字変換成功なら */
	    fit = TRUE;					/* その値を適用する */
	}
    }

    if (fit) {				/* 適用した値が有効範囲なら、セット */
	if (8000 <= val && val <= 48000) {
	    sd_cfg_now.sample_freq = xmame_cfg_set_sample_freq(val);
	}
    }

    if ((int)mode == 0) {		/* COMBO ないし ENTER時は、値を再表示*/
	sprintf(buf, "%5d", get_volume_audio_freq());
	q8tk_combo_set_text(widget, buf);
    }
}


static	Q8tkWidget	*volume_audio_freq(void)
{
    Q8tkWidget *box;
    char buf[16];
    const t_menulabel *l = data_volume_audio;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, GET_LABEL(l, DATA_VOLUME_AUDIO_FREQ));

	sprintf(buf, "%5d", get_volume_audio_freq());
	PACK_COMBO(box,
		   data_volume_audio_freq_combo,
		   COUNTOF(data_volume_audio_freq_combo),
		   get_volume_audio_freq(), buf, 6,
		   cb_volume_audio_freq, (void*)0,
		   cb_volume_audio_freq, (void*)1);
    }

    return box;
}

/*----------------------------------------------------------------------*/
						      /* サンプル音 */
static	int	get_volume_audio_sample(void)
{
    return sd_cfg_now.use_samples;
}
static	void	cb_volume_audio_sample(UNUSED_WIDGET, void *p)
{
    sd_cfg_now.use_samples = xmame_cfg_set_use_samples((int)p);
}


static	Q8tkWidget	*volume_audio_sample(void)
{
    Q8tkWidget *box;
    const t_menulabel *l = data_volume_audio;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, GET_LABEL(l, DATA_VOLUME_AUDIO_SAMPLE));

	PACK_RADIO_BUTTONS(box,
		  data_volume_audio_sample, COUNTOF(data_volume_audio_sample),
		  get_volume_audio_sample(), cb_volume_audio_sample);
    }

    return box;
}

/*----------------------------------------------------------------------*/

static	void	cb_audio_config(Q8tkWidget *widget, void *modes)
{
    int index = ((int)modes) / 2;
    int mode  = ((int)modes) & 1;	/* 0:ENTER 1:INPUT */

    T_SNDDRV_CONFIG *p       = sd_cfg_now.local[index].info;
    SD_CFG_LOCAL_VAL def_val = sd_cfg_now.local[index].val;

    const char       *entry_str = q8tk_entry_get_text(widget);
    char buf[16], *conv_end;
    SD_CFG_LOCAL_VAL val = { 0 };
    int fit = FALSE;
    int zero = FALSE;

    strncpy(buf, entry_str, 15);
    buf[15] = '\0';

    switch (p->type) {
    case SNDDRV_INT:
	val.i = (int)strtoul(buf, &conv_end, 10);
	if (val.i == 0) zero = TRUE;
	break;

    case SNDDRV_FLOAT:
	val.f = (float)strtod(buf, &conv_end);
	if (val.f == 0) zero = TRUE;
	break;
    }

    if (((int)mode == 0) &&				/* 空白 + ENTER や   */
	(strlen(buf) == 0 || zero)) {			/*   0  + ENTER 時は */
							/* 直前の有効値を適用*/
	val = def_val;
	fit = TRUE;

    } else if (*conv_end != '\0') {			/* 数字変換失敗なら */
	fit = FALSE;					/* その値は使えない */

    } else {						/* 数字変換成功なら */
	fit = TRUE;					/* その値を適用する */
    }

    if (fit) {				/* 適用した値が有効範囲なら、セット */
	switch (p->type) {
	case SNDDRV_INT:
	    if ((int)(p->low) <=val.i && val.i <= (int)(p->high)) {
		sd_cfg_now.local[index].val.i = 
		    *((int *)(p->work)) = val.i;
	    }
	    break;

	case SNDDRV_FLOAT:
	    if ((float)(p->low) <=val.f && val.f <= (float)(p->high)) {
		sd_cfg_now.local[index].val.f = 
		    *((float *)(p->work)) = val.f;
	    }
	    break;
	}
    }

    if ((int)mode == 0) {		/* COMBO ないし ENTER時は、値を再表示*/
	switch (p->type) {
	case SNDDRV_INT:
	    sprintf(buf, "%7d", sd_cfg_now.local[index].val.i);
	    break;

	case SNDDRV_FLOAT:
	    sprintf(buf, "%7.3f", sd_cfg_now.local[index].val.f);
	    break;
	}

	q8tk_entry_set_text(widget, buf);
    }

    /*if(p->type==SNDDRV_INT)  printf("%d\n", *((int *)(p->work)));*/
    /*if(p->type==SNDDRV_FLOAT)printf("%f\n", *((float *)(p->work)));fflush(stdout);*/
}

static	int	audio_config_widget(Q8tkWidget *box)
{
    Q8tkWidget *hbox;
    char buf[32];
    int i;
    T_SNDDRV_CONFIG *p;

    for (i=0; i<sd_cfg_now.local_cnt; i++) {

	p = sd_cfg_now.local[i].info;

	switch (p->type) {
	case SNDDRV_INT:
	    sprintf(buf, "%7d", sd_cfg_now.local[i].val.i);
	    break;

	case SNDDRV_FLOAT:
	    sprintf(buf, "%7.3f", sd_cfg_now.local[i].val.f);
	    break;
	}

	hbox = PACK_HBOX(NULL);
	{
	    PACK_LABEL(hbox, p->title);

	    PACK_ENTRY(hbox,
		       8, 9, buf,
		       cb_audio_config, (void *)(i*2),
		       cb_audio_config, (void *)(i*2 + 1));
	}
	q8tk_box_pack_start(box, hbox);
	PACK_LABEL(box, "");
    }

    return i;
}


/*----------------------------------------------------------------------*/

static	void	audio_create(void)
{
    int i;
    Q8tkWidget *vbox;
    const t_menulabel *l = data_volume;

    vbox = PACK_VBOX(NULL);
    {
	PACK_HSEP(vbox);

#ifdef	USE_FMGEN
	q8tk_box_pack_start(vbox, volume_audio_fmgen());
	PACK_HSEP(vbox);
#else
	PACK_LABEL(vbox, "");
	PACK_LABEL(vbox, "");
#endif
	q8tk_box_pack_start(vbox, volume_audio_freq());
	PACK_LABEL(vbox, "");
	q8tk_box_pack_start(vbox, volume_audio_sample());
	PACK_HSEP(vbox);

	i = audio_config_widget(vbox);

	for ( ; i<5; i++) {
	    PACK_LABEL(vbox, "");
	    PACK_LABEL(vbox, "");
	}
	PACK_HSEP(vbox);
    }

    audio_window = vbox;
}

static	void	cb_volume_audio_end(UNUSED_WIDGET, UNUSED_PARM)
{
    audio_finish();
}

static	void	audio_start(void)
{
    Q8tkWidget *w, *f, *v, *h, *b, *l;
    const t_menulabel *p = data_volume;

    {						/* メインとなるウインドウ */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	audio_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(audio_accel, w);
    }
    {						/* に、フレームを乗せて */
	f = q8tk_frame_new(GET_LABEL(p, DATA_VOLUME_AUDIO_SET));
	q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_OUT);
	q8tk_container_add(w, f);
	q8tk_widget_show(f);
    }
    {						/* それにボックスを乗せる */
	v = q8tk_vbox_new();
	q8tk_container_add(f, v);
	q8tk_widget_show(v);
							/* ボックスには     */
	{						/* AUDIOメニュー と */
	    q8tk_box_pack_start(v, audio_window);
	}
	{						/* さらにボックス   */
	    h = q8tk_hbox_new();
	    q8tk_box_pack_start(v, h);
	    q8tk_widget_show(h);
							/* ボックスには     */
	    {						/* 終了ボタンを配置 */
		b = PACK_BUTTON(h,
				GET_LABEL(p, DATA_VOLUME_AUDIO_QUIT),
				cb_volume_audio_end, NULL);

		q8tk_accel_group_add(audio_accel, Q8TK_KEY_ESC, b, "clicked");


							/* ラベルも配置 */
		l = PACK_LABEL(h, GET_LABEL(p, DATA_VOLUME_AUDIO_INFO));
		q8tk_misc_set_placement(l, 0, Q8TK_PLACEMENT_Y_CENTER);
	    }
	}
    }

    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b);


    audio[ AUDIO_WIN    ] = w;	/* ダイアログを閉じたときに備えて */
    audio[ AUDIO_FRAME  ] = f;	/* ウィジットを覚えておきます     */
    audio[ AUDIO_VBOX   ] = v;
    audio[ AUDIO_HBOX   ] = h;
    audio[ AUDIO_BUTTON ] = b;
    audio[ AUDIO_LABEL  ] = l;
}

/* サウンド詳細ウインドウの消去 */

static	void	audio_finish(void)
{
    q8tk_widget_destroy(audio[ AUDIO_LABEL  ]);
    q8tk_widget_destroy(audio[ AUDIO_BUTTON ]);
    q8tk_widget_destroy(audio[ AUDIO_HBOX   ]);
    q8tk_widget_destroy(audio[ AUDIO_VBOX   ]);
    q8tk_widget_destroy(audio[ AUDIO_FRAME  ]);

    q8tk_grab_remove(audio[ AUDIO_WIN ]);
    q8tk_widget_destroy(audio[ AUDIO_WIN ]);
    q8tk_widget_destroy(audio_accel);
}

#endif



/*===========================================================================
 *
 *	メインページ	ディスク
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/

typedef struct{
    Q8tkWidget	*list;			/* イメージ一覧のリスト	*/
/*  Q8tkWidget	*button[2];		 * ボタンと		*/
    Q8tkWidget	*label[2];		/* そのラベル (2個)	*/
    int		func[2];		/* ボタンの機能 IMG_xxx	*/
    Q8tkWidget	*stat_label;		/* 情報 - Busy/Ready	*/
    Q8tkWidget	*attr_label;		/* 情報 - RO/RW属性	*/
    Q8tkWidget	*num_label;		/* 情報 - イメージ数	*/
} T_DISK_INFO;

static	T_DISK_INFO	disk_info[2];	/* 2ドライブ分のワーク	*/

static	char		disk_filename[ QUASI88_MAX_FILENAME ];

static	int		disk_drv;	/* 操作するドライブの番号 */
static	int		disk_img;	/* 操作するイメージの番号 */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

static	void	set_disk_widget(void);


/* BOOT from DISK で、DISK を CLOSE した時や、
   BOOT from ROM  で、DISK を OPEN した時は、 DIP-SW 設定を強制変更 */
static	void	disk_update_dipsw_b_boot(void)
{
    if (disk_image_exist(0) || disk_image_exist(1)) {
	q8tk_toggle_button_set_state(widget_dipsw_b_boot_disk, TRUE);
    } else {
	q8tk_toggle_button_set_state(widget_dipsw_b_boot_rom,  TRUE);
    }
    set_reset_dipsw_boot();

    /* リセットしないでメニューモードを抜けると設定が保存されないので・・・ */
    boot_from_rom = reset_req.boot_from_rom;		/* thanks floi ! */
}


/*----------------------------------------------------------------------*/
/* 属性変更の各種処理							*/

enum {
    ATTR_RENAME,	/* drive[drv] のイメージ img をリネーム		*/
    ATTR_PROTECT,	/* drive[drv] のイメージ img をプロテクト	*/
    ATTR_FORMAT,	/* drive[drv] のイメージ img をアンフォーマット	*/
    ATTR_UNFORMAT,	/* drive[drv] のイメージ img をフォーマット	*/
    ATTR_APPEND,	/* drive[drv] に最後にイメージを追加		*/
    ATTR_CREATE		/* 新規にディスクイメージファイルを作成		*/
};

static	void	sub_disk_attr_file_ctrl(int drv, int img, int cmd, char *c)
{
    int ro = FALSE;
    int result = -1;
    OSD_FILE *fp;


    if (cmd != ATTR_CREATE) {		/* ドライブのファイルを変更する場合 */

	fp = drive[ drv ].fp;			/* そのファイルポインタを取得*/
	if (drive[ drv ].read_only) {
	    ro = TRUE;
	}

    } else {				/* 別のファイルを更新する場合 */

	fp = osd_fopen(FTYPE_DISK, c, "r+b");		/* "r+b" でオープン */
	if (fp == NULL) {
	    fp = osd_fopen(FTYPE_DISK, c, "rb");	/* "rb" でオープン */
	    if (fp) ro = TRUE;
	}

	if (fp) {					/* オープンできたら */
	    if      (fp == drive[ 0 ].fp) drv = 0;	/* すでにドライブに */
	    else if (fp == drive[ 1 ].fp) drv = 1;	/* 開いてないかを   */
	    else                          drv = -1;	/* チェックする     */
	}
	else {						/* オープンできない */
	    fp = osd_fopen(FTYPE_DISK, c, "ab");	/* 時は、新規に作成 */
	    drv = -1;
	}

    }


    if (fp == NULL) {			/* オープン失敗 */
	start_file_error_dialog(drv, ERR_CANT_OPEN);
	return;
    }
    else if (ro) {			/* リードオンリーなので処理不可 */
	if (drv < 0) osd_fclose(fp);
	if (cmd != ATTR_CREATE) start_file_error_dialog(drv, ERR_READ_ONLY);
	else                    start_file_error_dialog( -1, ERR_READ_ONLY);
	return;
    }
    else if (drv>=0 &&			/* 壊れたイメージが含まれるので不可 */
	     drive[ drv ].detect_broken_image) {
	start_file_error_dialog(drv, ERR_MAYBE_BROKEN);
	return;
    }


#if 0
    if (cmd == ATTR_CREATE || cmd == ATTR_APPEND) {
	/* この処理に時間がかかるような場合、メッセージをだす？？ */
	/* この処理がそんなに時間がかかることはない？？ */
    }
#endif

		/* 開いたファイルに対して、処理 */

    switch (cmd) {
    case ATTR_RENAME:	result = d88_write_name(fp, drv, img, c);	break;
    case ATTR_PROTECT:	result = d88_write_protect(fp, drv, img, c);	break;
    case ATTR_FORMAT:	result = d88_write_format(fp, drv, img);	break;
    case ATTR_UNFORMAT:	result = d88_write_unformat(fp, drv, img);	break;
    case ATTR_APPEND:
    case ATTR_CREATE:	result = d88_append_blank(fp, drv);		break;
    }

		/* その結果 */

    switch (result) {
    case D88_SUCCESS:	result = ERR_NO;			break;
    case D88_NO_IMAGE:	result = ERR_MAYBE_BROKEN;		break;
    case D88_BAD_IMAGE:	result = ERR_MAYBE_BROKEN;		break;
    case D88_ERR_READ:	result = ERR_MAYBE_BROKEN;		break;
    case D88_ERR_SEEK:	result = ERR_SEEK;			break;
    case D88_ERR_WRITE:	result = ERR_WRITE;			break;
    default:		result = ERR_UNEXPECTED;		break;
    }

		/* 終了処理。なお、エラー時はメッセージを出す */

    if (drv < 0) {		/* 新規オープンしたファイルを更新した場合 */
	osd_fclose(fp);			/* ファイルを閉じて終わり	  */

    } else {			/* ドライブのファイルを更新した場合	  */
	if (result == ERR_NO) {		/* メニュー画面を更新せねば	  */
	    set_disk_widget();
	    if (cmd != ATTR_CREATE) disk_update_dipsw_b_boot();
	}
    }

    if (result != ERR_NO) {
	start_file_error_dialog(drv, result);
    }

    return;
}

/*----------------------------------------------------------------------*/
/* 「リネーム」ダイアログ						*/

static	void	cb_disk_attr_rename_activate(UNUSED_WIDGET, void *p)
{
    char wk[16 + 1];

    if ((int)p) {		/* dialog_destroy() の前にエントリをゲット */
	strncpy(wk, dialog_get_entry(), 16);
	wk[16] = '\0';
    }

    dialog_destroy();

    if ((int)p) {
	sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_RENAME, wk);
    }
}
static	void	sub_disk_attr_rename(const char *image_name)
{
    int save_code;
    const t_menulabel *l = data_disk_attr_rename;


    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_RENAME_TITLE1 +disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_RENAME_TITLE2));

	dialog_set_separator();

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_entry(drive[disk_drv].image[disk_img].name,
			 16,
			 cb_disk_attr_rename_activate, (void*)TRUE);
	q8tk_set_kanjicode(save_code);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_RENAME_OK),
			  cb_disk_attr_rename_activate, (void*)TRUE);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_RENAME_CANCEL),
			  cb_disk_attr_rename_activate, (void*)FALSE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* 「プロテクト」ダイアログ						*/

static	void	cb_disk_attr_protect_clicked(UNUSED_WIDGET, void *p)
{
    char c;

    dialog_destroy();

    if ((int)p) {
	if ((int)p == 1) c = DISK_PROTECT_TRUE;
	else             c = DISK_PROTECT_FALSE;

	sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_PROTECT, &c);
    }
}
static	void	sub_disk_attr_protect(const char *image_name)
{
    int save_code;
    const t_menulabel *l = data_disk_attr_protect;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_TITLE1+disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_TITLE2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_SET),
			  cb_disk_attr_protect_clicked, (void*)1);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_UNSET),
			  cb_disk_attr_protect_clicked, (void*)2);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT_CANCEL),
			  cb_disk_attr_protect_clicked, (void*)0);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* 「フォーマット」ダイアログ						*/

static	void	cb_disk_attr_format_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	if ((int)p == 1)
	    sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_FORMAT,   NULL);
	else 
	    sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_UNFORMAT, NULL);
    }
}
static	void	sub_disk_attr_format(const char *image_name)
{
    int save_code;
    const t_menulabel *l = data_disk_attr_format;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_TITLE1 +disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_TITLE2));

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_WARNING));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_DO),
			  cb_disk_attr_format_clicked, (void*)1);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_NOT),
			  cb_disk_attr_format_clicked, (void*)2);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT_CANCEL),
			  cb_disk_attr_format_clicked, (void*)0);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* 「ブランクディスク」ダイアログ					*/

static	void	cb_disk_attr_blank_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	sub_disk_attr_file_ctrl(disk_drv, disk_img, ATTR_APPEND, NULL);
    }
}
static	void	sub_disk_attr_blank(void)
{
    const t_menulabel *l = data_disk_attr_blank;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_BLANK_TITLE1 + disk_drv));

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_BLANK_TITLE2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_BLANK_OK),
			  cb_disk_attr_blank_clicked, (void*)TRUE);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_BLANK_CANCEL),
			  cb_disk_attr_blank_clicked, (void*)FALSE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
/* 「属性変更」 ボタン押下時の処理  -  詳細選択のダイアログを開く	*/

static char disk_attr_image_name[20];
static	void	cb_disk_attr_clicked(UNUSED_WIDGET, void *p)
{
    char *name = disk_attr_image_name;

    dialog_destroy();

    switch ((int)p) {
    case DATA_DISK_ATTR_RENAME:	 sub_disk_attr_rename(name);	break;
    case DATA_DISK_ATTR_PROTECT: sub_disk_attr_protect(name);	break;
    case DATA_DISK_ATTR_FORMAT:	 sub_disk_attr_format(name);	break;
    case DATA_DISK_ATTR_BLANK:	 sub_disk_attr_blank();		break;
    }
}


static void sub_disk_attr(void)
{
    int save_code;
    const t_menulabel *l = data_disk_attr;

    sprintf(disk_attr_image_name,		/* イメージ名をセット */
	    "\"%-16s\"", drive[disk_drv].image[disk_img].name);

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_TITLE1 + disk_drv));

	save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	dialog_set_title(disk_attr_image_name);
	q8tk_set_kanjicode(save_code);

	dialog_set_title(GET_LABEL(l, DATA_DISK_ATTR_TITLE2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_RENAME),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_RENAME);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_PROTECT),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_PROTECT);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_FORMAT),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_FORMAT);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_BLANK),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_BLANK);

	dialog_set_button(GET_LABEL(l, DATA_DISK_ATTR_CANCEL),
			  cb_disk_attr_clicked, (void*)DATA_DISK_ATTR_CANCEL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}




/*----------------------------------------------------------------------*/
/* 「イメージファイルを開く」 ボタン押下時の処理			*/

static	int	disk_open_ro;
static	int	disk_open_cmd;
static void sub_disk_open_ok(void);

static void sub_disk_open(int cmd)
{
    const char *initial;
    int num;
    const t_menulabel *l = (disk_drv == 0) ? data_disk_open_drv1
					   : data_disk_open_drv2;

    disk_open_cmd = cmd;
    num = (cmd == IMG_OPEN) ? DATA_DISK_OPEN_OPEN : DATA_DISK_OPEN_BOTH;

    /* ディスクがあればそのファイルを、なければディスク用ディレクトリを取得 */
    initial = filename_get_disk_or_dir(disk_drv);

    START_FILE_SELECTION(GET_LABEL(l, num),
			 (menu_readonly) ? 1 : 0,     /* ReadOnly の選択が可 */
			 initial,

			 sub_disk_open_ok,
			 disk_filename,
			 QUASI88_MAX_FILENAME,
			 &disk_open_ro);
}

static void sub_disk_open_ok(void)
{
    if (disk_open_cmd == IMG_OPEN) {

	if (quasi88_disk_insert(disk_drv, disk_filename, 0, disk_open_ro)
								    == FALSE) {
	    start_file_error_dialog(disk_drv, ERR_CANT_OPEN);
	}
	else {
	    if (disk_same_file()) {	/* 反対側と同じファイルだった場合 */
		int dst = disk_drv;
		int src = disk_drv ^ 1;
		int img;

		if (drive[ src ].empty) {	    /* 反対側ドライブ 空なら */
		    img = 0;			    /*        最初のイメージ */
		} else {
		    if (disk_image_num(src) == 1) { /* イメージが1個の場合は */
			img = -1;		    /*        ドライブ 空に  */

		    } else {			    /* イメージが複数あれば  */
						    /*        次(前)イメージ */
			img = disk_image_selected(src)
						+ ((dst == DRIVE_1) ? -1 : +1);
			if ((img < 0) || 
			    (disk_image_num(dst) - 1 < img)) img = -1;
		    }
		}
		if (img < 0) drive_set_empty(dst);
		else         disk_change_image(dst, img);
	    }
	}

    } else {	/*   IMG_BOTH */

	if (quasi88_disk_insert_all(disk_filename, disk_open_ro) == FALSE) {

	    disk_drv = 0;
	    start_file_error_dialog(disk_drv, ERR_CANT_OPEN);

	}

    }

    if (filename_synchronize) {
	sub_misc_suspend_update();
	sub_misc_snapshot_update();
	sub_misc_waveout_update();
    }
    set_disk_widget();
    disk_update_dipsw_b_boot();
}

/*----------------------------------------------------------------------*/
/* 「イメージファイルを閉じる」 ボタン押下時の処理			*/

static void sub_disk_close(void)
{
    quasi88_disk_eject(disk_drv);

    if (filename_synchronize) {
	sub_misc_suspend_update();
	sub_misc_snapshot_update();
	sub_misc_waveout_update();
    }
    set_disk_widget();
    disk_update_dipsw_b_boot();
}

/*----------------------------------------------------------------------*/
/* 「反対ドライブと同じファイルを開く」 ボタン押下時の処理		*/

static void sub_disk_copy(void)
{
    int dst = disk_drv;
    int src = disk_drv ^ 1;
    int img;

    if (! disk_image_exist(src)) return;

    if (drive[ src ].empty) {			/* 反対側ドライブ 空なら */
	img = 0;				/*        最初のイメージ */
    } else {
	if (disk_image_num(src) == 1) {		/* イメージが1個の場合は */
	    img = -1;				/*        ドライブ 空に  */

	} else {				/* イメージが複数あれば  */
						/*        次(前)イメージ */
	    img = disk_image_selected(src) + ((dst == DRIVE_1) ? -1 : +1);
	    if ((img < 0) || 
		(disk_image_num(dst) -1 < img)) img = -1;
	}
    }

    if (quasi88_disk_insert_A_to_B(src, dst, img) == FALSE) {
	start_file_error_dialog(disk_drv, ERR_CANT_OPEN);
    }

    if (filename_synchronize) {
	sub_misc_suspend_update();
	sub_misc_snapshot_update();
	sub_misc_waveout_update();
    }
    set_disk_widget();
    disk_update_dipsw_b_boot();
}





/*----------------------------------------------------------------------*/
/* イメージのリストアイテム選択時の、コールバック関数			*/

static	void	cb_disk_image(UNUSED_WIDGET, void *p)
{
    int drv = ((int)p) & 0xff;
    int img = ((int)p) >> 8;

    if (img < 0) {			/* img == -1 で <<なし>> */
	drive_set_empty(drv);
    } else {				/* img >= 0 なら イメージ番号 */
	drive_unset_empty(drv);
	disk_change_image(drv, img);
    }
}

/*----------------------------------------------------------------------*/
/* ドライブ毎に存在するボタンの、コールバック関数			*/

static	void	cb_disk_button(UNUSED_WIDGET, void *p)
{
    int drv    = ((int)p) & 0xff;
    int button = ((int)p) >> 8;

    disk_drv = drv;
    disk_img = disk_image_selected(drv);

    switch (disk_info[drv].func[button]) {
    case IMG_OPEN:
    case IMG_BOTH:
	sub_disk_open(disk_info[drv].func[button]);
	break;
    case IMG_CLOSE:
	sub_disk_close();
	break;
    case IMG_COPY:
	sub_disk_copy();
	break;
    case IMG_ATTR:
	if (! drive_check_empty(drv)) {	     /* イメージ<<なし>>選択時は無効 */
	    sub_disk_attr();
	}
	break;
    }
}

/*----------------------------------------------------------------------*/
/* ファイルを開く毎に、disk_info[] に情報をセット			*/
/*		(イメージのリスト生成、ボタン・情報のラベルをセット)	*/

static	void	set_disk_widget(void)
{
    int i, drv, save_code;
    Q8tkWidget *item;
    T_DISK_INFO *w;
    const t_menulabel *inf = data_disk_info;
    const t_menulabel *l   = data_disk_image;
    const t_menulabel *btn;
    char wk[40], wk2[20];
    const char *s;


    for (drv=0; drv<2; drv++) {
	w = &disk_info[drv];

	if (menu_swapdrv) {
	    btn = (drv == 0) ? data_disk_button_drv1swap
			     : data_disk_button_drv2swap;
	} else {
	    btn = (drv == 0) ? data_disk_button_drv1
			     : data_disk_button_drv2;
	}

		/* イメージ名の LIST ITEM 生成 */

	q8tk_listbox_clear_items(w->list, 0, -1);

	item = q8tk_list_item_new_with_label(GET_LABEL(l,
						       DATA_DISK_IMAGE_EMPTY));
	q8tk_widget_show(item);
	q8tk_container_add(w->list, item);		/* <<なし>> ITEM */
	q8tk_signal_connect(item, "select",
			    cb_disk_image, (void *)((-1 << 8) + drv));

	if (disk_image_exist(drv)) {		/* ---- ディスク挿入済 ---- */
	    save_code = q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	    {
		for (i=0; i<disk_image_num(drv); i++) {
		    sprintf(wk, "%3d  %-16s  %s ", /*イメージNo イメージ名 RW*/
			    i+1,
			    drive[drv].image[i].name,
			    (drive[drv].image[i].protect) ? "RO" : "RW");

		    item = q8tk_list_item_new_with_label(wk);
		    q8tk_widget_show(item);
		    q8tk_container_add(w->list, item);
		    q8tk_signal_connect(item, "select",
					cb_disk_image, (void *)((i<<8) + drv));
		}
	    }
	    q8tk_set_kanjicode(save_code);

				/* <<なし>> or 選択image の ITEM をセレクト */
	    if (drive_check_empty(drv)) i = 0;
	    else                        i = disk_image_selected(drv) + 1;
	    q8tk_listbox_select_item(w->list, i);

	} else {				/* ---- ドライブ空っぽ ---- */
	    q8tk_listbox_select_item(w->list, 0);	    /* <<なし>> ITEM */
	}

		/* ボタンの機能 「閉じる」「属性変更」 / 「開く」「開く」 */

	if (disk_image_exist(drv)) {
	    w->func[0] = IMG_CLOSE;
	    w->func[1] = IMG_ATTR;
	} else {
	    w->func[0] = (disk_image_exist(drv^1)) ? IMG_COPY : IMG_BOTH;
	    w->func[1] = IMG_OPEN;
	}
	q8tk_label_set(w->label[0], GET_LABEL(btn, w->func[0]));
	q8tk_label_set(w->label[1], GET_LABEL(btn, w->func[1]));

		/* 情報 - Busy/Ready */

	if (get_drive_ready(drv)) s = GET_LABEL(inf,DATA_DISK_INFO_STAT_READY);
	else                      s = GET_LABEL(inf,DATA_DISK_INFO_STAT_BUSY);
	q8tk_label_set(w->stat_label, s);
	q8tk_label_set_reverse(w->stat_label,	/* BUSYなら反転表示 */
			       (get_drive_ready(drv)) ? FALSE : TRUE);

		/* 情報 - RO/RW属性 */

	if (disk_image_exist(drv)) {
	    if (drive[drv].read_only) s =GET_LABEL(inf,DATA_DISK_INFO_ATTR_RO);
	    else                      s =GET_LABEL(inf,DATA_DISK_INFO_ATTR_RW);
	} else {
	    s = "";
	}
	q8tk_label_set(w->attr_label, s);
	q8tk_label_set_color(w->attr_label,	/* ReadOnlyなら赤色表示 */
			     (drive[drv].read_only) ? Q8GR_PALETTE_RED : -1);

		/* 情報 - イメージ数 */

	if (disk_image_exist(drv)) {
	    if (drive[drv].detect_broken_image) {	/* 破損あり */
		s = GET_LABEL(inf, DATA_DISK_INFO_NR_BROKEN);
	    } else
	    if (drive[drv].over_image ||		/* イメージ多過ぎ */
		disk_image_num(drv) > 99) {
		s = GET_LABEL(inf, DATA_DISK_INFO_NR_OVER);
	    } else {
		s = "";
	    }
	    sprintf(wk, "%2d%s",
		    (disk_image_num(drv)>99) ? 99 : disk_image_num(drv), s);
	    sprintf(wk2, "%9.9s", wk);			/* 9文字右詰めに変換 */
	} else {
	    wk2[0] = '\0';
	}
	q8tk_label_set(w->num_label,  wk2);
    }
}


/*----------------------------------------------------------------------*/
/* 「ブランク作成」 ボタン押下時の処理					*/

static	void	sub_disk_blank_ok(void);
static	void	cb_disk_blank_warn_clicked(Q8tkWidget *, void *);


static	void	cb_disk_blank(UNUSED_WIDGET, UNUSED_PARM)
{
    const char *initial;
    const t_menulabel *l = data_disk_blank;

    /* ディスクがあればそのファイルを、なければディスク用ディレクトリを取得 */
    initial = filename_get_disk_or_dir(DRIVE_1);

    START_FILE_SELECTION(GET_LABEL(l, DATA_DISK_BLANK_FSEL),
			 -1,	/* ReadOnly の選択は不可 */
			 initial,

			 sub_disk_blank_ok,
			 disk_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static	void	sub_disk_blank_ok(void)
{
    const t_menulabel *l = data_disk_blank;

    switch (osd_file_stat(disk_filename)) {

    case FILE_STAT_NOEXIST:
	/* ファイルを新規に作成し、ブランクを作成 */
	sub_disk_attr_file_ctrl(0, 0, ATTR_CREATE, disk_filename);
	break;

    case FILE_STAT_DIR:
	/* ディレクトリなので、ブランクは追加できない */
	start_file_error_dialog(-1, ERR_CANT_OPEN);
	break;

    default:
	/* すでにファイルが存在します。ブランクを追加しますか？ */
	dialog_create();
	{
	    dialog_set_title(GET_LABEL(l, DATA_DISK_BLANK_WARN_0));

	    dialog_set_title(GET_LABEL(l, DATA_DISK_BLANK_WARN_1));

	    dialog_set_separator();

	    dialog_set_button(GET_LABEL(l, DATA_DISK_BLANK_WARN_APPEND),
			      cb_disk_blank_warn_clicked, (void*)TRUE);

	    dialog_set_button(GET_LABEL(l, DATA_DISK_BLANK_WARN_CANCEL),
			      cb_disk_blank_warn_clicked, (void*)FALSE);

	    dialog_accel_key(Q8TK_KEY_ESC);
	}
	dialog_start();
	break;
    }
}

static	void	cb_disk_blank_warn_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	/* ファイルに、ブランクを追記 */
	sub_disk_attr_file_ctrl(0, 0, ATTR_CREATE, disk_filename);
    }
}

/*----------------------------------------------------------------------*/
/* 「ファイル名確認」 ボタン押下時の処理				*/

static	void	cb_disk_fname_dialog_ok(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	void	cb_disk_fname(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_disk_fname;
    char filename[66 +5 +1];		/* 5 == strlen("[1:] "), 1 は '\0' */
    int save_code;
    int i, width, len;
    const char *ptr[2];
    const char *none = "(No Image File)";

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_TITLE));
	dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_LINE));

	{
	    save_code = q8tk_set_kanjicode(osd_kanji_code());

	    width = 0;
	    for (i=0; i<2; i++) {
		if ((ptr[i] = filename_get_disk(i)) == NULL) { ptr[i] = none; }
		len = sprintf(filename, "%.66s", ptr[i]);	/* == max 66 */
		width = MAX(width, len);
	    }

	    for (i=0; i<2; i++) {
		sprintf(filename, "[%d:] %-*.*s", i+1, width, width, ptr[i]);
		dialog_set_title(filename);
	    }

	    q8tk_set_kanjicode(save_code);
	}

	if (disk_image_exist(0) && disk_same_file()) {
	    dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_SAME));
	}

	if ((disk_image_exist(0) && drive[0].read_only) ||
	    (disk_image_exist(1) && drive[1].read_only)) {
	    dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_SEP));
	    dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO));

	    if (fdc_ignore_readonly == FALSE) {
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_1));
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_2));
	    } else {
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_X));
		dialog_set_title(GET_LABEL(l, DATA_DISK_FNAME_RO_Y));
	    }
	}


	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_FNAME_OK),
			  cb_disk_fname_dialog_ok, NULL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}


/*----------------------------------------------------------------------*/
					/* ドライブ表示位置 左右入れ換え */

static	void	cb_disk_dispswap_clicked(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	int	get_disk_dispswap(void)
{
    return menu_swapdrv;
}
static	void	cb_disk_dispswap(Q8tkWidget *widget, UNUSED_PARM)
{
    const t_menulabel *l = data_disk_dispswap;
    int parm = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    menu_swapdrv = parm;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_DISK_DISPSWAP_INFO_1));
	dialog_set_title(GET_LABEL(l, DATA_DISK_DISPSWAP_INFO_2));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_DISK_DISPSWAP_OK),
			  cb_disk_dispswap_clicked, NULL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*----------------------------------------------------------------------*/
					/* ステータスにイメージ名表示 */

static	void	cb_disk_dispstatus_clicked(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
}

static	int	get_disk_dispstatus(void)
{
    return status_imagename;
}
static	void	cb_disk_dispstatus(Q8tkWidget *widget, UNUSED_PARM)
{
    const t_menulabel *l = data_disk_dispstatus;
    int parm = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    status_imagename = parm;

    if (status_imagename) {
	/* オンにした時のみ、説明を表示 */
	dialog_create();
	{
	    dialog_set_title(GET_LABEL(l, DATA_DISK_DISPSTATUS_INFO));

	    dialog_set_separator();

	    dialog_set_button(GET_LABEL(l, DATA_DISK_DISPSTATUS_OK),
			      cb_disk_dispstatus_clicked, NULL);

	    dialog_accel_key(Q8TK_KEY_ESC);
	}
	dialog_start();
    }
}

/*======================================================================*/

static	Q8tkWidget	*menu_disk(void)
{
    Q8tkWidget *hbox, *vbox, *swin, *lab, *btn;
    Q8tkWidget *f, *vx, *hx;
    T_DISK_INFO *w;
    int i,j,k;
    const t_menulabel *l;


    hbox = PACK_HBOX(NULL);
    {
	for (k=0; k<COUNTOF(disk_info); k++) {

	    if (menu_swapdrv) { i = k ^ 1; }
	    else              { i = k;     }

	    w = &disk_info[i];
	    {
		vbox = PACK_VBOX(hbox);
		{
		    lab = PACK_LABEL(vbox, GET_LABEL(data_disk_image_drive,i));

		    if (menu_swapdrv)
			q8tk_misc_set_placement(lab, Q8TK_PLACEMENT_X_RIGHT,0);

		    {
			swin  = q8tk_scrolled_window_new(NULL, NULL);
			q8tk_widget_show(swin);
			q8tk_scrolled_window_set_policy(swin,
							Q8TK_POLICY_NEVER,
							Q8TK_POLICY_AUTOMATIC);
			q8tk_misc_set_size(swin, 29, 11);

			w->list = q8tk_listbox_new();
			q8tk_widget_show(w->list);
			q8tk_container_add(swin, w->list);

			q8tk_box_pack_start(vbox, swin);
		    }

		    for (j=0; j<2; j++) {
#if 0
						/* 空ラベルのウィジット確保 */
			w->label[j] = q8tk_label_new("");
			q8tk_widget_show(w->label[j]);
			w->button[j] = q8tk_button_new();
			q8tk_widget_show(w->button[j]);
			q8tk_container_add(w->button[j], w->label[j]);
			q8tk_signal_connect(w->button[j], "clicked",
					    cb_disk_button,
					    (void *)((j << 8) + i));

			q8tk_box_pack_start(vbox, w->button[j]);
#else
						/* 空ラベルのウィジット確保 */
			w->label[j] = q8tk_label_new("");
			q8tk_widget_show(w->label[j]);
			btn = q8tk_button_new();
			q8tk_widget_show(btn);
			q8tk_container_add(btn, w->label[j]);
			q8tk_signal_connect(btn, "clicked",
					    cb_disk_button,
					    (void *)((j << 8) + i));

			q8tk_box_pack_start(vbox, btn);
#endif
		    }
		}
	    }

	    PACK_VSEP(hbox);
	}

	{
	    vbox = PACK_VBOX(hbox);
	    {
		l = data_disk_info;
		for (i=0; i<COUNTOF(disk_info); i++) {
		    w = &disk_info[i];

		    vx = PACK_VBOX(NULL);
		    {
			hx = PACK_HBOX(vx);
			{
			    PACK_LABEL(hx, GET_LABEL(l, DATA_DISK_INFO_STAT));
						/* 空ラベルのウィジット確保 */
			    w->stat_label = PACK_LABEL(hx, "");
			}

			hx = PACK_HBOX(vx);
			{
			    PACK_LABEL(hx, GET_LABEL(l, DATA_DISK_INFO_ATTR));
						/* 空ラベルのウィジット確保 */
			    w->attr_label = PACK_LABEL(hx, "");
			}

			hx = PACK_HBOX(vx);
			{
			    PACK_LABEL(hx, GET_LABEL(l, DATA_DISK_INFO_NR));
						/* 空ラベルのウィジット確保 */
			    w->num_label = PACK_LABEL(hx, "");
			    q8tk_misc_set_placement(w->num_label,
						    Q8TK_PLACEMENT_X_RIGHT, 0);
			}
		    }

		    f = PACK_FRAME(vbox,
				   GET_LABEL(data_disk_info_drive, i), vx);
		    q8tk_frame_set_shadow_type(f, Q8TK_SHADOW_IN);
		}

		hx = PACK_HBOX(vbox);
		{
		    PACK_BUTTON(hx,
				GET_LABEL(data_disk_fname, DATA_DISK_FNAME),
				cb_disk_fname, NULL);
		}

		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_disk_dispswap,
					    DATA_DISK_DISPSWAP),
				  get_disk_dispswap(),
				  cb_disk_dispswap, NULL);

		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_disk_dispstatus, 0),
				  get_disk_dispstatus(),
				  cb_disk_dispstatus, NULL);

#if 0
		for (i=0; i<1; i++)	    /* 位置調整のためダミーを何個か */
		    PACK_LABEL(vbox, "");
#endif

		PACK_BUTTON(vbox,
			    GET_LABEL(data_disk_image, DATA_DISK_IMAGE_BLANK),
			    cb_disk_blank, NULL);
	    }
	}
    }


    set_disk_widget();

    return hbox;
}



/*===========================================================================
 *
 *	メインページ	キー設定
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
static Q8tkWidget *fkey_widget[20+1][2];

				    /* ファンクションキー割り当ての変更 */
static	int	get_key_fkey(int fn_key)
{
    return (function_f[ fn_key ] < 0x20) ? function_f[ fn_key ] : FN_FUNC;
}
static	void	cb_key_fkey(Q8tkWidget *widget, void *fn_key)
{
    int i;
    const t_menudata *p = data_key_fkey_fn;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_key_fkey_fn); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    function_f[ (int)fn_key ] = p->val;
	    q8tk_combo_set_text(fkey_widget[(int)fn_key][1],
				keymap_assign[0].str);
	    return;
	}
    }
}

static	int	get_key_fkey2(int fn_key)
{
    return (function_f[ fn_key ] >= 0x20) ? function_f[ fn_key ]
					  : KEY88_INVALID;
}
static	void	cb_key_fkey2(Q8tkWidget *widget, void *fn_key)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    function_f[ (int)fn_key ] = q->code;
	    q8tk_combo_set_text(fkey_widget[(int)fn_key][0],
				data_key_fkey_fn[0].str[menu_lang]);
	    return;
	}
    }
}


static	Q8tkWidget	*menu_key_fkey(void)
{
    int i;
    Q8tkWidget *vbox, *hbox;
    const t_menudata *p = data_key_fkey;

    vbox = PACK_VBOX(NULL);
    {
	for (i=0; i<COUNTOF(data_key_fkey); i++, p++) {

	    hbox = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(p, 0));

		fkey_widget[p->val][0] =
		    PACK_COMBO(hbox,
			       data_key_fkey_fn, COUNTOF(data_key_fkey_fn),
			       get_key_fkey(p->val), NULL, 42,
			       cb_key_fkey, (void*)(p->val),
			       NULL, NULL);

		fkey_widget[p->val][1] =
		    MAKE_KEY_COMBO(hbox,
				   &data_key_fkey2[i],
				   get_key_fkey2,
				   cb_key_fkey2);
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						      /* キー設定の変更 */
static	int	get_key_cfg(int type)
{
    switch (type) {
    case DATA_KEY_CFG_TENKEY:	return	tenkey_emu;
    case DATA_KEY_CFG_NUMLOCK:	return	numlock_emu;
    }
    return FALSE;
}
static	void	cb_key_cfg(Q8tkWidget *widget, void *type)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    switch ((int)type) {
    case DATA_KEY_CFG_TENKEY:	tenkey_emu  = (key) ? TRUE : FALSE;	break;
    case DATA_KEY_CFG_NUMLOCK:	numlock_emu = (key) ? TRUE : FALSE;	break;
    }
}


static	Q8tkWidget	*menu_key_cfg(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	PACK_CHECK_BUTTONS(vbox,
			   data_key_cfg, COUNTOF(data_key_cfg),
			   get_key_cfg, cb_key_cfg);
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
					      /* ソフトウェアキーボード */
static	void	keymap_start(void);
static	void	keymap_finish(void);

static	void	cb_key_softkeyboard(UNUSED_WIDGET, UNUSED_PARM)
{
    keymap_start();
}

static	Q8tkWidget	*menu_key_softkeyboard(void)
{
    Q8tkWidget *button;
    const t_menulabel *l = data_skey_set;

    button = PACK_BUTTON(NULL,
			 GET_LABEL(l, DATA_SKEY_BUTTON_SETUP),
			 cb_key_softkeyboard, NULL);

    return button;
}



/*----------------------------------------------------------------------*/
static	void	menu_key_cursor_setting(void);
					    /* カーソルキーカスタマイズ */
				     /* original idea by floi, thanks ! */
static	int		key_cursor_widget_init_done;
static	Q8tkWidget	*key_cursor_widget_sel;
static	Q8tkWidget	*key_cursor_widget_sel_none;
static	Q8tkWidget	*key_cursor_widget_sel_key;

static	int	get_key_cursor_key_mode(void)
{
    return cursor_key_mode;
}
static	void	cb_key_cursor_key_mode(UNUSED_WIDGET, void *p)
{
    cursor_key_mode = (int)p;

    menu_key_cursor_setting();
}
static	int	get_key_cursor_key(int type)
{
    return cursor_key_assign[ type ];
}
static	void	cb_key_cursor_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    cursor_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_key_cursor(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    key_cursor_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* キー割り当てモードの選択 */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_key_cursor_mode,
			       COUNTOF(data_key_cursor_mode),
			       get_key_cursor_key_mode(),
			       cb_key_cursor_key_mode);

	    for (i=0; i<2; i++) PACK_LABEL(vbox, "");	/* 位置調整のダミー */
	    
	    key_cursor_widget_sel = vbox;
	}

	PACK_VSEP(hbox);	/* 区切り棒 */

	{			/* キー割り当て なしの右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, GET_LABEL(data_key, DATA_KEY_CURSOR_SPACING));

	    key_cursor_widget_sel_none = vbox;
	}

	{			/* キー割り当て 任意の右画面 */
	    key_cursor_widget_sel_key =
		PACK_KEY_ASSIGN(hbox,
				data_key_cursor, COUNTOF(data_key_cursor),
				get_key_cursor_key, cb_key_cursor_key);
	}
    }

    key_cursor_widget_init_done = TRUE;
    menu_key_cursor_setting();

    return hbox;
}

static	void	menu_key_cursor_setting(void)
{
    if (key_cursor_widget_init_done == FALSE) return;

    {
	q8tk_widget_show(key_cursor_widget_sel);
	q8tk_widget_hide(key_cursor_widget_sel_none);
	q8tk_widget_hide(key_cursor_widget_sel_key);

	switch (cursor_key_mode) {
	case 0:
	case 1:   q8tk_widget_show(key_cursor_widget_sel_none);		break;
	case 2:   q8tk_widget_show(key_cursor_widget_sel_key);		break;
	}
    }
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_key(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2, *w;
    const t_menulabel *l = data_key;

    vbox = PACK_VBOX(NULL);
    {
	PACK_FRAME(vbox, GET_LABEL(l, DATA_KEY_FKEY), menu_key_fkey());

	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox, GET_LABEL(l, DATA_KEY_CURSOR), menu_key_cursor());

	    vbox2 = PACK_VBOX(NULL);
	    {
		PACK_LABEL(vbox2, GET_LABEL(l, DATA_KEY_SKEY2));

		w = menu_key_softkeyboard();
		q8tk_box_pack_start(vbox2, w);
		q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
					Q8TK_PLACEMENT_Y_CENTER);

		PACK_LABEL(vbox2, "");	/* 位置調整のダミー */
	    }
	    PACK_FRAME(hbox, GET_LABEL(l, DATA_KEY_SKEY), vbox2);
	}

	w = PACK_FRAME(vbox, "", menu_key_cfg());
	q8tk_frame_set_shadow_type(w, Q8TK_SHADOW_NONE);
    }

    return vbox;
}


/* = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = 
 *
 *	サブウインドウ	ソフトウェアキーボード
 *
 * = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = */

static	Q8tkWidget	*keymap[129];
static	int		keymap_num;
static	Q8tkWidget	*keymap_accel;

enum {			/* keymap[] は以下のウィジットの保存に使う */
    KEYMAP_WIN,

    KEYMAP_VBOX,
    KEYMAP_SCRL,
    KEYMAP_SEP,
    KEYMAP_HBOX,

    KEYMAP_BTN_1,
    KEYMAP_BTN_2,

    KEYMAP_LINES,
    KEYMAP_LINE_1,
    KEYMAP_LINE_2,
    KEYMAP_LINE_3,
    KEYMAP_LINE_4,
    KEYMAP_LINE_5,
    KEYMAP_LINE_6,

    KEYMAP_KEY
};

/*----------------------------------------------------------------------*/

static	int	get_key_softkey(int code)
{
    return softkey_is_pressed(code);
}
static	void	cb_key_softkey(Q8tkWidget *button, void *code)
{
    if (Q8TK_TOGGLE_BUTTON(button)->active) softkey_press  ((int)code);
    else                                    softkey_release((int)code);
}

static	void	cb_key_softkey_release(UNUSED_WIDGET, UNUSED_PARM)
{
    softkey_release_all();	/* 全てのキーを離した状態にする         */
    keymap_finish();		/* ソフトウェアキーの全ウィジットを消滅 */
}

static	void	cb_key_softkey_end(UNUSED_WIDGET, UNUSED_PARM)
{
    softkey_bug();		/* 複数キー同時押し時のハードバグを再現 */
    keymap_finish();		/* ソフトウェアキーの全ウィジットを消滅 */
}


/* ソフトウフェアキーボード ウインドウ生成・表示 */

static	void	keymap_start(void)
{
    Q8tkWidget *w, *v, *s, *l, *h, *b1, *b2, *vx, *hx, *n;
    int i,j;
    int model = (ROM_VERSION < '8') ? 0 : 1;

    for (i=0; i<COUNTOF(keymap); i++) keymap[i] = NULL;

    {						/* メインとなるウインドウ */
	w = q8tk_window_new(Q8TK_WINDOW_DIALOG);
	keymap_accel = q8tk_accel_group_new();
	q8tk_accel_group_attach(keymap_accel, w);
    }

    {						/* に、ボックスを乗せる */
	v = q8tk_vbox_new();
	q8tk_container_add(w, v);
	q8tk_widget_show(v);
    }

    {							/* ボックスには     */
	{						/* スクロール付 WIN */
	    s = q8tk_scrolled_window_new(NULL, NULL);
	    q8tk_box_pack_start(v, s);
	    q8tk_misc_set_size(s, 80, 21);
	    q8tk_scrolled_window_set_policy(s, Q8TK_POLICY_AUTOMATIC,
					       Q8TK_POLICY_NEVER);
	    q8tk_widget_show(s);
	}
	{						/* 見栄えのための空行*/
	    l = q8tk_label_new("");
	    q8tk_box_pack_start(v, l);
	    q8tk_widget_show(l);
	}
	{						/* ボタン配置用 HBOX */
	    h = q8tk_hbox_new();
	    q8tk_box_pack_start(v, h);
	    q8tk_misc_set_placement(h, Q8TK_PLACEMENT_X_CENTER, 0);
	    q8tk_widget_show(h);

	    {							/* HBOXには */
		const t_menulabel *l = data_skey_set;
		{						/* ボタン 1 */
		    b1 = q8tk_button_new_with_label(
					GET_LABEL(l,DATA_SKEY_BUTTON_OFF));
		    q8tk_signal_connect(b1, "clicked",
					cb_key_softkey_release, NULL);
		    q8tk_box_pack_start(h, b1);
		    q8tk_widget_show(b1);
		}
		{						/* ボタン 2 */
		    b2 = q8tk_button_new_with_label(
					GET_LABEL(l,DATA_SKEY_BUTTON_QUIT));
		    q8tk_signal_connect(b2, "clicked",
					cb_key_softkey_end, NULL);
		    q8tk_box_pack_start(h, b2);
		    q8tk_widget_show(b2);
		    q8tk_accel_group_add(keymap_accel,
					 Q8TK_KEY_ESC, b2, "clicked");
		}
	    }
	}
    }

    /* スクロール付 WIN に、キートップの文字のかかれた、ボタンを並べる */

    vx = q8tk_vbox_new();		/* キー6列分を格納する VBOX を配置 */
    q8tk_container_add(s, vx);
    q8tk_widget_show(vx);

    keymap[ KEYMAP_WIN   ] = w;
    keymap[ KEYMAP_VBOX  ] = v;
    keymap[ KEYMAP_SCRL  ] = s;
    keymap[ KEYMAP_SEP   ] = l;
    keymap[ KEYMAP_HBOX  ] = h;
    keymap[ KEYMAP_BTN_1 ] = b1;
    keymap[ KEYMAP_BTN_2 ] = b2;
    keymap[ KEYMAP_LINES ] = vx;

    keymap_num = KEYMAP_KEY;


    for (j=0; j<6; j++) {		/* キー6列分繰り返し */

	const t_keymap *p = keymap_line[ model ][ j ];

	hx = q8tk_hbox_new();		/* キー複数個を格納するためのHBOXに */
	q8tk_box_pack_start(vx, hx);
	q8tk_widget_show(hx);
	keymap[ KEYMAP_LINE_1 + j ] = hx;

	for (i=0; p[ i ].str; i++) {	/* キーを1個づつ配置しておく*/

	    if (keymap_num >= COUNTOF(keymap))	/* トラップ */
		{ fprintf(stderr, "%s %d\n", __FILE__, __LINE__); break; }
      
	    if (p[i].code)			/* キートップ文字 (ボタン) */
	    {
		n = q8tk_toggle_button_new_with_label(p[i].str);
		if (get_key_softkey(p[i].code)) {
		    q8tk_toggle_button_set_state(n, TRUE);
		}
		q8tk_signal_connect(n, "toggled",
				    cb_key_softkey, (void *)p[i].code);
	    }
	    else				/* パディング用空白 (ラベル) */
	    {
		n = q8tk_label_new(p[i].str);
	    }
	    q8tk_box_pack_start(hx, n);
	    q8tk_widget_show(n);

	    keymap[ keymap_num ++ ] = n;
	}
    }


    q8tk_widget_show(w);
    q8tk_grab_add(w);

    q8tk_widget_set_focus(b2);
}


/* キーマップダイアログの終了・消滅 */

static	void	keymap_finish(void)
{
    int i;
    for (i=keymap_num-1; i; i--) {
	if (keymap[i]) {
	    q8tk_widget_destroy(keymap[i]);
	}
    }

    q8tk_grab_remove(keymap[ KEYMAP_WIN ]);
    q8tk_widget_destroy(keymap[ KEYMAP_WIN ]);
    q8tk_widget_destroy(keymap_accel);
}








/*===========================================================================
 *
 *	メインページ	マウス
 *
 *===========================================================================*/

static	void	menu_mouse_mouse_setting(void);
static	void	menu_mouse_joy_setting(void);
static	void	menu_mouse_joy2_setting(void);
/*----------------------------------------------------------------------*/
						/* マウスモード切り替え */
static	int	get_mouse_mode(void)
{
    return mouse_mode;
}
static	void	cb_mouse_mode(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_mouse_mode;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_mouse_mode); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    mouse_mode = p->val;
	    menu_mouse_mouse_setting();
	    menu_mouse_joy_setting();
	    menu_mouse_joy2_setting();
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_mode(void)
{
    Q8tkWidget *hbox;


    hbox  = PACK_HBOX(NULL);
    {
	PACK_LABEL(hbox, " ");		/* インデント */

	PACK_COMBO(hbox,
		   data_mouse_mode, COUNTOF(data_mouse_mode),
		   get_mouse_mode(), NULL, 0,
		   cb_mouse_mode, NULL,
		   NULL, NULL);
    }

    return hbox;
}

/*----------------------------------------------------------------------*/
						      /* シリアルマウス */
static	int	get_mouse_serial(void)
{
    return use_siomouse;
}
static	void	cb_mouse_serial(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    use_siomouse = key;

    sio_mouse_init(use_siomouse);
}


static	Q8tkWidget	*menu_mouse_serial(void)
{
    Q8tkWidget *hbox;


    hbox  = PACK_HBOX(NULL);
    {
/*	PACK_LABEL(hbox, " ");*/

	PACK_CHECK_BUTTON(hbox,
			  GET_LABEL(data_mouse_serial, 0),
			  get_mouse_serial(),
			  cb_mouse_serial, NULL);
    }

    return hbox;
}

/*----------------------------------------------------------------------*/
						  /* マウス入力設定変更 */
static	int		mouse_mouse_widget_init_done;
static	Q8tkWidget	*mouse_mouse_widget_sel;
static	Q8tkWidget	*mouse_mouse_widget_sel_none;
static	Q8tkWidget	*mouse_mouse_widget_sel_tenkey;
static	Q8tkWidget	*mouse_mouse_widget_sel_key;
static	Q8tkWidget	*mouse_mouse_widget_con;
static	Q8tkWidget	*mouse_mouse_widget_con_con;

static	int	get_mouse_mouse_key_mode(void)
{
    return mouse_key_mode;
}
static	void	cb_mouse_mouse_key_mode(UNUSED_WIDGET, void *p)
{
    mouse_key_mode = (int)p;

    menu_mouse_mouse_setting();
}

static	Q8tkWidget	*mouse_swap_widget[2];
static	int	get_mouse_swap(void)
{
    return mouse_swap_button;
}
static	void	cb_mouse_swap(Q8tkWidget *widget, void *p)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    if (mouse_swap_button != key) {
	mouse_swap_button = key;
	if ((int)p >= 0) {
	    q8tk_toggle_button_set_state(mouse_swap_widget[(int)p], -1);
	}
    }
}

static	int	get_mouse_sensitivity(void)
{
    return mouse_sensitivity;
}
static	void	cb_mouse_sensitivity(Q8tkWidget *widget, UNUSED_PARM)
{
    int val = Q8TK_ADJUSTMENT(widget)->value;

    mouse_sensitivity = val;
}

static	int	get_mouse_mouse_key(int type)
{
    return mouse_key_assign[ type ];
}
static	void	cb_mouse_mouse_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    mouse_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_mouse(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    mouse_mouse_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* キー割り当てモードの選択 */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_mouse_mouse_key_mode,
			       COUNTOF(data_mouse_mouse_key_mode),
			       get_mouse_mouse_key_mode(),
			       cb_mouse_mouse_key_mode);

	    for (i=0; i<5; i++) PACK_LABEL(vbox, "");	/* 位置調整のダミー */

	    mouse_mouse_widget_sel = vbox;
	}

	{			/* キー割り当て不可 (接続中) */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");			/* 位置調整のダミー */

	    PACK_LABEL(vbox, GET_LABEL(data_mouse, DATA_MOUSE_CONNECTING));

	    for (i=0; i<6; i++) PACK_LABEL(vbox, "");	/* 位置調整のダミー */

	    mouse_mouse_widget_con = vbox;
	}

	PACK_VSEP(hbox);	/* 区切り棒 */

	{			/* キー割り当て なしの右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_mouse_widget_sel_none = vbox;
	}

	{			/* キー割り当て テンキーの右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_swap_widget[0] =
		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_mouse, DATA_MOUSE_SWAP_MOUSE),
				  get_mouse_swap(),
				  cb_mouse_swap, (void*)1);

	    mouse_mouse_widget_sel_tenkey = vbox;
	}

	{			/* キー割り当て 任意の右画面 */

	    mouse_mouse_widget_sel_key = 
		PACK_KEY_ASSIGN(hbox,
				data_mouse_mouse, COUNTOF(data_mouse_mouse),
				get_mouse_mouse_key, cb_mouse_mouse_key);
	}

	{			/* キー割り当て不可 右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_swap_widget[1] =
		PACK_CHECK_BUTTON(vbox,
				  GET_LABEL(data_mouse, DATA_MOUSE_SWAP_MOUSE),
				  get_mouse_swap(),
				  cb_mouse_swap, (void*)0);

	    for (i=0; i<4; i++) PACK_LABEL(vbox, "");	/* 位置調整のダミー */

	    {				/* マウス感度 */
		const t_volume *p = data_mouse_sensitivity;
		Q8tkWidget *hbox2, *scale;

		hbox2 = PACK_HBOX(vbox);
		{
		    PACK_LABEL(hbox2, GET_LABEL(p, 0));

		    scale = PACK_HSCALE(hbox2,
					 p,
					 get_mouse_sensitivity(),
					 cb_mouse_sensitivity, NULL);

		    q8tk_adjustment_set_length(scale->stat.scale.adj, 20);
		}
	    }

	    mouse_mouse_widget_con_con = vbox;
	}

    }

    mouse_mouse_widget_init_done = TRUE;
    menu_mouse_mouse_setting();

    return hbox;
}

static	void	menu_mouse_mouse_setting(void)
{
    if (mouse_mouse_widget_init_done == FALSE) return;

    if (mouse_mode == MOUSE_NONE ||		/* マウスはポートに未接続 */
	mouse_mode == MOUSE_JOYSTICK) {

	q8tk_widget_show(mouse_mouse_widget_sel);
	q8tk_widget_hide(mouse_mouse_widget_sel_none);
	q8tk_widget_hide(mouse_mouse_widget_sel_tenkey);
	q8tk_widget_hide(mouse_mouse_widget_sel_key);

	switch (mouse_key_mode) {
	case 0:   q8tk_widget_show(mouse_mouse_widget_sel_none);	break;
	case 1:   q8tk_widget_show(mouse_mouse_widget_sel_tenkey);	break;
	case 2:   q8tk_widget_show(mouse_mouse_widget_sel_key);		break;
	}

	q8tk_widget_hide(mouse_mouse_widget_con);
	q8tk_widget_hide(mouse_mouse_widget_con_con);

    } else {					/* マウスはポートに接続中 */
	q8tk_widget_hide(mouse_mouse_widget_sel);
	q8tk_widget_hide(mouse_mouse_widget_sel_none);
	q8tk_widget_hide(mouse_mouse_widget_sel_tenkey);
	q8tk_widget_hide(mouse_mouse_widget_sel_key);

	q8tk_widget_show(mouse_mouse_widget_con);
	q8tk_widget_hide(mouse_mouse_widget_con_con);

	if (mouse_mode == MOUSE_JOYMOUSE) {
	    q8tk_widget_show(mouse_mouse_widget_sel_tenkey);
	} else {
	    q8tk_widget_show(mouse_mouse_widget_con_con);
	}
    }
}

/*----------------------------------------------------------------------*/
					/* ジョイスティック入力設定変更 */
static	int		mouse_joy_widget_init_done;
static	Q8tkWidget	*mouse_joy_widget_sel;
static	Q8tkWidget	*mouse_joy_widget_sel_none;
static	Q8tkWidget	*mouse_joy_widget_sel_tenkey;
static	Q8tkWidget	*mouse_joy_widget_sel_key;
static	Q8tkWidget	*mouse_joy_widget_con;

static	int	get_mouse_joy_key_mode(void)
{
    return joy_key_mode;
}
static	void	cb_mouse_joy_key_mode(UNUSED_WIDGET, void *p)
{
    joy_key_mode = (int)p;

    menu_mouse_joy_setting();
}

static	int	get_joy_swap(void)
{
    return joy_swap_button;
}
static	void	cb_joy_swap(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    joy_swap_button = key;
}

static	int	get_mouse_joy_key(int type)
{
    return joy_key_assign[ type ];
}
static	void	cb_mouse_joy_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    joy_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_joy(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    mouse_joy_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* キー割り当てモードの選択 */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_mouse_joy_key_mode,
			       COUNTOF(data_mouse_joy_key_mode),
			       get_mouse_joy_key_mode(),
			       cb_mouse_joy_key_mode);

	    for (i=0; i<5; i++) PACK_LABEL(vbox, "");	/* 位置調整のダミー */
	    
	    mouse_joy_widget_sel = vbox;
	}

	{			/* キー割り当て不可 (接続中) */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");			/* 位置調整のダミー */

	    PACK_LABEL(vbox, GET_LABEL(data_mouse, DATA_MOUSE_CONNECTING));

	    for (i=0; i<6; i++) PACK_LABEL(vbox, "");	/* 位置調整のダミー */

	    mouse_joy_widget_con = vbox;
	}

	PACK_VSEP(hbox);	/* 区切り棒 */

	{			/* キー割り当て なしの右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_joy_widget_sel_none = vbox;
	}

	{			/* キー割り当て テンキーの右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_mouse, DATA_MOUSE_SWAP_JOY),
			      get_joy_swap(),
			      cb_joy_swap, NULL);

	    mouse_joy_widget_sel_tenkey = vbox;
	}

	{			/* キー割り当て 任意の右画面 */

	    mouse_joy_widget_sel_key = 
		PACK_KEY_ASSIGN(hbox,
				data_mouse_joy, COUNTOF(data_mouse_joy),
				get_mouse_joy_key, cb_mouse_joy_key);
	}
    }

    mouse_joy_widget_init_done = TRUE;
    menu_mouse_joy_setting();

    return hbox;
}

static	void	menu_mouse_joy_setting(void)
{
    if (mouse_joy_widget_init_done == FALSE) return;

    if (mouse_mode == MOUSE_NONE ||	/* ジョイスティックはポートに未接続 */
	mouse_mode == MOUSE_MOUSE ||
	mouse_mode == MOUSE_JOYMOUSE) {

	q8tk_widget_show(mouse_joy_widget_sel);
	q8tk_widget_hide(mouse_joy_widget_sel_none);
	q8tk_widget_hide(mouse_joy_widget_sel_tenkey);
	q8tk_widget_hide(mouse_joy_widget_sel_key);

	switch (joy_key_mode) {
	case 0:   q8tk_widget_show(mouse_joy_widget_sel_none);		break;
	case 1:   q8tk_widget_show(mouse_joy_widget_sel_tenkey);	break;
	case 2:   q8tk_widget_show(mouse_joy_widget_sel_key);		break;
	}

	q8tk_widget_hide(mouse_joy_widget_con);

    } else {				/* ジョイスティックはポートに接続中 */
	q8tk_widget_hide(mouse_joy_widget_sel);
	q8tk_widget_hide(mouse_joy_widget_sel_none);
	q8tk_widget_show(mouse_joy_widget_sel_tenkey);
	q8tk_widget_hide(mouse_joy_widget_sel_key);

	q8tk_widget_show(mouse_joy_widget_con);
    }
}

/*----------------------------------------------------------------------*/
				      /* ジョイスティック２入力設定変更 */
static	int		mouse_joy2_widget_init_done;
static	Q8tkWidget	*mouse_joy2_widget_sel;
static	Q8tkWidget	*mouse_joy2_widget_sel_none;
static	Q8tkWidget	*mouse_joy2_widget_sel_tenkey;
static	Q8tkWidget	*mouse_joy2_widget_sel_key;

static	int	get_mouse_joy2_key_mode(void)
{
    return joy2_key_mode;
}
static	void	cb_mouse_joy2_key_mode(UNUSED_WIDGET, void *p)
{
    joy2_key_mode = (int)p;

    menu_mouse_joy2_setting();
}

static	int	get_joy2_swap(void)
{
    return joy2_swap_button;
}
static	void	cb_joy2_swap(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    joy2_swap_button = key;
}

static	int	get_mouse_joy2_key(int type)
{
    return joy2_key_assign[ type ];
}
static	void	cb_mouse_joy2_key(Q8tkWidget *widget, void *type)
{
    int i;
    const t_keymap *q = keymap_assign;
    const char     *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(keymap_assign); i++, q++) {
	if (strcmp(q->str, combo_str) == 0) {
	    joy2_key_assign[ (int)type ] = q->code;
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_joy2(void)
{
    int i;
    Q8tkWidget *hbox, *vbox;

    mouse_joy2_widget_init_done = FALSE;

    hbox = PACK_HBOX(NULL);
    {
	{			/* キー割り当てモードの選択 */
	    vbox = PACK_VBOX(hbox);
	    PACK_RADIO_BUTTONS(vbox,
			       data_mouse_joy2_key_mode,
			       COUNTOF(data_mouse_joy2_key_mode),
			       get_mouse_joy2_key_mode(),
			       cb_mouse_joy2_key_mode);

	    for (i=0; i<5; i++) PACK_LABEL(vbox, "");	/* 位置調整のダミー */
	    
	    mouse_joy2_widget_sel = vbox;
	}

	PACK_VSEP(hbox);	/* 区切り棒 */

	{			/* キー割り当て なしの右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    mouse_joy2_widget_sel_none = vbox;
	}

	{			/* キー割り当て テンキーの右画面 */
	    vbox = PACK_VBOX(hbox);

	    PACK_LABEL(vbox, "");

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_mouse, DATA_MOUSE_SWAP_JOY2),
			      get_joy2_swap(),
			      cb_joy2_swap, NULL);

	    mouse_joy2_widget_sel_tenkey = vbox;
	}

	{			/* キー割り当て 任意の右画面 */
	    mouse_joy2_widget_sel_key = 
		PACK_KEY_ASSIGN(hbox,
				data_mouse_joy2, COUNTOF(data_mouse_joy2),
				get_mouse_joy2_key, cb_mouse_joy2_key);
	}
    }

    mouse_joy2_widget_init_done = TRUE;
    menu_mouse_joy2_setting();

    return hbox;
}

static	void	menu_mouse_joy2_setting(void)
{
    if (mouse_joy2_widget_init_done == FALSE) return;

    {
	q8tk_widget_show(mouse_joy2_widget_sel);
	q8tk_widget_hide(mouse_joy2_widget_sel_none);
	q8tk_widget_hide(mouse_joy2_widget_sel_tenkey);
	q8tk_widget_hide(mouse_joy2_widget_sel_key);

	switch (joy2_key_mode) {
	case 0:   q8tk_widget_show(mouse_joy2_widget_sel_none);		break;
	case 1:   q8tk_widget_show(mouse_joy2_widget_sel_tenkey);	break;
	case 2:   q8tk_widget_show(mouse_joy2_widget_sel_key);		break;
	}
    }
}

/*----------------------------------------------------------------------*/
				      /* マウス・ジョイ設定変更について */
static	Q8tkWidget	*menu_mouse_about(void)
{
    Q8tkWidget *vbox;
    char wk[128];
    int joy_num = event_get_joystick_num();

    vbox = PACK_VBOX(NULL);
    {
	sprintf(wk, GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_NUM), joy_num);

	/* ノートブックのサイズを統一するために、ここで目一杯のサイズを確保 */
	PACK_LABEL(vbox, "                                                                          ");
	PACK_LABEL(vbox, wk);
    }
    return vbox;
}

/*----------------------------------------------------------------------*/
static	int	menu_mouse_last_page = 0;	/* 前回時のタグを記憶 */
static	void	cb_mouse_notebook_changed(Q8tkWidget *widget, UNUSED_PARM)
{
    menu_mouse_last_page = q8tk_notebook_current_page(widget);
}

static	Q8tkWidget	*menu_mouse_device(void)
{
    Q8tkWidget *notebook, *w;

    notebook = q8tk_notebook_new();
    {
	w = menu_mouse_mouse();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_MOUSE));

	w = menu_mouse_joy();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_JOY));

	w = menu_mouse_joy2();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_JOY2));

	w = menu_mouse_about();
	q8tk_notebook_append(notebook, w,
			     GET_LABEL(data_mouse, DATA_MOUSE_DEVICE_ABOUT));
    }

    q8tk_notebook_set_page(notebook, menu_mouse_last_page);

    q8tk_signal_connect(notebook, "switch_page",
			cb_mouse_notebook_changed, NULL);
    q8tk_widget_show(notebook);

    return notebook;
}

/*----------------------------------------------------------------------*/
						      /* 各種設定の変更 */

/* デバッグ用：全マウス設定の組合せを検証 */
static int  get_mouse_debug_hide(void) { return hide_mouse; }
static int  get_mouse_debug_grab(void) { return grab_mouse; }
static void cb_mouse_debug_hide(UNUSED_WIDGET, void *p) { hide_mouse = (int)p;}
static void cb_mouse_debug_grab(UNUSED_WIDGET, void *p) { grab_mouse = (int)p;}

static	Q8tkWidget	*menu_mouse_debug(void)
{
    Q8tkWidget *hbox, *hbox2;

    hbox = PACK_HBOX(NULL);
    {
	hbox2 = PACK_HBOX(hbox);
	{
	    PACK_RADIO_BUTTONS(hbox2,
			data_mouse_debug_hide, COUNTOF(data_mouse_debug_hide),
			get_mouse_debug_hide(), cb_mouse_debug_hide);
	}
	PACK_VSEP(hbox);	/* 区切り棒 */
	PACK_VSEP(hbox);	/* 区切り棒 */
	hbox2 = PACK_HBOX(hbox);
	{
	    PACK_RADIO_BUTTONS(hbox2,
			data_mouse_debug_grab, COUNTOF(data_mouse_debug_grab),
			get_mouse_debug_grab(), cb_mouse_debug_grab);
	}
    }
    return hbox;
}



static	int	get_mouse_misc(void)
{
    if      (grab_mouse == AUTO_MOUSE) return -2;
    else if (grab_mouse == GRAB_MOUSE) return -1;
    else                               return hide_mouse;
}
static	void	cb_mouse_misc(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_mouse_misc;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_mouse_misc); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {
	    if (p->val == -2) {
		grab_mouse = AUTO_MOUSE;
		hide_mouse = SHOW_MOUSE;
	    } else if (p->val == -1) {
		grab_mouse = GRAB_MOUSE;
		/* hide_mouse = HIDE_MOUSE; */ /* 設定は保持する */
	    } else {
		grab_mouse = UNGRAB_MOUSE;
		hide_mouse = p->val;
	    }
	    return;
	}
    }
}


static	Q8tkWidget	*menu_mouse_misc(void)
{
    Q8tkWidget *vbox, *hbox;


    vbox = PACK_VBOX(NULL);
    {
	if (screen_attr_mouse_debug() == FALSE) {	/* 通常時 */
	    hbox  = PACK_HBOX(vbox);
	    {
		PACK_LABEL(hbox, GET_LABEL(data_mouse_misc_msg, 0));

		PACK_COMBO(hbox,
			   data_mouse_misc, COUNTOF(data_mouse_misc),
			   get_mouse_misc(), NULL, 0,
			   cb_mouse_misc, NULL,
			   NULL, NULL);
	    }
	} else {					/* デバッグ用 */
	    q8tk_box_pack_start(vbox, menu_mouse_debug());
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_mouse(void)
{
    Q8tkWidget *vbox, *hbox, *vbox2, *w;
    const t_menulabel *l = data_mouse;


    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_MOUSE_MODE),   menu_mouse_mode());
	    PACK_FRAME(hbox,
		       GET_LABEL(l, DATA_MOUSE_SERIAL), menu_mouse_serial());
	}

	PACK_LABEL(vbox, "");			/* 空行 */
	PACK_LABEL(vbox, GET_LABEL(l, DATA_MOUSE_SYSTEM));

	vbox2 = PACK_VBOX(vbox);
	{
	    w = menu_mouse_device();
	    q8tk_box_pack_start(vbox2, w);
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_RIGHT, 0);

	    w = menu_mouse_misc();
	    q8tk_box_pack_start(vbox2, w);
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_RIGHT, 0);
	}
    }

    return vbox;
}



/*===========================================================================
 *
 *	メインページ	テープ
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
				      /* ロードイメージ・セーブイメージ */
int			tape_mode;
static	char		tape_filename[ QUASI88_MAX_FILENAME ];

static	Q8tkWidget	*tape_name[2];
static	Q8tkWidget	*tape_rate[2];

static	Q8tkWidget	*tape_button_eject[2];
static	Q8tkWidget	*tape_button_rew;

/*----------------------------------------------------------------------*/
static	void	set_tape_name(int c)
{
    const char *p = filename_get_tape(c);

    q8tk_entry_set_text(tape_name[ c ], (p ? p : "(No File)"));
}
static	void	set_tape_rate(int c)
{
    char buf[16];
    long cur, end;

    if (c == CLOAD) {

	if (sio_tape_pos(&cur, &end)) {
	    if (end == 0) {
		sprintf(buf, "   END ");
	    } else {
		sprintf(buf, "   %3ld%%", cur * 100 / end);
	    }
	} else {
	    sprintf(buf, "   ---%%");
	}

	q8tk_label_set(tape_rate[ c ], buf);
    }
}


/*----------------------------------------------------------------------*/
/* 「EJECT」ボタン押下時の処理						*/

static	void	cb_tape_eject_do(UNUSED_WIDGET, void *c)
{
    if ((int)c == CLOAD) {
	quasi88_load_tape_eject();
    } else {
	quasi88_save_tape_eject();
    }

    set_tape_name((int)c);
    set_tape_rate((int)c);

    q8tk_widget_set_sensitive(tape_button_eject[(int)c], FALSE);
    if ((int)c == CLOAD) {
	q8tk_widget_set_sensitive(tape_button_rew, FALSE);
    }
    q8tk_widget_set_focus(NULL);
}

/*----------------------------------------------------------------------*/
/* 「REW」ボタン押下時の処理						*/

static	void	cb_tape_rew_do(UNUSED_WIDGET, void *c)
{
    if ((int)c == CLOAD) {
					/* イメージを巻き戻す */
	if (quasi88_load_tape_rewind()) {			/* 成功 */
	    ;
	} else {						/* 失敗 */
	    set_tape_name((int)c);
	}
	set_tape_rate((int)c);
    }
}

/*----------------------------------------------------------------------*/
/* 「OPEN」ボタン押下時の処理						*/

static	void	sub_tape_open(void);
static	void	sub_tape_open_do(void);
static	void	cb_tape_open_warn_clicked(Q8tkWidget *, void *);

static	void	cb_tape_open(UNUSED_WIDGET, void *c)
{
    const char *initial;
    const t_menulabel *l = ((int)c == CLOAD) ? data_tape_load : data_tape_save;

				/* 今から生成するファイルセレクションは */
    tape_mode = (int)c;		/* LOAD用 か SAVE用か を覚えておく      */

    initial = filename_get_tape_or_dir(tape_mode);

    START_FILE_SELECTION(GET_LABEL(l, DATA_TAPE_FSEL),
			 -1,	/* ReadOnly の選択は不可 */
			 initial,

			 sub_tape_open,
			 tape_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static	void	sub_tape_open(void)
{
    const t_menulabel *l = data_tape_save;

    switch (osd_file_stat(tape_filename)) {

    case FILE_STAT_NOEXIST:
	if (tape_mode == CLOAD) {		/* ファイル無いのでエラー   */
	    start_file_error_dialog(-1, ERR_CANT_OPEN);
	} else {				/* ファイル無いので新規作成 */
	    sub_tape_open_do();
	}
	break;

    case FILE_STAT_DIR:
	/* ディレクトリなので、開いちゃだめ */
	start_file_error_dialog(-1, ERR_CANT_OPEN);
	break;

    default:
	if (tape_mode == CSAVE) {
	    /* すでにファイルが存在します。イメージを追記しますか？ */
	    dialog_create();
	    {
		dialog_set_title(GET_LABEL(l, DATA_TAPE_WARN_0));

		dialog_set_title(GET_LABEL(l, DATA_TAPE_WARN_1));

		dialog_set_separator();

		dialog_set_button(GET_LABEL(l, DATA_TAPE_WARN_APPEND),
				  cb_tape_open_warn_clicked, (void*)TRUE);

		dialog_set_button(GET_LABEL(l, DATA_TAPE_WARN_CANCEL),
				  cb_tape_open_warn_clicked, (void*)FALSE);

		dialog_accel_key(Q8TK_KEY_ESC);
	    }
	    dialog_start();
	} else {
	    sub_tape_open_do();
	}
	break;
    }
}

static	void	sub_tape_open_do(void)
{
    int result, c = tape_mode;

    if (c == CLOAD) {			/* テープを開く */
	result = quasi88_load_tape_insert(tape_filename);
    } else {
	result = quasi88_save_tape_insert(tape_filename);
    }

    set_tape_name(c);
    set_tape_rate(c);


    if (result == FALSE) {
	start_file_error_dialog(-1, ERR_CANT_OPEN);
    }

    q8tk_widget_set_sensitive(tape_button_eject[(int)c], (result ?TRUE :FALSE));
    if ((int)c == CLOAD) {
	q8tk_widget_set_sensitive(tape_button_rew, (result ?TRUE :FALSE));
    }
}

static	void	cb_tape_open_warn_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	sub_tape_open_do();
    }
}




/*----------------------------------------------------------------------*/

INLINE	Q8tkWidget	*menu_tape_image_unit(const t_menulabel *l, int c)
{
    int save_code;
    Q8tkWidget *vbox, *hbox, *w, *e;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    w = PACK_LABEL(hbox, GET_LABEL(l, DATA_TAPE_FOR));
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);

	    {
		save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 65, NULL,
			       NULL, NULL, NULL, NULL);
		q8tk_entry_set_editable(e, FALSE);

		tape_name[ c ] = e;
		set_tape_name(c);

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_TAPE_CHANGE),
			cb_tape_open, (void*)c);

	    PACK_VSEP(hbox);

	    tape_button_eject[c] = PACK_BUTTON(hbox,
					       GET_LABEL(l, DATA_TAPE_EJECT),
					       cb_tape_eject_do, (void*)c);

	    if (c == CLOAD) {
		tape_button_rew = PACK_BUTTON(hbox,
					      GET_LABEL(l, DATA_TAPE_REWIND),
					      cb_tape_rew_do, (void*)c);
	    }
	    if (c == CLOAD) {
		w = PACK_LABEL(hbox, "");
		q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
					   Q8TK_PLACEMENT_Y_CENTER);
		tape_rate[ c ] = w;
		set_tape_rate(c);
	    }

	    if (c == CLOAD) {
		if (tape_readable() == FALSE) {
		    q8tk_widget_set_sensitive(tape_button_eject[c], FALSE);
		    q8tk_widget_set_sensitive(tape_button_rew, FALSE);
		}
	    } else {
		if (tape_writable() == FALSE) {
		    q8tk_widget_set_sensitive(tape_button_eject[c], FALSE);
		}
	    }
	}
    }

    return vbox;
}

static	Q8tkWidget	*menu_tape_image(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	q8tk_box_pack_start(vbox, menu_tape_image_unit(data_tape_load, CLOAD));

	PACK_HSEP(vbox);

	q8tk_box_pack_start(vbox, menu_tape_image_unit(data_tape_save, CSAVE));
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
					    /* テープ処理モード切り替え */
static	int	get_tape_intr(void)
{
    return cmt_intr;
}
static	void	cb_tape_intr(UNUSED_WIDGET, void *p)
{
    cmt_intr = (int)p;
}


static	Q8tkWidget	*menu_tape_intr(void)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(NULL);
    {
	PACK_RADIO_BUTTONS(vbox,
			   data_tape_intr, COUNTOF(data_tape_intr),
			   get_tape_intr(), cb_tape_intr);
    }

    return vbox;
}

/*======================================================================*/

static	Q8tkWidget	*menu_tape(void)
{
    Q8tkWidget *vbox;
    const t_menulabel *l = data_tape;

    vbox = PACK_VBOX(NULL);
    {
	PACK_FRAME(vbox, GET_LABEL(l, DATA_TAPE_IMAGE), menu_tape_image());

	PACK_FRAME(vbox, GET_LABEL(l, DATA_TAPE_INTR), menu_tape_intr());
    }

    return vbox;
}



/*===========================================================================
 *
 *	メインページ	その他
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
							  /* サスペンド */

static	Q8tkWidget	*misc_suspend_entry;
static	Q8tkWidget	*misc_suspend_combo;

static	char		state_filename[ QUASI88_MAX_FILENAME ];


/*	サスペンド時のメッセージダイアログを消す			  */
static	void	cb_misc_suspend_dialog_ok(UNUSED_WIDGET, void *result)
{
    dialog_destroy();

    if ((int)result == DATA_MISC_RESUME_OK ||
	(int)result == DATA_MISC_RESUME_ERR) {
	quasi88_exec();		/* ← q8tk_main_quit() 呼出済み */
    }
}

/*	サスペンド実行後のメッセージダイアログ				  */
static	void	sub_misc_suspend_dialog(int result)
{
    const t_menulabel *l = data_misc_suspend_err;
    char filename[60 +11 +1];	/* 11 == strlen("[DRIVE 1:] "), 1 は '\0' */
    int save_code;
    int i, width, len;
    const char *ptr[4];
    const char *none = "(No Image File)";
    const char *dev[] = { "[DRIVE 1:]", "[DRIVE 2:]",
			  "[TapeLOAD]", "[TapeSAVE]", };

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, result));	/* 結果表示 */

	if (result == DATA_MISC_SUSPEND_OK ||	/* 成功時はイメージ名表示 */
	    result == DATA_MISC_RESUME_OK) {

	    dialog_set_title(GET_LABEL(l, DATA_MISC_SUSPEND_LINE));
	    dialog_set_title(GET_LABEL(l, DATA_MISC_SUSPEND_INFO));

	    save_code = q8tk_set_kanjicode(osd_kanji_code());

	    width = 0;
	    for (i=0; i<4; i++) {
		if (i<2) {
		    if ((ptr[i] = filename_get_disk(i)) == NULL) ptr[i] = none;
		} else {
		    if ((ptr[i] = filename_get_tape(i-2))==NULL) ptr[i] = none;
		}
		len = sprintf(filename, "%.60s", ptr[i]);	/* == max 60 */
		width = MAX(width, len);
	    }

	    for (i=0; i<4; i++) {
		sprintf(filename, "%s %-*.*s", dev[i], width, width, ptr[i]);
		dialog_set_title(filename);
	    }

	    q8tk_set_kanjicode(save_code);
	}

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_AGREE),
			  cb_misc_suspend_dialog_ok, (void*)result);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*	レジューム実行前のメッセージダイアログ				  */
static	void	sub_misc_suspend_not_access(void)
{
    const t_menulabel *l = data_misc_suspend_err;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_MISC_RESUME_CANTOPEN));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_AGREE),
			  cb_misc_suspend_dialog_ok,
			  (void*)DATA_MISC_SUSPEND_AGREE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

/*	サスペンド実行前のメッセージダイアログ				  */
static	void	cb_misc_suspend_overwrite(UNUSED_WIDGET, UNUSED_PARM);
static	void	sub_misc_suspend_really(void)
{
    const t_menulabel *l = data_misc_suspend_err;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_MISC_SUSPEND_REALLY));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_OVERWRITE),
			  cb_misc_suspend_overwrite, NULL);
	dialog_set_button(GET_LABEL(l, DATA_MISC_SUSPEND_CANCEL),
			  cb_misc_suspend_dialog_ok,
			  (void*)DATA_MISC_SUSPEND_CANCEL);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}

static	void	cb_misc_suspend_overwrite(UNUSED_WIDGET, UNUSED_PARM)
{
    dialog_destroy();
    {
	if (quasi88_statesave(-1)) {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_OK);	/* 成功 */
	} else {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_ERR);	/* 失敗 */
	}
    }
}

/*----------------------------------------------------------------------*/
/*	サスペンド処理 (「セーブ」クリック時)				*/
static	void	cb_misc_suspend_save(UNUSED_WIDGET, UNUSED_PARM)
{
#if 0	/* いちいち上書き確認してくるのはうざい？ */
    if (statesave_check_file_exist()) {			/* ファイルある */
	sub_misc_suspend_really();
    } else
#endif
    {
	if (quasi88_statesave(-1)) {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_OK);	/* 成功 */
	} else {
	    sub_misc_suspend_dialog(DATA_MISC_SUSPEND_ERR);	/* 失敗 */
	}
    }
}

/*----------------------------------------------------------------------*/
/*	サスペンド処理 (「ロード」クリック時)				*/
static	void	cb_misc_suspend_load(UNUSED_WIDGET, UNUSED_PARM)
{
    if (stateload_check_file_exist() == FALSE) {	/* ファイルなし */
	sub_misc_suspend_not_access();
    } else {
	if (quasi88_stateload(-1)) {
	    sub_misc_suspend_dialog(DATA_MISC_RESUME_OK);	/* 成功 */
	} else {
	    sub_misc_suspend_dialog(DATA_MISC_RESUME_ERR);	/* 失敗 */
	}
    }
}

/*----------------------------------------------------------------------*/
/*	ファイル名前変更。エントリー changed (入力)時に呼ばれる。       */
/*		(ファイルセレクションでの変更時はこれは呼ばれない)      */

static void sub_misc_suspend_combo_update(void)
{
    int i;
    char buf[4];
				/* ステートファイルの連番に応じてコンボ変更 */
    i = filename_get_state_serial();
    if ('0' <= i && i <= '9') { buf[0] = i;   }
    else                      { buf[0] = ' '; }
    buf[1] = '\0';
    if (*(q8tk_combo_get_text(misc_suspend_combo)) != buf[0]) {
	q8tk_combo_set_text(misc_suspend_combo, buf);
    }
}

static void cb_misc_suspend_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_set_state(q8tk_entry_get_text(widget));

    sub_misc_suspend_combo_update();
}

/*----------------------------------------------------------------------*/
/*	ファイル選択処理。ファイルセレクションを使用			*/

static void sub_misc_suspend_update(void);
static void sub_misc_suspend_change(void);

static	void	cb_misc_suspend_fsel(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_misc_suspend;


    START_FILE_SELECTION(GET_LABEL(l, DATA_MISC_SUSPEND_FSEL),
			 -1,	/* ReadOnly の選択は不可 */
			 q8tk_entry_get_text(misc_suspend_entry),

			 sub_misc_suspend_change,
			 state_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static void sub_misc_suspend_change(void)
{
    filename_set_state(state_filename);
    q8tk_entry_set_text(misc_suspend_entry, state_filename);

    sub_misc_suspend_combo_update();
}

static void sub_misc_suspend_update(void)
{
    q8tk_entry_set_text(misc_suspend_entry, filename_get_state());

    sub_misc_suspend_combo_update();
}


static void cb_misc_suspend_num(Q8tkWidget *widget, UNUSED_PARM)
{
    int i;
    const t_menudata *p = data_misc_suspend_num;
    const char       *combo_str = q8tk_combo_get_text(widget);

    for (i=0; i<COUNTOF(data_misc_suspend_num); i++, p++) {
	if (strcmp(p->str[menu_lang], combo_str) == 0) {

	    filename_set_state_serial(p->val);

	    q8tk_entry_set_text(misc_suspend_entry, filename_get_state());
	    return;
	}
    }
}

/*----------------------------------------------------------------------*/

static	Q8tkWidget	*menu_misc_suspend(void)
{
    Q8tkWidget *vbox, *hbox;
    Q8tkWidget *w, *e;
    const t_menulabel *l = data_misc_suspend;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    {
		PACK_LABEL(hbox, GET_LABEL(data_misc, DATA_MISC_SUSPEND));
	    }
	    {
		int save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 74 - 11,
			       filename_get_state(),
			       NULL, NULL,
			       cb_misc_suspend_entry_change, NULL);
/*		q8tk_entry_set_position(e, 0);*/
		misc_suspend_entry = e;

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, "    ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SUSPEND_SAVE),
			cb_misc_suspend_save, NULL);

	    PACK_LABEL(hbox, " ");
	    PACK_VSEP(hbox);
	    PACK_LABEL(hbox, " ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SUSPEND_LOAD),
			cb_misc_suspend_load, NULL);

	    w = PACK_LABEL(hbox, GET_LABEL(l, DATA_MISC_SUSPEND_NUMBER));
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);

	    w = PACK_COMBO(hbox,
			   data_misc_suspend_num,
			   COUNTOF(data_misc_suspend_num),
			   filename_get_state_serial(), " ", 0,
			   cb_misc_suspend_num, NULL,
			   NULL, NULL);
	    q8tk_misc_set_placement(w, Q8TK_PLACEMENT_X_CENTER,
				       Q8TK_PLACEMENT_Y_CENTER);
	    misc_suspend_combo = w;

	    PACK_LABEL(hbox, "  ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SUSPEND_CHANGE),
			cb_misc_suspend_fsel, NULL);

	}
    }

    return vbox;
}



/*----------------------------------------------------------------------*/
					 /* スクリーン スナップショット */

static	Q8tkWidget	*misc_snapshot_entry;

static	char		snap_filename[ QUASI88_MAX_FILENAME ];


/*----------------------------------------------------------------------*/
/*	スナップショット セーブ (「実行」クリック時)			*/
static	void	cb_misc_snapshot_do(void)
{
    /* 念のため、スナップショットのファイル名を再設定 */
    filename_set_snap_base(q8tk_entry_get_text(misc_snapshot_entry));

    quasi88_screen_snapshot();

    /* スナップショットのファイル名は、実行時に変わることがあるので再設定 */
    q8tk_entry_set_text(misc_snapshot_entry, filename_get_snap_base());
}

/*----------------------------------------------------------------------*/
/*	画像フォーマット切り替え					*/
static	int	get_misc_snapshot_format(void)
{
    return snapshot_format;
}
static	void	cb_misc_snapshot_format(UNUSED_WIDGET, void *p)
{
    snapshot_format = (int)p;
}



/*----------------------------------------------------------------------*/
/*	ファイル名前変更。エントリー changed (入力)時に呼ばれる。	*/
/*		(ファイルセレクションでの変更時はこれは呼ばれない)	*/

static void cb_misc_snapshot_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_set_snap_base(q8tk_entry_get_text(widget));
}

/*----------------------------------------------------------------------*/
/*	ファイル選択処理。ファイルセレクションを使用			*/

static void sub_misc_snapshot_update(void);
static void sub_misc_snapshot_change(void);

static	void	cb_misc_snapshot_fsel(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_misc_snapshot;


    START_FILE_SELECTION(GET_LABEL(l, DATA_MISC_SNAPSHOT_FSEL),
			 -1,	/* ReadOnly の選択は不可 */
			 q8tk_entry_get_text(misc_snapshot_entry),

			 sub_misc_snapshot_change,
			 snap_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static void sub_misc_snapshot_change(void)
{
    filename_set_snap_base(snap_filename);
    q8tk_entry_set_text(misc_snapshot_entry, snap_filename);
}

static void sub_misc_snapshot_update(void)
{
    q8tk_entry_set_text(misc_snapshot_entry, filename_get_snap_base());
}


/*----------------------------------------------------------------------*/
#ifdef	USE_SSS_CMD
/*	コマンド実行状態変更 */
static	int	get_misc_snapshot_c_do(void)
{
    return snapshot_cmd_do;
}
static	void	cb_misc_snapshot_c_do(Q8tkWidget *widget, UNUSED_PARM)
{
    int key = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    snapshot_cmd_do = key;
}

/*	コマンド変更。エントリー changed (入力)時に呼ばれる。  */
static void cb_misc_snapshot_c_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    strncpy(snapshot_cmd, q8tk_entry_get_text(widget),
	    SNAPSHOT_CMD_SIZE - 1);
    snapshot_cmd[ SNAPSHOT_CMD_SIZE - 1 ] = '\0';
}
#endif

/*----------------------------------------------------------------------*/
static	Q8tkWidget	*menu_misc_snapshot(void)
{
    Q8tkWidget *hbox, *vbox, *hbox2, *vbox2, *vbox3, *hbox3;
    Q8tkWidget *e;
    const t_menulabel *l = data_misc_snapshot;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    {
		PACK_LABEL(hbox, GET_LABEL(data_misc, DATA_MISC_SNAPSHOT));
	    }
	    {
		int save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 74 - 11,
			       filename_get_snap_base(),
			       NULL, NULL,
			       cb_misc_snapshot_entry_change, NULL);
/*		q8tk_entry_set_position(e, 0);*/
		misc_snapshot_entry = e; 

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, "    ");

	    PACK_BUTTON(hbox,
			GET_LABEL(l, DATA_MISC_SNAPSHOT_BUTTON),
			cb_misc_snapshot_do, NULL);

	    PACK_LABEL(hbox, " ");
	    PACK_VSEP(hbox);


	    vbox2 = PACK_VBOX(hbox);
	    {
		hbox2 = PACK_HBOX(vbox2);
		{
		    vbox3 = PACK_VBOX(hbox2);

		    PACK_LABEL(vbox3, "");

		    hbox3 = PACK_HBOX(vbox3);
		    {
			PACK_LABEL(hbox3,
				   GET_LABEL(l, DATA_MISC_SNAPSHOT_FORMAT));

			PACK_RADIO_BUTTONS(PACK_HBOX(hbox3),
					   data_misc_snapshot_format,
					   COUNTOF(data_misc_snapshot_format),
					   get_misc_snapshot_format(),
					   cb_misc_snapshot_format);

			PACK_LABEL(hbox3,
				   GET_LABEL(l, DATA_MISC_SNAPSHOT_PADDING));
		    }

		    PACK_BUTTON(hbox2,
				GET_LABEL(l, DATA_MISC_SNAPSHOT_CHANGE),
				cb_misc_snapshot_fsel, NULL);
		}

#ifdef	USE_SSS_CMD
		if (snapshot_cmd_enable) {
		    hbox2 = PACK_HBOX(vbox2);
		    {
			PACK_CHECK_BUTTON(hbox2,
					  GET_LABEL(l, DATA_MISC_SNAPSHOT_CMD),
					  get_misc_snapshot_c_do(),
					  cb_misc_snapshot_c_do, NULL);

			PACK_LABEL(hbox2, " ");

			PACK_ENTRY(hbox2,
				   SNAPSHOT_CMD_SIZE, 38, snapshot_cmd,
				   NULL, NULL,
				   cb_misc_snapshot_c_entry_change, NULL);
		    }
		}
#endif	/* USE_SSS_CMD */
	    }
	}
    }

    return vbox;
}

/*----------------------------------------------------------------------*/
						  /* サウンド出力セーブ */

static	Q8tkWidget	*misc_waveout_entry;
static	Q8tkWidget	*misc_waveout_start;
static	Q8tkWidget	*misc_waveout_stop;
static	Q8tkWidget	*misc_waveout_change;

static	char		wave_filename[ QUASI88_MAX_FILENAME ];


static void sub_misc_waveout_sensitive(void)
{
    if (xmame_wavout_opened() == FALSE) {
	q8tk_widget_set_sensitive(misc_waveout_start,  TRUE);
	q8tk_widget_set_sensitive(misc_waveout_stop,   FALSE);
	q8tk_widget_set_sensitive(misc_waveout_change, TRUE);
	q8tk_widget_set_sensitive(misc_waveout_entry,  TRUE);
    } else {
	q8tk_widget_set_sensitive(misc_waveout_start,  FALSE);
	q8tk_widget_set_sensitive(misc_waveout_stop,   TRUE);
	q8tk_widget_set_sensitive(misc_waveout_change, FALSE);
	q8tk_widget_set_sensitive(misc_waveout_entry,  FALSE);
    }
    q8tk_widget_set_focus(NULL);
}
/*----------------------------------------------------------------------*/
/*	サウンド出力 保存開始 (「開始」クリック時)			*/
static	void	cb_misc_waveout_start(void)
{
    /* 念のため、サウンド出力のファイル名を再設定 */
    filename_set_wav_base(q8tk_entry_get_text(misc_waveout_entry));

    quasi88_waveout(TRUE);

    /* スナップショットのファイル名は、実行時に変わることがあるので再設定 */
    q8tk_entry_set_text(misc_waveout_entry, filename_get_wav_base());


    sub_misc_waveout_sensitive();
}

/*----------------------------------------------------------------------*/
/*	サウンド出力 保存終了 (「停止」クリック時)			*/
static	void	cb_misc_waveout_stop(void)
{
    quasi88_waveout(FALSE);

    sub_misc_waveout_sensitive();
}

/*----------------------------------------------------------------------*/
/*	ファイル名前変更。エントリー changed (入力)時に呼ばれる。	*/
/*		(ファイルセレクションでの変更時はこれは呼ばれない)	*/

static void cb_misc_waveout_entry_change(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_set_wav_base(q8tk_entry_get_text(widget));
}

/*----------------------------------------------------------------------*/
/*	ファイル選択処理。ファイルセレクションを使用			*/

static void sub_misc_waveout_update(void);
static void sub_misc_waveout_change(void);

static	void	cb_misc_waveout_fsel(UNUSED_WIDGET, UNUSED_PARM)
{
    const t_menulabel *l = data_misc_waveout;


    START_FILE_SELECTION(GET_LABEL(l, DATA_MISC_WAVEOUT_FSEL),
			 -1,	/* ReadOnly の選択は不可 */
			 q8tk_entry_get_text(misc_waveout_entry),

			 sub_misc_waveout_change,
			 wave_filename,
			 QUASI88_MAX_FILENAME,
			 NULL);
}

static void sub_misc_waveout_change(void)
{
    filename_set_wav_base(wave_filename);
    q8tk_entry_set_text(misc_waveout_entry, wave_filename);
}

static void sub_misc_waveout_update(void)
{
    q8tk_entry_set_text(misc_waveout_entry, filename_get_wav_base());
}


/*----------------------------------------------------------------------*/
static	Q8tkWidget	*menu_misc_waveout(void)
{
    Q8tkWidget *hbox, *vbox;
    Q8tkWidget *e;
    const t_menulabel *l = data_misc_waveout;

    vbox = PACK_VBOX(NULL);
    {
	hbox = PACK_HBOX(vbox);
	{
	    {
		PACK_LABEL(hbox, GET_LABEL(data_misc, DATA_MISC_WAVEOUT));
	    }
	    {
		int save_code = q8tk_set_kanjicode(osd_kanji_code());

		e = PACK_ENTRY(hbox,
			       QUASI88_MAX_FILENAME, 63,
			       filename_get_wav_base(),
			       NULL, NULL,
			       cb_misc_waveout_entry_change, NULL);
/*		q8tk_entry_set_position(e, 0);*/
		misc_waveout_entry = e; 

		q8tk_set_kanjicode(save_code);
	    }
	}

	hbox = PACK_HBOX(vbox);
	{
	    PACK_LABEL(hbox, "    ");

	    misc_waveout_start =
		PACK_BUTTON(hbox,
			    GET_LABEL(l, DATA_MISC_WAVEOUT_START),
			    cb_misc_waveout_start, NULL);

	    PACK_LABEL(hbox, " ");
	    PACK_VSEP(hbox);
	    PACK_LABEL(hbox, " ");

	    misc_waveout_stop =
		PACK_BUTTON(hbox,
			    GET_LABEL(l, DATA_MISC_WAVEOUT_STOP),
			    cb_misc_waveout_stop, NULL);

	    PACK_LABEL(hbox, GET_LABEL(l, DATA_MISC_WAVEOUT_PADDING));

	    misc_waveout_change =
		PACK_BUTTON(hbox,
			    GET_LABEL(l, DATA_MISC_WAVEOUT_CHANGE),
			    cb_misc_waveout_fsel, NULL);
	}
    }

    sub_misc_waveout_sensitive();

    return vbox;
}

/*----------------------------------------------------------------------*/
						    /* ファイル名合わせ */
static	int	get_misc_sync(void)
{
    return filename_synchronize;
}
static	void	cb_misc_sync(Q8tkWidget *widget, UNUSED_PARM)
{
    filename_synchronize = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
}



/*======================================================================*/

static	Q8tkWidget	*menu_misc(void)
{
    Q8tkWidget *vbox;
    Q8tkWidget *w;

    vbox = PACK_VBOX(NULL);
    {
	PACK_HSEP(vbox);

	q8tk_box_pack_start(vbox, menu_misc_suspend());

	PACK_HSEP(vbox);

	q8tk_box_pack_start(vbox, menu_misc_snapshot());

	PACK_HSEP(vbox);

	w = menu_misc_waveout();

	if (xmame_has_sound()) {
	    q8tk_box_pack_start(vbox, w);
	    PACK_HSEP(vbox);
	}

	PACK_CHECK_BUTTON(vbox,
			  GET_LABEL(data_misc_sync, 0),
			  get_misc_sync(),
			  cb_misc_sync, NULL);
    }

    return vbox;
}










/*===========================================================================
 *
 *	メインページ	バージョン情報
 *
 *===========================================================================*/

static	Q8tkWidget	*menu_about(void)
{
    int i;
    Q8tkWidget *vx, *hx, *vbox, *swin, *hbox, *w;

    vx = PACK_VBOX(NULL);
    {
	hx = PACK_HBOX(vx);				/* 上半分にロゴ表示 */
	{
	    PACK_LABEL(hx, " ");	/* インデント */

	    if (strcmp(Q_TITLE, "QUASI88") == 0) {
		w = q8tk_logo_new();
		q8tk_widget_show(w);
		q8tk_box_pack_start(hx, w);
	    } else {
		PACK_LABEL(hx, menu_lang == MENU_JAPAN ? Q_TITLE_KANJI : Q_TITLE);
	    }

	    vbox = PACK_VBOX(hx);
	    {
		i = Q8GR_LOGO_H;

		PACK_LABEL(vbox, "  " Q_COPYRIGHT);
		i--;

		for ( ; i>1; i--) PACK_LABEL(vbox, "");

		PACK_LABEL(vbox, "  ver. " Q_VERSION  "  <" Q_COMMENT ">");
	    }
	}
							/* 下半分は情報表示 */
	swin = q8tk_scrolled_window_new(NULL, NULL);
	{
	    hbox = PACK_HBOX(NULL);
	    {
		vbox = PACK_VBOX(hbox);
		{
		    {		/* サウンドに関する情報表示 */
			const char *(*s) = (menu_lang == 0) ? data_about_en
							    : data_about_jp;

			for (i=0; s[i]; i++) {
			    if (strcmp(s[i], "@MAMEVER") == 0) {
				PACK_LABEL(vbox, xmame_version_mame());
			    } else if (strcmp(s[i], "@FMGENVER") == 0) {
				PACK_LABEL(vbox, xmame_version_fmgen());
			    } else {
				PACK_LABEL(vbox, s[i]);
			    }
			}
		    }

		    {		/* システム依存部に関する情報表示 */
			int new_code, save_code = 0;
			const char *msg;
			char buf[256];
			int i;

			if (menu_about_osd_msg(menu_lang, &new_code, &msg)) {

			    if (menu_lang == MENU_JAPAN && new_code >= 0) {
				save_code = q8tk_set_kanjicode(new_code);
			    }

			    i = 0;
			    for (;;) {
				if (i == 255 || *msg == '\n' || *msg == '\0') {
				    buf[i] = '\0';
				    PACK_LABEL(vbox, buf);
				    i = 0;
				    if (*msg == '\n') msg++;
				    if (*msg == '\0') break;
				} else {
				    buf[i] = *msg++;
				    i++;
				}
			    }
			
			    if (menu_lang == MENU_JAPAN && new_code >= 0) {
				q8tk_set_kanjicode(save_code);
			    }
			}
		    }
		}
	    }
	    q8tk_container_add(swin, hbox);
	}

	q8tk_scrolled_window_set_policy(swin, Q8TK_POLICY_AUTOMATIC,
					      Q8TK_POLICY_AUTOMATIC);
	q8tk_misc_set_size(swin, 78, 18-Q8GR_LOGO_H);
	q8tk_widget_show(swin);
	q8tk_box_pack_start(vx, swin);
    }

    return vx;
}










/*===========================================================================
 *
 *	メインウインドウ
 *
 *===========================================================================*/

/*----------------------------------------------------------------------*/
				     /* NOTEBOOK に張り付ける、各ページ */
static	struct{
    int		data_num;
    Q8tkWidget	*(*menu_func)(void);
} menu_page[] =
{
    { DATA_TOP_RESET,	menu_reset,	},
    { DATA_TOP_CPU,	menu_cpu,	},
    { DATA_TOP_GRAPH,	menu_graph,	},
#ifdef	USE_SOUND
    { DATA_TOP_VOLUME,	menu_volume,	},
#endif
    { DATA_TOP_DISK,	menu_disk,	},
    { DATA_TOP_KEY,	menu_key,	},
    { DATA_TOP_MOUSE,	menu_mouse,	},
    { DATA_TOP_TAPE,	menu_tape,	},
    { DATA_TOP_MISC,	menu_misc,	},
    { DATA_TOP_ABOUT,	menu_about,	},
};

/*----------------------------------------------------------------------*/
/* NOTEBOOK の各ページを、ファンクションキーで選択出来るように、
   アクセラレータキーを設定する。そのため、ダミーウィジット利用 */

#define	cb_note_fake(fn,n)						\
static	void	cb_note_fake_##fn(UNUSED_WIDGET, Q8tkWidget *notebook)	\
{									\
    q8tk_notebook_set_page(notebook, n);				\
}
cb_note_fake(f1,0)
cb_note_fake(f2,1)
cb_note_fake(f3,2)
cb_note_fake(f4,3)
cb_note_fake(f5,4)
cb_note_fake(f6,5)
cb_note_fake(f7,6)
cb_note_fake(f8,7)
cb_note_fake(f9,8)
cb_note_fake(f10,9)

     /* 以下のアクセラレータキー処理は、 floi氏 提供。 Thanks ! */
static	void	cb_note_fake_prev(UNUSED_WIDGET, Q8tkWidget *notebook)
{
    int n = q8tk_notebook_current_page(notebook) - 1;
    if (n < 0) n = COUNTOF(menu_page) - 1;
    q8tk_notebook_set_page(notebook, n);
}

static	void	cb_note_fake_next(UNUSED_WIDGET, Q8tkWidget *notebook)
{
    int n = q8tk_notebook_current_page(notebook) + 1;
    if (COUNTOF(menu_page) <= n) n = 0;
    q8tk_notebook_set_page(notebook, n);
}

static	struct {
    int		key;
    void	(*cb_func)(Q8tkWidget *, Q8tkWidget *);
} menu_fkey[] =
{
    { Q8TK_KEY_F1,	cb_note_fake_f1,  },
    { Q8TK_KEY_F2,	cb_note_fake_f2,  },
    { Q8TK_KEY_F3,	cb_note_fake_f3,  },
    { Q8TK_KEY_F4,	cb_note_fake_f4,  },
    { Q8TK_KEY_F5,	cb_note_fake_f5,  },
    { Q8TK_KEY_F6,	cb_note_fake_f6,  },
    { Q8TK_KEY_F7,	cb_note_fake_f7,  },
    { Q8TK_KEY_F8,	cb_note_fake_f8,  },
    { Q8TK_KEY_F9,	cb_note_fake_f9,  },
    { Q8TK_KEY_F10,	cb_note_fake_f10, },

    { Q8TK_KEY_HOME,	cb_note_fake_prev, },
    { Q8TK_KEY_END,	cb_note_fake_next, },
};

/*----------------------------------------------------------------------*/
				/* 簡易リセットボタン ＋ モニターボタン */
static	Q8tkWidget	*monitor_widget;
static	Q8tkWidget	*quickres_widget;

static	int		top_misc_stat	= 1;
static	Q8tkWidget	*top_misc_button;

static	Q8tkWidget *menu_top_misc_quickres(void);
static	Q8tkWidget *menu_top_misc_monitor(void);

static	void	cb_top_misc_stat(UNUSED_WIDGET, UNUSED_PARM)
{
    top_misc_stat ^= 1;
    if (top_misc_stat) {
	q8tk_widget_hide(quickres_widget);
	q8tk_label_set(top_misc_button->child,  ">>");
	q8tk_widget_show(monitor_widget);
    } else {
	q8tk_widget_show(quickres_widget);
	q8tk_label_set(top_misc_button->child,  "<<");
	q8tk_widget_hide(monitor_widget);
    }
}
static	Q8tkWidget	*menu_top_button_misc(void)
{
    Q8tkWidget *box;

    box = PACK_HBOX(NULL);

    quickres_widget = menu_top_misc_quickres();
    monitor_widget  = menu_top_misc_monitor();

    top_misc_button = PACK_BUTTON(NULL,
				  "<<",
				  cb_top_misc_stat, NULL);

    q8tk_box_pack_start(box, quickres_widget);
    q8tk_box_pack_start(box, top_misc_button);
    q8tk_box_pack_start(box, monitor_widget);
    PACK_LABEL(box, "     ");

    top_misc_stat ^= 1;
    cb_top_misc_stat(0, 0);

    return box;
}

/*----------------------------------------------------------------------*/
						/* 簡易リセットボタン   */
static	int	get_quickres_basic(void)
{
    return reset_req.boot_basic;
}
static	void	cb_quickres_basic(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_basic != (int)p) {
	reset_req.boot_basic = (int)p;

	q8tk_toggle_button_set_state(widget_reset_basic[ 0 ][ (int)p ], TRUE);
    }
}
static	int	get_quickres_clock(void)
{
    return reset_req.boot_clock_4mhz;
}
static	void	cb_quickres_clock(UNUSED_WIDGET, void *p)
{
    if (reset_req.boot_clock_4mhz != (int)p) {
	reset_req.boot_clock_4mhz = (int)p;

	q8tk_toggle_button_set_state(widget_reset_clock[ 0 ][ (int)p ], TRUE);
    }
}

static	Q8tkWidget *menu_top_misc_quickres(void)
{
    Q8tkWidget *box;
    Q8List     *list;

    box = PACK_HBOX(NULL);
    {
	list = PACK_RADIO_BUTTONS(PACK_VBOX(box),
			data_quickres_basic, COUNTOF(data_quickres_basic),
			get_quickres_basic(), cb_quickres_basic);

	/* リストを手繰って、全ウィジットを取得 */
	widget_reset_basic[1][BASIC_V2 ] = list->data;	list = list->next;
	widget_reset_basic[1][BASIC_V1H] = list->data;	list = list->next;
	widget_reset_basic[1][BASIC_V1S] = list->data;	list = list->next;
	widget_reset_basic[1][BASIC_N  ] = list->data;


	list = PACK_RADIO_BUTTONS(PACK_VBOX(box),
			data_quickres_clock, COUNTOF(data_quickres_clock),
			get_quickres_clock(), cb_quickres_clock);

	/* リストを手繰って、全ウィジットを取得 */
	widget_reset_clock[1][CLOCK_4MHZ] = list->data;	list = list->next;
	widget_reset_clock[1][CLOCK_8MHZ] = list->data;


	PACK_BUTTON(box,
		    GET_LABEL(data_quickres_reset, 0),
		    cb_reset_now, NULL);

	PACK_VSEP(box);
    }
    q8tk_widget_hide(box);

    return box;
}

/*----------------------------------------------------------------------*/
						/* モニターボタン	*/
static	void	cb_top_monitor(UNUSED_WIDGET, UNUSED_PARM)
{
    quasi88_monitor();		/* ← q8tk_main_quit() 呼出済み */
}

static	Q8tkWidget *menu_top_misc_monitor(void)
{
    Q8tkWidget *box;

    box = PACK_HBOX(NULL);
    {
	PACK_LABEL(box, "  ");

	if (debug_mode) {
	    PACK_BUTTON(box,
			GET_LABEL(data_top_monitor, DATA_TOP_MONITOR_BTN),
			cb_top_monitor, NULL);
	} else {
	    PACK_LABEL(box,
		       GET_LABEL(data_top_monitor, DATA_TOP_MONITOR_PAD));
	}

	PACK_LABEL(box, "     ");
    }
    q8tk_widget_hide(box);

    return box;
}
/*----------------------------------------------------------------------*/
							/* ステータス	*/
static	int	get_top_status(void) { return quasi88_cfg_now_showstatus(); }
static	void	cb_top_status(Q8tkWidget *widget, UNUSED_PARM)
{
    int on = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;

    if (quasi88_cfg_can_showstatus()) {
	quasi88_cfg_set_showstatus(on);
    }
}

static	void	menu_top_status(Q8tkWidget *base_hbox)
{
    Q8tkWidget *vbox;

    vbox = PACK_VBOX(base_hbox);
    {
	PACK_LABEL(vbox, GET_LABEL(data_top_status, DATA_TOP_STATUS_PAD));

	if (quasi88_cfg_can_showstatus()) {

	    PACK_CHECK_BUTTON(vbox,
			      GET_LABEL(data_top_status, DATA_TOP_STATUS_CHK),
			      get_top_status(),
			      cb_top_status, NULL);

	    PACK_LABEL(vbox, GET_LABEL(data_top_status, DATA_TOP_STATUS_KEY));
	}
    }
}
/*----------------------------------------------------------------------*/
					/* メインウインドウ下部のボタン */
static	void	sub_top_savecfg(void);
static	void	sub_top_quit(void);

static	void	cb_top_button(UNUSED_WIDGET, void *p)
{
    switch ((int)p) {
    case DATA_TOP_SAVECFG:
	sub_top_savecfg();
	break;
    case DATA_TOP_EXIT:
	quasi88_exec();		/* ← q8tk_main_quit() 呼出済み */
	break;
    case DATA_TOP_QUIT:
	sub_top_quit();
	return;
    }
}

static	Q8tkWidget	*menu_top_button(void)
{
    int i;
    Q8tkWidget *hbox, *w;
    const t_menudata *p = data_top_button;

    hbox = PACK_HBOX(NULL);
    {
	w = menu_top_button_misc();
	q8tk_box_pack_start(hbox, w);

	menu_top_status(hbox);

	for (i=0; i<COUNTOF(data_top_button); i++, p++) {

	    w = PACK_BUTTON(hbox, GET_LABEL(p, 0),
			    cb_top_button, (void *)(p->val));

	    if (p->val == DATA_TOP_QUIT) {
		q8tk_accel_group_add(menu_accel, Q8TK_KEY_F12, w, "clicked");
	    }
	    if (p->val == DATA_TOP_EXIT) {
		q8tk_accel_group_add(menu_accel, Q8TK_KEY_ESC, w, "clicked");
	    }
	}
    }
    q8tk_misc_set_placement(hbox, Q8TK_PLACEMENT_X_RIGHT, 0);

    return hbox;
}


/*----------------------------------------------------------------------*/
				/* 設定保存ボタン押下時の、確認ダイアログ */

static	char	*top_savecfg_filename;

static	void	cb_top_savecfg_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	config_save(top_savecfg_filename);
    }

    free(top_savecfg_filename);
    top_savecfg_filename = NULL;
}
static	int	get_top_savecfg_auto(void)
{
    return save_config;
}
static	void	cb_top_savecfg_auto(Q8tkWidget *widget, UNUSED_PARM)
{
    int parm = (Q8TK_TOGGLE_BUTTON(widget)->active) ? TRUE : FALSE;
    save_config = parm;
}

static	void	sub_top_savecfg(void)
{
    const t_menulabel *l = data_top_savecfg;

    top_savecfg_filename = filename_alloc_global_cfgname();

    if (top_savecfg_filename) {
	dialog_create();
	{
	    dialog_set_title(GET_LABEL(l, DATA_TOP_SAVECFG_TITLE));
	    dialog_set_title(GET_LABEL(l, DATA_TOP_SAVECFG_INFO));
	    dialog_set_title("");
	    dialog_set_title(top_savecfg_filename);
	    dialog_set_title("");
	    dialog_set_check_button(GET_LABEL(l, DATA_TOP_SAVECFG_AUTO),
				    get_top_savecfg_auto(),
				    cb_top_savecfg_auto, NULL);

	    dialog_set_separator();

	    dialog_set_button(GET_LABEL(l, DATA_TOP_SAVECFG_OK),
			      cb_top_savecfg_clicked, (void*)TRUE);

	    dialog_accel_key(Q8TK_KEY_F12);

	    dialog_set_button(GET_LABEL(l, DATA_TOP_SAVECFG_CANCEL),
			      cb_top_savecfg_clicked, (void*)FALSE);

	    dialog_accel_key(Q8TK_KEY_ESC);
	}
	dialog_start();
    }
}


/*----------------------------------------------------------------------*/
				  /* QUITボタン押下時の、確認ダイアログ */

static	void	cb_top_quit_clicked(UNUSED_WIDGET, void *p)
{
    dialog_destroy();

    if ((int)p) {
	quasi88_quit();		/* ← q8tk_main_quit() 呼出済み */
    }
}
static	void	sub_top_quit(void)
{
    const t_menulabel *l = data_top_quit;

    dialog_create();
    {
	dialog_set_title(GET_LABEL(l, DATA_TOP_QUIT_TITLE));

	dialog_set_separator();

	dialog_set_button(GET_LABEL(l, DATA_TOP_QUIT_OK),
			  cb_top_quit_clicked, (void*)TRUE);

	dialog_accel_key(Q8TK_KEY_F12);

	dialog_set_button(GET_LABEL(l, DATA_TOP_QUIT_CANCEL),
			  cb_top_quit_clicked, (void*)FALSE);

	dialog_accel_key(Q8TK_KEY_ESC);
    }
    dialog_start();
}


/*----------------------------------------------------------------------*/
				  /* メニューのノートページが変わったら */

static	void	cb_top_notebook_changed(Q8tkWidget *widget, UNUSED_PARM)
{
    menu_last_page = q8tk_notebook_current_page(widget);
}

/*======================================================================*/

static Q8tkWidget *menu_top(void)
{
    int i;
    const t_menudata *l = data_top;
    Q8tkWidget *note_fake[ COUNTOF(menu_fkey) ];
    Q8tkWidget *win, *vbox, *notebook, *w;

    win = q8tk_window_new(Q8TK_WINDOW_TOPLEVEL);
    menu_accel = q8tk_accel_group_new();
    q8tk_accel_group_attach(menu_accel, win);
    q8tk_widget_show(win);

    vbox = PACK_VBOX(NULL);
    {
	{
	    /* 各メニューをノートページに乗せていく */

	    notebook = q8tk_notebook_new();
	    {
		for (i=0; i<COUNTOF(menu_page); i++) {

		    w = (*menu_page[i].menu_func)();
		    q8tk_notebook_append(notebook, w,
					 GET_LABEL(l, menu_page[i].data_num));

		    if (i<COUNTOF(menu_fkey)) {
			note_fake[i] = q8tk_button_new();
			q8tk_signal_connect(note_fake[i], "clicked",
					    menu_fkey[i].cb_func, notebook);
			q8tk_accel_group_add(menu_accel, menu_fkey[i].key,
					     note_fake[i], "clicked");
		    }
		}

		for ( ; i < COUNTOF(menu_fkey); i++) {
		    note_fake[i] = q8tk_button_new();
		    q8tk_signal_connect(note_fake[i], "clicked",
					menu_fkey[i].cb_func, notebook);
		    q8tk_accel_group_add(menu_accel, menu_fkey[i].key,
					 note_fake[i], "clicked");
		}
	    }
	    q8tk_signal_connect(notebook, "switch_page",
				cb_top_notebook_changed, NULL);
	    q8tk_widget_show(notebook);
	    q8tk_box_pack_start(vbox, notebook);

	    /* 実験：トップのノートブックは、フォーカスを適宜クリア
	             (タグ部分のアンダーライン表示がなくなる。ただそれだけ) */
	    q8tk_notebook_hook_focus_lost(notebook, TRUE);
	}
	{
	    /* おつぎは、ボタン */

	    w = menu_top_button();
	    q8tk_box_pack_start(vbox, w);
	}
    }
    q8tk_container_add(win, vbox);


	/* ノートブックを返します */
    return notebook;
}





/****************************************************************/
/* メニューモード メイン処理					*/
/****************************************************************/

void	menu_init(void)
{
    int i;

    for (i=0; i<0x10; i++) {			/* キースキャンワーク初期化 */
	if      (i == 0x08) key_scan[i] |= 0xdf;	/* カナは残す */
	else if (i == 0x0a) key_scan[i] |= 0x7f;	/* CAPSも残す */
	else                key_scan[i]  = 0xff;
    }

    /* 現在の、リセット情報を取得 */
    quasi88_get_reset_cfg(&reset_req);

    /* 現在の、サウンドの設定を保存 */
    sd_cfg_save();

    cpu_timing_save = cpu_timing;

    widget_reset_boot    = NULL;


    status_message_default(0, " MENU ");
    status_message_default(1, "<ESC> key to return");
    status_message_default(2, NULL);



    /* ここから、Q8TK 関連の初期化 */
    {
	Q8tkWidget *notebook;

	/* Q8TK 初期化 */
	q8tk_init();

	/* Q8TK 文字コードセット */
	if     (strcmp(menu_kanji_code, menu_kanji_code_euc) == 0)
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_EUC);
	}
	else if (strcmp(menu_kanji_code, menu_kanji_code_sjis) == 0)
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_SJIS);
	}
	else if (strcmp(menu_kanji_code, menu_kanji_code_utf8) == 0)
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_UTF8);
	}
	else
	{
	    q8tk_set_kanjicode(Q8TK_KANJI_ANK);
	}

	/* Q8TK カーソル有無設定 (初期化時に呼ぶ必要あり) */
	q8tk_set_cursor(now_swcursor);

	/* Q8TK メニュー生成 */
	notebook = menu_top();
	q8tk_notebook_set_page(notebook, menu_last_page);
    }
}



void	menu_main(void)
{
    /* Q8TK メイン処理 */
    q8tk_main_loop();


    /* メニューを抜けたら、メニューで変更した内容に応じて、再初期化 */
    if (quasi88_event_flags & EVENT_MODE_CHANGED) {

	if (quasi88_event_flags & EVENT_QUIT) {

	    /* QUASI88終了時は、なにもしない      */
	    /* (再初期化してもすぐに終了なので…) */

	} else {

#ifdef	USE_SOUND
	    if (sd_cfg_has_changed()) {	/* サウンド関連の設定に変更があれば */
		menu_sound_restart(TRUE);     /* サウンドドライバの再初期化 */
	    }
#endif

	    if (cpu_timing_save != cpu_timing) {
		emu_reset();
	    }

	    pc88main_bus_setup();
	    pc88sub_bus_setup();
	}

    } else {

	quasi88_event_flags |= EVENT_FRAME_UPDATE;
    }
}


/*---------------------------------------------------------------------------*/
/*
 * 現在のサウンドの設定値を記憶する (メニュー開始時に呼び出す)
 */
static	void	sd_cfg_save(void)
{
    int i;
    T_SNDDRV_CONFIG *p;


    memset(&sd_cfg_init, 0, sizeof(sd_cfg_init));
    memset(&sd_cfg_now,  0, sizeof(sd_cfg_now));

    sd_cfg_init.sound_board = sound_board;

#ifdef	USE_SOUND
    sd_cfg_init.sample_freq = xmame_cfg_get_sample_freq();
    sd_cfg_init.use_samples = xmame_cfg_get_use_samples();

#ifdef	USE_FMGEN
    sd_cfg_init.use_fmgen = xmame_cfg_get_use_fmgen();
#endif

    p = xmame_config_get_sndopt_tbl();

    if (p == NULL) {

	i = 0;

    } else {

	for (i=0; i<NR_SD_CFG_LOCAL; i++, p++) {
	    if (p->type == SNDDRV_NULL) break;

	    sd_cfg_init.local[i].info = p;

	    switch (p->type) {
	    case SNDDRV_INT:
		sd_cfg_init.local[i].val.i = *((int *)(p->work));
		break;

	    case SNDDRV_FLOAT:
		sd_cfg_init.local[i].val.f = *((float *)(p->work));
		break;
	    }
	}
    }

    sd_cfg_init.local_cnt = i;

#endif

    sd_cfg_now = sd_cfg_init;
}

/*
 * サウンドの設定値が、記憶した値と違うと、真を返す (メニュー終了時にチェック)
 */
static	int	sd_cfg_has_changed(void)
{
#ifdef	USE_SOUND
    int i;
    T_SNDDRV_CONFIG *p;

    /* サウンドボードの変更だけ、チェックするワークが違う・・・ */
    if (sd_cfg_init.sound_board != sound_board) {
	return TRUE;
    }


#ifdef	USE_FMGEN
    if (sd_cfg_init.use_fmgen != sd_cfg_now.use_fmgen) {
	return TRUE;
    }
#endif

    if (sd_cfg_init.sample_freq != sd_cfg_now.sample_freq) {
	return TRUE;
    }

    if (sd_cfg_init.use_samples != sd_cfg_now.use_samples) {
	return TRUE;
    }

    for (i = 0; i<sd_cfg_init.local_cnt; i++) {

	p = sd_cfg_init.local[i].info;

	switch (p->type) {
	case SNDDRV_INT:
	    if (sd_cfg_init.local[i].val.i != sd_cfg_now.local[i].val.i) {
		return TRUE;
	    }
	    break;

	case SNDDRV_FLOAT:
	    if (sd_cfg_init.local[i].val.f != sd_cfg_now.local[i].val.f) {
		return TRUE;
	    }
	    break;
	}
    }
#endif

    return FALSE;
}

void	menu_sound_restart(int output)
{
    xmame_sound_resume();		/* 中断したサウンドを復帰後に */
    xmame_sound_stop();			/* サウンドを停止させる。     */
    xmame_sound_start();		/* そして、サウンド再初期化   */


    /* サウンドドライバを再初期化すると、WAV出力が継続できない場合がある */
    if (xmame_wavout_damaged()) {
	quasi88_waveout(FALSE);
	XPRINTF("*** Waveout Stop ***\n");
    }


    /* 強制リスタート時は、ポートの再初期化は、呼び出し元にて実施する。
       そうでない場合は、ここで再初期化 */
    if (output) {
	sound_output_after_stateload();
    }


    /* メニューモードでこの関数が呼ばれた場合に備えて、ワークリセット */
    sd_cfg_save();


    XPRINTF("*** Sound Setting Is Applied ***\n\n");
}
