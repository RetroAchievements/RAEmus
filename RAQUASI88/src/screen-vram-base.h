/*======================================================================
 * ループ定数などの定義
 *======================================================================*/
#if	defined (COLOR) || defined (MONO) || defined (UNDISP)

#define		LINE200

#elif	defined (HIRESO)

#define		LINE400

#else
#error
#endif
/*----------------------------------------------------------------------*/
#if	(TEXT_WIDTH == 80)

#define		COLUMNS		(80)
#define		COLUMN_SKIP	(1)
#define		DIRTY_TYPE	unsigned char

#elif	(TEXT_WIDTH == 40)

#define		COLUMNS		(40)
#define		COLUMN_SKIP	(2)
#define		DIRTY_TYPE	unsigned short

#else
#error
#endif
/*----------------------------------------------------------------------*/
#if	(TEXT_HEIGHT == 25)

#define		ROWS		(25)

#elif	(TEXT_HEIGHT == 20)

#define		ROWS		(20)

#else
#error
#endif
/*----------------------------------------------------------------------*/
#if	defined (LINE200)

#define		CHARA_LINES	(200 / ROWS)

#elif	defined (LINE400)

#define		CHARA_LINES	(400 / ROWS)

#else
#error
#endif

/*======================================================================
 * 88VRAMメモリパレット情報を、描画バッファの色情報に変換
 *======================================================================*/

#ifdef LSB_FIRST

/* 2ドット分のピクセル値(0〜7)を bit7〜4 と bit3〜0 にパック */
#define get_pixel_index73(data)				\
	((((data) & ((bit32) 0x00000088)) >>  3) |	\
	 (((data) & ((bit32) 0x00008800)) >> 10) |	\
	 (((data) & ((bit32) 0x00880000)) >> 17))
#define get_pixel_index62(data)				\
	((((data) & ((bit32) 0x00000044)) >>  2) |	\
	 (((data) & ((bit32) 0x00004400)) >>  9) |	\
	 (((data) & ((bit32) 0x00440000)) >> 16))
#define get_pixel_index51(data)				\
	((((data) & ((bit32) 0x00000022)) >>  1) |	\
	 (((data) & ((bit32) 0x00002200)) >>  8) |	\
	 (((data) & ((bit32) 0x00220000)) >> 15))
#define get_pixel_index40(data)				\
	((((data) & ((bit32) 0x00000011)) >>  0) |	\
	 (((data) & ((bit32) 0x00001100)) >>  7) |	\
	 (((data) & ((bit32) 0x00110000)) >> 14))

/* 3ドット分のピクセル値(0〜7)を bit8〜6 と bit5〜3 と bit2〜0 にパック */
#define get_pixel_index_52(data)			\
	((((data) & ((bit32) 0x00000024)) >>  2) |	\
	 (((data) & ((bit32) 0x00002400)) >>  9) |	\
	 (((data) & ((bit32) 0x00240000)) >> 16))
#define get_pixel_index741(data)			\
	((((data) & ((bit32) 0x00000092)) >>  1) |	\
	 (((data) & ((bit32) 0x00009200)) >>  8) |	\
	 (((data) & ((bit32) 0x00920000)) >> 15))
#define get_pixel_index630(data)			\
	((((data) & ((bit32) 0x00000049)) >>  0) |	\
	 (((data) & ((bit32) 0x00004900)) >>  7) |	\
	 (((data) & ((bit32) 0x00490000)) >> 14))

#else

#define get_pixel_index73(data)				\
	((((data) & ((bit32) 0x88000000)) >> 27) |	\
	 (((data) & ((bit32) 0x00880000)) >> 18) |	\
	 (((data) & ((bit32) 0x00008800)) >>  9))
#define get_pixel_index62(data)				\
	((((data) & ((bit32) 0x44000000)) >> 26) |	\
	 (((data) & ((bit32) 0x00440000)) >> 17) |	\
	 (((data) & ((bit32) 0x00004400)) >>  8))
#define get_pixel_index51(data)				\
	((((data) & ((bit32) 0x22000000)) >> 25) |	\
	 (((data) & ((bit32) 0x00220000)) >> 16) |	\
	 (((data) & ((bit32) 0x00002200)) >>  7))
#define get_pixel_index40(data)				\
	((((data) & ((bit32) 0x11000000)) >> 24) |	\
	 (((data) & ((bit32) 0x00110000)) >> 15) |	\
	 (((data) & ((bit32) 0x00001100)) >>  6))

#define get_pixel_index_52(data)			\
	((((data) & ((bit32) 0x24000000)) >> 26) |	\
	 (((data) & ((bit32) 0x00240000)) >> 17) |	\
	 (((data) & ((bit32) 0x00002400)) >>  8))
#define get_pixel_index741(data)			\
	((((data) & ((bit32) 0x92000000)) >> 25) |	\
	 (((data) & ((bit32) 0x00920000)) >> 16) |	\
	 (((data) & ((bit32) 0x00009200)) >>  7))
#define get_pixel_index630(data)			\
	((((data) & ((bit32) 0x49000000)) >> 24) |	\
	 (((data) & ((bit32) 0x00490000)) >> 15) |	\
	 (((data) & ((bit32) 0x00004900)) >>  6))
#endif

/*----------------------------------------------------------------------*/

#ifdef LSB_FIRST
#define make_mask_mono( mask )						\
	do {								\
	    mask = 0xffffffff;						\
	    if (grph_pile & GRPH_PILE_BLUE ) mask &= 0x00ffff00;	\
	    if (grph_pile & GRPH_PILE_RED  ) mask &= 0x00ff00ff;	\
	    if (grph_pile & GRPH_PILE_GREEN) mask &= 0x0000ffff;	\
	} while(0)
#define get_pixel_mono( data, col )					\
		(TYPE) (((data) & 0x00808080) ? (col) : COLOR_PIXEL(0))
#else
#define make_mask_mono( mask )						\
	do {								\
	    mask = 0xffffffff;						\
	    if (grph_pile & GRPH_PILE_BLUE ) mask &= 0x00ffff00;	\
	    if (grph_pile & GRPH_PILE_RED  ) mask &= 0xff00ff00;	\
	    if (grph_pile & GRPH_PILE_GREEN) mask &= 0xffff0000;	\
	} while(0)
#define get_pixel_mono( data, col )					\
		(TYPE) (((data) & 0x80808000) ? (col) : COLOR_PIXEL(0))
#endif

/*----------------------------------------------------------------------*/

#ifdef LSB_FIRST
#define get_pixel_400_B( data, col )				\
		(TYPE) (((data) & 0x00000080) ? (col) : COLOR_PIXEL(0))
#define get_pixel_400_R( data, col )				\
		(TYPE) (((data) & 0x00008000) ? (col) : COLOR_PIXEL(0))
#else
#define get_pixel_400_B( data, col )				\
		(TYPE) (((data) & 0x80000000) ? (col) : COLOR_PIXEL(0))
#define get_pixel_400_R( data, col )				\
		(TYPE) (((data) & 0x00800000) ? (col) : COLOR_PIXEL(0))
#endif



/*======================================================================
 * VRAM → pixel 変換に使用するワークの定義
 *======================================================================*/

#if	defined (COLOR)

#define WORK_DEFINE()					\
	int	m;					\
	bit32	vram;					\
	bit32	vcol[4];


#define GET_PIXEL_VCOL3(vram)				\
	vcol[0] = get_pixel_index741( vram );		\
	vcol[1] = get_pixel_index630( vram );		\
	vcol[2] = get_pixel_index_52( vram );

#define C7	COLOR_PIXEL( vcol[0] >> 6      )
#define C6	COLOR_PIXEL( vcol[1] >> 6      )
#define C5	COLOR_PIXEL((vcol[2] >> 3) & 7 )
#define C4	COLOR_PIXEL((vcol[0] >> 3) & 7 )
#define C3	COLOR_PIXEL((vcol[1] >> 3) & 7 )
#define C2	COLOR_PIXEL( vcol[2]	   & 7 )
#define C1	COLOR_PIXEL( vcol[0]	   & 7 )
#define C0	COLOR_PIXEL( vcol[1]	   & 7 )

/*----------------------------------------------------------------------*/
#elif	defined (MONO)

#define WORK_DEFINE()					\
	int	m, l;					\
	bit32	vram;					\
	bit32	mask;					\
	make_mask_mono( mask );

/*----------------------------------------------------------------------*/
#elif	defined (UNDISP)

#define WORK_DEFINE()					\
	int	m;

/*----------------------------------------------------------------------*/
#elif	defined (HIRESO)

#define WORK_DEFINE()					\
	int	m, l;					\
	bit32	vram;

#else
#error
#endif
