#include "screen-vram-base.h"

/*======================================================================
 * 描画先の変数定義 (dst)
 *======================================================================*/

#if	defined (LINE200)	/*-------------------------------------------*/
#define IF_LINE200_OR_EVEN_LINE()	    /* nothing */

#define DST_DEFINE()		int	dst_w = SCREEN_WIDTH;		\
				TYPE	*dst  = (TYPE *) SCREEN_START;

#define DST_NEXT_LINE()		dst  += (dst_w);

#define DST_RESTORE_LINE()	dst  -= CHARA_LINES * dst_w;

#define DST_NEXT_CHARA()	dst  += 4 * COLUMN_SKIP;

#define DST_NEXT_TOP_CHARA()	dst  += CHARA_LINES * dst_w - 320;

#elif	defined (LINE400)	/*-------------------------------------------*/
#define IF_LINE200_OR_EVEN_LINE()	if (k & 1) continue;

#define DST_DEFINE()		int	dst_w = SCREEN_WIDTH;		\
				TYPE	*dst  = (TYPE *) SCREEN_START;

#define DST_NEXT_LINE()		dst  += dst_w;

#define DST_RESTORE_LINE()	dst  -= (CHARA_LINES / 2) * dst_w;

#define DST_NEXT_CHARA()	dst  += 4 * COLUMN_SKIP;

#define DST_NEXT_TOP_CHARA()	dst  += (CHARA_LINES / 2) * dst_w - 320;
#endif

/*======================================================================
 * 描画マクロ定義 (MASK_DOT, TRANS_DOT, STORE_DOT, COPY_DOT)
 *======================================================================*/

#if	defined (LINE200)
#if	    defined (INTERPOLATE)	/*-----------------------------------*/

#define COPY_8DOT()
#define COPY_16DOT()

#include "screen-vram-200h-p.h"			/*****************/

#else	    /* ! INTERPOLATE */		/*-----------------------------------*/

#define COPY_8DOT()
#define COPY_16DOT()

#include "screen-vram-200h.h"			/*****************/

#endif

#elif	defined (LINE400)	/*-------------------------------------------*/

#define COPY_8DOT()
#define COPY_16DOT()

#include "screen-vram-400h.h"			/*****************/

#endif

/*======================================================================
 * 描画関数定義
 *======================================================================*/

#include "screen-vram.h"
