#ifndef DEVICE_H_INCLUDED
#define DEVICE_H_INCLUDED


#include <X11/Xlib.h>
#include <X11/Xutil.h>


/*
 *	src/X11/ 以下でのグローバル変数
 */
extern	Display	*x11_display;
extern	Window	x11_window;
extern	Atom	x11_atom_kill_type;
extern	Atom	x11_atom_kill_data;

extern	int	x11_width;		/* ウインドウ(全画面)のサイズ	*/
extern	int	x11_height;

extern	int	x11_mouse_rel_move;	/* マウス相対移動量検知させるか	*/
extern	int	x11_get_focus;		/* 現在、フォーカスありかどうか	*/

extern	int	x11_scaling;		/* マウス座標のスケーリング有無	*/
extern	int	x11_scale_x_num;
extern	int	x11_scale_x_den;
extern	int	x11_scale_y_num;
extern	int	x11_scale_y_den;



/*
 *	src/X11/ 以下でのグローバル変数 (オプション設定可能な変数)
 */
extern	int	colormap_type;		/* カラーマップのタイプ	0/1/2	*/
extern	int	use_xsync;		/* XSync を使用するかどうか	*/
extern	int	use_xv;			/* Xv を使用するかどうか	*/
#ifdef MITSHM
extern	int	use_SHM;		/* MIT-SHM を使用するかどうか	*/
#endif

extern	int	keyboard_type;		/* キーボードの種類                */
extern	char	*file_keyboard;		/* キー設定ファイル名		   */
extern	int	use_xdnd;		/* XDnD に対応するかどうか	   */
extern	int	use_joydevice;		/* ジョイスティックデバイスを開く? */

extern	int	wait_sleep_min_us;	/* 残り idle時間がこの us以下の
					   場合は、 sleep せずに待つ。
					   (MAX 1秒 = 1,000,000us) */

extern	int	show_fps;		/* test */



void	x11_init(void);
void	x11_exit(void);

void	x11_set_attribute_focus_in(void);
void	x11_set_attribute_focus_out(void);

void	xdnd_initialize(void);
void	xdnd_start(void);


#endif	/* DEVICE_H_INCLUDED */
