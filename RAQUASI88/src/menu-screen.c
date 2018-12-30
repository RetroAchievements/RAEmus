/************************************************************************/
/*									*/
/* メニューモードにおける画面の表示					*/
/*									*/
/* QUASI88 の メニューモードでの画面表示には、以下の関数を用いる。	*/
/* なお、これらの関数は、いずれも Q8TK から呼び出される。		*/
/*									*/
/*									*/
/* 【関数】								*/
/*									*/
/* voie	menu_draw( void )						*/
/*	・メニュー画面の表示。ワーク T_Q8GR_SCREEN menu_screen[][] に	*/
/*	  応じて、screen_buf にイメージを作成し、パレットを設定して、	*/
/*	  表示 (menu_draw_screen()) する。				*/
/*									*/
/* void menu_draw_screen( void )					*/
/*	・メニュー画面を表示 (screen_buf を表示)する。			*/
/*	 「画面露出による再描画」の際にも呼ばれる。			*/
/*									*/
/*======================================================================*/
/*									*/
/*	メニュー画面のワーク menu_screen[25][80] の構造体である。	*/
/*	メニュー画面は、80文字×25行で構成されており、その1文字毎に、	*/
/*	T_Q8GR_SCREEN 型のワークが1個、割り当てられている。		*/
/*									*/
/*	実際に表示される文字は、addr (漢字ROMアドレス) で示されているが	*/
/*	font_type によって、その addr の意味が異なる。すなわち、	*/
/*									*/
/*	font_type   フォント						*/
/*	---------   ---------------------------------------------------	*/
/*	FONT_ANK    font_mem[addr] からの 8 バイト			*/
/*	FONT_QUART  kanji_rom[0][addr][0] からの 8 バイト (addr<0x0800)	*/
/*	FONT_HALF   kanji_rom[0][addr][0] からの 16 バイト(addr<0xc000)	*/
/*	FONT_KNJ1L  kanji_rom[0][addr][0] から 1バイトおきに16 バイト	*/
/*	FONT_KNJ1R  kanji_rom[0][addr][1] から 1バイトおきに16 バイト	*/
/*	FONT_KNJ2L  kanji_rom[1][addr][0] から 1バイトおきに16 バイト	*/
/*	FONT_KNJ2R  kanji_rom[1][addr][1] から 1バイトおきに16 バイト	*/
/*	FONT_KNJXL  kanji_dummy_rom[0][0] から 1バイトおきに16 バイト	*/
/*	FONT_KNJXR  kanji_dummy_rom[0][1] から 1バイトおきに16 バイト	*/
/*									*/
/*	が、表示するフォントのビットパターンとなる。			*/
/*	また、QUASI88 の 画面は 640x400 ドット なので、80文字×25行の	*/
/*	場合、1 文字あたり 8x16 ドットとなる。				*/
/*	なお、FONT_ANK や FONT_QUART の場合は、ビットパターンデータが	*/
/*	8x8 と、半分しかないので、縦方向に拡大して表示する必要がある。	*/
/*									*/
/************************************************************************/

#include <string.h>

#include "quasi88.h"
#include "screen.h"
#include "menu-screen.h"

#include "q8tk.h"
#include "graph.h"


/*----------------------------------------------------------------------*/
/* メニュー画面のソフトウェアカーソルの字形				*/
/*----------------------------------------------------------------------*/
#if 0		/* 矢印カーソル */
byte menu_cursor_on[]  = { 0x80, 0xc0, 0xe0, 0xf0, 0xf8, 0xfc, 0xfe, 0xff,
			   0xd8, 0xa8, 0x0c, 0x0c, 0x06, 0x06, 0x03, 0x03 };
#else		/* ブロックカーソル */
byte menu_cursor_on[]  = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
			   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
#endif

byte menu_cursor_off[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
			   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

int	menu_cursor_x;
int	menu_cursor_y;


/*----------------------------------------------------------------------*/
/* メニュー表示用パレットを設定し、システムに転送			*/
/*----------------------------------------------------------------------*/
void	screen_get_menu_palette(PC88_PALETTE_T pal[16])
{
    int i;

    static const struct {
	int index;
	int col;
    } menupal[ 16 ] = {
	{ Q8GR_PALETTE_FOREGROUND, MENU_COLOR_FOREGROUND,	},
	{ Q8GR_PALETTE_BACKGROUND, MENU_COLOR_BACKGROUND,	},
	{ Q8GR_PALETTE_LIGHT,      MENU_COLOR_LIGHT,		},
	{ Q8GR_PALETTE_SHADOW,     MENU_COLOR_SHADOW,		},
	{ Q8GR_PALETTE_FONT_FG,    MENU_COLOR_FONT_FG,		},
	{ Q8GR_PALETTE_FONT_BG,    MENU_COLOR_FONT_BG,		},
	{ Q8GR_PALETTE_LOGO_FG,    MENU_COLOR_LOGO_FG,		},
	{ Q8GR_PALETTE_LOGO_BG,    MENU_COLOR_LOGO_BG,		},
	{ Q8GR_PALETTE_BLACK,      0x000000,			},
	{ Q8GR_PALETTE_SCALE_SLD,  MENU_COLOR_SCALE_SLD,	},
	{ Q8GR_PALETTE_SCALE_BAR,  MENU_COLOR_SCALE_BAR,	},
	{ Q8GR_PALETTE_SCALE_ACT,  MENU_COLOR_SCALE_ACT,	},
	{ Q8GR_PALETTE_RED,        0xff0000,			},
	{ Q8GR_PALETTE_GREEN,      0x00ff00,			},
	{ Q8GR_PALETTE_BLUE,       0x0000ff,			},
	{ Q8GR_PALETTE_WHITE,      0xffffff,			},
    };

    for (i=0; i<COUNTOF(menupal); i++) {
	pal[ menupal[i].index ].red   = (menupal[i].col >> 16) & 0xff;
	pal[ menupal[i].index ].green = (menupal[i].col >>  8) & 0xff;
	pal[ menupal[i].index ].blue  = (menupal[i].col >>  0) & 0xff;
    }
}
