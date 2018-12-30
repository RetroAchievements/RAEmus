#ifdef		STATUS2SCREEN
void		STATUS2SCREEN(int kind, byte pixmap[], int w, int h)
{
    int x, y, pos;
    int top, start, end;
    int y_limit = MIN( h, status_sy[ kind ] );
    byte *src = pixmap;
    TYPE *dst = (TYPE *) status_start[ kind ];

    if (dst == NULL) return;

    if (kind == 0) {				/* 左揃え */
	top   = 0;
	start = 0;
	end   = MIN( w, status_sx[ kind ] );
    }
    else if (kind == 1) {			/* 中央揃え */
	if (w <= status_sx[ kind ]) {
	    top	  = 0;
	    start = (status_sx[ kind ] - w) / 2;
	    end	  = start + w;
	} else {
	    top	  = (w - status_sx[ kind ]) / 2;
	    start = 0;
	    end	  = status_sx[ kind ];
	}
    }
    else {					/* 右揃え */
	if (w <= status_sx[ kind ]) {
	    top	  = 0;
	    start = status_sx[ kind ] - w;
	    end	  = status_sx[ kind ];
	} else {
	    top	  = w - status_sx[ kind ];
	    start = 0;
	    end	  = status_sx[ kind ];
	}
    }

    for (y = 0; y < y_limit; y++) {
	for (x = 0; x < start; x++) {
	    dst[x] = (TYPE) status_pixel[ STATUS_BG ];
	}
	pos = top;
	for (	  ; x < end; x++) {
	    dst[x] = (TYPE) status_pixel[ src[ pos++ ] ];
	}
	for (	  ; x < status_sx[ kind ]; x++) {
	    dst[x] = (TYPE) status_pixel[ STATUS_BG ];
	}
	src += w;
	dst += SCREEN_WIDTH;
    }
    for (     ; y < status_sy[ kind ]; y++) {
	for (x = 0; x < status_sx[ kind ]; x++) {
	    dst[x] = (TYPE) status_pixel[ STATUS_BG ];
	}
	dst += SCREEN_WIDTH;
    }
}
#endif		/* STATUS2SCREEN */


#ifdef		STATUS_BUF_CLEAR
void		STATUS_BUF_CLEAR(void)
{
    unsigned int i, j;
    TYPE *p = (TYPE *) status_buf;

    if (p == NULL) return;

    for (j = STATUS_HEIGHT; j; j--) {
	for (i = SCREEN_WIDTH; i; i--) {
	    *p++ = BLACK;
	}
    }
}
#endif		/* STATUS_BUF_CLEAR */


#ifdef		STATUS_BUF_INIT
void		STATUS_BUF_INIT(void)
{
    unsigned int i, j;
    TYPE *p = (TYPE *) status_buf;

    if (p == NULL) return;

    for (j = 0; j < STATUS_HEIGHT; j++) {
	for (i = SCREEN_WIDTH; i; i--) {
	    if (j < 2) { *p++ = BLACK;				  }
	    else       { *p++ = (TYPE) status_pixel[ STATUS_BG ]; }
	}
    }
}
#endif		/* STATUS_BUF_INIT */
