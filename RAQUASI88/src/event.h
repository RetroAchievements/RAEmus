#ifndef	EVENT_H_INCLUDED
#define	EVENT_H_INCLUDED


/***********************************************************************
 * イベント処理 (システム依存)
 ************************************************************************/

/******************************************************************************
 * イベント処理の初期化／終了
 *
 * void event_init(void)
 *	ジョイスティック処理や、システム固有のキーバインディング処理があれば、
 *	ここでやっておく。
 *	この関数は、起動時に 1回だけ呼び出される。
 *
 * void event_exit(void)
 *	イベント処理の後始末を行う。
 *	この関数は、終了時に 1回だけ呼び出される。
 *
 *****************************************************************************/
void	event_init(void);
void	event_exit(void);


/******************************************************************************
 * イベント処理の再初期化
 *
 * void	event_switch(void)
 *	エミュ／メニュー／ポーズ／モニターモードの切替時に呼び出される。
 *	各モードに応じた、システムのイベント処理の準備をする。
 *	(キー入力イベント処理や、メニューバーの設定を変える場合など)
 *
 *****************************************************************************/
void	event_switch(void);


/******************************************************************************
 * イベント処理の実行
 *
 * void event_update(void)
 *	約1/60秒毎 (エミュレータの VSYNC毎) に呼び出される。
 *	ここで、全てのイベント処理を行い、エミュレータへの入力を更新する。
 *
 *	o キー押下／解放時
 *	  押されたキーに応じて、以下の関数を呼び出す。
 *		押下時: quasi88_key(KEY88_XXX, TRUE);
 *		開放時: quasi88_key(KEY88_XXX, FALSE);
 *
 *	  この時の、押されたキーと KEY88_XXX の対応は任意 (システム依存) だが、
 *	  ここで対応をしていない (割り当てていない) KEY88_XXX については、
 *	  QUASI88 で入力不可となる。
 *
 *	  特に、KEY_SPACE 〜 KEY88_TILDE を割り当てていなかった場合、メニュー
 *	  モードで これらに該当する ASCII文字が入力が出来なくなるので注意。
 *
 *	o マウスボタン押下／解放時
 *	  押されたボタンに応じて、以下の関数を呼び出す。
 *		押下時: quasi88_mouse(KEY88_MOUSE_XXX, TRUE);
 *		開放時: quasi88_mouse(KEY88_MOUSE_XXX, FALSE);
 *
 *	o ジョイスティック押下／解放時
 *	  押されたボタンに応じて、以下の関数を呼び出す。
 *		押下時: quasi88_pad(KEY88_PAD_XXX, TRUE);
 *		開放時: quasi88_pad(KEY88_PAD_XXX, FALSE);
 *
 *	o マウス移動時 (絶対座標 / 相対座標)
 *	  移動先 x, y (ないし移動量 dx, xy) に応じて、以下の関数を呼び出す。
 *		移動先: quasi88_mouse_move( x,  y, TRUE);
 *		移動量: quasi88_mouse_move(dx, dy, FALSE);
 *
 *	  絶対座標の場合、graph_setup() の戻り値にて応答した T_SCREEN_INFO 型の
 *	  width * height に対する値をセットすること。(値は範囲外でも可)
 *
 *	o フォーカスあり／なし時
 *	  ウインドウがフォーカスを得た(アクティブになった)時や、フォーカスを
 *	  失った(非アクティブになった)時に呼び出す。
 *	  フォーカスなし時にポーズモードするという隠しオプション (-focus) の
 *	  処理に使ってるだけなので、実装しなくても影響はない。
 *			quasi88_focus_in() / quasi88_focus_out()
 *
 *	o 強制終了時
 *	  ウインドウが強制的に閉じられた場合、以下の関数を呼び出す。
 *			quasi88_quit()
 *
 *****************************************************************************/
void	event_update(void);


/******************************************************************************
 * その他の雑多な関数
 *
 * int  event_numlock_on(void)
 * void event_numlock_off(void)
 *	ソフトウェア NumLock を有効／無効にする際に、呼び出される。
 *	どのキーを押したら、どのテンキーを押したことになるのかは、システム依存
 *	とするので、システムのキー配列に合わせて、処理する。
 *	変更できないならば、 event_numlock_on() の戻り値を 偽 にする。
 *
 * void	event_get_mouse_pos(int *x, int *y)
 *	現在のマウスの絶対座標をセットする。
 *	絶対座標の概念がない場合は、任意の値をセットしてもよい。
 *	セットする座標値は、graph_setup() の戻り値にて応答した T_SCREEN_INFO 型
 *	の width * height に対する値とする。(値は範囲外でも可)
 *	この関数は、モードの切り替わり時に呼び出される。
 *
 * int	event_get_joystick_num(void)
 *	使用可能なジョイスティックの数 (0〜2) を返す。
 *
 *****************************************************************************/
int	event_numlock_on(void);
void	event_numlock_off(void);

void	event_get_mouse_pos(int *x, int *y);

int	event_get_joystick_num(void);




/******************************************************************************
 * 以下は、上記のシステム依存な関数より、呼び出される関数
 *		そのうち、上手く整理しなくては…
 *****************************************************************************/

/*----------------------------------------------------------------------
 * イベント処理の対処
 *----------------------------------------------------------------------*/
void	quasi88_key  (int code, int on_flag);
void	quasi88_mouse(int code, int on_flag);
void	quasi88_pad  (int code, int on_flag);
void	quasi88_mouse_move(int x, int y, int abs_flag);

#define	quasi88_key_pressed(code)	quasi88_key  (code, TRUE )
#define	quasi88_key_released(code)	quasi88_key  (code, FALSE)
#define	quasi88_mouse_pressed(code)	quasi88_mouse(code, TRUE )
#define	quasi88_mouse_released(code)	quasi88_mouse(code, FALSE)
#define	quasi88_pad_pressed(code)	quasi88_pad  (code, TRUE )
#define	quasi88_pad_released(code)	quasi88_pad  (code, FALSE)
#define	quasi88_mouse_moved_abs(x, y)	quasi88_mouse_move(x, y, TRUE)
#define	quasi88_mouse_moved_rel(x, y)	quasi88_mouse_move(x, y, FALSE)



void	quasi88_expose(void);
void	quasi88_focus_in(void);
void	quasi88_focus_out(void);


/* 以下は、実装実験中。呼び出しに条件があるでの注意 */

typedef struct {
    int		boot_basic;
    int		boot_dipsw;
    int		boot_from_rom;
    int		boot_clock_4mhz;
    int		set_version;
    byte	baudrate_sw;
    int		use_extram;
    int		use_jisho_rom;
    int		sound_board;
} T_RESET_CFG;
void	quasi88_get_reset_cfg(T_RESET_CFG *cfg);
void	quasi88_reset(const T_RESET_CFG *cfg);
int	quasi88_stateload(int serial);
int	quasi88_statesave(int serial);
int	quasi88_screen_snapshot(void);
int	quasi88_waveout(int start);
int	quasi88_drag_and_drop(const char *filename);


int	quasi88_cfg_now_wait_rate(void);
void	quasi88_cfg_set_wait_rate(int rate);
int	quasi88_cfg_now_no_wait(void);
void	quasi88_cfg_set_no_wait(int enable);


int	quasi88_disk_insert_all(const char *filename, int ro);
int	quasi88_disk_insert(int drv, const char *filename, int image, int ro);
int	quasi88_disk_insert_A_to_B(int src_drv, int dst_drv, int dst_img);
void	quasi88_disk_eject_all(void);
void	quasi88_disk_eject(int drv);

void	quasi88_disk_image_select(int drv, int img);
void	quasi88_disk_image_empty(int drv);
void	quasi88_disk_image_next(int drv);
void	quasi88_disk_image_prev(int drv);

int	quasi88_load_tape_insert( const char *filename );
int	quasi88_load_tape_rewind( void );
void	quasi88_load_tape_eject( void );
int	quasi88_save_tape_insert( const char *filename );
void	quasi88_save_tape_eject( void );

int	quasi88_serial_in_connect( const char *filename );
void	quasi88_serial_in_remove( void );
int	quasi88_serial_out_connect( const char *filename );
void	quasi88_serial_out_remove( void );
int	quasi88_printer_connect( const char *filename );
void	quasi88_printer_remove( void );



#endif	/* EVENT_H_INCLUDED */
