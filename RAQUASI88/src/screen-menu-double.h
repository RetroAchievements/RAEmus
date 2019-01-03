#define FONT_H	32
#define FONT_W	16

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

#define WORK_DEFINE()	TYPE *dst2;

#define GET_FONT()	*font_ptr

#define GET_CURSOR()	*cur_ptr

#define PUT_FONT()							\
    dst2 = dst + SCREEN_WIDTH;						\
    if (style & 0x80) { dst[ 0] = dst[ 1] = dst2[ 0] = dst2[ 1] = fg; } \
    else	      { dst[ 0] = dst[ 1] = dst2[ 0] = dst2[ 1] = bg; } \
    if (style & 0x40) { dst[ 2] = dst[ 3] = dst2[ 2] = dst2[ 3] = fg; } \
    else	      { dst[ 2] = dst[ 3] = dst2[ 2] = dst2[ 3] = bg; } \
    if (style & 0x20) { dst[ 4] = dst[ 5] = dst2[ 4] = dst2[ 5] = fg; } \
    else	      { dst[ 4] = dst[ 5] = dst2[ 4] = dst2[ 5] = bg; } \
    if (style & 0x10) { dst[ 6] = dst[ 7] = dst2[ 6] = dst2[ 7] = fg; } \
    else	      { dst[ 6] = dst[ 7] = dst2[ 6] = dst2[ 7] = bg; } \
    if (style & 0x08) { dst[ 8] = dst[ 9] = dst2[ 8] = dst2[ 9] = fg; } \
    else	      { dst[ 8] = dst[ 9] = dst2[ 8] = dst2[ 9] = bg; } \
    if (style & 0x04) { dst[10] = dst[11] = dst2[10] = dst2[11] = fg; } \
    else	      { dst[10] = dst[11] = dst2[10] = dst2[11] = bg; } \
    if (style & 0x02) { dst[12] = dst[13] = dst2[12] = dst2[13] = fg; } \
    else	      { dst[12] = dst[13] = dst2[12] = dst2[13] = bg; } \
    if (style & 0x01) { dst[14] = dst[15] = dst2[14] = dst2[15] = fg; } \
    else	      { dst[14] = dst[15] = dst2[14] = dst2[15] = bg; } \
    dst += 2 * SCREEN_WIDTH;


#include "screen-menu.h"
