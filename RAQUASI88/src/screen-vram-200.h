/*****************************************************************************
 * カラー / 白黒 / 非表示 共通
 *****************************************************************************/

/* 【TEXTのみ描画】 */

#define MASK_8DOT()	for (m = 0; m <	 8; m++) { DST_T(m); }
#define MASK_16DOT()	for (m = 0; m < 16; m++) { DST_T(m); }

/*****************************************************************************
 * カラー (640x200x1)
 *****************************************************************************/
#if	defined (COLOR)

/* 【VRAMのみ描画】 */

#define TRANS_8DOT()			\
    vram = *(src + k * 80);		\
    GET_PIXEL_VCOL3(vram);		\
    DST_V(0, C7);			\
    DST_V(1, C6);			\
    DST_V(2, C5);			\
    DST_V(3, C4);			\
    DST_V(4, C3);			\
    DST_V(5, C2);			\
    DST_V(6, C1);			\
    DST_V(7, C0);

#define TRANS_16DOT()			\
    vram = *(src + k * 80);		\
    GET_PIXEL_VCOL3(vram);		\
    DST_V(0, C7);			\
    DST_V(1, C6);			\
    DST_V(2, C5);			\
    DST_V(3, C4);			\
    DST_V(4, C3);			\
    DST_V(5, C2);			\
    DST_V(6, C1);			\
    DST_V(7, C0);			\
    vram = *(src + k * 80 + 1);		\
    GET_PIXEL_VCOL3(vram);		\
    DST_V( 8, C7);			\
    DST_V( 9, C6);			\
    DST_V(10, C5);			\
    DST_V(11, C4);			\
    DST_V(12, C3);			\
    DST_V(13, C2);			\
    DST_V(14, C1);			\
    DST_V(15, C0);

/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()						\
    vram = *(src + k * 80);					\
    GET_PIXEL_VCOL3(vram);					\
    if (style & 0x80) { DST_T(0); } else { DST_V(0, C7); }	\
    if (style & 0x40) { DST_T(1); } else { DST_V(1, C6); }	\
    if (style & 0x20) { DST_T(2); } else { DST_V(2, C5); }	\
    if (style & 0x10) { DST_T(3); } else { DST_V(3, C4); }	\
    if (style & 0x08) { DST_T(4); } else { DST_V(4, C3); }	\
    if (style & 0x04) { DST_T(5); } else { DST_V(5, C2); }	\
    if (style & 0x02) { DST_T(6); } else { DST_V(6, C1); }	\
    if (style & 0x01) { DST_T(7); } else { DST_V(7, C0); }

#define STORE_16DOT()						\
    vram = *(src + k * 80);					\
    GET_PIXEL_VCOL3(vram);					\
    if (style & 0x80) { DST_T( 0);	 DST_T( 1);	}	\
    else	      { DST_V( 0, C7);	 DST_V( 1, C6); }	\
    if (style & 0x40) { DST_T( 2);	 DST_T( 3);	}	\
    else	      { DST_V( 2, C5);	 DST_V( 3, C4); }	\
    if (style & 0x20) { DST_T( 4);	 DST_T( 5);	}	\
    else	      { DST_V( 4, C3);	 DST_V( 5, C2); }	\
    if (style & 0x10) { DST_T( 6);	 DST_T( 7);	}	\
    else	      { DST_V( 6, C1);	 DST_V( 7, C0); }	\
    vram = *(src + k * 80 + 1);					\
    GET_PIXEL_VCOL3(vram);					\
    if (style & 0x08) { DST_T( 8);	 DST_T( 9);	}	\
    else	      { DST_V( 8, C7);	 DST_V( 9, C6); }	\
    if (style & 0x04) { DST_T(10);	 DST_T(11);	}	\
    else	      { DST_V(10, C5);	 DST_V(11, C4); }	\
    if (style & 0x02) { DST_T(12);	 DST_T(13);	}	\
    else	      { DST_V(12, C3);	 DST_V(13, C2); }	\
    if (style & 0x01) { DST_T(14);	 DST_T(15);	}	\
    else	      { DST_V(14, C1);	 DST_V(15, C0); }

/*****************************************************************************
 * 白黒 (640x200x3)
 *****************************************************************************/
#elif	defined (MONO)

/* 【VRAMのみ描画】 */

#define TRANS_8DOT()					\
    vram = *(src + k * 80);				\
    vram &= mask;					\
    for (l = 0; l < 8; l++, vram <<= 1) {		\
	DST_V(l, get_pixel_mono( vram, tcol ));		\
    }

#define TRANS_16DOT()					\
    vram = *(src + k * 80);				\
    vram &= mask;					\
    for (l = 0; l < 8; l++, vram <<= 1) {		\
	DST_V(l, get_pixel_mono( vram, tcol ));		\
    }							\
    vram = *(src + k * 80 + 1);				\
    vram &= mask;					\
    for (     ; l < 16; l++, vram <<= 1) {		\
	DST_V(l, get_pixel_mono( vram, tcol ));		\
    }

/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()							\
    vram = *(src + k * 80);						\
    vram &= mask;							\
    for (m = 0x80, l = 0; l < 8; l++, m >>= 1, vram <<= 1) {		\
	if (style & m) { DST_T(l);				 }	\
	else	       { DST_V(l, get_pixel_mono( vram, tcol )); }	\
    }

#define STORE_16DOT()							\
    vram = *(src + k * 80);						\
    vram &= mask;							\
    for (m = 0x80, l = 0; l < 8; l += 2, m >>= 1, vram <<= 2) {		\
	if (style & m) { DST_T(l  );					\
			 DST_T(l+1);				      } \
	else	       { DST_V(l  , get_pixel_mono( vram,    tcol ));	\
			 DST_V(l+1, get_pixel_mono( vram<<1, tcol )); } \
    }									\
    vram = *(src + k * 80 + 1);						\
    vram &= mask;							\
    for (		; l < 16; l += 2, m >>= 1, vram <<= 2) {	\
	if (style & m) { DST_T(l  );					\
			 DST_T(l+1);				      } \
	else	       { DST_V(l  , get_pixel_mono( vram,    tcol ));	\
			 DST_V(l+1, get_pixel_mono( vram<<1, tcol )); } \
    }

/*****************************************************************************
 * VRAM非表示
 *****************************************************************************/
#elif	defined (UNDISP)

/* 【VRAMのみ描画】 */

#define TRANS_8DOT()	for (m = 0; m <	 8; m++) { DST_B(m); }
#define TRANS_16DOT()	for (m = 0; m < 16; m++) { DST_B(m); }

/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()					\
    if (style & 0x80) { DST_T(0); } else { DST_B(0); }	\
    if (style & 0x40) { DST_T(1); } else { DST_B(1); }	\
    if (style & 0x20) { DST_T(2); } else { DST_B(2); }	\
    if (style & 0x10) { DST_T(3); } else { DST_B(3); }	\
    if (style & 0x08) { DST_T(4); } else { DST_B(4); }	\
    if (style & 0x04) { DST_T(5); } else { DST_B(5); }	\
    if (style & 0x02) { DST_T(6); } else { DST_B(6); }	\
    if (style & 0x01) { DST_T(7); } else { DST_B(7); }

#define STORE_16DOT()							      \
    if (style & 0x80) { DST_T( 0); DST_T( 1); } else { DST_B( 0); DST_B( 1); }\
    if (style & 0x40) { DST_T( 2); DST_T( 3); } else { DST_B( 2); DST_B( 3); }\
    if (style & 0x20) { DST_T( 4); DST_T( 5); } else { DST_B( 4); DST_B( 5); }\
    if (style & 0x10) { DST_T( 6); DST_T( 7); } else { DST_B( 6); DST_B( 7); }\
    if (style & 0x08) { DST_T( 8); DST_T( 9); } else { DST_B( 8); DST_B( 9); }\
    if (style & 0x04) { DST_T(10); DST_T(11); } else { DST_B(10); DST_B(11); }\
    if (style & 0x02) { DST_T(12); DST_T(13); } else { DST_B(12); DST_B(13); }\
    if (style & 0x01) { DST_T(14); DST_T(15); } else { DST_B(14); DST_B(15); }

#endif
