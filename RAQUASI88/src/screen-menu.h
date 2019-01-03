extern	byte	menu_cursor_on[];
extern	byte	menu_cursor_off[];
extern	int	menu_cursor_x;
extern	int	menu_cursor_y;

/******************************************************************************
 *	メニューモードの画面描画
 *****************************************************************************/
#ifdef		MENU2SCREEN
int		MENU2SCREEN(void)
{
    int x, y;
    T_Q8GR_SCREEN *old = &menu_screen[menu_screen_current ^ 1][0][0];
    T_Q8GR_SCREEN *src = &menu_screen[menu_screen_current    ][0][0];
    TYPE		*dst = (TYPE *) SCREEN_START;
    int x0 = Q8GR_SCREEN_X-1, x1 = 0,  y0 = Q8GR_SCREEN_Y-1, y1 = 0;

    /*menu_cursor_x = menu_cursor_y = -1;*/

    for (y = 0; y < Q8GR_SCREEN_Y; y++) {
	for (x = 0; x < Q8GR_SCREEN_X; x++) {

	    if (*((Uint *) src) != *((Uint *) old)) {

		int   i, font_inc, font_dup, font_skip;
		byte  style, *font_ptr;
		int   reverse	= (src->reverse) ? 0xff : 0x00;
		int   underline =  src->underline;
		TYPE  fg	= COLOR_PIXEL( src->foreground );
		TYPE  bg	= COLOR_PIXEL( src->background );
		byte *cur_ptr =(src->mouse) ? menu_cursor_on : menu_cursor_off;
		WORK_DEFINE();

		if (src->mouse) { menu_cursor_x = x; menu_cursor_y = y; }

		if(y<y0) y0=y;	if(y>y1) y1=y;	if(x<x0) x0=x;	if(x>x1) x1=x;

		switch (src->font_type) {
		case FONT_ANK:
		    font_ptr = &font_mem[ src->addr ];
		    FONT_8x8();
		    break;
		case FONT_QUART:
		    font_ptr = &kanji_rom[0][ src->addr ][0];
		    FONT_8x8();
		    break;
		case FONT_HALF:
		    font_ptr = &kanji_rom[0][ src->addr ][0];
		    FONT_8x16();
		    break;
		case FONT_KNJ1L:
		    font_ptr = &kanji_rom[0][ src->addr ][0];
		    FONT_16x16();
		    break;
		case FONT_KNJ1R:
		    font_ptr = &kanji_rom[0][ src->addr ][0];
		    font_ptr ++;
		    FONT_16x16();
		    break;
		case FONT_KNJ2L:
		    font_ptr = &kanji_rom[1][ src->addr ][0];
		    FONT_16x16();
		    break;
		case FONT_KNJ2R:
		    font_ptr = &kanji_rom[1][ src->addr ][1];
		    FONT_16x16();
		    break;
		case FONT_KNJXL:
		    font_ptr = &kanji_dummy_rom[0][0];
		    FONT_16x16();
		    break;
		case FONT_KNJXR:
		    font_ptr = &kanji_dummy_rom[0][1];
		    FONT_16x16();
		    break;
		case FONT_LOGO:
		    font_ptr = &q8gr_logo[ src->addr ];
		    FONT_LOGO8x16();
		    break;
		default:	/* trap */
		    font_ptr = &font_mem[0];
		    FONT_8x8();
		    break;
		}

		for (i = 16; i; i -= font_skip) {
		    style = GET_FONT() ^ reverse;
		    if (i <= 2 && underline) style = 0xff;
		    style ^= GET_CURSOR();

		    PUT_FONT()

		    if (font_dup == FALSE || (i & 1)) {
			font_ptr += font_inc;
		    }
		    cur_ptr += font_skip;
		}
		dst = dst - FONT_H * SCREEN_WIDTH;

	    }
	    old ++;
	    src ++;
	    dst += FONT_W;
	}
	dst += FONT_H * SCREEN_WIDTH - SCREEN_SX;
    }

    if (x0 <= x1) {
	return ((x0 << 24) | ((y0 << 16) * 8) | ((x1+1) << 8) | ((y1+1) * 8));
    } else {
	return -1;
    }
}
#endif		/* MENU2SCREEN */

/******************************************************************************
 *
 *****************************************************************************/
#undef	FONT_H
#undef	FONT_W
#undef	FONT_8x8
#undef	FONT_8x16
#undef	FONT_16x16
#undef	FONT_LOGO8x16
#undef	WORK_DEFINE
#undef	GET_FONT
#undef	GET_CURSOR
#undef	PUT_FONT
#undef	MENU2SCREEN
