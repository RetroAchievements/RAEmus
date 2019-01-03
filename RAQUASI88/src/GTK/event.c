/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/

#include <gdk/gdkkeysyms.h>

#include "quasi88.h"
#include "getconf.h"
#include "keyboard.h"

#include "event.h"



int	gtksys_get_focus;		/* 現在、フォーカスありかどうか	*/


/*==========================================================================
 * キー配列について
 *
 *  とりあえず、106キーボード決め打ちで作成
 *===========================================================================*/

/* ソフトウェアNumLock をオンした際の、キーバインディング変更テーブル */

typedef struct {
    int		type;		/* KEYCODE_INVALID / SYM / SCAN		*/
    int		code;		/* キーシンボル、ないし、スキャンコード	*/
    int		new_key88;	/* NumLock ON 時の QUASI88キーコード	*/
    int		org_key88;	/* NumLock OFF時の QUASI88キーコード	*/
} T_BINDING;


/* キーバインディングをデフォルト(初期値)から変更する際の、テーブル */

typedef struct {
    int		type;		/* KEYCODE_INVALID / SYM / SCAN		*/
    int		code;		/* キーシンボル、ないし、スキャンコード	*/
    int		key88;		/* 変更する QUASI88キーコード           */
} T_REMAPPING;


/* GDK の keysym を 0〜511 の範囲に丸める */

#define	LOCAL_KEYSYM(ks)						    \
	((((ks) & 0xff00) == 0xff00)			/* 機能キー */	    \
		? (((ks) & 0x00ff) | 0x100)				    \
		: ((((ks) & 0xff00) == 0x0000)		/* 文字キー */	    \
					? (ks) : 0))

/*----------------------------------------------------------------------
 * GDK の keyval を QUASI88 の キーコードに変換するテーブル
 *	文字キー と 機能キー のキーシンボルのみ。
 *
 *	キーシンボル GDK_xxx が押されたら、 
 *	keysym2key88[ GDK_xxx ] が押されたとする。
 *
 *	keysym2key88[] には、 KEY88_xxx をセットしておく。
 *	初期値は keysym2key88_default[] と同じ
 *----------------------------------------------------------------------*/
static int keysym2key88[ 256 + 256 ];

 

/*----------------------------------------------------------------------
 * ソフトウェア NumLock オン時の キーコード変換情報
 *
 *	binding[].code (GDK の keyval) が押されたら、
 *	binding[].new_key88 (KEY88_xxx) が押されたことにする。
 *
 *	ソフトウェア NumLock オン時は、この情報にしたがって、
 *	keysym2key88[] 、 keycode2key88[] を書き換える。
 *	変更できるキーの個数は、64個まで (これだけあればいいだろう)
 *----------------------------------------------------------------------*/
static T_BINDING binding[ 64 ];





/*----------------------------------------------------------------------
 * GDK_xxx → KEY88_xxx 変換テーブル (デフォルト)
 *----------------------------------------------------------------------*/

static const int keysym2key88_default[ 256 + 256 ] =
{
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /* 文字キー						0x0000〜0x00FF	*/
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  KEY88_SPACE,		/*	GDK_space               0x020	*/
  KEY88_EXCLAM,		/*	GDK_exclam              0x021	*/
  KEY88_QUOTEDBL,	/*	GDK_quotedbl            0x022	*/
  KEY88_NUMBERSIGN,	/*	GDK_numbersign          0x023	*/
  KEY88_DOLLAR,		/*	GDK_dollar              0x024	*/
  KEY88_PERCENT,	/*	GDK_percent             0x025	*/
  KEY88_AMPERSAND,	/*	GDK_ampersand           0x026	*/
  KEY88_APOSTROPHE,	/*	GDK_apostrophe          0x027	*/
  KEY88_PARENLEFT,	/*	GDK_parenleft           0x028	*/
  KEY88_PARENRIGHT,	/*	GDK_parenright          0x029	*/
  KEY88_ASTERISK,	/*	GDK_asterisk            0x02a	*/
  KEY88_PLUS,		/*	GDK_plus                0x02b	*/
  KEY88_COMMA,		/*	GDK_comma               0x02c	*/
  KEY88_MINUS,		/*	GDK_minus               0x02d	*/
  KEY88_PERIOD,		/*	GDK_period              0x02e	*/
  KEY88_SLASH,		/*	GDK_slash               0x02f	*/

  KEY88_0,		/*	GDK_0                   0x030	*/
  KEY88_1,		/*	GDK_1                   0x031	*/
  KEY88_2,		/*	GDK_2                   0x032	*/
  KEY88_3,		/*	GDK_3                   0x033	*/
  KEY88_4,		/*	GDK_4                   0x034	*/
  KEY88_5,		/*	GDK_5                   0x035	*/
  KEY88_6,		/*	GDK_6                   0x036	*/
  KEY88_7,		/*	GDK_7                   0x037	*/
  KEY88_8,		/*	GDK_8                   0x038	*/
  KEY88_9,		/*	GDK_9                   0x039	*/
  KEY88_COLON,		/*	GDK_colon               0x03a	*/
  KEY88_SEMICOLON,	/*	GDK_semicolon           0x03b	*/
  KEY88_LESS,		/*	GDK_less                0x03c	*/
  KEY88_EQUAL,		/*	GDK_equal               0x03d	*/
  KEY88_GREATER,	/*	GDK_greater             0x03e	*/
  KEY88_QUESTION,	/*	GDK_question            0x03f	*/

  KEY88_AT,		/*	GDK_at                  0x040	*/
  KEY88_A,		/*	GDK_A                   0x041	*/
  KEY88_B,		/*	GDK_B                   0x042	*/
  KEY88_C,		/*	GDK_C                   0x043	*/
  KEY88_D,		/*	GDK_D                   0x044	*/
  KEY88_E,		/*	GDK_E                   0x045	*/
  KEY88_F,		/*	GDK_F                   0x046	*/
  KEY88_G,		/*	GDK_G                   0x047	*/
  KEY88_H,		/*	GDK_H                   0x048	*/
  KEY88_I,		/*	GDK_I                   0x049	*/
  KEY88_J,		/*	GDK_J                   0x04a	*/
  KEY88_K,		/*	GDK_K                   0x04b	*/
  KEY88_L,		/*	GDK_L                   0x04c	*/
  KEY88_M,		/*	GDK_M                   0x04d	*/
  KEY88_N,		/*	GDK_N                   0x04e	*/
  KEY88_O,		/*	GDK_O                   0x04f	*/

  KEY88_P,		/*	GDK_P                   0x050	*/
  KEY88_Q,		/*	GDK_Q                   0x051	*/
  KEY88_R,		/*	GDK_R                   0x052	*/
  KEY88_S,		/*	GDK_S                   0x053	*/
  KEY88_T,		/*	GDK_T                   0x054	*/
  KEY88_U,		/*	GDK_U                   0x055	*/
  KEY88_V,		/*	GDK_V                   0x056	*/
  KEY88_W,		/*	GDK_W                   0x057	*/
  KEY88_X,		/*	GDK_X                   0x058	*/
  KEY88_Y,		/*	GDK_Y                   0x059	*/
  KEY88_Z,		/*	GDK_Z                   0x05a	*/
  KEY88_BRACKETLEFT,	/*	GDK_bracketleft         0x05b	*/
  KEY88_YEN,		/*	GDK_backslash           0x05c	*/
  KEY88_BRACKETRIGHT,	/*	GDK_bracketright        0x05d	*/
  KEY88_CARET,		/*	GDK_asciicircum         0x05e	*/
  KEY88_UNDERSCORE,	/*	GDK_underscore          0x05f	*/

  KEY88_BACKQUOTE,	/*	GDK_grave               0x060	*/
  KEY88_a,		/*	GDK_a                   0x061	*/
  KEY88_b,		/*	GDK_b                   0x062	*/
  KEY88_c,		/*	GDK_c                   0x063	*/
  KEY88_d,		/*	GDK_d                   0x064	*/
  KEY88_e,		/*	GDK_e                   0x065	*/
  KEY88_f,		/*	GDK_f                   0x066	*/
  KEY88_g,		/*	GDK_g                   0x067	*/
  KEY88_h,		/*	GDK_h                   0x068	*/
  KEY88_i,		/*	GDK_i                   0x069	*/
  KEY88_j,		/*	GDK_j                   0x06a	*/
  KEY88_k,		/*	GDK_k                   0x06b	*/
  KEY88_l,		/*	GDK_l                   0x06c	*/
  KEY88_m,		/*	GDK_m                   0x06d	*/
  KEY88_n,		/*	GDK_n                   0x06e	*/
  KEY88_o,		/*	GDK_o                   0x06f	*/

  KEY88_p,		/*	GDK_p                   0x070	*/
  KEY88_q,		/*	GDK_q                   0x071	*/
  KEY88_r,		/*	GDK_r                   0x072	*/
  KEY88_s,		/*	GDK_s                   0x073	*/
  KEY88_t,		/*	GDK_t                   0x074	*/
  KEY88_u,		/*	GDK_u                   0x075	*/
  KEY88_v,		/*	GDK_v                   0x076	*/
  KEY88_w,		/*	GDK_w                   0x077	*/
  KEY88_x,		/*	GDK_x                   0x078	*/
  KEY88_y,		/*	GDK_y                   0x079	*/
  KEY88_z,		/*	GDK_z                   0x07a	*/
  KEY88_BRACELEFT,	/*	GDK_braceleft           0x07b	*/
  KEY88_BAR,		/*	GDK_bar                 0x07c	*/
  KEY88_BRACERIGHT,	/*	GDK_braceright          0x07d	*/
  KEY88_TILDE,		/*	GDK_asciitilde          0x07e	*/
  0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,


  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /* 機能キー						0xFF00〜0xFFFF	*/
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_BS,		/*	GDK_BackSpace		0xFF08	   */
  KEY88_TAB,		/*	GDK_Tab			0xFF09	   */
  KEY88_RETURN,		/*	GDK_Linefeed		0xFF0A	無 */
  KEY88_HOME,		/*	GDK_Clear		0xFF0B	無 */
  0,
  KEY88_RETURNL,	/*	GDK_Return		0xFF0D	   */
  0, 0,

  0, 0, 0,
  KEY88_STOP,		/*	GDK_Pause		0xFF13	   */
  KEY88_KANA,		/*	GDK_Scroll_Lock		0xFF14	   */
  KEY88_COPY,		/*	GDK_Sys_Req		0xFF15	   */
  0, 0,
  0, 0, 0,
  KEY88_ESC, 		/*	GDK_Escape		0xFF1B	   */
  0, 0, 0, 0,

  0,			/*	GDK_Multi_key		0xFF20	無 */
  0,			/*	GDK_Kanji		0xFF21	   */
  KEY88_KETTEI,		/*	GDK_Muhenkan		0xFF22	   */
  KEY88_HENKAN,		/*	GDK_Henkan_Mode		0xFF23	   */
  KEY88_KANA,		/*	GDK_Romaji		0xFF24     */
  0,			/*	GDK_Hiragana		0xFF25  無 */
  0,			/*	GDK_Katakana		0xFF26  無 */
  KEY88_KANA,		/*	GDK_Hiragana_Katakana	0xFF27	   */
  0,			/*	GDK_Zenkaku		0xFF28  無 */
  0,			/*	GDK_Hankaku		0xFF29  無 */
  KEY88_ZENKAKU,	/*	GDK_Zenkaku_Hankaku	0xFF2A	   */
  0,			/*	GDK_Touroku		0xFF2B  無 */
  0,			/*	GDK_Massyo		0xFF2C  無 */
  KEY88_KANA,		/*	GDK_Kana_Lock		0xFF2D	無 */
  KEY88_KANA,		/*	GDK_Kana_Shift		0xFF2E  無 */
  0,			/*	GDK_Eisu_Shift		0xFF2F  無 */

  KEY88_CAPS,		/*	GDK_Eisu_toggle		0xFF30     */
  0, 0, 0, 0, 0, 0,
  0,			/*	GDK_Kanji_Bangou	0xFF37  無 */
  0, 0, 0, 0,
  0,			/*	GDK_SingleCandidate	0xFF3C	無 */
  0,			/*	GDK_Zen_Koho		0xFF3D	無 */
  0,			/*	GDK_Mae_Koho		0xFF3E	無 */
  0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  KEY88_HOME,		/*	GDK_Home		0xFF50	   */
  KEY88_LEFT,		/*	GDK_Left		0xFF51	   */
  KEY88_UP,		/*	GDK_Up			0xFF52	   */
  KEY88_RIGHT,		/*	GDK_Right		0xFF53	   */
  KEY88_DOWN,		/*	GDK_Down		0xFF54	   */
  KEY88_ROLLDOWN,	/*	GDK_Prior		0xFF55	   */
  KEY88_ROLLUP,		/*	GDK_Next		0xFF56	   */
  KEY88_HELP,		/*	GDK_End			0xFF57	   */
  0,			/*	GDK_Begin		0xFF58	無 */
  0, 0, 0, 0, 0, 0, 0,

  0,			/*	GDK_Select		0xFF60	無 */
  KEY88_COPY,		/*	GDK_Print		0xFF61	   */
  KEY88_COPY,		/*	GDK_Execute		0xFF62	   */
  KEY88_INS, 		/*	GDK_Insert		0xFF63	   */
  0,
  0,			/*	GDK_Undo		0xFF65	無 */
  0,			/*	GDK_Redo		0xFF66	無 */
  0,			/*	GDK_Menu		0xFF67	   */
  0,			/*	GDK_Find		0xFF68	無 */
  0,			/*	GDK_Cancel		0xFF69	無 */
  KEY88_HELP,		/*	GDK_Help		0xFF6A	無 */
  KEY88_STOP,		/*	GDK_Break		0xFF6B	   */
  0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_HENKAN,		/*	GDK_Mode_switch		0xFF7E	   */
  0,			/*	GDK_Num_Lock		0xFF7F	   */

  KEY88_SPACE,		/*	GDK_KP_Space		0xFF80	無 */
  0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_TAB,		/*	GDK_KP_Tab		0xFF89	無 */
  0, 0, 0,
  KEY88_RETURNR,	/*	GDK_KP_Enter		0xFF8D	   */
  0, 0,

  0,
  KEY88_F1,		/*	GDK_KP_F1		0xFF91	無 */
  KEY88_F2,		/*	GDK_KP_F2		0xFF92	無 */
  KEY88_F3,		/*	GDK_KP_F3		0xFF93	無 */
  KEY88_F4,		/*	GDK_KP_F4		0xFF94	無 */
  KEY88_KP_7,		/*	GDK_KP_Home		0xFF95	   */
  KEY88_KP_4,		/*	GDK_KP_Left		0xFF96	   */
  KEY88_KP_8,		/*	GDK_KP_Up		0xFF97	   */
  KEY88_KP_6,		/*	GDK_KP_Right		0xFF98	   */
  KEY88_KP_2,		/*	GDK_KP_Down		0xFF99	   */
  KEY88_KP_9,		/*	GDK_KP_Page_Up		0xFF9A	   */
  KEY88_KP_3,		/*	GDK_KP_Page_Down	0xFF9B	   */
  KEY88_KP_1,		/*	GDK_KP_End		0xFF9C	   */
  KEY88_KP_5,		/*	GDK_KP_Begin		0xFF9D	   */
  KEY88_KP_0,		/*	GDK_KP_Insert		0xFF9E	   */
  KEY88_KP_PERIOD,	/*	GDK_KP_Delete		0xFF9F	   */

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_KP_MULTIPLY,	/*	GDK_KP_Multiply		0xFFAA	   */
  KEY88_KP_ADD,		/*	GDK_KP_Add		0xFFAB	   */
  KEY88_KP_COMMA,	/*	GDK_KP_Separator	0xFFAC	無 */
  KEY88_KP_SUB,		/*	GDK_KP_Subtract		0xFFAD	   */
  KEY88_KP_PERIOD,	/*	GDK_KP_Decimal		0xFFAE	   */
  KEY88_KP_DIVIDE,	/*	GDK_KP_Divide		0xFFAF	   */

  KEY88_KP_0,		/*	GDK_KP_0		0xFFB0	   */
  KEY88_KP_1,		/*	GDK_KP_1		0xFFB1	   */
  KEY88_KP_2,		/*	GDK_KP_2		0xFFB2	   */
  KEY88_KP_3,		/*	GDK_KP_3		0xFFB3	   */
  KEY88_KP_4,		/*	GDK_KP_4		0xFFB4	   */
  KEY88_KP_5,		/*	GDK_KP_5		0xFFB5	   */
  KEY88_KP_6,		/*	GDK_KP_6		0xFFB6	   */
  KEY88_KP_7,		/*	GDK_KP_7		0xFFB7	   */
  KEY88_KP_8,		/*	GDK_KP_8		0xFFB8	   */
  KEY88_KP_9,		/*	GDK_KP_9		0xFFB9	   */
  0, 0, 0,
  KEY88_KP_EQUAL,	/*	GDK_KP_Equal		0xFFBD	無 */
  KEY88_F1,		/*	GDK_F1			0xFFBE	   */
  KEY88_F2,		/*	GDK_F2			0xFFBF	   */

  KEY88_F3,		/*	GDK_F3			0xFFC0	   */
  KEY88_F4,		/*	GDK_F4			0xFFC1	   */
  KEY88_F5,		/*	GDK_F5			0xFFC2	   */
  KEY88_F6,		/*	GDK_F6			0xFFC3	   */
  KEY88_F7,		/*	GDK_F7			0xFFC4	   */
  KEY88_F8,		/*	GDK_F8			0xFFC5	   */
  KEY88_F9,		/*	GDK_F9			0xFFC6	   */
  KEY88_F10,		/*	GDK_F10			0xFFC7	   */
  KEY88_F11,		/*	GDK_F11			0xFFC8	   */
  KEY88_F12,		/*	GDK_F12			0xFFC9	   */
  KEY88_F13,		/*	GDK_F13			0xFFCA	無 */
  KEY88_F14,		/*	GDK_F14			0xFFCB	無 */
  KEY88_F15,		/*	GDK_F15			0xFFCC	無 */
  KEY88_F16,		/*	GDK_F16			0xFFCD	無 */
  KEY88_F17,		/*	GDK_F17			0xFFCE	無 */
  KEY88_F18,		/*	GDK_F18			0xFFCF	無 */

  KEY88_F19,		/*	GDK_F19			0xFFD0	無 */
  KEY88_F20,		/*	GDK_F20			0xFFD1	無 */
  0,			/*	GDK_F21			0xFFD2	無 */
  0,			/*	GDK_F22			0xFFD3	無 */
  0,			/*	GDK_F23			0xFFD4	無 */
  0,			/*	GDK_F24			0xFFD5	無 */
  0,			/*	GDK_F25			0xFFD6	無 */
  0,			/*	GDK_F26			0xFFD7	無 */
  0,			/*	GDK_F27			0xFFD8	無 */
  0,			/*	GDK_F28			0xFFD9	無 */
  0,			/*	GDK_F29			0xFFDA	無 */
  0,			/*	GDK_F30			0xFFDB	無 */
  0,			/*	GDK_F31			0xFFDC	無 */
  0,			/*	GDK_F32			0xFFDD	無 */
  0,			/*	GDK_F33			0xFFDE	無 */
  0,			/*	GDK_F34			0xFFDF	無 */

  0,			/*	GDK_F35			0xFFE0	無 */
  KEY88_SHIFTL,		/*	GDK_Shift_L		0xFFE1	   */
  KEY88_SHIFTR,		/*	GDK_Shift_R		0xFFE2	   */
  KEY88_CTRL,		/*	GDK_Control_L		0xFFE3	   */
  KEY88_CTRL,		/*	GDK_Control_R		0xFFE4	   */
  KEY88_CAPS,		/*	GDK_Caps_Lock		0xFFE5	   */
  KEY88_CAPS,		/*	GDK_Shift_Lock		0xFFE6	無 */
  KEY88_GRAPH,		/*	GDK_Meta_L		0xFFE7	   */
  KEY88_GRAPH,		/*	GDK_Meta_R		0xFFE8	   */
  KEY88_GRAPH,		/*	GDK_Alt_L		0xFFE9	   */
  KEY88_GRAPH,		/*	GDK_Alt_R		0xFFEA	   */
  0,			/*	GDK_Super_L		0xFFEB	   */
  0,			/*	GDK_Super_R		0xFFEC	   */
  0,			/*	GDK_Hyper_L		0xFFED	無 */
  0,			/*	GDK_Hyper_R		0xFFEE	無 */
  0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_DEL,		/*	GDK_Delete		0xFFFF	   */

};



/*----------------------------------------------------------------------
 * ソフトウェア NumLock オン時の キーコード変換情報 (デフォルト)
 *----------------------------------------------------------------------*/

static const T_BINDING binding_default[] =
{
    {	KEYCODE_SYM,	GDK_5,		KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	GDK_6,		KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	GDK_7,		KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	GDK_8,		KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	GDK_9,		KEY88_KP_9,		0,	},
/*  {	KEYCODE_SCAN,	?,		KEY88_KP_MULTIPLY,	0,	},*/
    {	KEYCODE_SYM,	GDK_minus,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	GDK_asciicircum,KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	GDK_u,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	GDK_i,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	GDK_o,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	GDK_p,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	GDK_j,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	GDK_k,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	GDK_l,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	GDK_semicolon,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	GDK_m,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	GDK_comma,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	GDK_period,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	GDK_slash,	KEY88_RETURNR,		0,	},

    {	KEYCODE_SYM,	GDK_percent,	KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	GDK_ampersand,	KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	GDK_apostrophe,	KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	GDK_parenleft,	KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	GDK_parenright,	KEY88_KP_9,		0,	},
    {	KEYCODE_SYM,	GDK_equal,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	GDK_asciitilde,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	GDK_U,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	GDK_I,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	GDK_O,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	GDK_P,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	GDK_J,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	GDK_K,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	GDK_L,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	GDK_plus,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	GDK_M,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	GDK_less,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	GDK_greater,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	GDK_question,	KEY88_RETURNR,		0,	},

    {	KEYCODE_INVALID,0,		0,			0,	},
};



/******************************************************************************
 * イベントハンドリング
 *
 *	1/60毎に呼び出される。
 *****************************************************************************/

/*
 * これは 起動時に1回だけ呼ばれる
 */
void	event_init(void)
{
    const T_REMAPPING *map;
    const T_BINDING   *bin;
    int i;

    /* キーマッピング初期化 */

    memset(keysym2key88, 0, sizeof(keysym2key88));
    memset(binding, 0, sizeof(binding));

    memcpy(keysym2key88,
	   keysym2key88_default, sizeof(keysym2key88_default));
    memcpy(binding,
	   binding_default, sizeof(binding_default));


    /* ソフトウェアNumLock 時のキー差し替えの準備 */

    for (i = 0; i < COUNTOF(binding); i++) {

	if        (binding[i].type == KEYCODE_SYM) {

	    binding[i].org_key88 = keysym2key88[LOCAL_KEYSYM(binding[i].code)];

	} else {
	    break;
	}
    }

}



/*
 * 約 1/60 毎に呼ばれる
 */
void	event_update(void)
{
}

/*
 * これは 終了時に1回だけ呼ばれる
 */
void	event_exit(void)
{
}




/***********************************************************************
 * 現在のマウス座標取得関数
 *
 ************************************************************************/

void	event_get_mouse_pos(int *x, int *y)
{
#if 0
    gint xx, yy;

    gtk_widget_get_pointer(drawing_area, &xx, &yy);

    *x = xx;
    *y = yy;

#elif 0

    gint xx, yy;
    GdkModifierType mask;

    gdk_window_get_pointer(drawing_area->window,
			   &xx, &yy, &mask);

    *x = xx;
    *y = yy;

#else

    *x = 0;
    *y = 0;

#endif
}


/******************************************************************************
 * ソフトウェア NumLock 有効／無効
 *
 *****************************************************************************/

static	void	numlock_setup(int enable)
{
    int i;
    int keysym;

    for (i = 0; i < COUNTOF(binding); i++) {

	if        (binding[i].type == KEYCODE_SYM) {

	    keysym = LOCAL_KEYSYM(binding[i].code);

	    if (enable) {
		keysym2key88[ keysym ] = binding[i].new_key88;
	    } else {
		keysym2key88[ keysym ] = binding[i].org_key88;
	    }

	} else {
	    break;
	}
    }
}

int	event_numlock_on (void){ numlock_setup(TRUE);  return TRUE; }
void	event_numlock_off(void){ numlock_setup(FALSE); }



/******************************************************************************
 * エミュレート／メニュー／ポーズ／モニターモード の 開始時の処理
 *
 *****************************************************************************/

void	event_switch(void)
{
    if (quasi88_is_exec()) {		/*===================================*/

	menubar_setup(TRUE);

    } else {

	menubar_setup(FALSE);
    }
}



/******************************************************************************
 * ジョイスティック
 *
 *****************************************************************************/

int	event_get_joystick_num(void)
{
    return 0;
}


/*------------------------------------------------------------------------*/

#define		ENABLE_MOTION_EVENT

static gboolean destroy_event(GtkWidget *, gpointer);
static gboolean focusin_event(GtkWidget *, GdkEventFocus *, gpointer);
static gboolean focusout_event(GtkWidget *, GdkEventFocus *, gpointer);
static gboolean expose_event(GtkWidget *, GdkEventExpose *, gpointer);
static gboolean keypress_event(GtkWidget *, GdkEventKey *, gpointer);
static gboolean keyrelease_event(GtkWidget *, GdkEventKey *, gpointer);
static gboolean buttonpress_event(GtkWidget *, GdkEventButton *, gpointer);
static gboolean buttonrelease_event(GtkWidget *, GdkEventButton *, gpointer);
static gboolean motionnotify_event(GtkWidget *, GdkEventMotion *, gpointer);


/* メインウインドウのシグナルを設定 */
void	gtksys_set_signal_frame(GtkWidget *widget)
{
    gtk_signal_connect(GTK_OBJECT(widget), "destroy",
		       GTK_SIGNAL_FUNC(destroy_event), NULL);

    gtk_signal_connect(GTK_OBJECT(widget), "focus_in_event",
		       GTK_SIGNAL_FUNC(focusin_event), NULL);

    gtk_signal_connect(GTK_OBJECT(widget), "focus_out_event",
		       GTK_SIGNAL_FUNC(focusout_event), NULL);

    gtk_signal_connect(GTK_OBJECT(widget), "key_press_event",
		       GTK_SIGNAL_FUNC(keypress_event), NULL);

    gtk_signal_connect(GTK_OBJECT(widget), "key_release_event",
		       GTK_SIGNAL_FUNC(keyrelease_event), NULL);

    gtk_widget_set_events(widget,
			  gtk_widget_get_events(widget) |
			  GDK_KEY_PRESS_MASK |
			  GDK_KEY_RELEASE_MASK);
}

/* ドローイングエリアのシグナルを設定 */
void	gtksys_set_signal_view(GtkWidget *widget)
{
    gtk_signal_connect(GTK_OBJECT(widget), "expose_event",
		       GTK_SIGNAL_FUNC(expose_event), NULL);

    gtk_signal_connect(GTK_OBJECT(widget), "button_press_event",
		       GTK_SIGNAL_FUNC(buttonpress_event), NULL);

    gtk_signal_connect(GTK_OBJECT(widget), "button_release_event",
		       GTK_SIGNAL_FUNC(buttonrelease_event), NULL);

#ifdef	ENABLE_MOTION_EVENT
    gtk_signal_connect(GTK_OBJECT(widget), "motion_notify_event",
		       GTK_SIGNAL_FUNC(motionnotify_event), NULL);
#endif

    gtk_widget_set_events(widget,
			  gtk_widget_get_events(widget) |
#ifdef	ENABLE_MOTION_EVENT
			  GDK_POINTER_MOTION_MASK |
#endif
			  GDK_EXPOSURE_MASK |
			  GDK_KEY_PRESS_MASK |
			  GDK_KEY_RELEASE_MASK |
			  GDK_BUTTON_PRESS_MASK |
			  GDK_BUTTON_RELEASE_MASK);
}


static gboolean destroy_event(GtkWidget *widget, gpointer data)
{
    quasi88_quit();
    return TRUE;		/* TRUE … イベント処理はこれで終わり */
				/* FALSE … イベントを次に伝播させる */
}

static gboolean focusin_event(GtkWidget *widget, GdkEventFocus *event,
			      gpointer data)
{
    quasi88_focus_in();

    gtksys_get_focus = TRUE;
    gtksys_set_attribute_focus_in();

    return TRUE;
}

static gboolean focusout_event(GtkWidget *widget, GdkEventFocus *event,
			       gpointer data)
{
    quasi88_focus_out();

    gtksys_get_focus = FALSE;
    gtksys_set_attribute_focus_out();

    return TRUE;
}

static gboolean expose_event(GtkWidget *widget, GdkEventExpose *event,
			     gpointer data)
{
    quasi88_expose();
    return TRUE;
}

static void key_event(guint keysym, int is_pressed)
{
    int    key88;

    if (quasi88_is_exec()) {

	if ((keysym & 0xff00) == 0xff00) {		/* 機能キー */

	    key88 = keysym2key88[ (keysym & 0x00ff) | 0x100 ];

	} else if ((keysym & 0xff00) == 0x0000) {	/* 文字キー */

	    key88 = keysym2key88[ (keysym) ];

	} else if (keysym == GDK_ISO_Left_Tab) {	/* 0xFE20 */

	    key88 = KEY88_TAB;

	} else {					/* else... */

	    key88 = 0;

	}

    } else {

	if ((keysym & 0xff00) == 0xff00) {		/* 機能キー */

	    key88 = keysym2key88[ (keysym & 0x00ff) | 0x100 ];

	} else if ((keysym & 0xff00) == 0x0000) {	/* 文字キー */

	    key88 = keysym;

	} else if (keysym == GDK_ISO_Left_Tab) {	/* 0xFE20 */

	    key88 = KEY88_TAB;

	} else {					/* else... */

	    key88 = 0;

	}

    }

    if (key88) {
	quasi88_key(key88, (is_pressed));
    }
}

static gboolean keypress_event(GtkWidget *widget, GdkEventKey *event,
			       gpointer data)
{
    key_event(event->keyval, TRUE);
    return FALSE;
}

static gboolean keyrelease_event(GtkWidget *widget, GdkEventKey *event,
				 gpointer data)
{
    key_event(event->keyval, FALSE);
    return FALSE;
}

static void button_event(guint button, int x, int y, int is_pressed)
{
    int key88;

    if      (button == 1) key88 = KEY88_MOUSE_L;
    else if (button == 3) key88 = KEY88_MOUSE_R;
    else                  key88 = 0;

    if (key88) {
#ifndef	ENABLE_MOTION_EVENT
	quasi88_mouse_moved_abs(x, y);
#endif
	quasi88_mouse(key88, (is_pressed));
    }
}
static gboolean buttonpress_event(GtkWidget *widget, GdkEventButton *event,
				  gpointer data)
{
    button_event(event->button, (int)event->x, (int)event->y, TRUE);
    return FALSE;
}

static gboolean buttonrelease_event(GtkWidget *widget, GdkEventButton *event,
				    gpointer data)
{
    button_event(event->button, (int)event->x, (int)event->y, FALSE);
    return FALSE;
}

#ifdef	ENABLE_MOTION_EVENT
static gboolean motionnotify_event(GtkWidget *widget, GdkEventMotion *event,
				   gpointer data)
{
    int x, y;

    x = (int)(event->x);
    y = (int)(event->y);

    quasi88_mouse_moved_abs(x, y);

    return FALSE;
}
#endif
