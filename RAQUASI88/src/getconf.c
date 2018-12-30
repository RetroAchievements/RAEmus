/************************************************************************/
/*									*/
/* 起動直後の引数の処理と、ワークの初期化				*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "getconf.h"
#include "event.h"

#include "pc88main.h"
#include "pc88sub.h"
#include "graph.h"
#include "intr.h"
#include "keyboard.h"
#include "memory.h"
#include "screen.h"
#include "soundbd.h"
#include "fdc.h"

#include "emu.h"
#include "file-op.h"
#include "drive.h"
#include "menu.h"
#include "status.h"
#include "monitor.h"
#include "snddrv.h"
#include "wait.h"
#include "snapshot.h"
#include "suspend.h"


/*----------------------------------------------------------------------*/

/* -f6 .. -f10 オプションの引数と、機能の対応一覧 */
static const struct {	
    int		num;
    char	*str;
} fn_index[] =
{
    { FN_FUNC,        NULL,          },
    { FN_FRATE_UP,    "FRATE-UP",    },
    { FN_FRATE_DOWN,  "FRATE-DOWN",  },
    { FN_VOLUME_UP,   "VOLUME-UP",   },
    { FN_VOLUME_DOWN, "VOLUME-DOWN", },
    { FN_PAUSE,       "PAUSE",       },
    { FN_RESIZE,      "RESIZE",      },
    { FN_NOWAIT,      "NOWAIT",      },
    { FN_SPEED_UP,    "SPEED-UP",    },
    { FN_SPEED_DOWN,  "SPEED-DOWN",  },
    { FN_FULLSCREEN,  "FULLSCREEN",  },
    { FN_FULLSCREEN,  "DGA",         },	/* 互換のため */
    { FN_SNAPSHOT,    "SNAPSHOT",    },
    { FN_IMAGE_NEXT1, "IMAGE-NEXT1", },
    { FN_IMAGE_PREV1, "IMAGE-PREV1", },
    { FN_IMAGE_NEXT2, "IMAGE-NEXT2", },
    { FN_IMAGE_PREV2, "IMAGE-PREV2", },
    { FN_NUMLOCK,     "NUMLOCK",     },
    { FN_RESET,       "RESET",       },
    { FN_KANA,        "KANA",        },
    { FN_ROMAJI,      "ROMAJI",      },
    { FN_CAPS,        "CAPS",        },
    { FN_STATUS,      "STATUS",      },
    { FN_MENU,        "MENU",        },
    { FN_MAX_SPEED,   "MAX-SPEED",   },
    { FN_MAX_CLOCK,   "MAX-CLOCK",   },
    { FN_MAX_BOOST,   "MAX-BOOST",   },
};


/*----------------------------------------------------------------------*/

/* オプションをいくつかのグループに分けて、優先度を設定する。*/
#define	OPT_GROUPS	(400)
static	signed char	opt_prioroty[ OPT_GROUPS ];

/*----------------------------------------------------------------------*/

	T_CONFIG_IMAGE	config_image;	/* 引数で指定されたイメージファイル */

static	int	load_config = TRUE;	/* 真で、起動時に設定読み込む */
	int	save_config = FALSE;	/* 真で、終了時に設定保存する */

/* ヘルプ表示用の、コマンド (argv[0]) */
static	char	*command = "QUASI88";

/* システム依存のヘルプ表示関数 */
static	void	(*help_msg_osd)(void) = NULL;

/* システム依存のオプションテーブル */
static	const	T_CONFIG_TABLE *option_table_osd;

/* サウンド系のオプションテーブル */
static	const	T_CONFIG_TABLE *option_table_sound;

/*----------------------------------------------------------------------*/

/* コールバック関数の引数 (char *) が未使用の場合、
 * ワーニングが出て鬱陶しいので、 gcc に頼んで許してもらう。 */
#if defined(__GNUC__)
#define	UNUSED_ARG	__attribute__((__unused__)) char *dummy
#else
#define	UNUSED_ARG	char *dummy
#endif

#include "help.h"

static int o_help(UNUSED_ARG)
{
    help_msg_common();
    if (help_msg_osd) {
	(help_msg_osd)();
    }
    help_msg_config();
    xmame_config_show_option();
    exit(0);
    return 0;
}

static	int	arg_baudrate;
static const int baudrate_table[] = {
    75, 150, 300, 600, 1200, 2400, 4800, 9600, 19200,
};
static int o_baudrate(UNUSED_ARG)
{
    int i;
    for (i=0; i<COUNTOF(baudrate_table); i++) {
	if (arg_baudrate == baudrate_table[i]) {
	    baudrate_sw = i;
	    return 0;
	}
    }
    return 1;
}

static int o_4mhz(UNUSED_ARG) { cpu_clock_mhz = CONST_4MHZ_CLOCK; return 0; }
static int o_8mhz(UNUSED_ARG) { cpu_clock_mhz = CONST_8MHZ_CLOCK; return 0; }

static int o_width (UNUSED_ARG) { WIDTH  &= ~7; return 0; }
static int o_height(UNUSED_ARG) { HEIGHT &= ~1; return 0; }

static int o_set_version(UNUSED_ARG) { set_version += '0'; return 0; }

static int o_kanjikey(UNUSED_ARG)
{
    function_f[  6 ] = FN_KANA;
    function_f[  7 ] = KEY88_KETTEI;
    function_f[  8 ] = KEY88_HENKAN;
    function_f[  9 ] = KEY88_ZENKAKU;
    function_f[ 10 ] = FN_ROMAJI;
    return 0;
}

static int oo_setfn(int key, char *str)
{
    int i, fn = FN_FUNC;

    for (i=1; i<COUNTOF(fn_index); i++) {
	if (my_strcmp(str, fn_index[i].str) == 0) {
	    fn = fn_index[i].num;
	    break;
	}
    }
    if (fn == FN_FUNC) {
	fn = keyboard_str2key88(str);
	if (fn < 0) {
	    return 1;
	}
    }
    function_f[ key ] = fn;
    return 0;
}
static int o_setfn_1 (char *str) { return oo_setfn( 1, str); }
static int o_setfn_2 (char *str) { return oo_setfn( 2, str); }
static int o_setfn_3 (char *str) { return oo_setfn( 3, str); }
static int o_setfn_4 (char *str) { return oo_setfn( 4, str); }
static int o_setfn_5 (char *str) { return oo_setfn( 5, str); }
static int o_setfn_6 (char *str) { return oo_setfn( 6, str); }
static int o_setfn_7 (char *str) { return oo_setfn( 7, str); }
static int o_setfn_8 (char *str) { return oo_setfn( 8, str); }
static int o_setfn_9 (char *str) { return oo_setfn( 9, str); }
static int o_setfn_10(char *str) { return oo_setfn(10, str); }
static int o_setfn_11(char *str) { return oo_setfn(11, str); }
static int o_setfn_12(char *str) { return oo_setfn(12, str); }

static int oo_setinput(int type, int key, char *keysym)
{
    int code = keyboard_str2key88(keysym);
    if (code < 0) {
	return 1;
    } else {
	if (type == 0) {		/* カーソルキー */
	    cursor_key_mode = 2;
	    cursor_key_assign[ key ] = code;
	}
	else if (type == 1) {		/* マウス */
	    mouse_key_mode = 2;
	    mouse_key_assign[ key ] = code;
	}
	else if (type == 2) {		/* ジョイスティック */
	    joy_key_mode = 2;
	    joy_key_assign[ key ] = code;
	}
	return 0;
    }
}
static int o_setkey_up   (char *keysym) { return oo_setinput(0, 0, keysym); }
static int o_setkey_down (char *keysym) { return oo_setinput(0, 1, keysym); }
static int o_setkey_left (char *keysym) { return oo_setinput(0, 2, keysym); }
static int o_setkey_right(char *keysym) { return oo_setinput(0, 3, keysym); }

static int o_setmouse_up   (char *ksym) { return oo_setinput(1, 0, ksym); }
static int o_setmouse_down (char *ksym) { return oo_setinput(1, 1, ksym); }
static int o_setmouse_left (char *ksym) { return oo_setinput(1, 2, ksym); }
static int o_setmouse_right(char *ksym) { return oo_setinput(1, 3, ksym); }
static int o_setmouse_l    (char *ksym) { return oo_setinput(1, 4, ksym); }
static int o_setmouse_r    (char *ksym) { return oo_setinput(1, 5, ksym); }

static int o_setjoy_up   (char *keysym) { return oo_setinput(2, 0, keysym); }
static int o_setjoy_down (char *keysym) { return oo_setinput(2, 1, keysym); }
static int o_setjoy_left (char *keysym) { return oo_setinput(2, 2, keysym); }
static int o_setjoy_right(char *keysym) { return oo_setinput(2, 3, keysym); }
static int o_setjoy_a    (char *keysym) { return oo_setinput(2, 4, keysym); }
static int o_setjoy_b    (char *keysym) { return oo_setinput(2, 5, keysym); }
static int o_setjoy_c    (char *keysym) { return oo_setinput(2, 6, keysym); }
static int o_setjoy_d    (char *keysym) { return oo_setinput(2, 7, keysym); }
static int o_setjoy_e    (char *keysym) { return oo_setinput(2, 8, keysym); }
static int o_setjoy_f    (char *keysym) { return oo_setinput(2, 9, keysym); }
static int o_setjoy_g    (char *keysym) { return oo_setinput(2,10, keysym); }
static int o_setjoy_h    (char *keysym) { return oo_setinput(2,11, keysym); }

static int o_menu   (UNUSED_ARG){ quasi88_menu();    return 0; }
#ifdef  USE_MONITOR
static int o_monitor(UNUSED_ARG){ quasi88_monitor(); return 0; }
#endif

static int oo_resumefilename(char *filename, int force)
{
    if (filename && (strlen(filename) >= QUASI88_MAX_FILENAME)) {
	fprintf(stderr, "filename %s too long, ignored\n", filename);
	resume_flag  = FALSE;
	resume_force = FALSE;
	resume_file  = FALSE;
	filename_set_state(NULL);
    } else {
	resume_flag  = TRUE;
	resume_force = force;
	resume_file  = (filename) ? TRUE : FALSE;
	filename_set_state(filename);
    }
    return 0;
}
static int o_resume     (UNUSED_ARG)  { return oo_resumefilename(NULL,  FALSE); }
static int o_resumefile (char *fname) { return oo_resumefilename(fname, FALSE); }
static int o_resumeforce(char *fname) { return oo_resumefilename(fname, TRUE);  }


static int oo_setdir(int type, char *dir)
{
    const char *opt = "";
    int result = FALSE;
	
    switch (type) {
    case 0:  opt = "romdir";   result = osd_set_dir_rom(dir);	break;
    case 1:  opt = "diskdir";  result = osd_set_dir_disk(dir);	break;
    case 2:  opt = "tapedir";  result = osd_set_dir_tape(dir);	break;
    case 3:  opt = "snapdir";  result = osd_set_dir_snap(dir);	break;
    case 4:  opt = "statedir"; result = osd_set_dir_state(dir);	break;
    }

    if (result == FALSE) {
	fprintf(stderr, "-%s %s failed, ignored\n", opt, dir);
    }
    return 0;
}
static int o_romdir  (char *dir) { return oo_setdir(0, dir); }
static int o_diskdir (char *dir) { return oo_setdir(1, dir); }
static int o_tapedir (char *dir) { return oo_setdir(2, dir); }
static int o_snapdir (char *dir) { return oo_setdir(3, dir); }
static int o_statedir(char *dir) { return oo_setdir(4, dir); }

static int oo_image(char **filename)
{
    if (strlen(*filename) >= QUASI88_MAX_FILENAME) {
	fprintf(stderr, "filename %s too long, ignored\n", *filename);
	free(*filename);
	*filename = NULL;
    }
    return 0;
}
static int o_tapeload (UNUSED_ARG) { return oo_image(&config_image.t[CLOAD]); }
static int o_tapesave (UNUSED_ARG) { return oo_image(&config_image.t[CSAVE]); }
static int o_printer  (UNUSED_ARG) { return oo_image(&config_image.prn);  }
static int o_serialin (UNUSED_ARG) { return oo_image(&config_image.sin);  }
static int o_serialout(UNUSED_ARG) { return oo_image(&config_image.sout); }

static int o_diskimage(UNUSED_ARG)
{
/*  config_image.d[DRIVE_1] = 設定済み */
    config_image.d[DRIVE_2] = NULL;
    config_image.n[DRIVE_1] = 0;
    config_image.n[DRIVE_2] = 0;

    return oo_image(&config_image.d[DRIVE_1]);
}



/*----------------------------------------------------------------------*/

/* 無効なオプションで、引数を伴う場合、ダミーの int型変数を用意しておく */
static	int	invalid_arg;

/* 設定保存の際に、特別な処理をする関数 */
static int save_bau(const struct T_CONFIG_TABLE *op, char opt_arg[255]);
static int save_ver(const struct T_CONFIG_TABLE *op, char opt_arg[255]);
static int save_fn(const struct T_CONFIG_TABLE *op, char opt_arg[255]);
static int save_cur(const struct T_CONFIG_TABLE *op, char opt_arg[255]);
static int save_mou(const struct T_CONFIG_TABLE *op, char opt_arg[255]);
static int save_joy(const struct T_CONFIG_TABLE *op, char opt_arg[255]);

static const T_CONFIG_TABLE option_table[] =
{
  /*  1〜30 : PC-8801設定オプション */

  {   1, "n",            X_FIX,  &boot_basic,      BASIC_N,               0,0, OPT_SAVE },
  {   1, "v1s",          X_FIX,  &boot_basic,      BASIC_V1S,             0,0, OPT_SAVE },
  {   1, "v1h",          X_FIX,  &boot_basic,      BASIC_V1H,             0,0, OPT_SAVE },
  {   1, "v2",           X_FIX,  &boot_basic,      BASIC_V2,              0,0, OPT_SAVE },
  {   2, "4mhz",         X_FIX,  &boot_clock_4mhz, TRUE,  0, o_4mhz,           OPT_SAVE },
  {   2, "8mhz",         X_FIX,  &boot_clock_4mhz, FALSE, 0, o_8mhz,           OPT_SAVE },
  {   3, "sd",           X_FIX,  &sound_board,     SOUND_I,               0,0, OPT_SAVE },
  {   3, "sd2",          X_FIX,  &sound_board,     SOUND_II,              0,0, OPT_SAVE },
  {   4, "dipsw",        X_INT,  &boot_dipsw,      0x0000, 0xffff,          0, OPT_SAVE },
  {   5, "baudrate",     X_INT,  &arg_baudrate,    75, 19200, o_baudrate,      save_bau },
  {   6, "romboot",      X_FIX,  &boot_from_rom,   TRUE,                  0,0, 0        },
  {   6, "diskboot",     X_FIX,  &boot_from_rom,   FALSE,                 0,0, 0        },
  {   7, "extram",       X_INT,  &use_extram,      0, 64,                   0, OPT_SAVE },
  {   7, "noextram",     X_FIX,  &use_extram,      0,                     0,0, OPT_SAVE },
  {   8, "jisho",        X_FIX,  &use_jisho_rom,   TRUE,                  0,0, OPT_SAVE },
  {   8, "nojisho",      X_FIX,  &use_jisho_rom,   FALSE,                 0,0, OPT_SAVE },
  {   9, "mouse",        X_FIX,  &mouse_mode,      1,                     0,0, OPT_SAVE },
  {   9, "nomouse",      X_FIX,  &mouse_mode,      0,                     0,0, OPT_SAVE },
  {   9, "joymouse",     X_FIX,  &mouse_mode,      2,                     0,0, OPT_SAVE },
  {   9, "joystick",     X_FIX,  &mouse_mode,      3,                     0,0, OPT_SAVE },
  {  10, "analog",       X_FIX,  &monitor_analog,  TRUE,                  0,0, OPT_SAVE },
  {  10, "digital",      X_FIX,  &monitor_analog,  FALSE,                 0,0, OPT_SAVE },
  {  11, "24k",          X_FIX,  &monitor_15k,     0x00,                  0,0, OPT_SAVE },
  {  11, "15k",          X_FIX,  &monitor_15k,     0x02,                  0,0, OPT_SAVE },
  {  12, "pcg",          X_FIX,  &use_pcg,         TRUE,                  0,0, OPT_SAVE },
  {  12, "nopcg",        X_FIX,  &use_pcg,         FALSE,                 0,0, OPT_SAVE },
  {  13, "tapeload",     X_STR,  &config_image.t[CLOAD], 0, 0, o_tapeload,     0        },
  {  14, "tapesave",     X_STR,  &config_image.t[CSAVE], 0, 0, o_tapesave,     0        },
  {  15, "serialmouse",  X_FIX,  &use_siomouse,    TRUE,                  0,0, OPT_SAVE },
  {  15, "noserialmouse",X_FIX,  &use_siomouse,    FALSE,                 0,0, OPT_SAVE },

  /*  31〜60 : エミュレーション設定オプション */

  {  31, "cpu",          X_INT,  &cpu_timing,      0, 2,                    0, OPT_SAVE },
  {  32, "cpu1count",    X_INT,  &CPU_1_COUNT,     1, 65536,                0, 0        },
  {  33, "cpu2us",       X_INT,  &cpu_slice_us,    1, 1000,                 0, 0        },
  {  34, "fdc_wait",     X_FIX,  &fdc_wait,        1,                     0,0, OPT_SAVE },
  {  34, "fdc_nowait",   X_FIX,  &fdc_wait,        0,                     0,0, OPT_SAVE },
  {  35, "clock",        X_DBL,  &cpu_clock_mhz,   0.001, 65536.0,          0, OPT_SAVE },
  {  36, "speed",        X_INT,  &wait_rate,       5, 5000,                 0, OPT_SAVE },
  {  37, "nowait",       X_FIX,  &no_wait,         TRUE,                  0,0, OPT_SAVE },
  {  37, "wait",         X_FIX,  &no_wait,         FALSE,                 0,0, OPT_SAVE },
  {  38, "boost",        X_INT,  &boost,           1, 100,                  0, OPT_SAVE },
  {  39, "cmt_intr",     X_FIX,  &cmt_intr,        TRUE,                  0,0, OPT_SAVE },
  {  39, "cmt_poll",     X_FIX,  &cmt_intr,        FALSE,                 0,0, OPT_SAVE },
  {  40, "cmt_speed",    X_INT,  &cmt_speed,       0, 0xffff,               0, OPT_SAVE },
  {  41, "hsbasic",      X_FIX,  &highspeed_mode,  TRUE,                  0,0, OPT_SAVE },
  {  41, "nohsbasic",    X_FIX,  &highspeed_mode,  FALSE,                 0,0, OPT_SAVE },
  {  42, "mem_wait",     X_FIX,  &memory_wait,     TRUE,                  0,0, OPT_SAVE },
  {  42, "mem_nowait",   X_FIX,  &memory_wait,     FALSE,                 0,0, OPT_SAVE },
  {  43, "setver",       X_INT,  &set_version,     0, 9, o_set_version,        save_ver },
  {  44, "exchange",     X_FIX,  &disk_exchange,   TRUE,                  0,0, OPT_SAVE },
  {  44, "noexchange",   X_FIX,  &disk_exchange,   FALSE,                 0,0, OPT_SAVE },

  /*  61〜90 : 画面表示設定オプション */

  {  61, "frameskip",    X_INT,  &frameskip_rate,  1, 65536,                0, OPT_SAVE },
  {  62, "autoskip",     X_FIX,  &use_auto_skip,   TRUE,                  0,0, OPT_SAVE },
  {  62, "noautoskip",   X_FIX,  &use_auto_skip,   FALSE,                 0,0, OPT_SAVE },
  {  63, "half",         X_FIX,  &screen_size,     SCREEN_SIZE_HALF,      0,0, OPT_SAVE },
  {  63, "full",         X_FIX,  &screen_size,     SCREEN_SIZE_FULL,      0,0, OPT_SAVE },
#ifdef	SUPPORT_DOUBLE
  {  63, "double",       X_FIX,  &screen_size,     SCREEN_SIZE_DOUBLE,    0,0, OPT_SAVE },
#else
  {  63, "double",       X_INV,                                       0,0,0,0, 0        },
#endif
  {  64, "fullscreen",   X_FIX,  &use_fullscreen,  TRUE,                  0,0, OPT_SAVE },
  {  64, "dga",          X_FIX,  &use_fullscreen,  TRUE,                  0,0, 0        },
  {  64, "window",       X_FIX,  &use_fullscreen,  FALSE,                 0,0, OPT_SAVE },
  {  64, "nodga",        X_FIX,  &use_fullscreen,  FALSE,                 0,0, 0        },
  {  65, "aspect",       X_DBL,  &mon_aspect,      0.0, 10.0,               0, OPT_SAVE },
  {  66, "width",        X_INT,  &WIDTH,           1, 65536, o_width,          0        },
  {  67, "height",       X_INT,  &HEIGHT,          1, 65536, o_height,         0        },
  {  68, "interp",       X_FIX,  &use_half_interp, TRUE,                  0,0, OPT_SAVE },
  {  68, "nointerp",     X_FIX,  &use_half_interp, FALSE,                 0,0, OPT_SAVE },
  {  69, "skipline",     X_FIX,  &use_interlace,   SCREEN_INTERLACE_SKIP, 0,0, OPT_SAVE },
  {  69, "noskipline",   X_FIX,  &use_interlace,   SCREEN_INTERLACE_NO,   0,0, OPT_SAVE },
  {  69, "interlace",    X_FIX,  &use_interlace,   SCREEN_INTERLACE_YES,  0,0, OPT_SAVE },
  {  69, "nointerlace",  X_FIX,  &use_interlace,   SCREEN_INTERLACE_NO,   0,0, OPT_SAVE },
  {  70, "hide_mouse",   X_FIX,  &hide_mouse,      HIDE_MOUSE,            0,0, OPT_SAVE },
  {  70, "show_mouse",   X_FIX,  &hide_mouse,      SHOW_MOUSE,            0,0, OPT_SAVE },
  {  70, "auto_mouse",   X_FIX,  &hide_mouse,      AUTO_MOUSE,            0,0, OPT_SAVE },
  {  71, "grab_mouse",   X_FIX,  &grab_mouse,      GRAB_MOUSE,            0,0, OPT_SAVE },
  {  71, "ungrab_mouse", X_FIX,  &grab_mouse,      UNGRAB_MOUSE,          0,0, OPT_SAVE },
  {  71, "auto_grab",    X_FIX,  &grab_mouse,      AUTO_MOUSE,            0,0, OPT_SAVE },
  {  72, "status",       X_FIX,  &show_status,     TRUE,                  0,0, OPT_SAVE },
  {  72, "nostatus",     X_FIX,  &show_status,     FALSE,                 0,0, OPT_SAVE },
  {  73, "status_fg",    X_INT,  &status_fg,       0, 0xffffff,             0, OPT_SAVE },
  {  74, "status_bg",    X_INT,  &status_bg,       0, 0xffffff,             0, OPT_SAVE },
  {  75, "statusimage",  X_FIX,  &status_imagename,TRUE,                  0,0, OPT_SAVE },
  {  75, "nostatusimage",X_FIX,  &status_imagename,FALSE,                 0,0, OPT_SAVE },

  /*  91〜160: キー設定オプション */

  {  91, "tenkey",       X_FIX,  &tenkey_emu,      TRUE,                  0,0, OPT_SAVE },
  {  91, "notenkey",     X_FIX,  &tenkey_emu,      FALSE,                 0,0, OPT_SAVE },
  {  92, "numlock",      X_FIX,  &numlock_emu,     TRUE,                  0,0, OPT_SAVE },
  {  92, "nonumlock",    X_FIX,  &numlock_emu,     FALSE,                 0,0, OPT_SAVE },
  {  93, "cursor_up",    X_STR,  NULL,             0, 0, o_setkey_up,          save_cur },
  {  94, "cursor_down",  X_STR,  NULL,             0, 0, o_setkey_down,        save_cur },
  {  95, "cursor_left",  X_STR,  NULL,             0, 0, o_setkey_left,        save_cur },
  {  96, "cursor_right", X_STR,  NULL,             0, 0, o_setkey_right,       save_cur },
  {  97, "cursor",       X_FIX,  &cursor_key_mode, 1,                     0,0, OPT_SAVE },
  {  97, "nocursor",     X_FIX,  &cursor_key_mode, 0,                     0,0, OPT_SAVE },
  {  98, "mouse_up",     X_STR,  NULL,             0, 0, o_setmouse_up,        save_mou },
  {  99, "mouse_down",   X_STR,  NULL,             0, 0, o_setmouse_down,      save_mou },
  { 100, "mouse_left",   X_STR,  NULL,             0, 0, o_setmouse_left,      save_mou },
  { 101, "mouse_right",  X_STR,  NULL,             0, 0, o_setmouse_right,     save_mou },
  { 102, "mouse_l",      X_STR,  NULL,             0, 0, o_setmouse_l,         save_mou },
  { 103, "mouse_r",      X_STR,  NULL,             0, 0, o_setmouse_r,         save_mou },
  { 104, "mousekey",     X_FIX,  &mouse_key_mode,  1,                     0,0, OPT_SAVE },
  { 104, "nomousekey",   X_FIX,  &mouse_key_mode,  0,                     0,0, OPT_SAVE },
  { 105, "joy_up",       X_STR,  NULL,             0, 0, o_setjoy_up,          save_joy },
  { 106, "joy_down",     X_STR,  NULL,             0, 0, o_setjoy_down,        save_joy },
  { 107, "joy_left",     X_STR,  NULL,             0, 0, o_setjoy_left,        save_joy },
  { 108, "joy_right",    X_STR,  NULL,             0, 0, o_setjoy_right,       save_joy },
  { 109, "joy_a",        X_STR,  NULL,             0, 0, o_setjoy_a,           save_joy },
  { 110, "joy_b",        X_STR,  NULL,             0, 0, o_setjoy_b,           save_joy },
  { 111, "joy_c",        X_STR,  NULL,             0, 0, o_setjoy_c,           save_joy },
  { 112, "joy_d",        X_STR,  NULL,             0, 0, o_setjoy_d,           save_joy },
  { 113, "joy_e",        X_STR,  NULL,             0, 0, o_setjoy_e,           save_joy },
  { 114, "joy_f",        X_STR,  NULL,             0, 0, o_setjoy_f,           save_joy },
  { 115, "joy_g",        X_STR,  NULL,             0, 0, o_setjoy_g,           save_joy },
  { 116, "joy_h",        X_STR,  NULL,             0, 0, o_setjoy_h,           save_joy },
  { 117, "joykey",       X_FIX,  &joy_key_mode,    1,                     0,0, OPT_SAVE },
  { 117, "nojoykey",     X_FIX,  &joy_key_mode,    0,                     0,0, OPT_SAVE },
  { 118, "f1",           X_STR,  NULL,             0, 0, o_setfn_1,            0        },
  { 119, "f2",           X_STR,  NULL,             0, 0, o_setfn_2,            0        },
  { 120, "f3",           X_STR,  NULL,             0, 0, o_setfn_3,            0        },
  { 121, "f4",           X_STR,  NULL,             0, 0, o_setfn_4,            0        },
  { 122, "f5",           X_STR,  NULL,             0, 0, o_setfn_5,            0        },
  { 123, "f6",           X_STR,  NULL,             0, 0, o_setfn_6,            save_fn  },
  { 124, "f7",           X_STR,  NULL,             0, 0, o_setfn_7,            save_fn  },
  { 125, "f8",           X_STR,  NULL,             0, 0, o_setfn_8,            save_fn  },
  { 126, "f9",           X_STR,  NULL,             0, 0, o_setfn_9,            save_fn  },
  { 127, "f10",          X_STR,  NULL,             0, 0, o_setfn_10,           save_fn  },
  { 128, "f11",          X_STR,  NULL,             0, 0, o_setfn_11,           0        },
  { 129, "f12",          X_STR,  NULL,             0, 0, o_setfn_12,           0        },
  { 130, "fn_max_speed", X_INT,  &fn_max_speed,    5, 5000,                 0, 0        },
  { 131, "fn_max_clock", X_DBL,  &fn_max_clock,    0.001, 65536.0,          0, 0        },
  { 132, "fn_max_boost", X_INT,  &fn_max_boost,    1, 100,                  0, 0        },
  { 133, "romaji",       X_INT,  &romaji_type,     0, 2,                    0, OPT_SAVE },
  { 134, "kanjikey",     X_NOP,  0,                0, 0, o_kanjikey,           0        },
  { 135, "joyswap",      X_FIX,  &joy_swap_button, TRUE,                  0,0, OPT_SAVE },
  { 136, "mouseswap",    X_FIX,  &mouse_swap_button, TRUE,                0,0, OPT_SAVE },
  { 137, "mousespeed",   X_INT,  &mouse_sensitivity, 5, 400,                0, OPT_SAVE },

  /* 161〜180: メニュー設定オプション */

  { 161, "menu",         X_NOP,  0,                0, 0, o_menu,               0        },
  { 162, "english",      X_FIX,  &menu_lang,       MENU_ENGLISH,          0,0, 0        },
  { 162, "japanese",     X_FIX,  &menu_lang,       MENU_JAPAN,            0,0, 0        },
  { 163, "utf8",         X_FIX,  &file_coding,     2,                     0,0, 0        },
  { 163, "sjis",         X_FIX,  &file_coding,     1,                     0,0, 0        },
  { 163, "euc",          X_FIX,  &file_coding,     0,                     0,0, 0        },
  { 164, "bmp",          X_FIX,  &snapshot_format, SNAPSHOT_FMT_BMP,      0,0, OPT_SAVE },
  { 164, "ppm",          X_FIX,  &snapshot_format, SNAPSHOT_FMT_PPM,      0,0, OPT_SAVE },
  { 164, "raw",          X_FIX,  &snapshot_format, SNAPSHOT_FMT_RAW,      0,0, OPT_SAVE },
  { 165, "swapdrv",      X_FIX,  &menu_swapdrv,    TRUE,                  0,0, OPT_SAVE },
  { 165, "noswapdrv"  ,  X_FIX,  &menu_swapdrv,    FALSE,                 0,0, OPT_SAVE },
  { 166, "menucursor",   X_FIX,  &use_swcursor,    TRUE,                  0,0, 0        },
  { 166, "nomenucursor", X_FIX,  &use_swcursor,    FALSE,                 0,0, 0        },

  /* 181〜250: システム設定オプション */

  { 181, "romdir",       X_STR,  NULL,             0, 0, o_romdir,             0        },
  { 182, "diskdir",      X_STR,  NULL,             0, 0, o_diskdir,            0        },
  { 183, "tapedir",      X_STR,  NULL,             0, 0, o_tapedir,            0        },
  { 184, "snapdir",      X_STR,  NULL,             0, 0, o_snapdir,            0        },
  { 185, "statedir",     X_STR,  NULL,             0, 0, o_statedir,           0        },
  { 186, "noconfig",     X_FIX,  &load_config,     FALSE,                 0,0, 0        },
  { 187, "compatrom",    X_STR,  &file_compatrom,                       0,0,0, 0        },
  { 188, "resume",       X_NOP,  0,                0, 0, o_resume,             0        },
  { 189, "resumefile",   X_STR,  NULL,             0, 0, o_resumefile,         0        },
  { 190, "resumeforce",  X_STR,  NULL,             0, 0, o_resumeforce,        0        },
  { 191, "focus",        X_FIX,  &need_focus,      TRUE,                  0,0, 0        },
  { 191, "nofocus",      X_FIX,  &need_focus,      FALSE,                 0,0, 0        },
  { 192, "sleep",        X_FIX,  &wait_by_sleep,   TRUE,                  0,0, OPT_SAVE },
  { 192, "nosleep",      X_FIX,  &wait_by_sleep,   FALSE,                 0,0, OPT_SAVE },
  /*193  削除 */
  { 194, "ro",           X_FIX,  &menu_readonly,   TRUE,                  0,0, 0        },
  { 194, "rw",           X_FIX,  &menu_readonly,   FALSE,                 0,0, 0        },
  { 195, "ignore_ro",    X_FIX,  &fdc_ignore_readonly,  TRUE,             0,0, 0        },
  { 195, "noignore_ro",  X_FIX,  &fdc_ignore_readonly,  FALSE,            0,0, 0        },
  { 196, "diskimage",    X_STR,  &config_image.d[DRIVE_1], 0, 0, o_diskimage,  0        },
  { 197, "saveconfig",   X_FIX,  &save_config,     TRUE,                  0,0, OPT_SAVE },
  { 197, "nosaveconfig", X_FIX,  &save_config,     FALSE,                 0,0, OPT_SAVE },

  /* 251〜299: デバッグ用オプション */

  { 251, "help",         X_NOP,  0,                0, 0, o_help,               0        },
  { 252, "verbose",      X_INT,  &verbose_level,   0x00, 0xff,              0, 0        },
  { 253, "printer",      X_STR,  &config_image.prn,  0, 0, o_printer,          0        },
  { 254, "serialin",     X_STR,  &config_image.sin,  0, 0, o_serialin,         0        },
  { 255, "serialout",    X_STR,  &config_image.sout, 0, 0, o_serialout,        0        },
  { 256, "record",       X_STR,  &file_rec,                             0,0,0, 0        },
  { 257, "playback",     X_STR,  &file_pb,                              0,0,0, 0        },
  { 258, "timestop",     X_FIX,  &calendar_stop,   TRUE,                  0,0, 0        },
  { 259, "vsync",        X_DBL,  &vsync_freq_hz,   10.0, 240.0,             0, 0        },
  { 260, "soundclock",   X_DBL,  &sound_clock_mhz, 0.001, 65536.0,          0, 0        },
  { 261, "subload",      X_INT,  &sub_load_rate,   0, 65536,                0, 0        },
  { 262, "cmt_wait",     X_FIX,  &cmt_wait,        TRUE,                  0,0, 0        },
  { 262, "cmt_nowait",   X_FIX,  &cmt_wait,        FALSE,                 0,0, 0        },
  { 263, "linear_ram",   X_FIX,  &linear_ext_ram,  TRUE,                  0,0, 0        },
  { 263, "nolinear_ram", X_FIX,  &linear_ext_ram,  FALSE,                 0,0, 0        },
  { 264, "cmd_sing",     X_FIX,  &use_cmdsing,     TRUE,                  0,0, 0        },
  { 264, "no_cmd_sing",  X_FIX,  &use_cmdsing,     FALSE,                 0,0, 0        },

#ifdef  USE_MONITOR
  { 271, "debug",        X_FIX,  &debug_mode,      TRUE,                  0,0, 0        },
  { 271, "nodebug",      X_FIX,  &debug_mode,      FALSE,                 0,0, 0        },
  { 272, "monitor",      X_FIX,  &debug_mode,      TRUE, 0, o_monitor,         0        },
  { 273, "fdcdebug",     X_FIX,  &fdc_debug_mode,  TRUE ,                 0,0, 0        },
#else
  {   0, "debug",        X_INV,                                       0,0,0,0, 0        },
  {   0, "monitor",      X_INV,                                       0,0,0,0, 0        },
  {   0, "fdcdebug",     X_INV,                                       0,0,0,0, 0        },
#endif

  { 281, "nofont",       X_FIX,  &use_built_in_font,TRUE,                 0,0, 0        },
  { 281, "font",         X_FIX,  &use_built_in_font,FALSE,                0,0, 0        },
  { 282, "profiler",     X_INT,  &debug_profiler,  0x00, 0xff,              0, 0        },
  { 283, "pio_debug",    X_INT,  &pio_debug,       0, 3,                    0, 0        },
  { 284, "fdc_debug",    X_INT,  &fdc_debug,       0, 3,                    0, 0        },
  { 285, "main_debug",   X_INT,  &main_debug,      0, 3,                    0, 0        },
  { 286, "sub_debug",    X_INT,  &sub_debug,       0, 3,                    0, 0        },


#if 0
  /* 以下のオプションは、すべて廃止 */
  {   0, "menukey",      X_INV,                                       0,0,0,0, 0        },
  {   0, "joyassign",    X_INT,  &invalid_arg,                          0,0,0, 0        },
  {   0, "joykey",       X_INV,                                       0,0,0,0, 0        },
  {   0, "waitfreq",     X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "button2menu",  X_INV,                                       0,0,0,0, 0        },
  {   0, "nobutton2menu",X_INV,                                       0,0,0,0, 0        },
  {   0, "logo",         X_INV,                                       0,0,0,0, 0        },
  {   0, "nologo",       X_INV,                                       0,0,0,0, 0        },
  {   0, "load",         X_INV,  &invalid_arg,                          0,0,0, 0        },
#endif


  /* 300〜349: システム依存オプション */
  /* この範囲のグループは、システム依存オプションのテーブル用に予約 */


  /* 350〜399: サウンド依存オプション */
  /* この範囲のグループは、サウンド依存オプションのテーブル用に予約 */

#ifndef	USE_SOUND		/* サウンドなし時のオプション (無視) */

  /* src/snddrv/src/unix/sound.c */
  {   0, "samples",            X_INV,                                 0,0,0,0, 0        },
  {   0, "sam",                X_INV,                                 0,0,0,0, 0        },
  {   0, "nosamples",          X_INV,                                 0,0,0,0, 0        },
  {   0, "nosam",              X_INV,                                 0,0,0,0, 0        },
  {   0, "samplefreq",         X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "sf",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "bufsize",            X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "bs",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "volume",             X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "v",                  X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "audiodevice",        X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "ad",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "mixerdevice",        X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "md",                 X_INV,  &invalid_arg,                    0,0,0, 0        },

  {   0, "sound",              X_INV,                                 0,0,0,0, 0        },
  {   0, "snd",                X_INV,                                 0,0,0,0, 0        },
  {   0, "nosound",            X_INV,                                 0,0,0,0, 0        },
  {   0, "nosnd",              X_INV,                                 0,0,0,0, 0        },
  {   0, "audio",              X_INV,                                 0,0,0,0, 0        },
  {   0, "ao",                 X_INV,                                 0,0,0,0, 0        },
  {   0, "noaudio",            X_INV,                                 0,0,0,0, 0        },
  {   0, "noao",               X_INV,                                 0,0,0,0, 0        },
  {   0, "fmgen",              X_INV,                                 0,0,0,0, 0        },
  {   0, "nofmgen",            X_INV,                                 0,0,0,0, 0        },
  {   0, "fmvol",              X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "fv",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "psgvol",             X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "pv",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "beepvol",            X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "bv",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "rhythmvol",          X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "rv",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "adpcmvol",           X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "av",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "fmgenvol",           X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "fmv",                X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "samplevol",          X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "spv",                X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "close",              X_INV,                                 0,0,0,0, 0        },
  {   0, "noclose",            X_INV,                                 0,0,0,0, 0        },
  {   0, "bufnum",             X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "bn",                 X_INV,  &invalid_arg,                    0,0,0, 0        },

  /* src/snddrv/src/unix/sysdep/sysdep_dsp.c */
  {   0, "dsp-plugin",         X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "dp",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "list-dsp-plugins",   X_INV,                                 0,0,0,0, 0        },
  {   0, "ldp",                X_INV,                                 0,0,0,0, 0        },
  {   0, "timer",              X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "notimer",            X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "ti",                 X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "noti",               X_INV,  &invalid_arg,                    0,0,0, 0        },

  /* src/snddrv/src/unix/sysdep/sysdep_mixer.c */
  {   0, "sound-mixer-plugin", X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "smp",                X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "list-mixer-plugins", X_INV,                                 0,0,0,0, 0        },
  {   0, "lmp",                X_INV,                                 0,0,0,0, 0        },

  /* src/snddrv/src/unix/sysdep/dsp-drivers/.c */
  {   0, "list-alsa-cards",    X_INV,                                 0,0,0,0, 0        },
  {   0, "list-alsa-pcm",      X_INV,                                 0,0,0,0, 0        },
  {   0, "alsa-buffer",        X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "abuf",               X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "artsBufferTime",     X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "abt",                X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "audio-preferred",    X_INV,                                 0,0,0,0, 0        },
  {   0, "noaudio-preferred",  X_INV,                                 0,0,0,0, 0        },
  {   0, "audio-primary",      X_INV,                                 0,0,0,0, 0        },
  {   0, "noaudio-primary",    X_INV,                                 0,0,0,0, 0        },

  /* src/snddrv/src/unix/sysdep/mixer-drivers/.c */
  {   0, "alsa-mixer",         X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "amixer",             X_INV,  &invalid_arg,                    0,0,0, 0        },

  /* src/snddrv/quasi88/SDL/snddrv-SDL.c */
  {   0, "sdlbufsize",         X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "sdlbufnum",          X_INV,  &invalid_arg,                    0,0,0, 0        },

  /* obsolate */
  {   0, "fakesound",          X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "fsnd",               X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "alsa-pcm",           X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "apcm",               X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "streamspace",        X_INV,  &invalid_arg,                    0,0,0, 0        },
  {   0, "ss",                 X_INV,  &invalid_arg,                    0,0,0, 0        },

#endif

  /* 終端 */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};



/*--------------------------------------------------------------------------
 * 2個の連続した引数 opt1, opt2 を処理する。
 * まず、オプションテーブル osd_options から合致するかチェックする。
 * 合致しなければ、 sound_options から合致するかチェックする。
 * まだ合致しなければ、 xmame_config_check_option() を呼び出す。
 * 合致した場合でも、優先度 priority によっては、処理がスキップされる。
 *
 *	戻り値	1  … 処理した引数が1個 (opt1 のみ処理。 opt2 は未処理)
 *		2  … 処理した引数が2個 (opt1 と opt2 を処理)
 *		0  … opt1 が未知の引数のため、 opt1 opt2 ともに未処理
 *		-1 … 内部で致命的な異常が発生
 *
 *	処理途中の異常 (引数で指定した値が範囲外など) や、
 *	優先度により処理がスキップされたような場合は、
 *	正しく処理できた場合と同様に、 1 か 2 を返す。
 *--------------------------------------------------------------------------*/

static	int	check_option(char *opt1, char *opt2, int priority,
			     const T_CONFIG_TABLE *osd_options,
			     const T_CONFIG_TABLE *sound_options)
{
    const T_CONFIG_TABLE *op;
    int  ret_val = 1;
    int  ignore, applied;
    char *end;


    if (opt1 == NULL)   return 0;
    if (opt1[0] != '-') return -1;


    /* オプション文字列にに合致するものを探しましょう */

    for (op = &option_table[0]; op->name; op++) {
	if (strcmp(&opt1[1], op->name) == 0) break;
    }

    if (op->name == NULL) {

	/* 見つからなければ、 osd_options のオプションから探します */

	if (osd_options) {
	    for (op = &osd_options[0]; op->name; op++) {
		if (strcmp(&opt1[1], op->name) == 0) break;
	    }
	}

	if (op->name == NULL) {

	    /* 見つからなければ、 sound_options のオプションから探します */

	    if (sound_options) {
		for (op = &sound_options[0]; op->name; op++) {
		    if (strcmp(&opt1[1], op->name) == 0) break;
		}
	    }

	    if (op->name == NULL) {

		/* それでも見つからなければ、MAME のオプションから探します */

		ret_val = xmame_config_check_option(opt1, opt2, priority);
		return ret_val;
	    }
	}
    }

	/* 見つかれば処理。ただし、反映は '優先度が高い' か '同じ' 場合のみ */

    if (priority < opt_prioroty[ op->group ]) {
	ignore = TRUE;
    } else {
	ignore = FALSE;
    }
    applied = FALSE;


	/* オプションのタイプ別に処理します */

    switch (op->type) {

    case X_FIX:		/* なし:   *var = (int)val1 [定数]           */
	{
	    if (ignore == FALSE) {
		*((int*)op->var) = (int)op->val1;
		applied = TRUE;
	    }
	}
	break;

    case X_INT:		/* int:    *var = argv  [範囲 var1〜val2]    */
	{
	    int low, high, work;

	    if (opt2) {
		ret_val ++;
		if (ignore == FALSE) {
		    low  = (int)op->val1;
		    high = (int)op->val2;
		    work = strtol(opt2, &end, 0);

		    if ((*end == '\0') && (low <= work) && (work <= high)) {
			*((int*)op->var) = work;
			applied = TRUE;
		    } else {
			fprintf(stderr,
				"error: invalid value %s %s\n", opt1, opt2);
		    }
		}
	    } else {
		fprintf(stderr, "error: %s requires an argument\n", opt1);
	    }
	}
	break;

    case X_DBL:		/* double: *var = argv  [範囲 var1〜val2]    */
	{
	    double low, high, work;

	    if (opt2) {
		ret_val ++;
		if (ignore == FALSE) {
		    low  = (double)op->val1;
		    high = (double)op->val2;
		    work = strtod(opt2, &end);

		    if ((*end == '\0') && (low <= work) && (work <= high)) {
			*((double*)op->var) = work;
			applied = TRUE;
		    } else {
			fprintf(stderr,
				"error: invalid value %s %s\n", opt1, opt2);
		    }
		}
	    } else {
		fprintf(stderr, "error: %s requires an argument\n", opt1);
	    }
	}
	break;

    case X_STR:		/* 文字列: strcpy(var, argv);                */
	{
	    char *work;

	    if (opt2) {
		ret_val ++;
		if (ignore == FALSE) {
		    if (op->var) {
			work = (char*)malloc(strlen(opt2) + 1);
			if (work == NULL) {
			    fprintf(stderr,
				    "error: malloc failed for %s\n", opt1);
			    return -1;
			} else {
			    strcpy(work, opt2);
			    if (*(char **)op->var) {
				free(*(char **)op->var);
			    }
			    *(char **)op->var = work;
			    applied = TRUE;
			}
		    } else {
			applied = TRUE;
		    }
		}
	    } else {
		fprintf(stderr, "error: %s requires an argument\n", opt1);
	    }
	}
	break;

    case X_NOP:		/* 無処理:                                   */
	if (op->var) {
	    if (opt2) {
		ret_val ++;
		if (ignore == FALSE) {
		    applied = TRUE;
		}
	    } else {
		fprintf(stderr, "error: %s requires an argument\n", opt1);
	    }
	} else {
	    if (ignore == FALSE) {
		applied = TRUE;
	    }
	}
	break;

    case X_INV:		/* 無効:                                     */
	if (op->var && opt2) {
	    ret_val ++;
	    fprintf(stderr, "error: invalid option %s %s\n", opt1, opt2);
	} else {
	    fprintf(stderr, "error: invalid option %s\n", opt1);
	}
	break;

    default:
	break;
    }


    /* 処理後の呼び出し関数があれば、それを呼びます */

    if (op->func && applied) {
	if ((op->func)(opt2) != 0) {
	    fprintf(stderr, "error: invalid option %s %s\n", opt1, opt2);
	}
    }


    /* 優先度を書き換えておわり */

    if (applied) {
	opt_prioroty[ op->group ] = priority;
    }

    /*if (ret_val==1) printf("done:%s\n",opt1);*/
    /*if (ret_val==2) printf("done:%s %s\n",opt1,opt2);*/

    return ret_val;
}





/*--------------------------------------------------------------------------
 * 起動時のオプションを解析する。
 *	戻り値は、継続不能な異常が発生した時は偽、それ以外は真。
 *
 *	-help オプションをつけると、強制的に終了する。
 *--------------------------------------------------------------------------*/
static	int	get_option(int argc, char *argv[], int priority,
			   const T_CONFIG_TABLE *osd_options,
			   const T_CONFIG_TABLE *sound_options)
{
    int  i, j;
    int  drive = DRIVE_1;
    char *p;

    if ((argc == 0) || (argv == NULL)) return TRUE;

    for (i=1; i<argc; ) {

	/* '-' 以外で始まるオプションは、ディスクイメージのファイル名 */
	if (*argv[i] != '-') {
	    char *fname = NULL;

	    /* イメージファイルが指定可能かどうかをチェック (不可でも継続) */
	    if (strlen(argv[i]) >= QUASI88_MAX_FILENAME) {
		fprintf(stderr,
			"error: image file name \"%s\" is too long\n",
			argv[i]);
	    } else {
		if (drive >= NR_DRIVE) {
		    fprintf(stderr, "warning: too many image file\n");
		} else {
		    fname = argv[i];

		    p = (char*)malloc(strlen(fname) + 1);
		    if (p == NULL) {
			fprintf(stderr, "error: malloc failed for arg\n");
			return FALSE;
		    }
		    strcpy(p, fname);

		    free(config_image.d[ drive ]);
		    config_image.d[ drive ] = p;
		    config_image.n[ drive ] = 0;
		    drive ++;
		}
	    }

	    /* 以降の引数で、数値が続けば、それはイメージ番号を示す */
	    for (j=0; i+j+1 < argc; j++) {	/* 引数が続く限り処理 */

		char *end;
		int   img = strtol(argv[i+j+1], &end, 0);

		if (*end != '\0') break;	/* 引数が数値でなければ中断 */

		if (fname) {
		    if (j == 0) drive --;

		    if (drive >= NR_DRIVE) {
			fprintf(stderr, "error: too many image-number\n");
		    } else {
			p = (char*)malloc(strlen(fname) + 1);
			if (p == NULL) {
			    fprintf(stderr, "error: malloc failed for arg\n");
			    return FALSE;
			}
			strcpy(p, fname);

			if ((img < 1) || (img > MAX_NR_IMAGE)) {
			    fprintf(stderr, "error: invalid image-number %d."
				    " (change image-no to 1)\n", img);
			    img = 1;	/* イメージ番号が範囲外なら、1 にする*/
			}

			free(config_image.d[ drive ]);
			config_image.d[ drive ] = p;
			config_image.n[ drive ] = img;
			drive ++;
		    }
		} else {
		    fprintf(stderr, "error: ignored image-number %d\n", img);
		}
	    }

	    i += j + 1;			/* 処理した引数の個数を加算 */

	} else {		/* '-' で始まる引数は、オプション */

	    j = check_option(argv[i], (i+1 < argc) ? argv[i+1] : NULL,
			     priority, osd_options, sound_options);
	    if (j < 0) {		/* 致命的エラーなら、解析失敗 */
		return FALSE;
	    }
	    if (j == 0) {		/* 未知のオプションは、スキップ */
		fprintf(stderr, "error: unknown option %s\n", argv[i]);
		j = 1;
	    }

	    i += j;			/* 処理した引数の個数を加算 */
	}
    }

    return TRUE;
}






/*--------------------------------------------------------------------------
 * 環境ファイルのオプションを解析する。
 *	戻り値は、継続不能な異常が発生した時は偽、それ以外は真。
 *
 *	-help オプションをつけると、強制的に終了する。
 *--------------------------------------------------------------------------*/

/* 環境ファイル1行あたりの最大文字数 */
#define	MAX_RCFILE_LINE	(256)


static	int	get_config_file(OSD_FILE *fp, int priority,
				const T_CONFIG_TABLE *osd_options,
				const T_CONFIG_TABLE *sound_options)
{
    int  result;
    char line[ MAX_RCFILE_LINE ];
    char buffer[ MAX_RCFILE_LINE ], *b;
    char *parm1, *parm2, *parm3, *str;

    int  line_cnt = 0;


		/* 設定ファイルを1行づつ解析 */

    while (osd_fgets(line, MAX_RCFILE_LINE, fp)) {

	line_cnt ++;
	parm1 = parm2 = parm3 = NULL;
	str = line;

		/* パラメータを parm1〜parm3 にセット */

	{                      b = &buffer[0];    str = my_strtok(b, str); }
	if (str) { parm1 = b;  b += strlen(b)+1;  str = my_strtok(b, str); }
	if (str) { parm2 = b;  b += strlen(b)+1;  str = my_strtok(b, str); }
	if (str) { parm3 = b;  }


		/* パラメータがなければ次の行へ、あれば解析処理 */

	if      (parm1 == NULL) {		/* パラメータなし    */
	    ;

	} else if (parm3) {			/* パラメータ3個以上 */
	    fprintf(stderr,
		    "warning: too many argument in line %d\n", line_cnt);

	} else {				/* パラメータ1〜2個  */
	    result = check_option(parm1, parm2, priority,
				  osd_options, sound_options);

	    if ((result == 1 && parm2 == NULL) || (result == 2 && parm2)) {
		;
	    } else if (result < 0) {	/* 致命的エラーなら、解析失敗 */
		return FALSE;
	    } else {			/* エラー時は エラー行を表示 */
		fprintf(stderr, "warning: error in line %d\n", line_cnt);
	    }
	}

    }

    return TRUE;
}















/***********************************************************************
 * 引数の処理
 *	エラー発生などで処理を続行できない場合、偽を返す。
 ************************************************************************/

static void set_verbose(void)
{
    verbose_proc    = verbose_level & 0x01;
    verbose_z80     = verbose_level & 0x02;
    verbose_io      = verbose_level & 0x04;
    verbose_pio     = verbose_level & 0x08;
    verbose_fdc     = verbose_level & 0x10;
    verbose_wait    = verbose_level & 0x20;
    verbose_suspend = verbose_level & 0x40;
    verbose_snd     = verbose_level & 0x80;
}


int	config_init(int argc,
		    char *argv[],
		    const T_CONFIG_TABLE *osd_options,
		    void	(*osd_help)(void))
{
    int  i, step;
    char *fname;

    if (argv && argv[0]) {
	command = argv[0];
    }
    help_msg_osd = osd_help;
    option_table_osd = osd_options;


    for (i=0; i<NR_DRIVE; i++) {
	config_image.d[i]  = NULL;
	config_image.n[i]  = 0;
	config_image.ro[i] = FALSE;
    }
    config_image.t[CLOAD] = NULL;
    config_image.t[CSAVE] = NULL;
    config_image.prn      = NULL;
    config_image.sin      = NULL;
    config_image.sout     = NULL;


	/* XMAMEサウンド関連の設定を初期化 */

    xmame_config_init();

    option_table_sound = xmame_config_get_opt_tbl();


	/* 設定ファイルのディレクトリ名などを初期化 */

    if (osd_file_config_init() == FALSE) {
	return FALSE;
    }


	/* 起動時のオプションを解析 */

    if (get_option(argc, argv, 2,
		   option_table_osd, option_table_sound) == FALSE) {
	return FALSE;
    }


	/* ディスクイメージ指定ありなら、そのファイル名(パス名)を補完する */

    if (resume_flag == FALSE) {
	int same = FALSE;

		/* 同じファイル(名)を指定しているかを、チェック */
	if (config_image.d[DRIVE_1] &&
	    config_image.d[DRIVE_2] &&
	    strcmp(config_image.d[DRIVE_1], config_image.d[DRIVE_2]) == 0) {
	    same = TRUE;
	}

	for (i=0; i<NR_DRIVE; i++) {

	    if (config_image.d[i]) {
		fname = filename_alloc_diskname(config_image.d[i]);
		if (fname == NULL) {
		    printf("\n");
		    printf("[[[ %-26s ]]]\n", "Open failed");
		    printf("[[[   drive %d: %-15s ]]]\n" "\n",
			   i+1, config_image.d[i]);
		}
		free(config_image.d[i]);
		config_image.d[i] = fname;

		/* 同じファイルを指定している場合、1回目の処理で抜ける */
		if (i == DRIVE_1 && same) {
		    free(config_image.d[DRIVE_2]);
		    config_image.d[DRIVE_2] = fname;
		    break;
		}
	    }
	}
    }


	/* 設定ファイル処理			*/
	/*	step 0 : 共通設定ファイルを解析	*/
	/*	step 1 : 個別設定ファイルを解析	*/

    for (step=0; step<2; step ++) {

	OSD_FILE *fp;
	char *alias;

	set_verbose();

	if (step == 0) {

	    if (load_config == FALSE) continue;

	    /* 共通設定ファイルのファイル名 */
	    fname = filename_alloc_global_cfgname();
	    alias = "Global Config File";

	} else { /* step == 1 */

	    if (resume_flag) continue;

	    if (load_config == FALSE) continue;

	    /* 個別設定ファイルのファイル名 (ディスク or テープ名) */
	    if      (config_image.d[DRIVE_1]) fname = config_image.d[DRIVE_1];
	    else if (config_image.d[DRIVE_2]) fname = config_image.d[DRIVE_2];
	    else if (config_image.t[CLOAD])   fname = config_image.t[CLOAD];
	    else break;

	    fname = filename_alloc_local_cfgname(fname);
	    alias = "Local Config File";
	}

	if (fname) fp = osd_fopen(FTYPE_CFG, fname, "r");
	else       fp = NULL;

	if (verbose_proc) {
	    if (fp) { printf("\"%s\" read and initialize\n", fname); }
	    else    { printf("\"%s\" open failed\n", (fname) ? fname : alias); }
	}
	if (fname) free(fname);

	if (fp) {
	    int result = get_config_file(fp, 1,
					 option_table_osd, option_table_sound);
	    osd_fclose(fp);
	    if (result == FALSE) return FALSE;
	}

    }

    set_verbose();


	/* ディスクイメージのその他の情報をセット */

    for (i=0; i<NR_DRIVE; i++) {
	config_image.n[i] --;		/* イメージ番号は 1減ずる(0〜にする) */
	config_image.ro[i] = menu_readonly;
    }


	/* 互換ROM指定時に、ファイルがあるかチェック */

    if (file_compatrom) {
	fname = filename_alloc_romname(file_compatrom);
	if (fname) {
	    file_compatrom = fname;
	}
    }


	/* 各種ディレクトリの表示 (デバッグ用) */

    if (verbose_proc) {
	const char *d;
	d = osd_dir_cwd();  printf("cwd  directory = %s\n", d ? d : "(undef)");
	d = osd_dir_rom();  printf("rom  directory = %s\n", d ? d : "(undef)");
	d = osd_dir_disk(); printf("disk directory = %s\n", d ? d : "(undef)");
	d = osd_dir_tape(); printf("tape directory = %s\n", d ? d : "(undef)");
	d = osd_dir_snap(); printf("snap directory = %s\n", d ? d : "(undef)");
	d = osd_dir_state();printf("stat directory = %s\n", d ? d : "(undef)");
	d = osd_dir_gcfg(); printf("gcfg directory = %s\n", d ? d : "(undef)");
	d = osd_dir_lcfg(); printf("lcfg directory = %s\n", d ? d : "(undef)");
    }

    return TRUE;
}


void	config_exit(void)
{
    if (save_config) {
	config_save(NULL);
    }

    /* 設定ファイルのディレクトリ名などを後片づけ */
    osd_file_config_exit();

    /* XMAMEサウンド関連の設定を後片付け */
    xmame_config_exit();
}







/***********************************************************************
 * 設定の保存
 *
 ************************************************************************/

static int save_normal(const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    switch (op->type) {
    case X_FIX:
	if (*((int*)op->var) == (int)op->val1) {
	    return TRUE;
	}
	break;

    case X_INT:
	sprintf(opt_arg, "%d", *((int*)op->var));
	return TRUE;
	
    case X_DBL:
	sprintf(opt_arg, "%f", *((double*)op->var));
	return TRUE;
	
    case X_STR:
	if (op->var) {
	    strcat(opt_arg, op->var);
	    return TRUE;
	}
	break;

    case X_NOP:
	return FALSE;

    case X_INV:
	return FALSE;
    }

    return FALSE;
}



static int save_bau(const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    if (baudrate_sw < COUNTOF(baudrate_table)) {
	sprintf(opt_arg, "%d", baudrate_table[ baudrate_sw ]);
	return TRUE;
    }
    return FALSE;
}

static int save_ver(const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    /* 変更時のみ保存すべきだが、ステートセーブ・ロード時も保存されてしまう…*/
    if ('0' <= set_version && set_version <= '9') {
	opt_arg[0] = set_version;
	opt_arg[1] = '\0';
	return TRUE;
    }
    return FALSE;
}



typedef struct {
    const char *name;
    int        *work;
} T_SAVEKEY_TABLE;

static int save_key_sub(int type,
			const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    T_SAVEKEY_TABLE table_func[] =
    {
	{ "",    &function_f[0],  },	/* ダミー */
	{ "f1",  &function_f[1],  },
	{ "f2",  &function_f[2],  },
	{ "f3",  &function_f[3],  },
	{ "f4",  &function_f[4],  },
	{ "f5",  &function_f[5],  },
	{ "f6",  &function_f[6],  },
	{ "f7",  &function_f[7],  },
	{ "f8",  &function_f[8],  },
	{ "f9",  &function_f[9],  },
	{ "f10", &function_f[10], },
	{ "f11", &function_f[11], },
	{ "f12", &function_f[12], },
    };

    T_SAVEKEY_TABLE table_cursor[] =
    {
	{ "cursor_up",    &cursor_key_assign[0], },
	{ "cursor_down",  &cursor_key_assign[1], },
	{ "cursor_left",  &cursor_key_assign[2], },
	{ "cursor_right", &cursor_key_assign[3], },
    };

    T_SAVEKEY_TABLE table_mouse[] =
    {
	{ "mouse_up",    &mouse_key_assign[0], },
	{ "mouse_down",  &mouse_key_assign[1], },
	{ "mouse_left",  &mouse_key_assign[2], },
	{ "mouse_right", &mouse_key_assign[3], },
	{ "mouse_l",     &mouse_key_assign[4], },
	{ "mouse_r",     &mouse_key_assign[5], },
    };

    T_SAVEKEY_TABLE table_joy[] =
    {
	{ "joy_up",      &joy_key_assign[0],  },
	{ "joy_down",    &joy_key_assign[1],  },
	{ "joy_left",    &joy_key_assign[2],  },
	{ "joy_right",   &joy_key_assign[3],  },
	{ "joy_a",       &joy_key_assign[4],  },
	{ "joy_b",       &joy_key_assign[5],  },
	{ "joy_c",       &joy_key_assign[6],  },
	{ "joy_d",       &joy_key_assign[7],  },
	{ "joy_e",       &joy_key_assign[8],  },
	{ "joy_f",       &joy_key_assign[9],  },
	{ "joy_g",       &joy_key_assign[10], },
	{ "joy_h",       &joy_key_assign[11], },
    };

    T_SAVEKEY_TABLE *table;
    int nr_table;
    int key, j;
    const char *s;

    switch (type) {
    case 0:  table = table_func;      nr_table =COUNTOF(table_func);	break;
    case 1:  table = table_cursor;    nr_table =COUNTOF(table_cursor);	break;
    case 2:  table = table_mouse;     nr_table =COUNTOF(table_mouse);	break;
    case 3:  table = table_joy;       nr_table =COUNTOF(table_joy);	break;
    default: return FALSE;
    }

    for (key=0; key<nr_table; key++) {
	if (strcmp(op->name, table[key].name) == 0) {

	    if (*table[key].work) {

		if (type == 0) {	/* ファンクションキーのみ */
		    for (j=0; j<COUNTOF(fn_index); j++) {
			if (*table[key].work == fn_index[j].num) {
			    strcpy(opt_arg, fn_index[j].str);
			    return TRUE;
			}
		    }
		}

		s = keyboard_key882str(*table[key].work);
		if (s) {
		    strcpy(opt_arg, s);
		    return TRUE;
		}
	    }
	    break;
	}
    }

    return FALSE;
}

static int save_fn(const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    return save_key_sub(0, op, opt_arg);
}
static int save_cur(const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    return save_key_sub(1, op, opt_arg);
}
static int save_mou(const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    return save_key_sub(2, op, opt_arg);
}
static int save_joy(const struct T_CONFIG_TABLE *op, char opt_arg[255])
{
    return save_key_sub(3, op, opt_arg);
}



#define	AUTO_SAVE_COMMENT	"# AUTO"



static OSD_FILE *fp_config_write;
static void config_write(const char *opt_name, const char *opt_arg)
{
    char line[ MAX_RCFILE_LINE ];
    char buf[ MAX_RCFILE_LINE ];

    if (opt_name) {
	sprintf(buf, "-%s %s", opt_name, (opt_arg ? opt_arg : ""));
    } else {
	if (opt_arg == NULL) return;
	sprintf(buf, "# -%s", opt_arg);
    }
    sprintf(line, "%-70s %s\n", buf, AUTO_SAVE_COMMENT);
    osd_fputs(line, fp_config_write);
}



int	config_save(const char *fname)
{
    int i;
    signed char saved_option[ OPT_GROUPS ];	/* 設定出力済のグループ一覧 */

    const T_CONFIG_TABLE *op;
    int (*func)(const struct T_CONFIG_TABLE *, char *opt_arg);

    char opt_arg[256];
    char line[ MAX_RCFILE_LINE ];
    char buf[ MAX_RCFILE_LINE ];

    OSD_FILE *fp     = NULL;
    OSD_FILE *fp_bak = NULL;
    char *fname_bak  = NULL;
    int backup_ok;
    int need_lf = FALSE;
    int malloc_fname = FALSE;


    /*** 引数 fname が NULL なら、共通設定ファイルのファイル名を使用 ***/

    if (fname == NULL) {
	fname = filename_alloc_global_cfgname();
	if (fname == NULL) {
	    return FALSE;
	}
	malloc_fname = TRUE;
    }

    /*** 設定ファイルをバックアップする ***/

    backup_ok = FALSE;

    fp = osd_fopen(FTYPE_CFG, fname, "rb");	/* 設定ファイルを開く */
    if (fp) {
						/* バックアップファイル名生成*/
	int flen = strlen(fname);
	int slen = strlen(CONFIG_SUFFIX);

	fname_bak = malloc(flen + 10);			/* +10 は余分に */
	if (fname_bak) {

	    strcpy(fname_bak, fname);

	    /* fname_bak の末尾が、 CONFIG_SUFFIX なら、それを削除 */
	    if ((flen > slen) &&
		my_strcmp(&fname_bak[flen - slen], CONFIG_SUFFIX) == 0) {

		fname_bak[flen - slen] = '\0';
	    }

	    /* fname_bak の末尾は、 .bak */
	    strcat(fname_bak, ".bak");

						/* バックアップファイル開く */
	    fp_bak = osd_fopen(FTYPE_CFG, fname_bak, "wb");
	    if (fp_bak) {
		backup_ok = TRUE;
						/* コピーする */
		while ((i = osd_fgetc(fp)) != EOF) {
		    if (osd_fputc(i, fp_bak) == EOF) {
			backup_ok = FALSE;
			break;
		    }
		}

		osd_fclose(fp_bak);
		fp_bak = NULL;
	    }

	    if (backup_ok == FALSE) { free(fname_bak); }

	    /* ・・・ rename() を使うほうがいいのか？
	       と思ったけど、リネーム先のファイルが存在する場合の挙動が
	       処理系依存らしいので、やめといたほうがいいか・・・ */
	}
	osd_fclose(fp);
	fp = NULL;
    }


    /*** バックアップ成功時は、設定ファイルを新規作成し、設定を一部コピー ***/
    /*** バックアップ失敗時は、設定ファイルは既存のまま、追記             ***/

    if (backup_ok) {				/* バックアップファイル開く */
	fp_bak = osd_fopen(FTYPE_CFG, fname_bak, "r");
	if (fp_bak == NULL) {
	    backup_ok = FALSE;
	}
	free(fname_bak);
    }

    if (backup_ok) {				/* OKなら設定ファイル新規作成*/
	fp = osd_fopen(FTYPE_CFG, fname, "w");
	if (fp == NULL) {
	    osd_fclose(fp_bak);
	    if (malloc_fname) { free((void*)fname); }
	    return FALSE;
	}

	/* バックアップファイルから設定ファイルにコピーする。
	   この時、行末が # AUTO で終る行はコピーしない */

	while (osd_fgets(line, MAX_RCFILE_LINE, fp_bak)) {
	    int flen = strlen(line);
	    int slen = strlen(AUTO_SAVE_COMMENT);

	    if ((flen - 1> slen) &&
		strncmp(&line[flen - 1 - slen], AUTO_SAVE_COMMENT, slen) == 0){
		/* skip */
	    } else {
		osd_fputs(line, fp);

		if (line[flen-1] == '\n') { need_lf = FALSE; }
		else                      { need_lf = TRUE; }
	    }
	    /* このへん、エラーをチェックしてないぞ… */
	}

	osd_fclose(fp_bak);

    } else {					/* NGなら設定ファイルは既存で*/
	fp = osd_fopen(FTYPE_CFG, fname, "a");
	if (fp == NULL) {
	    if (malloc_fname) { free((void*)fname); }
	    return FALSE;
	}
    }


    /*** 設定ファイルに、オプションテーブルの設定を追記していく ***/

    memset(saved_option, 0, sizeof(saved_option));

    /* 最後が改行でない場合は、 \n を追加 (append時はチェックできない…) */
    if (need_lf) {
	osd_fputs("\n", fp);
    }

    /* ここから追記開始 */
    sprintf(buf, "# The following settings are written by %s",
						    Q_TITLE " ver " Q_VERSION);
    sprintf(line, "%-70s %s\n", buf, AUTO_SAVE_COMMENT);
    osd_fputs(line, fp);

    for (i=0; i<3; i++) {

	if      (i==0) op = &option_table[0];
	else if (i==1) op = option_table_osd;
	else           op = option_table_sound;

	if (op == NULL) continue;

	for (; op->name; op++) {

	    if (saved_option[ op->group ] == FALSE) {

		if (op->save_func != NULL) {

		    memset(opt_arg, 0, sizeof(opt_arg));

		    if (op->save_func == OPT_SAVE) func = save_normal;
		    else                           func = op->save_func;

		    if ((func)(op, opt_arg)) {

			saved_option[ op->group ] = TRUE;

			sprintf(buf, "-%s %s", op->name, opt_arg);
			sprintf(line, "%-70s %s\n", buf, AUTO_SAVE_COMMENT);
			osd_fputs(line, fp);
		    }
		}
	    }
	}
    }

    /* とりあえずひと区切り… */
    sprintf(buf, "# ------------------------------");
    sprintf(line, "%-70s %s\n", buf, AUTO_SAVE_COMMENT);
    osd_fputs(line, fp);


    /* さらに、追記すべきサウンドオプション設定があれば、処理 */
    fp_config_write = fp;
    xmame_config_save_option(config_write);


    osd_fclose(fp);

    if (malloc_fname) { free((void*)fname); }


    /* ステータスに表示 */
    if (quasi88_is_exec()) {
	status_message(1, STATUS_INFO_TIME, "Config-file saved");
    }
    return TRUE;
}
