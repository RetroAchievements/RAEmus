/************************************************************************/
/*									*/
/* ステータス部の表示 (FDD表示、ほかメッセージ表示)			*/
/*									*/
/************************************************************************/

#include <stdio.h>
#include <string.h>

#include "quasi88.h"
#include "initval.h"
#include "status.h"

#include "pc88main.h"		/* boot_basic	*/
#include "memory.h"		/* font_mem	*/
#include "keyboard.h"		/* key_scan	*/
#include "screen.h"
#include "menu.h"

#include "drive.h"		/* get_drive_ready()	*/
#include "event.h"		/* get_keysym_menu()	*/



/*---------------------------------------------------------------------------*/

int	status_imagename	= FALSE;	/* イメージ名表示有無 */


#define	STATUS_LENGTH	(48)

/*
 * 表示するステータスのイメージ用バッファ
 *	バッファは8ドットフォント48文字分あるが、実際の表示はもっと小さい
 */
#define	PIXMAP_WIDTH	(STATUS_LENGTH * 8)
#define	PIXMAP_HEIGHT	(16)

static	byte	pixmap[3][ PIXMAP_WIDTH * PIXMAP_HEIGHT ];



/* 実際の表示処理は、以下のワークを経由して、
   表示するステータスのイメージ用バッファと、そのサイズを取得する */

T_STATUS_INFO	status_info[3];			/* ステータスイメージ */

/*---------------------------------------------------------------------------*/

/*
 * ローカルなワーク
 */
enum {
    STATUS_DISP_MSG,		/* 文字列表示				*/
    STATUS_DISP_MODE,		/* BASICモード表示			*/
    STATUS_DISP_FDD,		/* FDDランプ表示			*/
    STATUS_DISP_TIMEUP		/* 制限時間つき文字列表示		*/
};

static struct {
    int  dirty;			/* 真で表示の必要あり			*/

    int  disp;			/* 現在表示している内容			*/
				/*	STATUS_DISP_XXX			*/

    int  timer;			/* STATUS_DISP_TIMEUP のタイマー	*/
    int  timeup_disp;		/* タイムアップ後の表示内容		*/

    char msg[STATUS_LENGTH + 1];/* STATUS_DISP_MSG  の表示内容 +1は'\0'	*/
    int  mode;			/* STATUS_DISP_MODE の表示内容		*/
    int  fdd;			/* STATUS_DISP_FDD  の表示内容		*/

    int  default_disp;		/* デフォルトの表示内容			*/
				/*	左側 ： STATUS_DISP_MODE	*/
				/*	中央 ： STATUS_DISP_MSG		*/
				/*	右側 ： STATUS_DISP_FDD		*/
} status_wk[3];


/*
 * ローカルなフォント
 */
enum {
  FNT_START = 0xe0-1,

  FNT_2__1,  FNT_2__2,  FNT_2__3,	/* ドライブ2 */
  FNT_2D_1,  FNT_2D_2,  FNT_2D_3,

  FNT_1__1,  FNT_1__2,  FNT_1__3,	/* ドライブ1 */
  FNT_1D_1,  FNT_1D_2,  FNT_1D_3,

  FNT_T__1,  FNT_T__2,  FNT_T__3,	/* テープ */
  FNT_TR_1,  FNT_TR_2,  FNT_TR_3,
  FNT_TW_1,  FNT_TW_2,  FNT_TW_3,

  FNT_CAP_1, FNT_CAP_2,			/* CAPS     */
  FNT_KAN_1, FNT_KAN_2,			/* カナ     */
  FNT_RMJ_1, FNT_RMJ_2,			/* ローマ字 */
  FNT_NUM_1, FNT_NUM_2,			/* NUMlock  */

  FNT_END
};



static	const	byte	status_font[ 0x20 ][ 8*16 ] =
{
#define X	STATUS_BG
#define F	STATUS_FG
#define W	STATUS_WHITE
#define B	STATUS_BLACK
#define R	STATUS_RED
#define G	STATUS_GREEN


  {				/* ドライブ2(左側) 消灯：左−− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,W,W,W,
    X,X,X,X,W,W,W,W,
    X,X,X,W,W,W,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,B,B,B,
    X,X,X,W,W,W,B,B,
    X,X,X,X,W,W,W,W,
    X,X,X,X,X,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ2(左側) 消灯：−中− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ2(左側) 消灯：−−右 */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    W,W,W,W,W,W,W,W,
    B,B,B,B,B,B,W,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,B,W,
    B,B,B,B,B,B,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ2(左側) 点灯：左−− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,W,W,W,
    X,X,X,X,W,W,W,W,
    X,X,X,W,W,W,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,R,R,R,
    X,X,X,W,W,W,R,R,
    X,X,X,X,W,W,W,W,
    X,X,X,X,X,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ2(左側) 点灯：−中− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ2(左側) 点灯：−−右 */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    W,W,W,W,W,W,W,W,
    R,R,R,R,R,R,W,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,R,W,
    R,R,R,R,R,R,W,W,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ1(右側) 消灯：左−− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,B,B,B,B,B,B,B,
    W,W,B,B,B,B,B,B,
    W,W,W,W,W,W,W,W,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ1(右側) 消灯：−中− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    B,B,B,B,B,B,B,B,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ1(右側) 消灯：−−右 */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,X,X,X,X,X,
    W,W,W,W,X,X,X,X,
    B,B,W,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,B,W,W,X,X,X,
    B,B,W,W,W,X,X,X,
    W,W,W,W,X,X,X,X,
    W,W,W,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ1(右側) 点灯：左−− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    W,W,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,R,R,R,R,R,R,R,
    W,W,R,R,R,R,R,R,
    W,W,W,W,W,W,W,W,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ1(右側) 点灯：−中− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    W,W,W,W,W,W,W,W,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ドライブ1(右側) 点灯：−−右 */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    W,W,W,X,X,X,X,X,
    W,W,W,W,X,X,X,X,
    R,R,W,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,R,W,W,X,X,X,
    R,R,W,W,W,X,X,X,
    W,W,W,W,X,X,X,X,
    W,W,W,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ あり：左−− */
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,W,F,F,F,F,F,F,
    X,W,F,F,X,X,X,X,
    X,W,F,X,F,X,X,X,
    X,W,F,X,X,F,X,X,
    X,W,F,X,X,X,F,F,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,F,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,X,
    X,W,F,X,X,X,X,X,
    X,W,F,F,F,F,F,F,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ あり：−中− */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    F,X,X,X,X,X,X,F,
    F,F,X,X,X,X,F,F,
    F,X,X,X,X,X,X,F,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ あり：−−右 */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    F,F,F,F,F,F,W,X,
    X,X,X,X,F,F,W,X,
    X,X,X,F,X,F,W,X,
    X,X,F,X,X,F,W,X,
    F,F,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    F,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    X,X,X,X,X,F,W,X,
    F,F,F,F,F,F,W,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ 再生：左−− */
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,W,X,X,X,X,X,X,
    X,W,X,X,F,F,F,F,
    X,W,X,F,X,F,F,F,
    X,W,X,F,F,X,F,F,
    X,W,X,F,F,F,X,X,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,X,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,F,
    X,W,X,F,F,F,F,F,
    X,W,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ 再生：−中− */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    F,F,F,F,F,F,F,F,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,F,
    X,F,F,F,F,F,F,X,
    X,X,F,F,F,F,X,X,
    X,F,F,F,F,F,F,X,
    F,F,F,F,F,F,F,F,
    F,F,F,F,F,F,F,F,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ 再生：−−右 */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,W,X,
    F,F,F,F,X,X,W,X,
    F,F,F,X,F,X,W,X,
    F,F,X,F,F,X,W,X,
    X,X,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    X,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    F,F,F,F,F,X,W,X,
    X,X,X,X,X,X,W,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ 録音：左−− */
    X,X,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,W,X,X,X,X,X,X,
    X,W,X,X,R,R,R,R,
    X,W,X,R,X,R,R,R,
    X,W,X,R,R,X,R,R,
    X,W,X,R,R,R,X,X,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,X,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,R,
    X,W,X,R,R,R,R,R,
    X,W,X,X,X,X,X,X,
    X,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ 録音：−中− */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    X,X,X,X,X,X,X,X,
    R,R,R,R,R,R,R,R,
    X,R,R,R,R,R,R,X,
    X,X,R,R,R,R,X,X,
    X,R,R,R,R,R,R,X,
    R,R,R,R,R,R,R,R,
    R,R,R,R,R,R,R,R,
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,W,
    X,X,X,X,X,X,X,X,
  },
  {				/* テープ 録音：−−右 */
    X,X,X,X,X,X,X,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,W,X,
    R,R,R,R,X,X,W,X,
    R,R,R,X,R,X,W,X,
    R,R,X,R,R,X,W,X,
    X,X,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    X,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    R,R,R,R,R,X,W,X,
    X,X,X,X,X,X,W,X,
    W,W,W,W,W,W,W,X,
    X,X,X,X,X,X,X,X,
  },



  {				/* CAPS：左− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* CAPS：−右  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    X,F,F,X,X,X,X,F,
    F,X,X,F,X,X,X,F,
    F,X,X,F,X,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    F,F,F,F,F,X,X,F,
    X,X,X,X,X,F,X,F,
    X,X,X,X,X,F,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* カナ：左− */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* カナ：−右  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    F,X,X,X,X,X,X,F,
    F,F,F,F,F,X,X,F,
    F,X,X,X,F,X,X,F,
    F,X,X,X,F,X,X,F,
    F,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,F,X,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* ローマ字：左−  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,F,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,F,F,
    X,X,X,X,F,X,F,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* ローマ字：−右 */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,F,X,X,F,
    F,F,F,F,F,X,X,F,
    X,X,X,X,F,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },
  {				/* 数字：左−  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,F,X,X,X,
    X,X,X,X,X,F,F,F,
    X,X,X,X,X,X,X,X,
  },
  {				/* 数字：−右  */
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    X,X,X,X,X,X,X,X,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,F,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,X,F,X,X,X,X,F,
    X,F,F,F,X,X,X,F,
    X,X,X,X,X,X,X,F,
    F,F,F,F,F,F,F,X,
    X,X,X,X,X,X,X,X,
  },



#undef X
#undef F
#undef W
#undef B
#undef R
#undef G
};




/*
 * 文字列をステータスのイメージ用バッファ (status_info) に転送
 */
static	void	status_puts(int pos, const char *str)
{
    int i, j, k, c, w, h16;
    const byte *p;
    byte mask;
    byte *dst = status_info[ pos ].pixmap;
  
    if (str) {
	w = MIN(strlen(str) * 8, PIXMAP_WIDTH);

	for (i=0; i<w; i+=8) {
	    c = *(const byte *)str;
	    str ++;
	    if (c=='\0') break;

	    if (c < 0xe0) {

		if (has_kanji_rom && 			/* 漢字ROMあり */
		    ((0x20 <= c && c <= 0x7f) ||	/* ASCII    */
		     (0xa0 <= c && c <= 0xdf))) {	/* カタカナ */
		    p = &kanji_rom[0][ c*8 ][0];
		    h16 = TRUE;
		} else {
		    p = &font_mem[ c*8 ];
		    h16 = FALSE;
		}
		for (j=0; j<16; j++) {
		    for (mask=0x80, k=0;  k<8;  k++, mask>>=1) {
			if (*p & mask) dst[ j*w +i +k ] = STATUS_FG;
			else           dst[ j*w +i +k ] = STATUS_BG;
		    }
		    if (h16 || j&1) p++;
		}

	    } else {		/* 0xe0〜0xff は ローカルなフォントを使用 */

		p = &status_font[ (c-0xe0) ][0];
		for (j=0; j<16; j++) {
		    for (k=0;  k<8;  k++) {
			dst[ j*w +i +k ] = *p;
			p++;
		    }
		}

	    }
	}
	status_info[ pos ].w = i;
    } else {
	status_info[ pos ].w = 0;
    }
}



#if 0	/* ステータス部に任意の画像を表示したくなったら、考えよう */
/*
 * ピックスマップをステータスのイメージ用バッファに転送
 */
static	void	status_bitmap(int pos, const byte bitmap[], int size)
{
    if (bitmap) {
	memcpy(status_info[ pos ].pixmap, bitmap, size);
	status_info[ pos ].w = size / PIXMAP_HEIGHT;
    } else {
	status_info[ pos ].w = 0;
    }
}
#endif




/***************************************************************************
 * ステータス関係のワーク類を初期化
 ****************************************************************************/
void	status_init(void)
{
    int i, disp;
    for (i=0; i<3; i++) {

	if      (i==0) disp = STATUS_DISP_MODE;
	else if (i==1) disp = STATUS_DISP_MSG;
	else           disp = STATUS_DISP_FDD;

	status_wk[i].dirty        = TRUE;
	status_wk[i].disp         = disp;
	status_wk[i].timer        = 0;
	status_wk[i].timeup_disp  = disp;
	status_wk[i].default_disp = disp;

	status_wk[i].msg[0] = '\0';
	status_wk[i].mode   = -1;
	status_wk[i].fdd    = -1;

	status_info[i].pixmap = &pixmap[i][0];
	status_info[i].w      = 0;
	status_info[i].h      = PIXMAP_HEIGHT;
    }
}



/***************************************************************************
 * ステータス表示・非表示切替の際の、ワーク再初期化
 ****************************************************************************/
void	status_setup(int show)
{
    int i;
    if (show) {						/* 表示するなら  */
	for (i=0; i<3; i++) status_wk[i].dirty = TRUE;	/* 描画フラグON  */
    }
}




/***************************************************************************
 * ステータスにメッセージ(文字列)表示。
 * (このメッセージをデフォルトとする)
 *	msg	表示するデフォルトの文字列
 *		NULL の場合は、既定の内容を表示する。
 *		(既定の内容 … 左:BASICモード / 中:空白 / 右:FDD状態)
 ****************************************************************************/
static const char *status_get_filename(void);
void	status_message_default(int pos, const char *msg)
{
    if (msg) {			/* メッセージ指定ある場合 */

	status_puts(pos, msg);
	status_wk[ pos ].dirty = TRUE;

	status_wk[ pos ].disp        = STATUS_DISP_MSG;
	status_wk[ pos ].timeup_disp = STATUS_DISP_MSG;

	status_wk[ pos ].msg[0] = '\0';
	strncat(status_wk[ pos ].msg, msg, STATUS_LENGTH);

    } else {			/* NULL を指定された場合 */

	switch (status_wk[ pos ].default_disp) {

	case STATUS_DISP_MSG:
	    if (status_imagename == FALSE) {
		status_puts(pos, "");
		status_wk[ pos ].dirty = TRUE;

		status_wk[ pos ].msg[0] = '\0';
	    } else {
		const char *s = status_get_filename();
		status_puts(pos, s);
		status_wk[ pos ].dirty = TRUE;

		strcpy(status_wk[ pos ].msg, s);
	    }
	    break;

	case STATUS_DISP_MODE:
	    status_wk[ pos ].mode = -1;
	    break;

	case STATUS_DISP_FDD:
	    status_wk[ pos ].fdd = -1;
	    break;
	}
	    
	status_wk[ pos ].disp        = status_wk[ pos ].default_disp;
	status_wk[ pos ].timeup_disp = status_wk[ pos ].default_disp;
    }
}



/***************************************************************************
 * ステータスにメッセージ(文字列)表示。
 * (一定時間経過で、デフォルトのメッセージに戻す)
 *	frames	表示する時間。 0 なら、 msg に関係なくデフォルト表示となる
 *	msg	表示する文字列。NULLなら、空白とする
 ****************************************************************************/
void	status_message(int pos, int frames, const char *msg)
{
    if (frames) {

	status_puts(pos, msg);
	status_wk[ pos ].dirty = TRUE;

	status_wk[ pos ].disp        = STATUS_DISP_TIMEUP;
	status_wk[ pos ].timer       = frames;

    } else {

	status_message_default(pos, NULL);

    }
}



/***************************************************************************
 * ステータス表示用のイメージを更新
 *	表示イメージのピックスマップを status_info に転送する。
 *	引数 force が 偽の場合は、前回から変化のあったステータスのみ更新
 *	     force が 真の場合は、前回の状態とは関わらずステータスを更新
 *	戻り値は、実際に更新したステータス番号が、ビット 0〜2 に
 *		  セットされる。(ビットが 1 ならそのステータスは更新)
 ****************************************************************************/

static void status_mode(int pos)
{
    int mode = 0;	/* bit :  ....  num kana caps 8mhz basic basic */
    byte buf[16];
    static const char *mode_str[] =
    {
	"N   4MHz       ",
	"V1S 4MHz       ",
	"V1H 4MHz       ",
	"V2  4MHz       ",
	"N   8MHz       ",
	"V1S 8MHz       ",
	"V1H 8MHz       ",
	"V2  8MHz       ",
    };

    switch (boot_basic) {
    case BASIC_N:	mode += 0;	break;
    case BASIC_V1S:	mode += 1;	break;
    case BASIC_V1H:	mode += 2;	break;
    case BASIC_V2:	mode += 3;	break;
    }
    if (boot_clock_4mhz == FALSE) mode += 4;

    if ((key_scan[0x0a] & 0x80) == 0) mode += 8;
    if ((key_scan[0x08] & 0x20) == 0) mode += 16;
    if (numlock_emu) mode += 32;

    if (status_wk[pos].mode != mode) {		/* ワーク変更したら表示更新 */
	status_wk[pos].mode = mode;

	strcpy((char *)buf, mode_str[ mode & 0x7 ]);
	if (mode & 8) {
	    buf[ 9] = FNT_CAP_1;
	    buf[10] = FNT_CAP_2;
	}
	if (mode & 16) {
	    if (romaji_input_mode) {
		buf[11] = FNT_RMJ_1;
		buf[12] = FNT_RMJ_2;
	    } else {
		buf[11] = FNT_KAN_1;
		buf[12] = FNT_KAN_2;
	    }
	}
	if (mode & 32) {
	    buf[13] = FNT_NUM_1;
	    buf[14] = FNT_NUM_2;
	}

	status_puts(pos, (const char *)buf);
	status_wk[ pos ].dirty = TRUE;
    }
}

static void status_fdd(int pos)
{
    byte *p, buf[16];
    int fdd = 0;	/* bit :  ....  tape tape drv2 drv1 */

    if (! get_drive_ready(0)) { fdd |= 1 << 0; }	/* FDD 1: ランプON */
    if (! get_drive_ready(1)) { fdd |= 1 << 1; }	/* FDD 2: ランプON */
    /* drive_check_empty(n) でディスクの有無もわかるけど… */

    if      (tape_writing()) fdd |= (3 << 2);
    else if (tape_reading()) fdd |= (2 << 2);
    else if (tape_exist())   fdd |= (1 << 2);

    if (status_wk[pos].fdd != fdd) {		/* ワーク変更したら表示更新 */
	status_wk[pos].fdd = fdd;

	p = buf;

	switch ((fdd >> 2) & 3) {
	case 1:
	    *p ++ = FNT_T__1;
	    *p ++ = FNT_T__2;
	    *p ++ = FNT_T__3;
	    *p ++ = ' ';
	    break;
	case 2:
	    *p ++ = FNT_TR_1;
	    *p ++ = FNT_TR_2;
	    *p ++ = FNT_TR_3;
	    *p ++ = ' ';
	    break;
	case 3:
	    *p ++ = FNT_TW_1;
	    *p ++ = FNT_TW_2;
	    *p ++ = FNT_TW_3;
	    *p ++ = ' ';
	    break;
	}

	if (fdd & (1<<1)) {
	    *p ++ = FNT_2D_1;
	    *p ++ = FNT_2D_2;
	    *p ++ = FNT_2D_3;
	} else {
	    *p ++ = FNT_2__1;
	    *p ++ = FNT_2__2;
	    *p ++ = FNT_2__3;
	}

	if (fdd & (1<<0)) {
	    *p ++ = FNT_1D_1;
	    *p ++ = FNT_1D_2;
	    *p ++ = FNT_1D_3;
	} else {
	    *p ++ = FNT_1__1;
	    *p ++ = FNT_1__2;
	    *p ++ = FNT_1__3;
	}

	*p = '\0';

	status_puts(pos, (const char *)buf);
	status_wk[ pos ].dirty = TRUE;
    }
}

static const char *status_get_filename(void)
{
    static char buf[STATUS_LENGTH + 1];
    int i, drv;
    char str[2][25];

    for (i=0; i<2; i++) {
	if (menu_swapdrv == FALSE) {
	    if (i == 0) drv = DRIVE_1;
	    else        drv = DRIVE_2;
	} else {
	    if (i == 0) drv = DRIVE_2;
	    else        drv = DRIVE_1;
	}

	if (disk_image_exist(drv) &&
	    drive_check_empty(drv) == FALSE) {

	    sprintf(str[i], "%1d: %-16s",
		    drv + 1,
		    drive[drv].image[ disk_image_selected(drv) ].name);
	} else {
	    sprintf(str[i], "%1d:    (No Disk)    ", drv + 1);
	}
    }

    strcpy(buf, str[0]);
    strcat(buf, str[1]);

    return buf;
}



/*
 *	VSYNC毎に呼び出す
 */
void	status_update(void)
{
    int i;

    for (i = 0; i < 3; i++) {

	if (status_wk[i].disp == STATUS_DISP_TIMEUP) {	/* タイマー指定時は */
	    if (--status_wk[i].timer < 0) {		/* タイムアウトで   */
		switch (status_wk[i].timeup_disp) {	/* 表示内容を変更   */

		case STATUS_DISP_MSG:
		    status_puts(i, status_wk[i].msg);
		    status_wk[ i ].dirty = TRUE;
		    break;

		case STATUS_DISP_MODE:
		    status_wk[ i ].mode = -1;
		    break;

		case STATUS_DISP_FDD:
		    status_wk[ i ].fdd = -1;
		    break;
		}

		status_wk[i].disp = status_wk[i].timeup_disp;
	    }
	}

	switch (status_wk[i].disp) {
	case STATUS_DISP_MODE:	status_mode(i);		break;
	case STATUS_DISP_FDD:	status_fdd(i);		break;
	}

	if (status_wk[i].dirty) {		/* 結果、表示内容に変化あり */
	    status_wk[i].dirty = FALSE;
	    screen_dirty_status |= 1 << i;
	}

    }
}
