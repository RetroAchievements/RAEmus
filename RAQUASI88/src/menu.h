#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED

extern	int	menu_lang;			/* メニューの言語           */
extern	int	menu_readonly;			/* ディスク選択ダイアログは */
						/* 初期状態は ReadOnly ?    */
extern	int	menu_swapdrv;			/* ドライブの表示順序       */


extern	int	file_coding;			/* ファイル名の漢字コード   */
extern	int	filename_synchronize;		/* ファイル名を同調させる   */


	/* メニューモード */

void	menu_init(void);
void	menu_main(void);



/***********************************************************************
 * メニュー画面で表示する、システム固有のメッセージを取得する関数
 *
 * int	menu_about_osd_msg(int        req_japanese,
 *			   int        *result_code,
 *			   const char *message[])
 *
 *	メニュー画面の初期化時に呼び出される。
 *
 *	req_japanese … 真なら、日本語のメッセージ取得を、
 *			偽なら、英語(ASCII)メッセージ取得を要求する。
 *	
 *	result_code  … 日本語のメッセージの場合、漢字コードをセットする。
 *			EUC なら 1、SJIS なら 2、指定なしなら、-1。
 *			英語(ASCII)メッセージなら、不定でよい。
 *
 *	message      … メッセージ文字列へのポインタをセットする。
 *
 *	戻り値       … メッセージがある場合、真を返す。ないなら、偽を返す。
 *			偽を返す場合、 code, message は不定でよい。
 ************************************************************************/
int	menu_about_osd_msg(int        req_japanese,
			   int        *result_code,
			   const char *message[]);


void	menu_sound_restart(int output);
/*----------------------------------------------------------------------
 * イベント処理の対処
 *----------------------------------------------------------------------*/
void	q8tk_event_key_on(int code);
void	q8tk_event_key_off(int code);
void	q8tk_event_mouse_on(int code);
void	q8tk_event_mouse_off(int code);
void	q8tk_event_mouse_moved(int x, int y);
void	q8tk_event_quit(void);




#endif	/* MENU_H_INCLUDED */
