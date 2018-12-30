#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <Windows.h>
#include <Dialogs.h>
#include <Menus.h>
#include <QuickTime.h>


/*
 *	src/CLASSIC/ 以下でのグローバル変数
 */
extern	WindowRef	macWin;
extern	GWorldPtr	macGw;

extern	QDGlobals	macQd;



/*
 *	src/CLASSIC/ 以下でのグローバル変数 (オプション設定可能な変数)
 */
extern	int		mac_8bpp;	/* 優先的に、256色モードで動作させる */



void	mac_init(void);
void	mac_exit(void);

Boolean mac_create_menubar(void);
void	menubar_setup(int active);
void	doMenuCommand(long menuResult);
void	mac_error_dialog(const char *message);

int	mac_is_fullscreen(void);
void	mac_draw_immidiate(void);


#endif	/* DEVICE_H_INCLUDED */
