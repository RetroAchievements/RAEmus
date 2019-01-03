#define FONT_H	16
#define FONT_W	8

#define FONT_8x8()			\
    font_inc  = 1;			\
    font_dup  = TRUE;			\
    font_skip = 1;

#define FONT_8x16()			\
    font_inc  = 1;			\
    font_dup  = FALSE;			\
    font_skip = 1;

#define FONT_16x16()			\
    font_inc  = 2;			\
    font_dup  = FALSE;			\
    font_skip = 1;

#define FONT_LOGO8x16()			\
    font_inc  = Q8GR_LOGO_W;		\
    font_dup  = FALSE;			\
    font_skip = 1;

#define WORK_DEFINE()

#define GET_FONT()	*font_ptr

#define GET_CURSOR()	*cur_ptr

#define PUT_FONT()						\
    if (style & 0x80) { dst[0] = fg; } else { dst[0] = bg; }	\
    if (style & 0x40) { dst[1] = fg; } else { dst[1] = bg; }	\
    if (style & 0x20) { dst[2] = fg; } else { dst[2] = bg; }	\
    if (style & 0x10) { dst[3] = fg; } else { dst[3] = bg; }	\
    if (style & 0x08) { dst[4] = fg; } else { dst[4] = bg; }	\
    if (style & 0x04) { dst[5] = fg; } else { dst[5] = bg; }	\
    if (style & 0x02) { dst[6] = fg; } else { dst[6] = bg; }	\
    if (style & 0x01) { dst[7] = fg; } else { dst[7] = bg; }	\
    dst += SCREEN_WIDTH;


#include "screen-menu.h"
