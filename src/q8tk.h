#ifndef Q8TK_H_INCLUDED
#define Q8TK_H_INCLUDED

#include "quasi88.h"
#include "keyboard.h"

/*--------------------------------------------------------------
 * メニュー表示用の画面
 *--------------------------------------------------------------*/

#define	Q8GR_SCREEN_X	(80)
#define	Q8GR_SCREEN_Y	(25)


enum {			/* 構造体 T_Q8GR_SCREEN のメンバ font_type の値 */
    FONT_UNUSED = 0,
    FONT_1_BYTE,
    FONT_ANK    = FONT_1_BYTE,	/* ASCII (Alphabet, Number, Kana etc)	*/
    FONT_QUART,			/* 1/4角文字 (ANK)			*/
    FONT_HALF,			/* 半角文字  (英数字、片仮名、平仮名)	*/
    FONT_LOGO,			/* メニュー用ロゴ			*/
    FONT_2_BYTE,
    FONT_KNJ1L  = FONT_2_BYTE,	/* 漢字 第一水準 (左半分)		*/
    FONT_KNJ1R,			/* 漢字 第一水準 (右半分)		*/
    FONT_KNJ2L,			/* 漢字 第二水準 (左半分)		*/
    FONT_KNJ2R,			/* 漢字 第二水準 (右半分)		*/
    FONT_KNJXL,			/* 漢字 ダミー   (左半分)		*/
    FONT_KNJXR,			/* 漢字 ダミー   (右半分)		*/

				/* 最大で16種類(4bit)まで拡張可能	*/
};


typedef	struct {
    Uint background:	4;	/* 背景パレットコード (0〜15)		*/
    Uint foreground:	4;	/* 表示パレットコード (0〜15)		*/
    Uint rsv:		1;
    Uint mouse:		1;	/* マウスポインタ	なし=0 あり=1	*/
    Uint reverse:	1;	/* 反転表示		通常=0 反転=1	*/
    Uint underline:	1;	/* アンダーライン	なし=0 あり=1	*/
    Uint font_type:	4;	/* フォントタイプ (下参照)		*/
    Uint addr:		16;	/* 漢字ROM アドレス			*/
} T_Q8GR_SCREEN;   /* 計32bit == unsigned int と決めつけ (;_;)		*/


extern	int		menu_screen_current;
extern	T_Q8GR_SCREEN	menu_screen[2][ Q8GR_SCREEN_Y ][ Q8GR_SCREEN_X ];


/*--------------------------------------------------------------
 * メニューにて表示するタイトルロゴ (データは、8x16ドット単位)
 *--------------------------------------------------------------*/

#define	Q8GR_LOGO_W	(24)
#define	Q8GR_LOGO_H	(3)

extern	byte	q8gr_logo[ Q8GR_LOGO_W * Q8GR_LOGO_H * 16 ];


/*--------------------------------------------------------------
 * メニュー画面のパレットコード
 *--------------------------------------------------------------*/

#define	Q8GR_PALETTE_FOREGROUND	(0)
#define	Q8GR_PALETTE_BACKGROUND	(1)
#define	Q8GR_PALETTE_LIGHT	(2)
#define	Q8GR_PALETTE_SHADOW	(3)
#define	Q8GR_PALETTE_FONT_FG	(4)
#define	Q8GR_PALETTE_FONT_BG	(5)
#define	Q8GR_PALETTE_LOGO_FG	(6)
#define	Q8GR_PALETTE_LOGO_BG	(7)
#define	Q8GR_PALETTE_BLACK	(8)
#define	Q8GR_PALETTE_SCALE_SLD	(9)
#define	Q8GR_PALETTE_SCALE_BAR	(10)
#define	Q8GR_PALETTE_SCALE_ACT	(11)
#define	Q8GR_PALETTE_RED	(12)
#define	Q8GR_PALETTE_GREEN	(13)
#define	Q8GR_PALETTE_BLUE	(14)
#define	Q8GR_PALETTE_WHITE	(15)


/*--------------------------------------------------------------
 * メニューで使用するキーコード (Q8TK 専用の特殊キー)
 *--------------------------------------------------------------*/

#define	Q8TK_KEY_BS		KEY88_BS
#define	Q8TK_KEY_DEL		KEY88_DEL
#define	Q8TK_KEY_TAB		KEY88_TAB
#define	Q8TK_KEY_RET		KEY88_RETURN
#define	Q8TK_KEY_ESC		KEY88_ESC
#define	Q8TK_KEY_RIGHT		KEY88_RIGHT
#define	Q8TK_KEY_LEFT		KEY88_LEFT
#define	Q8TK_KEY_UP		KEY88_UP
#define	Q8TK_KEY_DOWN		KEY88_DOWN
#define	Q8TK_KEY_PAGE_UP	KEY88_ROLLDOWN
#define	Q8TK_KEY_PAGE_DOWN	KEY88_ROLLUP
#define	Q8TK_KEY_SPACE		KEY88_SPACE
#define	Q8TK_KEY_SHIFT		KEY88_SHIFT
#define	Q8TK_KEY_HOME		KEY88_HOME
#define	Q8TK_KEY_END		KEY88_HELP

#define	Q8TK_KEY_F1		KEY88_F1
#define	Q8TK_KEY_F2		KEY88_F2
#define	Q8TK_KEY_F3		KEY88_F3
#define	Q8TK_KEY_F4		KEY88_F4
#define	Q8TK_KEY_F5		KEY88_F5
#define	Q8TK_KEY_F6		KEY88_F6
#define	Q8TK_KEY_F7		KEY88_F7
#define	Q8TK_KEY_F8		KEY88_F8
#define	Q8TK_KEY_F9		KEY88_F9
#define	Q8TK_KEY_F10		KEY88_F10
#define	Q8TK_KEY_F11		KEY88_F11
#define	Q8TK_KEY_F12		KEY88_F12


#define	Q8TK_BUTTON_L		KEY88_MOUSE_L
#define	Q8TK_BUTTON_R		KEY88_MOUSE_R
#define	Q8TK_BUTTON_U		KEY88_MOUSE_WUP
#define	Q8TK_BUTTON_D		KEY88_MOUSE_WDN

#define	Q8TK_BUTTON_OFF		(0)
#define	Q8TK_BUTTON_ON		(1)


/*--------------------------------------------------------------
 * ウィジットの構造体
 *--------------------------------------------------------------*/

/*--------------------------------------
 * アジャストメント
 *--------------------------------------*/

typedef	struct	_Q8Adjust	Q8Adjust;
struct	_Q8Adjust {
    int		value;		/* 現在値 */
    int		lower;		/* 最小値 */
    int		upper;		/* 最大値 */
    int		step_increment;	/* 増分(小) */
    int		page_increment;	/*     (大) */
    int		max_length;	/* バーサイズ(矢印除)、0で自動 */
    int		x, y;		/* 表示時 : 座標           */
    int		length;		/*	  : スケールサイズ */
    int		pos;		/*        : スライダー位置 */
    int		horizontal;	/*        : TRUEで水平配置 */
    int		arrow;		/*        : TRUEで矢印あり */
    float	scale;		/*        : 表示倍率	   */

    int		listbox_changed;/* LISTBOXの変更時例外処理 */
};

/*--------------------------------------
 * ウィジット共通
 *--------------------------------------*/

typedef	struct	_Q8tkWidget	Q8tkWidget;
typedef	struct	_Q8List		Q8List;

typedef void (*Q8tkSignalFunc)	();

struct	_Q8tkWidget {

    int		type;		/* ウィジットの種類	Q8TK_TYPE_	*/
    int		attr;		/* コンテナ属性		Q8TK_ATTR_	*/
    int		visible;	/* 表示の有無				*/
    int		sensitive;	/* 有効・無効				*/

    int		placement_x;	/* 表示位置(天地左右中)	Q8TK_PLACEMENT_	*/
    int		placement_y;

    int		x, y, sx, sy;	/* 表示座標、表示サイズ			*/

    Q8tkWidget	*parent;	/* ウィジット連結構造			*/
    Q8tkWidget	*child;		/*	(表示の時にこのリンクをたどる)	*/
    Q8tkWidget	*prev;
    Q8tkWidget	*next;

    char	key_up_used;	/* カーソルキーの動作が			*/
    char	key_down_used;	/*		予約されている場合、真	*/
    char	key_left_used;
    char	key_right_used;

    char	*name;		/* mallocされた領域のへポインタ		*/
				/*     ・ラベルの文字列			*/
				/*     ・フレームの文字列		*/
				/*     ・ノートページの文字列		*/
				/*     ・エントリの文字列		*/
				/*     ・リストアイテムの情報(文字列)	*/

    int		code;		/* name の文字コード			*/

    int		with_label;	/* XXX_new_with_label()	にて		*/
				/* ラベルを自動生成した場合、真		*/


    union {			/* ウィジット別ワーク			*/

	struct {			/* ---- ウインドウ ---- */
	    int		no_frame;
	    int		shadow_type;
	    int		set_position;
	    int		x, y;
	    int		type;
	    Q8tkWidget	*work;
	    Q8tkWidget	*accel;
	} window;

	struct {			/* ---- フレーム ---- */
	    int		shadow_type;
	} frame;

	struct {			/* ---- ラベル ---- */
	    int		foreground;
	    int		background;
	    int		reverse;
	} label;

	struct {			/* ---- 各種ボタン ---- */
	    int	active;
	    Q8List	*list;
	} button;

	struct {			/* ---- ノートブック ---- */
	    Q8tkWidget	*page;			/* 現在選択中の PAGE	*/
	    struct notebook_draw {		/* 描画時のワーク	*/
		int	drawing;
		int	x, y;
		int	x0, x1;
		int	selected;
	    } draw;
	    int		lost_focus;
	} notebook;

	struct {			/* ---- コンボ ---- */
	    Q8tkWidget	*entry;			/* 配下にあるエントリ	*/
	    Q8List	*list;			/* LIST ITEM のリスト	*/
	    int		nr_items;		/* LIST ITEM の数	*/
	    int		length;			/* LIST ITEM 最大文字長	*/
	    int		width;			/* 表示バイト数		*/
	    Q8tkWidget	*popup_window;		/* POPUPのウインドウ	*/
	    Q8tkWidget	*popup_scrolled_window;	/* POPUPの    〃	*/
	    Q8tkWidget	*popup_list;		/* POPUPのリスト	*/
	    Q8tkWidget	*popup_accel_group;	/* POPUPのESCキー	*/
	    Q8tkWidget	*popup_fake;		/* POPUPのダミーキー	*/
	} combo;

	struct {			/* ---- リストボックス ---- */
	    Q8tkWidget	*selected;
	    Q8tkWidget	*active;
	    int		width;
	    int		scrollin_top;
	    int		scrollin_left;
	} listbox;	/* SELECTION TYPE は BROWSE のみ */

	Q8Adjust	adj;		/* ---- アジャストメント ---- */

	struct {			/* ---- スケール ---- */
	    Q8tkWidget	*adj;
	    int		draw_value;
	    int		value_pos;
	} scale;

	struct {			/* ---- スクロールドウインドウ ---- */
	    Q8tkWidget	*hadj;
	    Q8tkWidget	*vadj;
	    int		hpolicy;
	    int		vpolicy;
	    int		width;
	    int		height;
	    int		hscrollbar;
	    int		vscrollbar;
	    int		child_x0, child_y0;
	    int		child_sx, child_sy;
	    int		vadj_value;
	} scrolled;

	struct {			/* ---- エントリー ---- */
	    int		max_length;		/* 入力可能上限 0で無限	*/
	    int		malloc_length;		/* mallocしたサイズ	*/
	    int		cursor_pos;		/* カーソルバイト位置	*/
	    int		disp_pos;		/* 表示開始バイト位置	*/
	    int		width;			/* 表示するサイズ	*/
	    int		editable;		/* エントリ編集可否	*/
	    Q8tkWidget	*combo;			/* 上位のコンボボックス	*/
	} entry;

	struct {			/* ---- アクセラレーターキー ---- */
	    Q8tkWidget	*widget;
	    int		key;
	} accel;

	struct {			/* ---- ダイアログ ---- */
	    Q8tkWidget	*vbox;
	    Q8tkWidget	*action_area;
	} dialog;

	struct {			/* ---- ファイルセレクション ---- */
	    Q8tkWidget	*file_list;
	    Q8tkWidget	*selection_entry;
	    Q8tkWidget	*ro_button;
	    Q8tkWidget	*ok_button;
	    Q8tkWidget	*cancel_button;
	    Q8tkWidget	*view_button;
	    Q8tkWidget	*dir_name;
	    Q8tkWidget	*nr_files;
	    Q8tkWidget	*scrolled_window;
	    int		selection_changed;
	    char	*pathname;
	    char	*filename;
	    int		width;
	} fselect;

	struct {			/* --- その他・・・ --- */
	    int		data[4];
	} any;

    } stat;

				/* イベント処理関数 */

    void	(*event_button_on)(Q8tkWidget *);
    void	(*event_key_on)   (Q8tkWidget *, int);
    void	(*event_dragging) (Q8tkWidget *);
    void	(*event_drag_off) (Q8tkWidget *);

				/* イベント処理ユーザ関数 */

    void	(*user_event_0)(Q8tkWidget *, void *);
    void	*user_event_0_parm;
    void	(*user_event_1)(Q8tkWidget *, void *);
    void	*user_event_1_parm;

};

enum {				/* (Q8tkWidget*)->type	*/
    Q8TK_TYPE_WINDOW,		/* ウインドウ		*/
    Q8TK_TYPE_BUTTON,		/* ボタン		*/
    Q8TK_TYPE_TOGGLE_BUTTON,	/* トグルボタン		*/
    Q8TK_TYPE_CHECK_BUTTON,	/* チェックボタン	*/
    Q8TK_TYPE_RADIO_BUTTON,	/* ラジオボタン		*/
    Q8TK_TYPE_FRAME,		/* フレーム		*/
    Q8TK_TYPE_LABEL,		/* ラベル		*/
    Q8TK_TYPE_LOGO,		/* ロゴ			*/
    Q8TK_TYPE_NOTEBOOK,		/* ノートブック		*/
    Q8TK_TYPE_NOTEPAGE,		/* ノートブックのページ	*/
    Q8TK_TYPE_VBOX,		/* 縦ボックス		*/
    Q8TK_TYPE_HBOX,		/* 横ボックス		*/
    Q8TK_TYPE_VSEPARATOR,	/* 縦区切り線		*/
    Q8TK_TYPE_HSEPARATOR,	/* 横区切り線		*/
    Q8TK_TYPE_COMBO,		/* コンボボックス	*/
    Q8TK_TYPE_LISTBOX,		/* リスト		*/
    Q8TK_TYPE_LIST_ITEM,	/* リストアイテム	*/
    Q8TK_TYPE_ADJUSTMENT,	/*   アジャストメント	*/
    Q8TK_TYPE_HSCALE,		/* 横スケール		*/
    Q8TK_TYPE_VSCALE,		/* 縦スケール		*/
    Q8TK_TYPE_SCROLLED_WINDOW,	/* スクロールウインドウ	*/
    Q8TK_TYPE_ENTRY,		/* エントリー		*/

    Q8TK_TYPE_ACCEL_GROUP,	/* アクセラレータキー	*/
    Q8TK_TYPE_ACCEL_KEY,	/* 〃			*/

    Q8TK_TYPE_DIALOG,		/* ダイアログ		*/
    Q8TK_TYPE_FILE_SELECTION,	/* ファイルセレクション	*/

    Q8TK_TYPE_END
};
enum {				/* (Q8tkWidget*)->attr	*/
    Q8TK_ATTR_CONTAINER       = (1<<0),		/* コンテナ		*/
    Q8TK_ATTR_LABEL_CONTAINER = (1<<1),		/* コンテナ(LABEL専用)	*/
    Q8TK_ATTR_MENU_CONTAINER  = (1<<2),		/* コンテナ(MENU専用)	*/
    Q8TK_ATTR_END
};
enum {				/* (Q8tkWidget*)->placement_x	*/
    Q8TK_PLACEMENT_X_LEFT,
    Q8TK_PLACEMENT_X_CENTER,
    Q8TK_PLACEMENT_X_RIGHT,
    Q8TK_PLACEMENT_X_END
};
enum {				/* (Q8tkWidget*)->placement_y	*/
    Q8TK_PLACEMENT_Y_TOP,
    Q8TK_PLACEMENT_Y_CENTER,
    Q8TK_PLACEMENT_Y_BOTTOM,
    Q8TK_PLACEMENT_Y_END
};


enum {				/* window_new() の引数	*/
    Q8TK_WINDOW_TOPLEVEL,			/* トップのウインドウ	*/
    Q8TK_WINDOW_DIALOG,
    Q8TK_WINDOW_POPUP,
    Q8TK_WINDOW_END
};
enum {				/* フレームのタイプ */
    Q8TK_SHADOW_NONE,
    Q8TK_SHADOW_IN,
    Q8TK_SHADOW_OUT,
    Q8TK_SHADOW_ETCHED_IN,
    Q8TK_SHADOW_ETCHED_OUT,
    Q8TK_SHADOW_END
};
enum {				/* スクロールウインドウの属性 */
    Q8TK_POLICY_ALWAYS,
    Q8TK_POLICY_AUTOMATIC,
    Q8TK_POLICY_NEVER,
    Q8TK_POLICY_END
};


enum {				/* 汎用位置指定 */
    Q8TK_POS_LEFT,
    Q8TK_POS_RIGHT,
    Q8TK_POS_TOP,
    Q8TK_POS_BOTTOM,
    Q8TK_POS_END
};


enum {				/* 表示可能な漢字コード */
    Q8TK_KANJI_ANK,
    Q8TK_KANJI_EUC,
    Q8TK_KANJI_SJIS,
    Q8TK_KANJI_UTF8,
    Q8TK_KANJI_END
};


/*--------------------------------------------------------------
 * リスト構造
 *--------------------------------------------------------------*/
struct _Q8List {
    void	*data;
    Q8List	*prev;
    Q8List	*next;
};

Q8List		*q8_list_append(Q8List *list, void *data);
Q8List		*q8_list_insert(Q8List *list, void *data, int position);
Q8List		*q8_list_remove(Q8List *list, void *data);
void		q8_list_free(Q8List *list);
Q8List		*q8_list_first(Q8List *list);
Q8List		*q8_list_last(Q8List *list);
Q8List		*q8_list_find(Q8List *list, void *data);



/*--------------------------------------------------------------
 * APIなど
 *--------------------------------------------------------------*/

#define	Q8TKMAX(a, b)		((a)>(b)?(a):(b))


int	q8tk_set_kanjicode(int code);
void	q8tk_set_cursor(int enable);

void	q8tk_init(void);
int	q8tk_main_loop(void);
void	q8tk_main_quit(void);



void	q8tk_grab_add(Q8tkWidget *widget);
void	q8tk_grab_remove(Q8tkWidget *widget);

Q8tkWidget	*q8tk_window_new(int window_type);


/* TOGGLE/CHECK/RADIO BUTTON の active を見るには、必ず下のマクロを通す	*/
/* 例）									*/
/*     Q8tkWidget *toggle = q8tk_tobble_button_new();			*/
/*     if (Q8TK_TOBBLE_BUTTON(toggle)->active) {			*/
/*        :								*/
/*        :								*/
/*     }								*/

#define	Q8TK_TOGGLE_BUTTON(w)	(&((w)->stat.button))

Q8tkWidget	*q8tk_button_new(void);
Q8tkWidget	*q8tk_button_new_with_label(const char *label);

Q8tkWidget	*q8tk_toggle_button_new(void);
Q8tkWidget	*q8tk_toggle_button_new_with_label(const char *label);
void		q8tk_toggle_button_set_state(Q8tkWidget *widget, int status);

Q8tkWidget	*q8tk_check_button_new(void);
Q8tkWidget	*q8tk_check_button_new_with_label(const char *label);

Q8tkWidget	*q8tk_radio_button_new(Q8tkWidget *group);
Q8tkWidget	*q8tk_radio_button_new_with_label(Q8tkWidget *group,
						  const char *label);
Q8List		*q8tk_radio_button_get_list(Q8tkWidget *group);

Q8tkWidget	*q8tk_combo_new(void);
void		q8tk_combo_append_popdown_strings(Q8tkWidget *combo, 
						  const char *entry_str,
						  const char *disp_str);
const	char	*q8tk_combo_get_text(Q8tkWidget *combo);
void		q8tk_combo_set_text(Q8tkWidget *combo, const char *text);
void		q8tk_combo_set_editable(Q8tkWidget *combo, int editable);

Q8tkWidget	*q8tk_listbox_new(void);
void		q8tk_listbox_clear_items(Q8tkWidget *wlist,
					 int start, int end);
void		q8tk_listbox_select_item(Q8tkWidget *wlist, int item);
void		q8tk_listbox_select_child(Q8tkWidget *wlist,
					  Q8tkWidget *child);
void		q8tk_listbox_set_placement(Q8tkWidget *widget,
					   int top_pos, int left_pos);

Q8tkWidget	*q8tk_list_item_new(void);
Q8tkWidget	*q8tk_list_item_new_with_label(const char *label);
void		q8tk_list_item_set_string(Q8tkWidget *w, const char *str);

Q8tkWidget	*q8tk_label_new(const char *label);
void		q8tk_label_set(Q8tkWidget *w, const char *label);
void		q8tk_label_set_reverse(Q8tkWidget *w, int reverse);
void		q8tk_label_set_color(Q8tkWidget *w, int foreground);

Q8tkWidget	*q8tk_logo_new(void);

Q8tkWidget	*q8tk_frame_new(const char *label);
void		q8tk_frame_set_shadow_type(Q8tkWidget *frame, int shadow_type);

Q8tkWidget	*q8tk_hbox_new(void);

Q8tkWidget	*q8tk_vbox_new(void);

Q8tkWidget	*q8tk_notebook_new(void);
void		q8tk_notebook_append(Q8tkWidget *notebook,
				     Q8tkWidget *widget, const char *label);
int		q8tk_notebook_current_page(Q8tkWidget *notebook);
void		q8tk_notebook_set_page(Q8tkWidget *notebook, int page_num);
void		q8tk_notebook_next_page(Q8tkWidget *notebook);
void		q8tk_notebook_prev_page(Q8tkWidget *notebook);
void		q8tk_notebook_hook_focus_lost(Q8tkWidget *notebook, 
					      int focus_lost);

Q8tkWidget	*q8tk_vseparator_new(void);

Q8tkWidget	*q8tk_hseparator_new(void);


/* ADJUSTMENT の value などを見るには、必ず下のマクロを通す		*/
/* 例）									*/
/*     Q8tkWidget *adj = q8tk_adjustment_new();				*/
/*     val = Q8TK_ADJUSTMENT(adj)->value;				*/

#define	Q8TK_ADJUSTMENT(w)	(&((w)->stat.adj))
Q8tkWidget	*q8tk_adjustment_new(int value, int lower, int upper,
				     int step_increment, int page_increment);
void		q8tk_adjustment_clamp_page(Q8tkWidget *adj,
					   int lower, int upper);
void		q8tk_adjustment_set_value(Q8tkWidget *adj, int value);
void		q8tk_adjustment_set_arrow(Q8tkWidget *adj, int arrow);
void		q8tk_adjustment_set_length(Q8tkWidget *adj, int length);
void		q8tk_adjustment_set_increment(Q8tkWidget *adj,
					      int step_increment,
					      int page_increment);

Q8tkWidget	*q8tk_hscale_new(Q8tkWidget *adjustment);
Q8tkWidget	*q8tk_vscale_new(Q8tkWidget *adjustment);
void		q8tk_scale_set_value_pos(Q8tkWidget *scale, int pos);
void		q8tk_scale_set_draw_value(Q8tkWidget *scale, int draw_value);

Q8tkWidget	*q8tk_scrolled_window_new(Q8tkWidget *hadjustment,
					  Q8tkWidget *vadjustment);
void		q8tk_scrolled_window_set_policy(Q8tkWidget *scrolledw,
						int hscrollbar_policy,
						int vscrollbar_policy);

Q8tkWidget	*q8tk_entry_new(void);
Q8tkWidget	*q8tk_entry_new_with_max_length(int max);
const	char	*q8tk_entry_get_text(Q8tkWidget *entry);
void		q8tk_entry_set_text(Q8tkWidget *entry, const char *text);
void		q8tk_entry_set_position(Q8tkWidget *entry, int position);
void		q8tk_entry_set_max_length(Q8tkWidget *entry, int max);
void		q8tk_entry_set_editable(Q8tkWidget *entry, int editable);

Q8tkWidget	*q8tk_accel_group_new(void);
void		q8tk_accel_group_attach(Q8tkWidget *accel_group,
					Q8tkWidget *window);
void		q8tk_accel_group_detach(Q8tkWidget *accel_group,
					Q8tkWidget *window);
void		q8tk_accel_group_add(Q8tkWidget *accel_group, int accel_key,
				     Q8tkWidget *widget, const char *signal);


/* DIALOG の vhox, action_area を見るには、必ず下のマクロを通す		*/
/* 例）									*/
/*     Q8tkWidget *dialog = q8tk_dialog_new();				*/
/*     q8tk_box_pack_start(Q8TK_DIALOGE(dialog)->vbox, button);		*/

#define	Q8TK_DIALOG(w)		(&((w)->stat.window.work->stat.dialog))
Q8tkWidget	*q8tk_dialog_new(void);


/* FILE SELECTION の ok_button などを見るには、必ず下のマクロを通す	*/
/* 例）									*/
/*     Q8tkWidget *fselect = q8tk_file_selection_new("LOAD", FALSE);	*/
/*     q8tk_signal_connect(Q8TK_FILE_SELECTION(fselect)->ok_button,	*/
/*			    func, fselect);				*/

#define	Q8TK_FILE_SELECTION(w)	(&((w)->stat.window.work->stat.fselect))


/* FILE_SELECTION で扱えるパス込みのファイル名バイト数 (NUL含む)	*/

#define	Q8TK_MAX_FILENAME	(QUASI88_MAX_FILENAME)
/* #define	Q8TK_MAX_FILENAME	(OSD_MAX_FILENAME) */


Q8tkWidget	*q8tk_file_selection_new(const char *title, int select_ro);
const	char	*q8tk_file_selection_get_filename(Q8tkWidget *fselect);
void		q8tk_file_selection_set_filename(Q8tkWidget *fselect,
						 const char *filename);
int		q8tk_file_selection_get_readonly(Q8tkWidget *fselect);

int		q8tk_utility_view(const char *filename);




void	q8tk_misc_set_placement(Q8tkWidget *widget,
				int placement_x, int placement_y);
void	q8tk_misc_set_size(Q8tkWidget *widget, int width, int height);

void	q8tk_misc_redraw(void);


void	q8tk_container_add(Q8tkWidget *container, Q8tkWidget *widget);
void	q8tk_box_pack_start(Q8tkWidget *box, Q8tkWidget *widget);
void	q8tk_box_pack_end(Q8tkWidget *box, Q8tkWidget *widget);
void	q8tk_container_remove(Q8tkWidget *container, Q8tkWidget *widget);

void	q8tk_widget_show(Q8tkWidget *widget);
void	q8tk_widget_hide(Q8tkWidget *widget);

void	q8tk_widget_set_sensitive(Q8tkWidget *widget, int sensitive);

void	q8tk_widget_destroy(Q8tkWidget *widget);

void	q8tk_widget_set_focus(Q8tkWidget *widget);


/* ↓ 返り値は、無効 (必ず 0) */
int	q8tk_signal_connect(Q8tkWidget *widget, const char *name,
			    Q8tkSignalFunc func, void *func_data);
void	q8tk_signal_handlers_destroy(Q8tkWidget *widget);





#endif	/* Q8TK_H_INCLUDED */
