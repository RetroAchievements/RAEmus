#if	(TEXT_WIDTH == 80)

#define MASK_DOT()	MASK_8DOT()
#define TRANS_DOT()	TRANS_8DOT()
#define STORE_DOT()	STORE_8DOT()
#define COPY_DOT()	COPY_8DOT()

#elif	(TEXT_WIDTH == 40)

#define MASK_DOT()	MASK_16DOT()
#define TRANS_DOT()	TRANS_16DOT()
#define STORE_DOT()	STORE_16DOT()
#define COPY_DOT()	COPY_16DOT()

#endif

/******************************************************************************
 *	VRAM / TEXT の、前回描画から更新された部分だけを、描画
 *****************************************************************************/
#ifdef		VRAM2SCREEN_DIFF
static	int	VRAM2SCREEN_DIFF(void)
{
    int x0 = COLUMNS-1, x1 = 0, y0 = ROWS-1, y1 = 0;	/* 更新したエリア */
    int i, j, k;
    int changed_line;	/* 更新するラインをビット(0〜CHARA_LINES-1)で表す */

    unsigned short text, *text_attr= &text_attr_buf[ text_attr_flipflop	  ][0];
    unsigned short old,	 *old_attr = &text_attr_buf[ text_attr_flipflop^1 ][0];
    T_GRYPH fnt;				/* フォントの字形 1文字分   */
    int	    fnt_idx;				/* フォントの字形 参照位置  */
    bit8    style = 0;				/* フォントの字形 8ドット分 */
    int	    tpal;				/* フォントの色コード	    */
    TYPE    tcol;				/* フォントの色		    */
    DIRTY_TYPE *up =
	(DIRTY_TYPE *) &screen_dirty_flag[0];	/* VRAM更新フラグへのポインタ*/
    bit32 *src = main_vram4;			/* VRAMへのポインタ	     */
    DST_DEFINE()				/* 描画エリアへのポインタ    */
    WORK_DEFINE()				/* 処理に必要なワーク	     */

    /* 1文字単位で描画する。  行×桁 分、ループ	 (40桁なら2文字分同時に処理) */

    for (i = 0; i < ROWS; i++) {/*===========================================*/

	for (j = 0; j < COLUMNS; j++) {/*------------------------------------*/

	    text = *text_attr;	text_attr += COLUMN_SKIP;  /* テキストコード */
	    old	 = *old_attr;	old_attr  += COLUMN_SKIP;  /* ・属性を取得   */

	    if (text != old) {		    /* テキスト新旧不一致 ?    */
		changed_line = ~(0);		/* yes 1文字分強制更新 */
	    }
	    else {				/* no 更新するラインを探す */
		changed_line = 0;
		for (k = 0; k < CHARA_LINES; k++) {
		    if (up[ k * COLUMNS ]) { changed_line |= (1 << k); }
		}
	    }

	    if (changed_line) {		    /* いずれかのラインを描画 ? */
		if (i<y0) y0=i; if (i>y1) y1=i; if (j<x0) x0=j; if (j>x1) x1=j;

		get_font_gryph( text, &fnt, &tpal );  /* フォントの形を取得 */
		tcol = COLOR_PIXEL( tpal );
		fnt_idx = 0;

		for (k = 0; k < CHARA_LINES; k++) {/*- - - - - - - - - - - - */
						   /*1文字のライン数分,ループ*/
		    
		    IF_LINE200_OR_EVEN_LINE()	/* 200LINE／400LINE偶数line時*/
			style = fnt.b[ fnt_idx++ ]; /* フォント8ドットを取得 */

		    if (changed_line & (1 << k)) {  /* このラインを描画 ?    */

			if	  (style == 0xff) {	/* TEXT部 のみを描画 */
			    MASK_DOT();			/*		     */
			} else if (style == 0x00) {	/* VRAM部 のみを描画 */
			    TRANS_DOT();		/*		     */
			} else {			/* TEXT/VRAM合成描画 */
			    STORE_DOT();		/*		     */
			}				/*		     */
			COPY_DOT();			/* ラインの 隙間埋め */
		    }

		    DST_NEXT_LINE();		    /* 次のライン位置に進む  */
		}				/*- - - - - - - - - - - - - -*/

		DST_RESTORE_LINE();		/* ライン先頭に戻す	     */
	    }

	    up++;			    /* 次の文字位置に進む	     */
	    src += COLUMN_SKIP;
	    DST_NEXT_CHARA();
	}			       /*------------------------------------*/

	up  += (CHARA_LINES - 1) * COLUMNS;	/* 次の行の先頭文字位置に進む*/
	src += (CHARA_LINES - 1) * 80;
	DST_NEXT_TOP_CHARA();
    }				/*===========================================*/

    if (x0 <= x1) {
	return ((( x0	   * COLUMN_SKIP ) << 24) |
		(((y0	 ) * (200 / ROWS)) << 16) |
		(((x1 + 1) * COLUMN_SKIP ) <<  8) |
		(((y1 + 1) * (200 / ROWS))	));
    } else {
	return -1;
    }
}
#endif		/* VRAM2SCREEN_DIFF */


/******************************************************************************
 *	VRAM / TEXT の全画面分を、描画
 *****************************************************************************/
#ifdef		VRAM2SCREEN_ALL
static	int	VRAM2SCREEN_ALL(void)
{

    int i, j, k;


    unsigned short text, *text_attr= &text_attr_buf[ text_attr_flipflop	  ][0];

    T_GRYPH fnt;				/* フォントの字形 1文字分   */
    int	    fnt_idx;				/* フォントの字形 参照位置  */
    bit8    style = 0;				/* フォントの字形 8ドット分 */
    int	    tpal;				/* フォントの色コード	    */
    TYPE    tcol;				/* フォントの色		    */


    bit32 *src = main_vram4;			/* VRAMへのポインタ	     */
    DST_DEFINE()				/* 描画エリアへのポインタ    */
    WORK_DEFINE()				/* 処理に必要なワーク	     */

    /* 1文字単位で描画する。  行×桁 分、ループ	 (40桁なら2文字分同時に処理) */

    for (i = 0; i < ROWS; i++) {/*===========================================*/

	for (j = 0; j < COLUMNS; j++) {/*------------------------------------*/

	    text = *text_attr;	text_attr += COLUMN_SKIP;  /* テキストコード */












	    {				    /* 全てのラインを描画 */


		get_font_gryph( text, &fnt, &tpal );  /* フォントの形を取得 */
		tcol = COLOR_PIXEL( tpal );
		fnt_idx = 0;

		for (k = 0; k < CHARA_LINES; k++) {/*- - - - - - - - - - - - */
						   /*1文字のライン数分,ループ*/

		    IF_LINE200_OR_EVEN_LINE()	/* 200LINE／400LINE偶数line時*/
			style = fnt.b[ fnt_idx++ ]; /* フォント8ドットを取得 */

		    {				    /* ライン単位で描画	     */

			if	  (style == 0xff) {	/* TEXT部 のみを描画 */
			    MASK_DOT();			/*		     */
			} else if (style == 0x00) {	/* VRAM部 のみを描画 */
			    TRANS_DOT();		/*		     */
			} else {			/* TEXT/VRAM合成描画 */
			    STORE_DOT();		/*		     */
			}				/*		     */
			COPY_DOT();			/* ラインの 隙間埋め */
		    }

		    DST_NEXT_LINE();		    /* 次のライン位置に進む  */
		}				/*- - - - - - - - - - - - - -*/

		DST_RESTORE_LINE();		/* ライン先頭に戻す	     */
	    }


					    /* 次の文字位置に進む	     */
	    src += COLUMN_SKIP;
	    DST_NEXT_CHARA();
	}			       /*------------------------------------*/

						/* 次の行の先頭文字位置に進む*/
	src += (CHARA_LINES - 1) * 80;
	DST_NEXT_TOP_CHARA();
    }				/*===========================================*/

    return (((0) << 24) | ((0) << 16) | ((COLUMNS * COLUMN_SKIP) <<8) | (200));
}
#endif		/* VRAM2SCREEN_ALL */

/******************************************************************************
 *
 *****************************************************************************/
#undef	TEXT_WIDTH
#undef	TEXT_HEIGHT

#undef	VRAM2SCREEN_DIFF
#undef	VRAM2SCREEN_ALL

#undef	LINE200
#undef	LINE400
#undef	COLUMNS
#undef	COLUMN_SKIP
#undef	ROWS
#undef	CHARA_LINES
#undef	DIRTY_TYPE

#undef	get_pixel_index73
#undef	get_pixel_index62
#undef	get_pixel_index51
#undef	get_pixel_index40
#undef	get_pixel_index_52
#undef	get_pixel_index741
#undef	get_pixel_index630
#undef	make_mask_mono
#undef	get_pixel_mono
#undef	get_pixel_400_B
#undef	get_pixel_400_R
#undef	WORK_DEFINE
#undef	C7
#undef	C6
#undef	C5
#undef	C4
#undef	C3
#undef	C2
#undef	C1
#undef	C0

#undef	DST_DEFINE_ADD
#undef	IF_LINE200_OR_EVEN_LINE
#undef	DST_DEFINE
#undef	DST_NEXT_LINE
#undef	DST_RESTORE_LINE
#undef	DST_NEXT_CHARA
#undef	DST_NEXT_TOP_CHARA

#undef	DST_V
#undef	DST_T
#undef	DST_B

#undef	MASK_DOT
#undef	MASK_8DOT
#undef	MASK_16DOT
#undef	TRANS_DOT
#undef	TRANS_8DOT
#undef	TRANS_16DOT
#undef	STORE_DOT
#undef	STORE_8DOT
#undef	STORE_16DOT
#undef	COPY_DOT
#undef	COPY_8DOT
#undef	COPY_16DOT
