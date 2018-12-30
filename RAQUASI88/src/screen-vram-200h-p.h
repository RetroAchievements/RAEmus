/*****************************************************************************
 * カラー / 白黒 / 非表示 共通
 *****************************************************************************/

/* 【TEXTのみ描画】 */

#define MASK_8DOT()	for (m = 0; m < 4; m++) { dst[m] = tcol; }
#define MASK_16DOT()	for (m = 0; m < 8; m++) { dst[m] = tcol; }

/*****************************************************************************
 * カラー (640x200x1)
 *****************************************************************************/
#if	defined (COLOR)

/* 【VRAMのみ描画】 */

#define TRANS_8DOT()					\
    vram = *(src + k * 80);				\
    vcol[0] = get_pixel_index73( vram );		\
    vcol[1] = get_pixel_index62( vram );		\
    vcol[2] = get_pixel_index51( vram );		\
    vcol[3] = get_pixel_index40( vram );		\
    dst[0] = MIXED_PIXEL( vcol[0] >> 4, vcol[1] >> 4 ); \
    dst[1] = MIXED_PIXEL( vcol[2] >> 4, vcol[3] >> 4 ); \
    dst[2] = MIXED_PIXEL( vcol[0] &  7, vcol[1] &  7 ); \
    dst[3] = MIXED_PIXEL( vcol[2] &  7, vcol[3] &  7 );

#define TRANS_16DOT()					\
    vram = *(src + k * 80);				\
    vcol[0] = get_pixel_index73( vram );		\
    vcol[1] = get_pixel_index62( vram );		\
    vcol[2] = get_pixel_index51( vram );		\
    vcol[3] = get_pixel_index40( vram );		\
    dst[0] = MIXED_PIXEL( vcol[0] >> 4, vcol[1] >> 4 ); \
    dst[1] = MIXED_PIXEL( vcol[2] >> 4, vcol[3] >> 4 ); \
    dst[2] = MIXED_PIXEL( vcol[0] &  7, vcol[1] &  7 ); \
    dst[3] = MIXED_PIXEL( vcol[2] &  7, vcol[3] &  7 ); \
    vram = *(src + k * 80 + 1);				\
    vcol[0] = get_pixel_index73( vram );		\
    vcol[1] = get_pixel_index62( vram );		\
    vcol[2] = get_pixel_index51( vram );		\
    vcol[3] = get_pixel_index40( vram );		\
    dst[4] = MIXED_PIXEL( vcol[0] >> 4, vcol[1] >> 4 ); \
    dst[5] = MIXED_PIXEL( vcol[2] >> 4, vcol[3] >> 4 ); \
    dst[6] = MIXED_PIXEL( vcol[0] &  7, vcol[1] &  7 ); \
    dst[7] = MIXED_PIXEL( vcol[2] &  7, vcol[3] &  7 );


/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()							\
    vram = *(src + k * 80);						\
    vcol[0] = get_pixel_index73( vram );				\
    vcol[1] = get_pixel_index62( vram );				\
    vcol[2] = get_pixel_index51( vram );				\
    vcol[3] = get_pixel_index40( vram );				\
    {									\
	int h, l;							\
	if (style & 0x80) { h = tpal; } else { h = vcol[0] >> 4; }	\
	if (style & 0x40) { l = tpal; } else { l = vcol[1] >> 4; }	\
	dst[0] = MIXED_PIXEL( h , l );					\
	if (style & 0x20) { h = tpal; } else { h = vcol[2] >> 4; }	\
	if (style & 0x10) { l = tpal; } else { l = vcol[3] >> 4; }	\
	dst[1] = MIXED_PIXEL( h , l );					\
	if (style & 0x08) { h = tpal; } else { h = vcol[0] &  7; }	\
	if (style & 0x04) { l = tpal; } else { l = vcol[1] &  7; }	\
	dst[2] = MIXED_PIXEL( h , l );					\
	if (style & 0x02) { h = tpal; } else { h = vcol[2] &  7; }	\
	if (style & 0x01) { l = tpal; } else { l = vcol[3] &  7; }	\
	dst[3] = MIXED_PIXEL( h , l );					\
    }

#define STORE_16DOT()							      \
    vram = *(src + k * 80);						      \
    vcol[0] = get_pixel_index73( vram );				      \
    vcol[1] = get_pixel_index62( vram );				      \
    vcol[2] = get_pixel_index51( vram );				      \
    vcol[3] = get_pixel_index40( vram );				      \
    if (style & 0x80) { dst[0] = tcol;					    } \
    else	      { dst[0] = MIXED_PIXEL( vcol[0] >> 4, vcol[1] >> 4 ); } \
    if (style & 0x40) { dst[1] = tcol;					    } \
    else	      { dst[1] = MIXED_PIXEL( vcol[2] >> 4, vcol[3] >> 4 ); } \
    if (style & 0x20) { dst[2] = tcol;					    } \
    else	      { dst[2] = MIXED_PIXEL( vcol[0] &	 7, vcol[1] &  7 ); } \
    if (style & 0x10) { dst[3] = tcol;					    } \
    else	      { dst[3] = MIXED_PIXEL( vcol[2] &	 7, vcol[3] &  7 ); } \
    vram = *(src + k * 80 + 1);						      \
    vcol[0] = get_pixel_index73( vram );				      \
    vcol[1] = get_pixel_index62( vram );				      \
    vcol[2] = get_pixel_index51( vram );				      \
    vcol[3] = get_pixel_index40( vram );				      \
    if (style & 0x08) { dst[4] = tcol;					    } \
    else	      { dst[4] = MIXED_PIXEL( vcol[0] >> 4, vcol[1] >> 4 ); } \
    if (style & 0x04) { dst[5] = tcol;					    } \
    else	      { dst[5] = MIXED_PIXEL( vcol[2] >> 4, vcol[3] >> 4 ); } \
    if (style & 0x02) { dst[6] = tcol;					    } \
    else	      { dst[6] = MIXED_PIXEL( vcol[0] &	 7, vcol[1] &  7 ); } \
    if (style & 0x01) { dst[7] = tcol;					    } \
    else	      { dst[7] = MIXED_PIXEL( vcol[2] &	 7, vcol[3] &  7 ); }

/*****************************************************************************
 * 白黒 (640x200x3)
 *****************************************************************************/
#elif	defined (MONO)

/* 【VRAMのみ描画】 */

#define TRANS_8DOT()				\
    vram  = *(src + k * 80);			\
    vram &= mask;				\
    for (l = 0; l < 4; l++, vram <<= 2) {	\
	dst[l] = get_pixel_mono( vram, tcol );	\
    }

#define TRANS_16DOT()				\
    vram = *(src + k * 80);			\
    vram &= mask;				\
    for (l = 0; l < 4; l++, vram <<= 2) {	\
	dst[l] = get_pixel_mono( vram, tcol );	\
    }						\
    vram = *(src + k * 80 + 1);			\
    vram &= mask;				\
    for (     ; l < 8; l++, vram <<= 2) {	\
	dst[l] = get_pixel_mono( vram, tcol );	\
    }

/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()							\
    vram  = *(src + k * 80);						\
    vram &= mask;							\
    for (m = 0xc0, l = 0; l < 4; l++, m >>= 2, vram <<= 2) {		\
	if (style & m) { dst[l] = tcol;				}	\
	else	       { dst[l] = get_pixel_mono( vram, tcol ); }	\
    }

#define STORE_16DOT()							\
    vram = *(src + k * 80);						\
    vram &= mask;							\
    for (m = 0x80, l = 0; l < 4; l++, m >>= 1, vram <<= 2) {		\
	if (style & m) { dst[l] = tcol;				}	\
	else	       { dst[l] = get_pixel_mono( vram, tcol ); }	\
    }									\
    vram = *(src + k * 80 + 1);						\
    vram &= mask;							\
    for (		; l < 8; l++, m >>= 1, vram <<= 2) {		\
	if (style & m) { dst[l] = tcol;				}	\
	else	       { dst[l] = get_pixel_mono( vram, tcol ); }	\
    }

/*****************************************************************************
 * VRAM非表示
 *****************************************************************************/
#elif	defined (UNDISP)

/* 【VRAMのみ描画】 */

#define TRANS_8DOT()	for (m = 0; m < 4; m++) { dst[m] = BLACK; }
#define TRANS_16DOT()	for (m = 0; m < 8; m++) { dst[m] = BLACK; }

/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()						\
    {								\
	int bpal = (grph_ctrl & GRPH_CTRL_COLOR) ? 8 : 7;	\
	int h, l;						\
	if (style & 0x80) { h = tpal; } else { h = bpal; }	\
	if (style & 0x40) { l = tpal; } else { l = bpal; }	\
	dst[0] = MIXED_PIXEL( h , l );				\
	if (style & 0x20) { h = tpal; } else { h = bpal; }	\
	if (style & 0x10) { l = tpal; } else { l = bpal; }	\
	dst[1] = MIXED_PIXEL( h , l );				\
	if (style & 0x08) { h = tpal; } else { h = bpal; }	\
	if (style & 0x04) { l = tpal; } else { l = bpal; }	\
	dst[2] = MIXED_PIXEL( h , l );				\
	if (style & 0x02) { h = tpal; } else { h = bpal; }	\
	if (style & 0x01) { l = tpal; } else { l = bpal; }	\
	dst[3] = MIXED_PIXEL( h , l );				\
    }

#define STORE_16DOT()							\
    if (style & 0x80) { dst[0] = tcol; } else { dst[0] = BLACK; }	\
    if (style & 0x40) { dst[1] = tcol; } else { dst[1] = BLACK; }	\
    if (style & 0x20) { dst[2] = tcol; } else { dst[2] = BLACK; }	\
    if (style & 0x10) { dst[3] = tcol; } else { dst[3] = BLACK; }	\
    if (style & 0x08) { dst[4] = tcol; } else { dst[4] = BLACK; }	\
    if (style & 0x04) { dst[5] = tcol; } else { dst[5] = BLACK; }	\
    if (style & 0x02) { dst[6] = tcol; } else { dst[6] = BLACK; }	\
    if (style & 0x01) { dst[7] = tcol; } else { dst[7] = BLACK; }

#endif
