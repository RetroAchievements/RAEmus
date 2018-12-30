#define FONT_H	8
#define FONT_W	4

#define FONT_8x8()			\
    font_inc  = 1;			\
    font_dup  = FALSE;			\
    font_skip = 2;

#define FONT_8x16()			\
    font_inc  = 2;			\
    font_dup  = FALSE;			\
    font_skip = 2;

#define FONT_16x16()			\
    font_inc  = 4;			\
    font_dup  = FALSE;			\
    font_skip = 2;

#define FONT_LOGO8x16()			\
    font_inc  = 2 * Q8GR_LOGO_W;	\
    font_dup  = FALSE;			\
    font_skip = 2;

#define WORK_DEFINE()

#define GET_FONT()	(*font_ptr | *(font_ptr + font_inc / 2))

#define GET_CURSOR()	(*cur_ptr  | *(cur_ptr + 1))

#define PUT_FONT()							    \
    if (reverse) {							    \
	if ((style & 0xc0) == 0xc0) { dst[0] = fg; } else { dst[0] = bg; }  \
	if ((style & 0x30) == 0x30) { dst[1] = fg; } else { dst[1] = bg; }  \
	if ((style & 0x0c) == 0x0c) { dst[2] = fg; } else { dst[2] = bg; }  \
	if ((style & 0x03) == 0x03) { dst[3] = fg; } else { dst[3] = bg; }  \
    } else {								    \
	if (style & 0xc0)	    { dst[0] = fg; } else { dst[0] = bg; }  \
	if (style & 0x30)	    { dst[1] = fg; } else { dst[1] = bg; }  \
	if (style & 0x0c)	    { dst[2] = fg; } else { dst[2] = bg; }  \
	if (style & 0x03)	    { dst[3] = fg; } else { dst[3] = bg; }  \
    }									    \
    dst += SCREEN_WIDTH;


#include "screen-menu.h"
