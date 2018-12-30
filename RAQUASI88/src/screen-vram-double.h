#include "screen-vram-base.h"

/*======================================================================
 * 描画先の変数定義 (dst)
 *======================================================================*/

#if	defined (DIRECT) && defined (INTERLACE)
#define DST_DEFINE_ADD		TYPE	dstbuf[32], dstbuf3[32];
#elif	defined (DIRECT)
#define DST_DEFINE_ADD		TYPE	dstbuf[32];
#else
#define DST_DEFINE_ADD
#endif

#if	defined (LINE200)	/*-------------------------------------------*/
#define IF_LINE200_OR_EVEN_LINE()	    /* nothing */

#define DST_DEFINE()		int	dst_w = SCREEN_WIDTH;		\
				DST_DEFINE_ADD				\
				TYPE	*dst  = (TYPE *) SCREEN_START;	\
				TYPE	*dst2 = dst  + dst_w;		\
				TYPE	*dst3 = dst2 + dst_w;		\
				TYPE	*dst4 = dst3 + dst_w;

#define DST_NEXT_LINE()		dst  += (4 * dst_w);			\
				dst2 += (4 * dst_w);			\
				dst3 += (4 * dst_w);			\
				dst4 += (4 * dst_w);

#define DST_RESTORE_LINE()	dst  -= CHARA_LINES * 4 * dst_w;	\
				dst2 -= CHARA_LINES * 4 * dst_w;	\
				dst3 -= CHARA_LINES * 4 * dst_w;	\
				dst4 -= CHARA_LINES * 4 * dst_w;

#define DST_NEXT_CHARA()	dst  += 16 * COLUMN_SKIP;		\
				dst2 += 16 * COLUMN_SKIP;		\
				dst3 += 16 * COLUMN_SKIP;		\
				dst4 += 16 * COLUMN_SKIP;

#define DST_NEXT_TOP_CHARA()	dst  += CHARA_LINES * 4 * dst_w -1280;	\
				dst2 += CHARA_LINES * 4 * dst_w -1280;	\
				dst3 += CHARA_LINES * 4 * dst_w -1280;	\
				dst4 += CHARA_LINES * 4 * dst_w -1280;

#elif	defined (LINE400)	/*-------------------------------------------*/
#define IF_LINE200_OR_EVEN_LINE()	if ((k & 1) == 0)

#define DST_DEFINE()		int	dst_w = SCREEN_WIDTH;		\
				DST_DEFINE_ADD				\
				TYPE	*dst  = (TYPE *) SCREEN_START;	\
				TYPE	*dst2 = dst  + dst_w;

#define DST_NEXT_LINE()		dst  += (2 * dst_w);			\
				dst2 += (2 * dst_w);

#define DST_RESTORE_LINE()	dst  -= CHARA_LINES * 2 * dst_w;	\
				dst2 -= CHARA_LINES * 2 * dst_w;

#define DST_NEXT_CHARA()	dst  += 16 * COLUMN_SKIP;		\
				dst2 += 16 * COLUMN_SKIP;

#define DST_NEXT_TOP_CHARA()	dst  += CHARA_LINES * 2 * dst_w - 1280; \
				dst2 += CHARA_LINES * 2 * dst_w - 1280;
#endif

/*======================================================================
 * 描画マクロ定義 (MASK_DOT, TRANS_DOT, STORE_DOT, COPY_DOT)
 *======================================================================*/

#if	defined (LINE200)
#if	    defined (NORMAL)	/*-------------------------------------------*/
#if		defined (DIRECT)

#define DST_V(idx,c)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = c;
#define DST_T(idx)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = tcol;
#define DST_B(idx)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = BLACK;

#define COPY_8DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*16 );	\
			memcpy( dst2, dstbuf, sizeof(TYPE)*16 );	\
			memcpy( dst3, dstbuf, sizeof(TYPE)*16 );	\
			memcpy( dst4, dstbuf, sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*32 );	\
			memcpy( dst2, dstbuf, sizeof(TYPE)*32 );	\
			memcpy( dst3, dstbuf, sizeof(TYPE)*32 );	\
			memcpy( dst4, dstbuf, sizeof(TYPE)*32 );

#else		/* ! DIRECT */

#define DST_V(idx,c)	dst[2*(idx)] = dst[2*(idx)+1] = c;
#define DST_T(idx)	dst[2*(idx)] = dst[2*(idx)+1] = tcol;
#define DST_B(idx)	dst[2*(idx)] = dst[2*(idx)+1] = BLACK;

#define COPY_8DOT()	memcpy( dst2, dst, sizeof(TYPE)*16 );	\
			memcpy( dst3, dst, sizeof(TYPE)*16 );	\
			memcpy( dst4, dst, sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst2, dst, sizeof(TYPE)*32 );	\
			memcpy( dst3, dst, sizeof(TYPE)*32 );	\
			memcpy( dst4, dst, sizeof(TYPE)*32 );

#endif		/* DIRECT */
#elif	    defined (SKIPLINE)	/*-------------------------------------------*/
#if		defined (DIRECT)

#define DST_V(idx,c)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = c;
#define DST_T(idx)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = tcol;
#define DST_B(idx)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = BLACK;

#define COPY_8DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*16 );	\
			memcpy( dst2, dstbuf, sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*32 );	\
			memcpy( dst2, dstbuf, sizeof(TYPE)*32 );

#else		/* ! DIRECT */

#define DST_V(idx,c)	dst[2*(idx)] = dst[2*(idx)+1] = c;
#define DST_T(idx)	dst[2*(idx)] = dst[2*(idx)+1] = tcol;
#define DST_B(idx)	dst[2*(idx)] = dst[2*(idx)+1] = BLACK;

#define COPY_8DOT()	memcpy( dst2, dst,   sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst2, dst,   sizeof(TYPE)*32 );

#endif		/* DIRECT */
#elif	    defined (INTERLACE) /*-------------------------------------------*/
#if		defined (DIRECT)

#define DST_V(idx,c)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = c;      dstbuf3[2*(idx)] = dstbuf3[2*(idx)+1] = BLACK;
#define DST_T(idx)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = tcol;   dstbuf3[2*(idx)] = dstbuf3[2*(idx)+1] = tcol;
#define DST_B(idx)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = BLACK;  dstbuf3[2*(idx)] = dstbuf3[2*(idx)+1] = BLACK;

#define COPY_8DOT()	memcpy( dst,  dstbuf,  sizeof(TYPE)*16 );	\
			memcpy( dst2, dstbuf,  sizeof(TYPE)*16 );	\
			memcpy( dst3, dstbuf3, sizeof(TYPE)*16 );	\
			memcpy( dst4, dstbuf3, sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst,  dstbuf,  sizeof(TYPE)*32 );	\
			memcpy( dst2, dstbuf,  sizeof(TYPE)*32 );	\
			memcpy( dst3, dstbuf3, sizeof(TYPE)*32 );	\
			memcpy( dst4, dstbuf3, sizeof(TYPE)*32 );

#else		/* ! DIRECT */

#define DST_V(idx,c)	dst[2*(idx)] = dst[2*(idx)+1] = c;	dst3[2*(idx)] = dst3[2*(idx)+1] = BLACK;
#define DST_T(idx)	dst[2*(idx)] = dst[2*(idx)+1] = tcol;	dst3[2*(idx)] = dst3[2*(idx)+1] = tcol;
#define DST_B(idx)	dst[2*(idx)] = dst[2*(idx)+1] = BLACK;	dst3[2*(idx)] = dst3[2*(idx)+1] = BLACK;

#define COPY_8DOT()	memcpy( dst2, dst,   sizeof(TYPE)*16 );		\
			memcpy( dst4, dst3,  sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst2, dst,   sizeof(TYPE)*32 );		\
			memcpy( dst4, dst3,  sizeof(TYPE)*32 );

#endif		/* DIRECT */
#endif
#include "screen-vram-200.h"		/*****************/

#elif	defined (LINE400)	/*-------------------------------------------*/
#if	    defined (DIRECT)

#define DST_V(idx,c)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = c;
#define DST_T(idx)	dstbuf[2*(idx)] = dstbuf[2*(idx)+1] = tcol;

#define COPY_8DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*16 );	\
			memcpy( dst2, dstbuf, sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst,  dstbuf, sizeof(TYPE)*32 );	\
			memcpy( dst2, dstbuf, sizeof(TYPE)*32 );

#else		/* ! DIRECT */

#define DST_V(idx,c)	dst[2*(idx)] = dst[2*(idx)+1] = c;
#define DST_T(idx)	dst[2*(idx)] = dst[2*(idx)+1] = tcol;

#define COPY_8DOT()	memcpy( dst2, dst, sizeof(TYPE)*16 );
#define COPY_16DOT()	memcpy( dst2, dst, sizeof(TYPE)*32 );

#endif		/* DIRECT */
#include "screen-vram-400.h"		/*****************/
#endif

/*======================================================================
 * 描画関数定義
 *======================================================================*/

#include "screen-vram.h"
