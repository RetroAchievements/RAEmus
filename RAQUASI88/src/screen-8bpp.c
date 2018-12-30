/************************************************************************/
/*									*/
/* 画面の表示		8bpp						*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "screen.h"
#include "screen-func.h"
#include "crtcdmac.h"
#include "memory.h"
#include "q8tk.h"


#ifdef	SUPPORT_8BPP

#define TYPE		bit8

#define SCREEN_WIDTH		WIDTH
#define SCREEN_HEIGHT		HEIGHT
#define SCREEN_SX		SCREEN_W
#define SCREEN_SY		SCREEN_H
#define SCREEN_TOP		screen_buf
#define SCREEN_START		screen_start

#define COLOR_PIXEL(x)		(TYPE) color_pixel[ x ]
#define MIXED_PIXEL(a,b)	(TYPE) color_half_pixel[ a ][ b ]
#define BLACK			(TYPE) black_pixel

/*===========================================================================
 * 等倍サイズ
 *===========================================================================*/

/*----------------------------------------------------------------------
 *			● 200ライン			標準
 *----------------------------------------------------------------------*/
#define NORMAL

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_F_N__8
#include					"screen-vram-full.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_F_N__8
#include					"screen-vram-full.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_F_N__8
#include					"screen-vram-full.h"
#undef	UNDISP

#undef	NORMAL
/*----------------------------------------------------------------------
 *			● 200ライン			ラインスキップ
 *----------------------------------------------------------------------*/
#define SKIPLINE

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_F_S__8
#include					"screen-vram-full.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_F_S__8
#include					"screen-vram-full.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_F_S__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_F_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_F_S__8
#include					"screen-vram-full.h"
#undef	UNDISP

#undef	SKIPLINE
/*----------------------------------------------------------------------
 *			● 200ライン			インターレース
 *----------------------------------------------------------------------*/
#define INTERLACE

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_F_I__8
#include					"screen-vram-full.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_F_I__8
#include					"screen-vram-full.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_F_I__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_F_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_F_I__8
#include					"screen-vram-full.h"
#undef	UNDISP

#undef	INTERLACE
/*----------------------------------------------------------------------
 *			● 400ライン			標準
 *----------------------------------------------------------------------*/
#define HIRESO						/* 白黒	 640x400 */

#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H80x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H80x20_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x25_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H40x25_F_N__8
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x20_F_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H40x20_F_N__8
#include					"screen-vram-full.h"

#undef	HIRESO

/*===========================================================================
 * 半分サイズ
 *===========================================================================*/

/*----------------------------------------------------------------------
 *			● 200ライン			標準
 *----------------------------------------------------------------------*/
#define NORMAL

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_H_N__8
#include					"screen-vram-half.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_H_N__8
#include					"screen-vram-half.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */

#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_H_N__8
#include					"screen-vram-half.h"
#undef	UNDISP

#undef	NORMAL
/*----------------------------------------------------------------------
 *			● 200ライン			色補完
 *----------------------------------------------------------------------*/
#define INTERPOLATE

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_H_P__8
#include					"screen-vram-half.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_H_P__8
#include					"screen-vram-half.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_H_P__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_H_P__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_H_P__8
#include					"screen-vram-half.h"
#undef	UNDISP

#undef	INTERPOLATE
/*----------------------------------------------------------------------
 *			● 400ライン			標準
 *----------------------------------------------------------------------*/
#define HIRESO						/* 白黒	 640x400 */

#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H80x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H80x20_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x25_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H40x25_H_N__8
#include					"screen-vram-half.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x20_H_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H40x20_H_N__8
#include					"screen-vram-half.h"

#undef	HIRESO

/*===========================================================================
 * 二倍サイズ
 *===========================================================================*/
#ifdef	SUPPORT_DOUBLE
/*----------------------------------------------------------------------
 *			● 200ライン			標準
 *----------------------------------------------------------------------*/
#define NORMAL

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_D_N__8
#include					"screen-vram-double.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_D_N__8
#include					"screen-vram-double.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_D_N__8
#include					"screen-vram-double.h"
#undef	UNDISP

#undef	NORMAL
/*----------------------------------------------------------------------
 *			● 200ライン			ラインスキップ
 *----------------------------------------------------------------------*/
#define SKIPLINE

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_D_S__8
#include					"screen-vram-double.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_D_S__8
#include					"screen-vram-double.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_D_S__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_D_S__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_D_S__8
#include					"screen-vram-double.h"
#undef	UNDISP

#undef	SKIPLINE
/*----------------------------------------------------------------------
 *			● 200ライン			インターレース
 *----------------------------------------------------------------------*/
#define INTERLACE

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_D_I__8
#include					"screen-vram-double.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_D_I__8
#include					"screen-vram-double.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_D_I__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_D_I__8
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_D_I__8
#include					"screen-vram-double.h"
#undef	UNDISP

#undef	INTERLACE
/*----------------------------------------------------------------------
 *			● 400ライン			標準
 *----------------------------------------------------------------------*/
#define HIRESO						/* 白黒	 640x400 */

#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H80x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H80x20_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x25_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H40x25_D_N__8
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x20_D_N__8
#define		VRAM2SCREEN_ALL			v2s_all_H40x20_D_N__8
#include					"screen-vram-double.h"
#undef	HIRESO

#endif	/* SUPPORT_DOUBLE */


#define	DIRECT
/*===========================================================================
 * 等倍サイズ
 *===========================================================================*/

/*----------------------------------------------------------------------
 *			● 200ライン			標準
 *----------------------------------------------------------------------*/
#define NORMAL

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_F_N__8_d
#include					"screen-vram-full.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_F_N__8_d
#include					"screen-vram-full.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_F_N__8_d
#include					"screen-vram-full.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_F_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_F_N__8_d
#include					"screen-vram-full.h"
#undef	UNDISP

#undef	NORMAL

/*===========================================================================
 * 二倍サイズ
 *===========================================================================*/
#ifdef	SUPPORT_DOUBLE
/*----------------------------------------------------------------------
 *			● 200ライン			標準
 *----------------------------------------------------------------------*/
#define NORMAL

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_D_N__8_d
#include					"screen-vram-double.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_D_N__8_d
#include					"screen-vram-double.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_D_N__8_d
#include					"screen-vram-double.h"
#undef	UNDISP

#undef	NORMAL
/*----------------------------------------------------------------------
 *			● 200ライン			ラインスキップ
 *----------------------------------------------------------------------*/
#define SKIPLINE

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_D_S__8_d
#include					"screen-vram-double.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_D_S__8_d
#include					"screen-vram-double.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_D_S__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_D_S__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_D_S__8_d
#include					"screen-vram-double.h"
#undef	UNDISP

#undef	SKIPLINE
/*----------------------------------------------------------------------
 *			● 200ライン			インターレース
 *----------------------------------------------------------------------*/
#define INTERLACE

#define COLOR						/* カラー640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x25_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x25_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C80x20_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C80x20_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x25_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x25_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_C40x20_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_C40x20_D_I__8_d
#include					"screen-vram-double.h"
#undef	COLOR

#define MONO						/* 白黒	 640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x25_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x25_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M80x20_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M80x20_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x25_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x25_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_M40x20_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_M40x20_D_I__8_d
#include					"screen-vram-double.h"
#undef	MONO

#define UNDISP						/* 非表示640x200 */
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x25_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x25_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U80x20_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U80x20_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x25_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x25_D_I__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_U40x20_D_I__8_d
#define		VRAM2SCREEN_ALL			v2s_all_U40x20_D_I__8_d
#include					"screen-vram-double.h"
#undef	UNDISP

#undef	INTERLACE
/*----------------------------------------------------------------------
 *			● 400ライン			標準
 *----------------------------------------------------------------------*/
#define HIRESO						/* 白黒	 640x400 */

#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_H80x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	80
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H80x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_H80x20_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		25
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x25_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_H40x25_D_N__8_d
#include					"screen-vram-double.h"
#define		TEXT_WIDTH	40
#define		TEXT_HEIGHT		20
#define		VRAM2SCREEN_DIFF		v2s_dif_H40x20_D_N__8_d
#define		VRAM2SCREEN_ALL			v2s_all_H40x20_D_N__8_d
#include					"screen-vram-double.h"
#undef	HIRESO

#endif	/* SUPPORT_DOUBLE */
#undef	DIRECT


/*===========================================================================
 * 画面消去
 *===========================================================================*/

#define		SCREEN_BUF_INIT			screen_buf_init__8
#include					"screen-vram-clear.h"


/*===========================================================================
 * メニュー画面
 *===========================================================================*/

#define		MENU2SCREEN			menu2screen_F_N__8
#include					"screen-menu-full.h"

#define		MENU2SCREEN			menu2screen_H_N__8
#include					"screen-menu-half.h"

#define		MENU2SCREEN			menu2screen_H_P__8
#include					"screen-menu-half-p.h"

#ifdef	SUPPORT_DOUBLE
#define		MENU2SCREEN			menu2screen_D_N__8
#include					"screen-menu-double.h"
#endif


/*===========================================================================
 * ステータス
 *===========================================================================*/

#define		STATUS2SCREEN			status2screen__8
#define		STATUS_BUF_INIT			status_buf_init__8
#define		STATUS_BUF_CLEAR		status_buf_clear__8
#include					"screen-status.h"


#undef	TYPE		/* bit8 */












/* ========================================================================= */
/* 等倍サイズ - 標準 */

int (*vram2screen_list_F_N__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_F_N__8, v2s_all_C80x25_F_N__8 },
	{ v2s_dif_C80x20_F_N__8, v2s_all_C80x20_F_N__8 },
	{ v2s_dif_C40x25_F_N__8, v2s_all_C40x25_F_N__8 },
	{ v2s_dif_C40x20_F_N__8, v2s_all_C40x20_F_N__8 },
    },
    {
	{ v2s_dif_M80x25_F_N__8, v2s_all_M80x25_F_N__8 },
	{ v2s_dif_M80x20_F_N__8, v2s_all_M80x20_F_N__8 },
	{ v2s_dif_M40x25_F_N__8, v2s_all_M40x25_F_N__8 },
	{ v2s_dif_M40x20_F_N__8, v2s_all_M40x20_F_N__8 },
    },
    {
	{ v2s_dif_U80x25_F_N__8, v2s_all_U80x25_F_N__8 },
	{ v2s_dif_U80x20_F_N__8, v2s_all_U80x20_F_N__8 },
	{ v2s_dif_U40x25_F_N__8, v2s_all_U40x25_F_N__8 },
	{ v2s_dif_U40x20_F_N__8, v2s_all_U40x20_F_N__8 },
    },
    {
	{ v2s_dif_H80x25_F_N__8, v2s_all_H80x25_F_N__8 },
	{ v2s_dif_H80x20_F_N__8, v2s_all_H80x20_F_N__8 },
	{ v2s_dif_H40x25_F_N__8, v2s_all_H40x25_F_N__8 },
	{ v2s_dif_H40x20_F_N__8, v2s_all_H40x20_F_N__8 },
    },
};

/* 等倍サイズ - スキップライン */

int (*vram2screen_list_F_S__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_F_S__8, v2s_all_C80x25_F_S__8 },
	{ v2s_dif_C80x20_F_S__8, v2s_all_C80x20_F_S__8 },
	{ v2s_dif_C40x25_F_S__8, v2s_all_C40x25_F_S__8 },
	{ v2s_dif_C40x20_F_S__8, v2s_all_C40x20_F_S__8 },
    },
    {
	{ v2s_dif_M80x25_F_S__8, v2s_all_M80x25_F_S__8 },
	{ v2s_dif_M80x20_F_S__8, v2s_all_M80x20_F_S__8 },
	{ v2s_dif_M40x25_F_S__8, v2s_all_M40x25_F_S__8 },
	{ v2s_dif_M40x20_F_S__8, v2s_all_M40x20_F_S__8 },
    },
    {
	{ v2s_dif_U80x25_F_S__8, v2s_all_U80x25_F_S__8 },
	{ v2s_dif_U80x20_F_S__8, v2s_all_U80x20_F_S__8 },
	{ v2s_dif_U40x25_F_S__8, v2s_all_U40x25_F_S__8 },
	{ v2s_dif_U40x20_F_S__8, v2s_all_U40x20_F_S__8 },
    },
    {
	{ v2s_dif_H80x25_F_N__8, v2s_all_H80x25_F_N__8 },
	{ v2s_dif_H80x20_F_N__8, v2s_all_H80x20_F_N__8 },
	{ v2s_dif_H40x25_F_N__8, v2s_all_H40x25_F_N__8 },
	{ v2s_dif_H40x20_F_N__8, v2s_all_H40x20_F_N__8 },
    },
};

/* 等倍サイズ - インターレース */

int (*vram2screen_list_F_I__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_F_I__8, v2s_all_C80x25_F_I__8 },
	{ v2s_dif_C80x20_F_I__8, v2s_all_C80x20_F_I__8 },
	{ v2s_dif_C40x25_F_I__8, v2s_all_C40x25_F_I__8 },
	{ v2s_dif_C40x20_F_I__8, v2s_all_C40x20_F_I__8 },
    },
    {
	{ v2s_dif_M80x25_F_I__8, v2s_all_M80x25_F_I__8 },
	{ v2s_dif_M80x20_F_I__8, v2s_all_M80x20_F_I__8 },
	{ v2s_dif_M40x25_F_I__8, v2s_all_M40x25_F_I__8 },
	{ v2s_dif_M40x20_F_I__8, v2s_all_M40x20_F_I__8 },
    },
    {
	{ v2s_dif_U80x25_F_I__8, v2s_all_U80x25_F_I__8 },
	{ v2s_dif_U80x20_F_I__8, v2s_all_U80x20_F_I__8 },
	{ v2s_dif_U40x25_F_I__8, v2s_all_U40x25_F_I__8 },
	{ v2s_dif_U40x20_F_I__8, v2s_all_U40x20_F_I__8 },
    },
    {
	{ v2s_dif_H80x25_F_N__8, v2s_all_H80x25_F_N__8 },
	{ v2s_dif_H80x20_F_N__8, v2s_all_H80x20_F_N__8 },
	{ v2s_dif_H40x25_F_N__8, v2s_all_H40x25_F_N__8 },
	{ v2s_dif_H40x20_F_N__8, v2s_all_H40x20_F_N__8 },
    },
};

/* ========================================================================= */
/* 半分サイズ - 標準 */

int (*vram2screen_list_H_N__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_H_N__8, v2s_all_C80x25_H_N__8 },
	{ v2s_dif_C80x20_H_N__8, v2s_all_C80x20_H_N__8 },
	{ v2s_dif_C40x25_H_N__8, v2s_all_C40x25_H_N__8 },
	{ v2s_dif_C40x20_H_N__8, v2s_all_C40x20_H_N__8 },
    },
    {
	{ v2s_dif_M80x25_H_N__8, v2s_all_M80x25_H_N__8 },
	{ v2s_dif_M80x20_H_N__8, v2s_all_M80x20_H_N__8 },
	{ v2s_dif_M40x25_H_N__8, v2s_all_M40x25_H_N__8 },
	{ v2s_dif_M40x20_H_N__8, v2s_all_M40x20_H_N__8 },
    },
    {
	{ v2s_dif_U80x25_H_N__8, v2s_all_U80x25_H_N__8 },
	{ v2s_dif_U80x20_H_N__8, v2s_all_U80x20_H_N__8 },
	{ v2s_dif_U40x25_H_N__8, v2s_all_U40x25_H_N__8 },
	{ v2s_dif_U40x20_H_N__8, v2s_all_U40x20_H_N__8 },
    },
    {
	{ v2s_dif_H80x25_H_N__8, v2s_all_H80x25_H_N__8 },
	{ v2s_dif_H80x20_H_N__8, v2s_all_H80x20_H_N__8 },
	{ v2s_dif_H40x25_H_N__8, v2s_all_H40x25_H_N__8 },
	{ v2s_dif_H40x20_H_N__8, v2s_all_H40x20_H_N__8 },
    },
};

/* 半分サイズ - 色補完 */

int (*vram2screen_list_H_P__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_H_P__8, v2s_all_C80x25_H_P__8 },
	{ v2s_dif_C80x20_H_P__8, v2s_all_C80x20_H_P__8 },
	{ v2s_dif_C40x25_H_P__8, v2s_all_C40x25_H_P__8 },
	{ v2s_dif_C40x20_H_P__8, v2s_all_C40x20_H_P__8 },
    },
    {
	{ v2s_dif_M80x25_H_P__8, v2s_all_M80x25_H_P__8 },
	{ v2s_dif_M80x20_H_P__8, v2s_all_M80x20_H_P__8 },
	{ v2s_dif_M40x25_H_P__8, v2s_all_M40x25_H_P__8 },
	{ v2s_dif_M40x20_H_P__8, v2s_all_M40x20_H_P__8 },
    },
    {
	{ v2s_dif_U80x25_H_P__8, v2s_all_U80x25_H_P__8 },
	{ v2s_dif_U80x20_H_P__8, v2s_all_U80x20_H_P__8 },
	{ v2s_dif_U40x25_H_P__8, v2s_all_U40x25_H_P__8 },
	{ v2s_dif_U40x20_H_P__8, v2s_all_U40x20_H_P__8 },
    },
    {
	{ v2s_dif_H80x25_H_N__8, v2s_all_H80x25_H_N__8 },
	{ v2s_dif_H80x20_H_N__8, v2s_all_H80x20_H_N__8 },
	{ v2s_dif_H40x25_H_N__8, v2s_all_H40x25_H_N__8 },
	{ v2s_dif_H40x20_H_N__8, v2s_all_H40x20_H_N__8 },
    },
};

/* ========================================================================= */
#ifdef	SUPPORT_DOUBLE
/* 二倍サイズ - 標準 */

int (*vram2screen_list_D_N__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_D_N__8, v2s_all_C80x25_D_N__8 },
	{ v2s_dif_C80x20_D_N__8, v2s_all_C80x20_D_N__8 },
	{ v2s_dif_C40x25_D_N__8, v2s_all_C40x25_D_N__8 },
	{ v2s_dif_C40x20_D_N__8, v2s_all_C40x20_D_N__8 },
    },
    {
	{ v2s_dif_M80x25_D_N__8, v2s_all_M80x25_D_N__8 },
	{ v2s_dif_M80x20_D_N__8, v2s_all_M80x20_D_N__8 },
	{ v2s_dif_M40x25_D_N__8, v2s_all_M40x25_D_N__8 },
	{ v2s_dif_M40x20_D_N__8, v2s_all_M40x20_D_N__8 },
    },
    {
	{ v2s_dif_U80x25_D_N__8, v2s_all_U80x25_D_N__8 },
	{ v2s_dif_U80x20_D_N__8, v2s_all_U80x20_D_N__8 },
	{ v2s_dif_U40x25_D_N__8, v2s_all_U40x25_D_N__8 },
	{ v2s_dif_U40x20_D_N__8, v2s_all_U40x20_D_N__8 },
    },
    {
	{ v2s_dif_H80x25_D_N__8, v2s_all_H80x25_D_N__8 },
	{ v2s_dif_H80x20_D_N__8, v2s_all_H80x20_D_N__8 },
	{ v2s_dif_H40x25_D_N__8, v2s_all_H40x25_D_N__8 },
	{ v2s_dif_H40x20_D_N__8, v2s_all_H40x20_D_N__8 },
    },
};

/* 二倍サイズ - スキップライン */

int (*vram2screen_list_D_S__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_D_S__8, v2s_all_C80x25_D_S__8 },
	{ v2s_dif_C80x20_D_S__8, v2s_all_C80x20_D_S__8 },
	{ v2s_dif_C40x25_D_S__8, v2s_all_C40x25_D_S__8 },
	{ v2s_dif_C40x20_D_S__8, v2s_all_C40x20_D_S__8 },
    },
    {
	{ v2s_dif_M80x25_D_S__8, v2s_all_M80x25_D_S__8 },
	{ v2s_dif_M80x20_D_S__8, v2s_all_M80x20_D_S__8 },
	{ v2s_dif_M40x25_D_S__8, v2s_all_M40x25_D_S__8 },
	{ v2s_dif_M40x20_D_S__8, v2s_all_M40x20_D_S__8 },
    },
    {
	{ v2s_dif_U80x25_D_S__8, v2s_all_U80x25_D_S__8 },
	{ v2s_dif_U80x20_D_S__8, v2s_all_U80x20_D_S__8 },
	{ v2s_dif_U40x25_D_S__8, v2s_all_U40x25_D_S__8 },
	{ v2s_dif_U40x20_D_S__8, v2s_all_U40x20_D_S__8 },
    },
    {
	{ v2s_dif_H80x25_D_N__8, v2s_all_H80x25_D_N__8 },
	{ v2s_dif_H80x20_D_N__8, v2s_all_H80x20_D_N__8 },
	{ v2s_dif_H40x25_D_N__8, v2s_all_H40x25_D_N__8 },
	{ v2s_dif_H40x20_D_N__8, v2s_all_H40x20_D_N__8 },
    },
};

/* 二倍サイズ - インターレース */

int (*vram2screen_list_D_I__8[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_D_I__8, v2s_all_C80x25_D_I__8 },
	{ v2s_dif_C80x20_D_I__8, v2s_all_C80x20_D_I__8 },
	{ v2s_dif_C40x25_D_I__8, v2s_all_C40x25_D_I__8 },
	{ v2s_dif_C40x20_D_I__8, v2s_all_C40x20_D_I__8 },
    },
    {
	{ v2s_dif_M80x25_D_I__8, v2s_all_M80x25_D_I__8 },
	{ v2s_dif_M80x20_D_I__8, v2s_all_M80x20_D_I__8 },
	{ v2s_dif_M40x25_D_I__8, v2s_all_M40x25_D_I__8 },
	{ v2s_dif_M40x20_D_I__8, v2s_all_M40x20_D_I__8 },
    },
    {
	{ v2s_dif_U80x25_D_I__8, v2s_all_U80x25_D_I__8 },
	{ v2s_dif_U80x20_D_I__8, v2s_all_U80x20_D_I__8 },
	{ v2s_dif_U40x25_D_I__8, v2s_all_U40x25_D_I__8 },
	{ v2s_dif_U40x20_D_I__8, v2s_all_U40x20_D_I__8 },
    },
    {
	{ v2s_dif_H80x25_D_N__8, v2s_all_H80x25_D_N__8 },
	{ v2s_dif_H80x20_D_N__8, v2s_all_H80x20_D_N__8 },
	{ v2s_dif_H40x25_D_N__8, v2s_all_H40x25_D_N__8 },
	{ v2s_dif_H40x20_D_N__8, v2s_all_H40x20_D_N__8 },
    },
};
#endif	/* SUPPORT_DOUBLE */

/* ------------------------------------------------------------------------- */
/* 等倍サイズ - 標準 */

int (*vram2screen_list_F_N__8_d[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_F_N__8_d, v2s_all_C80x25_F_N__8_d },
	{ v2s_dif_C80x20_F_N__8_d, v2s_all_C80x20_F_N__8_d },
	{ v2s_dif_C40x25_F_N__8_d, v2s_all_C40x25_F_N__8_d },
	{ v2s_dif_C40x20_F_N__8_d, v2s_all_C40x20_F_N__8_d },
    },
    {
	{ v2s_dif_M80x25_F_N__8_d, v2s_all_M80x25_F_N__8_d },
	{ v2s_dif_M80x20_F_N__8_d, v2s_all_M80x20_F_N__8_d },
	{ v2s_dif_M40x25_F_N__8_d, v2s_all_M40x25_F_N__8_d },
	{ v2s_dif_M40x20_F_N__8_d, v2s_all_M40x20_F_N__8_d },
    },
    {
	{ v2s_dif_U80x25_F_N__8_d, v2s_all_U80x25_F_N__8_d },
	{ v2s_dif_U80x20_F_N__8_d, v2s_all_U80x20_F_N__8_d },
	{ v2s_dif_U40x25_F_N__8_d, v2s_all_U40x25_F_N__8_d },
	{ v2s_dif_U40x20_F_N__8_d, v2s_all_U40x20_F_N__8_d },
    },
    {
	{ v2s_dif_H80x25_F_N__8, v2s_all_H80x25_F_N__8 },
	{ v2s_dif_H80x20_F_N__8, v2s_all_H80x20_F_N__8 },
	{ v2s_dif_H40x25_F_N__8, v2s_all_H40x25_F_N__8 },
	{ v2s_dif_H40x20_F_N__8, v2s_all_H40x20_F_N__8 },
    },
};

#ifdef	SUPPORT_DOUBLE
/* 二倍サイズ - 標準 */

int (*vram2screen_list_D_N__8_d[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_D_N__8_d, v2s_all_C80x25_D_N__8_d },
	{ v2s_dif_C80x20_D_N__8_d, v2s_all_C80x20_D_N__8_d },
	{ v2s_dif_C40x25_D_N__8_d, v2s_all_C40x25_D_N__8_d },
	{ v2s_dif_C40x20_D_N__8_d, v2s_all_C40x20_D_N__8_d },
    },
    {
	{ v2s_dif_M80x25_D_N__8_d, v2s_all_M80x25_D_N__8_d },
	{ v2s_dif_M80x20_D_N__8_d, v2s_all_M80x20_D_N__8_d },
	{ v2s_dif_M40x25_D_N__8_d, v2s_all_M40x25_D_N__8_d },
	{ v2s_dif_M40x20_D_N__8_d, v2s_all_M40x20_D_N__8_d },
    },
    {
	{ v2s_dif_U80x25_D_N__8_d, v2s_all_U80x25_D_N__8_d },
	{ v2s_dif_U80x20_D_N__8_d, v2s_all_U80x20_D_N__8_d },
	{ v2s_dif_U40x25_D_N__8_d, v2s_all_U40x25_D_N__8_d },
	{ v2s_dif_U40x20_D_N__8_d, v2s_all_U40x20_D_N__8_d },
    },
    {
	{ v2s_dif_H80x25_D_N__8_d, v2s_all_H80x25_D_N__8_d },
	{ v2s_dif_H80x20_D_N__8_d, v2s_all_H80x20_D_N__8_d },
	{ v2s_dif_H40x25_D_N__8_d, v2s_all_H40x25_D_N__8_d },
	{ v2s_dif_H40x20_D_N__8_d, v2s_all_H40x20_D_N__8_d },
    },
};

/* 二倍サイズ - スキップライン */

int (*vram2screen_list_D_S__8_d[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_D_S__8_d, v2s_all_C80x25_D_S__8_d },
	{ v2s_dif_C80x20_D_S__8_d, v2s_all_C80x20_D_S__8_d },
	{ v2s_dif_C40x25_D_S__8_d, v2s_all_C40x25_D_S__8_d },
	{ v2s_dif_C40x20_D_S__8_d, v2s_all_C40x20_D_S__8_d },
    },
    {
	{ v2s_dif_M80x25_D_S__8_d, v2s_all_M80x25_D_S__8_d },
	{ v2s_dif_M80x20_D_S__8_d, v2s_all_M80x20_D_S__8_d },
	{ v2s_dif_M40x25_D_S__8_d, v2s_all_M40x25_D_S__8_d },
	{ v2s_dif_M40x20_D_S__8_d, v2s_all_M40x20_D_S__8_d },
    },
    {
	{ v2s_dif_U80x25_D_S__8_d, v2s_all_U80x25_D_S__8_d },
	{ v2s_dif_U80x20_D_S__8_d, v2s_all_U80x20_D_S__8_d },
	{ v2s_dif_U40x25_D_S__8_d, v2s_all_U40x25_D_S__8_d },
	{ v2s_dif_U40x20_D_S__8_d, v2s_all_U40x20_D_S__8_d },
    },
    {
	{ v2s_dif_H80x25_D_N__8_d, v2s_all_H80x25_D_N__8_d },
	{ v2s_dif_H80x20_D_N__8_d, v2s_all_H80x20_D_N__8_d },
	{ v2s_dif_H40x25_D_N__8_d, v2s_all_H40x25_D_N__8_d },
	{ v2s_dif_H40x20_D_N__8_d, v2s_all_H40x20_D_N__8_d },
    },
};

/* 二倍サイズ - インターレース */

int (*vram2screen_list_D_I__8_d[4][4][2])(void) =
{
    {
	{ v2s_dif_C80x25_D_I__8_d, v2s_all_C80x25_D_I__8_d },
	{ v2s_dif_C80x20_D_I__8_d, v2s_all_C80x20_D_I__8_d },
	{ v2s_dif_C40x25_D_I__8_d, v2s_all_C40x25_D_I__8_d },
	{ v2s_dif_C40x20_D_I__8_d, v2s_all_C40x20_D_I__8_d },
    },
    {
	{ v2s_dif_M80x25_D_I__8_d, v2s_all_M80x25_D_I__8_d },
	{ v2s_dif_M80x20_D_I__8_d, v2s_all_M80x20_D_I__8_d },
	{ v2s_dif_M40x25_D_I__8_d, v2s_all_M40x25_D_I__8_d },
	{ v2s_dif_M40x20_D_I__8_d, v2s_all_M40x20_D_I__8_d },
    },
    {
	{ v2s_dif_U80x25_D_I__8_d, v2s_all_U80x25_D_I__8_d },
	{ v2s_dif_U80x20_D_I__8_d, v2s_all_U80x20_D_I__8_d },
	{ v2s_dif_U40x25_D_I__8_d, v2s_all_U40x25_D_I__8_d },
	{ v2s_dif_U40x20_D_I__8_d, v2s_all_U40x20_D_I__8_d },
    },
    {
	{ v2s_dif_H80x25_D_N__8_d, v2s_all_H80x25_D_N__8_d },
	{ v2s_dif_H80x20_D_N__8_d, v2s_all_H80x20_D_N__8_d },
	{ v2s_dif_H40x25_D_N__8_d, v2s_all_H40x25_D_N__8_d },
	{ v2s_dif_H40x20_D_N__8_d, v2s_all_H40x20_D_N__8_d },
    },
};
#endif	/* SUPPORT_DOUBLE */

#endif	/* SUPPORT_8BPP */
