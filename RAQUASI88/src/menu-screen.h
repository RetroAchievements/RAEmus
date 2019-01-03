#ifndef MENU_SCREEN_H_INCLUDED
#define MENU_SCREEN_H_INCLUDED


/*--------------------------------------------------------------
 * パレットに設定するカラー
 *--------------------------------------------------------------*/

#define	MENU_COLOR_FOREGROUND	(0x000000)
#define	MENU_COLOR_BACKGROUND	(0xd6d6d6)
#define	MENU_COLOR_LIGHT	(0xffffff)
#define	MENU_COLOR_SHADOW	(0x000000)
#define	MENU_COLOR_FONT_FG	(0x00009c)
#define	MENU_COLOR_FONT_BG	(0xffffff)
#define	MENU_COLOR_SCALE_SLD	(0xf0f0f0)
#define	MENU_COLOR_SCALE_BAR	(0xb0b0b0)
#define	MENU_COLOR_SCALE_ACT	(0x0000e0)
#define	MENU_COLOR_LOGO_FG	(0x000000)
#define	MENU_COLOR_LOGO_BG	(0xd6d6d6)



/*--------------------------------------------------------------
 *
 *--------------------------------------------------------------*/

extern	byte	menu_cursor_on[16];
extern	byte	menu_cursor_off[16];



#endif	/* MENU_SCREEN_H_INCLUDED */
