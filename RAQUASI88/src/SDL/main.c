/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <SDL.h>

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */
#include "keyboard.h"	/* romaji_type */


/***********************************************************************
 * オプション
 ************************************************************************/
static int oo_env(const char *var, const char *str)
{
    char *buf = (char*)malloc(strlen(var) + strlen(str) + 1);
    sprintf(buf, "%s%s", var, str);
    SDL_putenv(buf);
    free(buf);
    return 0;
}
static int o_videodrv(char *str) { return oo_env("SDL_VIDEODRIVER=", str); }
static int o_audiodrv(char *str) { return oo_env("SDL_AUDIODRIVER=", str); }

static	int	invalid_arg;
static	const	T_CONFIG_TABLE sdl_options[] =
{
  /* 300〜349: システム依存オプション */

  /*  -- GRAPHIC -- */
  { 300, "hwsurface",    X_FIX,  &use_hwsurface,   TRUE,                  0,0, 0        },
  { 300, "swsurface",    X_FIX,  &use_hwsurface,   FALSE,                 0,0, 0        },
  { 301, "doublebuf",    X_FIX,  &use_doublebuf,   TRUE,                  0,0, 0        },
  { 301, "nodoublebuf",  X_FIX,  &use_doublebuf,   FALSE,                 0,0, 0        },

  /*  -- INPUT -- */
  { 311, "use_joy",      X_FIX,  &use_joydevice,   TRUE,                  0,0, 0        },
  { 311, "nouse_joy",    X_FIX,  &use_joydevice,   FALSE,                 0,0, 0        },
  { 312, "keyboard",     X_INT,  &keyboard_type,   0, 2,                    0, 0        },
  { 313, "keyconf",      X_STR,  &file_keyboard,                        0,0,0, 0        },
  { 314, "cmdkey",       X_FIX,  &use_cmdkey,      TRUE,                  0,0, 0        },
  { 314, "nocmdkey",     X_FIX,  &use_cmdkey,      FALSE,                 0,0, 0        },

  /*  -- SYSTEM -- */
  { 320, "videodrv",     X_STR,  NULL,             0, 0, o_videodrv,           0        },
  { 321, "audiodrv",     X_STR,  NULL,             0, 0, o_audiodrv,           0        },
  { 322, "show_fps",     X_FIX,  &show_fps,        TRUE,                  0,0, 0        },
  { 322, "hide_fps",     X_FIX,  &show_fps,        FALSE,                 0,0, 0        },


  /*  -- 無視 -- (他システムの引数つきオプション) */
  {   0, "cmap",         X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "sleepparm",    X_INT,  &invalid_arg,                          0,0,0, 0        },


  /* 終端 */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};

static	void	help_msg_sdl(void)
{
  fprintf
  (
   stdout,
   "  ** GRAPHIC (SDL depend) **\n"
   "    -hwsurface/-swsurface   use Hardware/Software surface [-hwsurface]\n"
   "    -doublebuf/-nodoublebuf Use/Not use double buffer [-nodoublebuf]\n"
   "  ** INPUT (SDL depend) **\n"
   "    -use_joy/-nouse_joy     Enable/Disabel system joystick [-use_joy]\n"
   "    -keyboard <0|1|2>       Set keyboard type (0:config/1:106key/2:101key) [1]\n"
   "    -keyconf <filename>     Specify keyboard configuration file <filename>\n"
   "    -cmdkey/-nocmdkey       Command key to menu (Classic Mac OS only) [-cmdkey]\n"
   "  ** SYSTEM (SDL depend) **\n"
   "    -videodrv <drv>         do putenv(\"SDL_VIDEODRIVER=drv\")\n"
   "    -audiodrv <drv>         do putenv(\"SDL_AUDIODRIVER=drv\")\n"
   "    -show_fps/-hide_fps     Show/Hide FPS (experimentral)\n"
  );
}



/***********************************************************************
 * メイン処理
 ************************************************************************/
static	void	finish(void);

int	main(int argc, char *argv[])
{
    int x = 1;

	/* エンディアンネスチェック */

#ifdef LSB_FIRST
    if (*(char *)&x != 1) {
	fprintf(stderr,
		"%s CAN'T EXCUTE !\n"
		"This machine is Big-Endian.\n"
		"Compile again comment-out 'LSB_FIRST = 1' in Makefile.\n",
		argv[0]);
	return -1;
    }
#else
    if (*(char *)&x == 1) {
	fprintf(stderr,
		"%s CAN'T EXCUTE !\n"
		"This machine is Little-Endian.\n"
		"Compile again comment-in 'LSB_FIRST = 1' in Makefile.\n",
		argv[0]);
	return -1;
    }
#endif

#ifdef	WIN32
    /* 一部の初期値を改変 (いいやり方はないかな…) */
    romaji_type = 1;			/* ローマ字変換の規則を MS-IME風に */
#endif


    if (config_init(argc, argv,		/* 環境初期化 & 引数処理 */
		    sdl_options,
		    help_msg_sdl)) {

	if (sdl_init()) {		/* SDL関連の初期化 */

	    quasi88_atexit(finish);	/* quasi88() 実行中に強制終了した際の
					   コールバック関数を登録する */
	    quasi88();			/* PC-8801 エミュレーション */

	    sdl_exit();			/* SDL関連後始末 */
	}

	config_exit();			/* 引数処理後始末 */
    }

    return 0;
}



/*
 * 強制終了時のコールバック関数 (quasi88_exit()呼出時に、処理される)
 */
static	void	finish(void)
{
    sdl_exit();				/* SDL関連後始末 */
    config_exit();			/* 引数処理後始末 */
}



/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

/*	他の情報すべてがロード or セーブされた後に呼び出される。
 *	必要に応じて、システム固有の情報を付加してもいいかと。
 */

int	stateload_system( void )
{
  return TRUE;
}
int	statesave_system( void )
{
  return TRUE;
}



/***********************************************************************
 * メニュー画面に表示する、システム固有メッセージ
 ************************************************************************/

int	menu_about_osd_msg(int        req_japanese,
			   int        *result_code,
			   const char *message[])
{
    return FALSE;
}
