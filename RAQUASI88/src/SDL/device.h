#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <SDL.h>


/*
 *	src/SDL/ 以下でのグローバル変数
 */
extern	int	sdl_mouse_rel_move;	/* マウス相対移動量検知可能か	*/



/*
 *	src/SDL/ 以下でのグローバル変数 (オプション設定可能な変数)
 */
extern	int	use_hwsurface;		/* HW SURFACE を使うかどうか	*/
extern	int	use_doublebuf;		/* ダブルバッファを使うかどうか	*/

extern	int	use_cmdkey;		/* Commandキーでメニューへ遷移     */
extern	int	keyboard_type;		/* キーボードの種類                */
extern	char	*file_keyboard;		/* キー設定ファイル名		   */
extern	int	use_joydevice;		/* ジョイスティックデバイスを開く? */
extern	int	show_fps;		/* test */



int	sdl_init(void);
void	sdl_exit(void);

#endif	/* DEVICE_H_INCLUDED */
