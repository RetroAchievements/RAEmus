#include "screen-vram-base.h"

/*======================================================================
 * 描画先の変数定義 (dst)
 *======================================================================*/

#if	defined (DIRECT)
#define DST_DEFINE_ADD		TYPE	dstbuf[16];
#else
#define DST_DEFINE_ADD
#endif

#if	defined (LINE200)	/*-------------------------------------------*/
#define IF_LINE200_OR_EVEN_LINE()	    /* nothing */

#define DST_DEFINE()		int	dst_w = SCREEN_WIDTH;		\
				DST_DEFINE_ADD				\
				TYPE	*dst  = (TYPE *) SCREEN_START;	\
				TYPE	*dst2 = dst + dst_w;

#define DST_NEXT_LINE()		dst  += (2 * dst_w);			\
				dst2 += (2 * dst_w);

#define DST_RESTORE_LINE()	dst  -= CHARA_LINES * 2 * dst_w;	\
				dst2 -= CHARA_LINES * 2 * dst_w;

#define DST_NEXT_CHARA()	dst  += 8 * COLUMN_SKIP;		\
				dst2 += 8 * COLUMN_SKIP;

#define DST_NEXT_TOP_CHARA()	dst  += CHARA_LINES * 2 * dst_w - 640;	\
				dst2 += CHARA_LINES * 2 * dst_w - 640;

#elif	defined (LINE400)	/*-------------------------------------------*/
#define IF_LINE200_OR_EVEN_LINE()	if ((k & 1) == 0)

#define DST_DEFINE()		int	dst_w = SCREEN_WIDTH;		\
				TYPE	*dst  = (TYPE *) SCREEN_START;

#define DST_NEXT_LINE()		dst  += dst_w;

#define DST_RESTORE_LINE()	dst  -= CHARA_LINES * dst_w;

#define DST_NEXT_CHARA()	dst  += 8 * COLUMN_SKIP;

#define DST_NEXT_TOP_CHARA()	dst  += CHARA_LINES * dst_w - 640;
#endif

/*======================================================================
 * 描画マクロ定義 (MASK_DOT, TRANS_DOT, STORE_DOT, COPY_DOT)
 *======================================================================*/

#if	defined (LINE200)
#if	    defined (NORMAL)	/*-------------------------------------------*/
#if		defined (DIRECT)

#define DST_V(idx,c)	dstbuf[(idx)] = c;
#define DST_T(idx)	dstbuf[(idx)] = tcol;
#define DST_B(idx)	dstbuf[(idx)] = BLACK;

#define COPY_8DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*8 );		\
			memcpy( dst2, dstbuf, sizeof(TYPE)*8 );
#define COPY_16DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*16 );	\
			memcpy( dst2, dstbuf, sizeof(TYPE)*16 );

#else		/* ! DIRECT */

#define DST_V(idx,c)	dst[(idx)] = c;
#define DST_T(idx)	dst[(idx)] = tcol;
#define DST_B(idx)	dst[(idx)] = BLACK;
#define COPY_8DOT()	memcpy( dst2, dst, sizeof(TYPE)*8 );
#define COPY_16DOT()	memcpy( dst2, dst, sizeof(TYPE)*16 );

#endif		/* DIRECT */
#elif	    defined (SKIPLINE)	/*-------------------------------------------*/

#define DST_V(idx,c)	dst[(idx)] = c;
#define DST_T(idx)	dst[(idx)] = tcol;
#define DST_B(idx)	dst[(idx)] = BLACK;

#define COPY_8DOT()
#define COPY_16DOT()

#elif	    defined (INTERLACE) /*-------------------------------------------*/

#define DST_V(idx,c)	dst[(idx)] = c;	     dst2[(idx)] = BLACK;
#define DST_T(idx)	dst[(idx)] = tcol;   dst2[(idx)] = tcol;
#define DST_B(idx)	dst[(idx)] = BLACK;  dst2[(idx)] = BLACK;

#define COPY_8DOT()
#define COPY_16DOT()

#endif
#include "screen-vram-200.h"		/*****************/

#elif	defined (LINE400)	/*-------------------------------------------*/

#define DST_V(idx,c)	dst[(idx)] = c;
#define DST_T(idx)	dst[(idx)] = tcol;

#define COPY_8DOT()
#define COPY_16DOT()

#include "screen-vram-400.h"		/*****************/
#endif

/*======================================================================
 * 描画関数定義
 *======================================================================*/

#include "screen-vram.h"
