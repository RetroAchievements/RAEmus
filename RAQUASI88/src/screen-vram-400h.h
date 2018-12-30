/*****************************************************************************
 * ハイレゾ (640x400x1)
 *****************************************************************************/

/* 【TEXTのみ描画】 */

#define MASK_8DOT()	for (m = 0; m < 4; m++) { dst[m] = tcol; }
#define MASK_16DOT()	for (m = 0; m < 8; m++) { dst[m] = tcol; }


/* 【VRAMのみ描画】 */

#undef		UPPER_SCREEN
#if	(ROWS == 20)
#define		UPPER_SCREEN	(i<10)
#elif	(ROWS == 25)
#define		UPPER_SCREEN	((i < 12) || (i == 12 && k < 8))
#endif

#define TRANS_8DOT()					\
    if (UPPER_SCREEN) {					\
	vram  = *(src + k * 80);			\
	for (l = 0; l < 4; l++, vram <<= 2) {		\
	    dst[l] = get_pixel_400_B( vram, tcol );	\
	}						\
    } else {						\
	vram  = *(src + k * 80 - 80*200);		\
	for (l = 0; l < 4; l++, vram <<= 2) {		\
	    dst[l] = get_pixel_400_R( vram, tcol );	\
	}						\
    }

#define TRANS_16DOT()					\
    if (UPPER_SCREEN) {					\
	vram = *(src + k * 80);				\
	for (l = 0; l < 4; l++, vram <<= 2) {		\
	    dst[l] = get_pixel_400_B( vram, tcol );	\
	}						\
	vram = *(src + k * 80 + 1);			\
	for (	  ; l < 8; l++, vram <<= 2) {		\
	    dst[l] = get_pixel_400_B( vram, tcol );	\
	}						\
    } else {						\
	vram = *(src + k * 80 - 80*200);		\
	for (l = 0; l < 4; l++, vram <<= 2) {		\
	    dst[l] = get_pixel_400_R( vram, tcol );	\
	}						\
	vram = *(src + k * 80 + 1 - 80*200);		\
	for (	  ; l < 8; l++, vram <<= 2) {		\
	    dst[l] = get_pixel_400_R( vram, tcol );	\
	}						\
    }

/* 【TEXT/VRAM重ね合わせ描画】 */

#define STORE_8DOT()							\
    if (UPPER_SCREEN) {							\
	vram  = *(src + k * 80);					\
	for (m = 0xc0, l = 0; l < 4; l++, m >>= 2, vram <<= 2) {	\
	    if (style & m) { dst[l] = tcol;			     }	\
	    else	   { dst[l] = get_pixel_400_B( vram, tcol ); }	\
	}								\
    } else {								\
	vram  = *(src + k * 80 - 80*200);				\
	for (m = 0xc0, l = 0; l < 4; l++, m >>= 2, vram <<= 2) {	\
	    if (style & m) { dst[l] = tcol;			     }	\
	    else	   { dst[l] = get_pixel_400_R( vram, tcol ); }	\
	}								\
    }

#define STORE_16DOT()							\
    if (UPPER_SCREEN) {							\
	vram = *(src + k * 80);						\
	for (m = 0x80, l = 0; l < 4; l++, m >>= 1, vram <<= 2) {	\
	    if (style & m) { dst[l] = tcol;			     }	\
	    else	   { dst[l] = get_pixel_400_B( vram, tcol ); }	\
	}								\
	vram = *(src + k * 80 + 1);					\
	for (		    ; l < 8; l++, m >>= 1, vram <<= 2) {	\
	    if (style & m) { dst[l] = tcol;			     }	\
	    else	   { dst[l] = get_pixel_400_B( vram, tcol ); }	\
	}								\
    } else {								\
	vram = *(src + k * 80 - 80*200);				\
	for (m = 0x80, l = 0; l < 4; l++, m >>= 1, vram <<= 2) {	\
	    if (style & m) { dst[l] = tcol;			     }	\
	    else	   { dst[l] = get_pixel_400_R( vram, tcol ); }	\
	}								\
	vram = *(src + k * 80 + 1 - 80*200);				\
	for (		    ; l < 8; l++, m >>= 1, vram <<= 2) {	\
	    if (style & m) { dst[l] = tcol;			     }	\
	    else	   { dst[l] = get_pixel_400_R( vram, tcol ); }	\
	}								\
    }
