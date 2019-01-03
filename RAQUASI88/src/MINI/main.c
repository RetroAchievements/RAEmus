/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */


/***********************************************************************
 * メイン処理
 ************************************************************************/
static	void	finish(void);

int	main(void)
{
    if (config_init(0, NULL,		/* 環境初期化 & 引数処理 */
		    NULL,
		    NULL)) {

	quasi88_atexit(finish);		/* quasi88() 実行中に強制終了した際の
					   コールバック関数を登録する */
	quasi88();			/* PC-8801 エミュレーション */

	config_exit();			/* 引数処理後始末 */
    }

    return 0;
}



/*
 * 強制終了時のコールバック関数 (quasi88_exit()呼出時に、処理される)
 */
static	void	finish(void)
{
    config_exit();			/* 引数処理後始末 */
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
    return FALSE;
}
