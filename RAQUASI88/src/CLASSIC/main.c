/************************************************************************/
/*									*/
/*				QUASI88					*/
/*									*/
/************************************************************************/

/*----------------------------------------------------------------------*
 * Classicバージョンのソースコードの大部分は、                          *
 * Koichi NISHIDA 氏の Classic iP6 PC-6001/mk2/6601 emulator のソースを *
 * 参考にさせていただきました。                                         *
 *                                                   (c) Koichi NISHIDA *
 *----------------------------------------------------------------------*/

#include "quasi88.h"
#include "device.h"

#include "getconf.h"	/* config_init */
#include "keyboard.h"	/* romaji_type */
#include "suspend.h"	/* stateload_system */
#include "menu.h"	/* menu_about_osd_msg */

#include "intr.h"
#include "screen.h"


/***********************************************************************
 * オプション
 ************************************************************************/
static	int	invalid_arg;
static	const	T_CONFIG_TABLE classic_options[] =
{
  /* 300〜349: システム依存オプション */

  /*  -- GRAPHIC -- */
  { 300, "8bpp",         X_FIX,  &mac_8bpp,        TRUE,                  0,0, 0        },
  { 300, "15bpp",        X_FIX,  &mac_8bpp,        FALSE,                 0,0, 0        },
  { 300, "16bpp",        X_FIX,  &mac_8bpp,        FALSE,                 0,0, 0        },

  /*  -- 無視 -- (他システムの引数つきオプション) */
  {   0, "cmap",         X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "keyboard",     X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "keyconf",      X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "sleepparm",    X_INT,  &invalid_arg,                          0,0,0, 0        },
  {   0, "videodrv",     X_INV,  &invalid_arg,                          0,0,0, 0        },
  {   0, "audiodrv",     X_INV,  &invalid_arg,                          0,0,0, 0        },

  /* 終端 */
  {   0, NULL,           X_INV,                                       0,0,0,0, 0        },
};



/***********************************************************************
 * メイン処理
 ************************************************************************/
static	void	finish(void);

int	main(void)
{
    /* メモリ関連の設定 */

    /* 拡張するスタックのサイズを指定 ↓       どれだけ確保すべきなの？	*/
    SetApplLimit(GetApplLimit() - 65536*2);	/* スタックサイズを拡張	*/
    MaxApplZone();				/* ヒープ領域を拡張	*/
    MoreMasters();				/* なんのおまじない？	*/



    /* 一部の初期値を改変 (いいやり方はないかな…) */
    romaji_type = 1;			/* ローマ字変換の規則を MS-IME風に */


    if (config_init(0, NULL,		/* 環境初期化 & 引数処理 */
		    classic_options,
		    NULL)) {

	mac_init();			/* CLASSIC関連の初期化 */

	{   /* CLASSIC環境に最適(？) な設定値に書き換えておく */
	    vsync_freq_hz  = 60.0;  /* 時間経過の計測が1/60秒単位固定なので */
	    file_coding = 1;        /* SJIS固定 */
	}

	quasi88_atexit(finish);		/* quasi88() 実行中に強制終了した際の
					   コールバック関数を登録する */
	quasi88();			/* PC-8801 エミュレーション */

	mac_exit();			/* CLASSIC関連後始末 */

	config_exit();			/* 引数処理後始末 */
    }

    return 0;
}



/*
 * 強制終了時のコールバック関数 (quasi88_exit()呼出時に、処理される)
 */
static	void	finish(void)
{
    mac_exit();				/* CLASSIC関連後始末 */
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
    static const char *about_en =
    {
	"Mouse and joystick are not supported.\n"
	"\n"
	"Many many menu items are not available.\n"
    };

    static const char *about_jp =
    {
	"速度に関する設定は変更できません\n"
	"マウス、ジョイスティックは使用できません\n"
	"マウスカーソルの表示制御はサポートされていません\n"
	"ソフトウェア NumLock はサポートされていません\n"
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
