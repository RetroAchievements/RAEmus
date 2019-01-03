/*****************************************************************************
 * ハイレゾ (640x400x1)
 *****************************************************************************/

/* 【TEXTのみ描画】 */

#define MASK_8DOT()	for (m = 0; m <	 8; m++) { DST_T(m); }
#define MASK_16DOT()	for (m = 0; m < 16; m++) { DST_T(m); }


/* 【VRAMのみ描画】 */

#undef		UPPER_SCREEN
#if	(ROWS == 20)
#define		UPPER_SCREEN	(i < 10)
#elif	(ROWS == 25)
#define		UPPER_SCREEN	((i < 12) || (i == 12 && k < 8))
#endif

#define TRANS_8DOT()					\
    if (UPPER_SCREEN) {					\
	vram  = *(src + k * 80);			\
	for (l = 0; l < 8; l++, vram <<= 1) {		\
	    DST_V(l, get_pixel_400_B(vram, tcol));	\
	}						\
    } else {						\
	vram  = *(src + k * 80 - 80*200);		\
	for (l = 0; l < 8; l++, vram <<= 1) {		\
	    DST_V(l, get_pixel_400_R(vram, tcol));	\
	}						\
    }

#define TRANS_16DOT()					\
    if (UPPER_SCREEN) {					\
	vram = *(src + k * 80);				\
	for (l = 0; l < 8; l++, vram <<= 1) {		\
	    DST_V(l, get_pixel_400_B(vram, tcol));	\
	}						\
	vram = *(src + k * 80 + 1);			\
	for (	  ; l < 16; l++, vram <<= 1) {		\
	    DST_V(l, get_pixel_400_B(vram, tcol));	\
	}						\
    } else {						\
	vram = *(src + k * 80 - 80*200);		\
	for (l = 0; l < 8; l++, vram <<= 1) {		\
	    DST_V(l, get_pixel_400_R(vram, tcol));	\
	}						\
	vram = *(src + k * 80 + 1 - 80*200);		\
	for (	  ; l < 16; l++, vram <<= 1) {		\
	    DST_V(l, get_pixel_400_R(vram, tcol));	\
	}						\
    }

/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()							\
    if (UPPER_SCREEN) {							\
	vram  = *(src + k * 80);					\
	for (m = 0x80, l = 0; l < 8; l++, m >>= 1, vram <<= 1) {	\
	    if (style & m) { DST_T(l);				    }	\
	    else	   { DST_V(l, get_pixel_400_B(vram, tcol)); }	\
	}								\
    } else {								\
	vram  = *(src + k * 80 - 80*200);				\
	for (m = 0x80, l = 0; l < 8; l++, m >>= 1, vram <<= 1) {	\
	    if (style & m) { DST_T(l);				    }	\
	    else	   { DST_V(l, get_pixel_400_R(vram, tcol)); }	\
	}								\
    }

#define STORE_16DOT()							    \
    if (UPPER_SCREEN) {							    \
	vram = *(src + k * 80);						    \
	for (m = 0x80, l = 0; l < 8; l += 2, m >>= 1, vram <<= 2) {	    \
	    if (style & m) { DST_T(l  );				    \
			     DST_T(l+1);				 }  \
	    else	   { DST_V(l  , get_pixel_400_B(vram,	 tcol));    \
			     DST_V(l+1, get_pixel_400_B(vram<<1, tcol)); }  \
	}								    \
	vram = *(src + k * 80 + 1);					    \
	for (		    ; l < 16; l += 2, m >>= 1, vram <<= 2) {	    \
	    if (style & m) { DST_T(l  );				    \
			     DST_T(l+1);				 }  \
	    else	   { DST_V(l  , get_pixel_400_B(vram,	 tcol));    \
			     DST_V(l+1, get_pixel_400_B(vram<<1, tcol)); }  \
	}								    \
    } else {								    \
	vram = *(src + k * 80 - 80*200);				    \
	for (m = 0x80, l = 0; l < 8; l += 2, m >>= 1, vram <<= 2) {	    \
	    if (style & m) { DST_T(l  );				    \
			     DST_T(l+1);				 }  \
	    else	   { DST_V(l  , get_pixel_400_R(vram,	 tcol));    \
			     DST_V(l+1, get_pixel_400_R(vram<<1, tcol)); }  \
	}								    \
	vram = *(src + k * 80 + 1 - 80*200);				    \
	for (		    ; l < 16; l += 2, m >>= 1, vram <<= 2) {	    \
	    if (style & m) { DST_T(l  );				    \
			     DST_T(l+1);				 }  \
	    else	   { DST_V(l  , get_pixel_400_R(vram,	 tcol));    \
			     DST_V(l+1, get_pixel_400_R(vram<<1, tcol)); }  \
	}								    \
    }
