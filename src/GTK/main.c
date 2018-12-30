/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <gtk/gtk.h>

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */


/***********************************************************************
 * オプション
 ************************************************************************/
static	const	T_CONFIG_TABLE gtksys_options[] =
{
  /* 300〜349: システム依存オプション */

  /*  -- GRAPHIC -- */
  { 300, "gdkimage",     X_FIX,  &use_gdk_image,   TRUE,                  0,0, 0        },
  { 300, "nogdkimage",   X_FIX,  &use_gdk_image,   FALSE,                 0,0, 0        },


  /* 終端 */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};

static	void	help_msg_gtksys(void)
{
  fprintf
  (
   stdout,
   "  ** GRAPHIC (GTK depend) **\n"
   "    -gdkimage/-nogdkimage   use GdkImage/use GdkRGB [-gdkimage]\n"
   );
}



/***********************************************************************
 * メイン処理
 ************************************************************************/
static	void	gtksys_init(void);
static	void	gtksys_exit(void);
static	void	finish(void);

int	main(int argc, char **argv)
{
    gtk_set_locale();
#ifdef __CYGWIN__
    _Xsetlocale(LC_ALL, "ja_JP.EUC");	/* ??? */
#endif	// __CYGWIN__

    gtk_init(&argc, &argv);
    /* gtk_rc_parse("./gtkrc"); */

    if (config_init(argc, argv,		/* 環境初期化 & 引数処理 */
		    gtksys_options,
		    help_msg_gtksys)) {

	quasi88_atexit(finish);		/* quasi88() 実行中に強制終了した際の
					   コールバック関数を登録する */

	gtksys_init();

	gtk_main();

	config_exit();			/* 引数処理後始末 */
    }

    return 0;
}



/*
 * 強制終了時のコールバック関数 (quasi88_exit()呼出時に、処理される)
 */
static	void	finish(void)
{
    gtksys_exit();

    config_exit();			/* 引数処理後始末 */
}



/*---------------------------------------------------------------------------*/
static	gint		idle_id = 0;
static	int		start_flg = FALSE;
static	gboolean	idle_callback(gpointer dummy);

static	void	gtksys_init(void)
{
    idle_id = g_idle_add(idle_callback, NULL);

    start_flg = TRUE;

    quasi88_start();
}

static	void	gtksys_exit(void)
{
    quasi88_stop(TRUE);

    if (idle_id) {
	g_source_remove(idle_id);
	idle_id = 0;
    }
    if (start_flg) {
	gtk_main_quit();
	start_flg = FALSE;
    }
}

static	gboolean idle_callback(gpointer dummy)
{
    switch (quasi88_loop()) {
    case QUASI88_LOOP_EXIT:		/* 終了 */
	gtksys_exit();
	/* exit(0); */				/* Motifなら 最後に exit() ? */
	return FALSE;

    case QUASI88_LOOP_ONE:		/* 1フレーム経過 */
#if 0
    	{
	    static int i = 0, j=0;
	    if (++j > 55) {
		printf( "%d\n", i );
		i++;
		j=0;
	    }
	}
#endif
	return TRUE;

    case QUASI88_LOOP_BUSY:		/* 無視 */
    default:
	return TRUE;
    }
}



/***********************************************************************
 * ステートロード／ステートセーブ
 ************************************************************************/

/*	他の情報すべてがロード or セーブされた後に呼び出される。
 *	必要に応じて、システム固有の情報を付加してもいいかと。
 */

int	stateload_system(void)
{
    return TRUE;
}
int	statesave_system(void)
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
    static const char *about_en =
    {
	"Fullscreen mode not supported.\n"
	"Joystick not supported.\n"
    };

    static const char *about_jp =
    {
	"フルスクリーン表示はサポートされていません\n"
	"ジョイスティックはサポートされていません\n"
	"マウスカーソルの表示制御はサポートされていません\n"
	"キー設定ファイルの読み込みはサポートされていません\n"
    };


    *result_code = -1;				/* 文字コード指定なし */

    if (req_japanese == FALSE) {
	*message = about_en;
    } else {
	*message = about_jp;
    }

    return TRUE;
}
