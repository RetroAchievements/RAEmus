/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/

/* ----------------------------------------------------------------------
 *		ヘッダファイル部
 * ---------------------------------------------------------------------- */

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "quasi88.h"
#include "getconf.h"
#include "keyboard.h"
#include "joystick.h"

#include "drive.h"

#include "emu.h"
#include "device.h"
#include "status.h"
#include "screen.h"
#include "event.h"
#include "file-op.h"



#ifdef	XDEEP32
int	x11_get_focus = TRUE;
#else
int	x11_get_focus;			/* 現在、フォーカスありかどうか	*/
#endif

int	x11_scaling = FALSE;		/* マウス座標のスケーリング有無	*/
int	x11_scale_x_num = 0;		/* 以下、その際のパラメータ	*/
int	x11_scale_x_den = 1;
int	x11_scale_y_num = 0;
int	x11_scale_y_den = 1;



int	keyboard_type = 1;		/* キーボードの種類                */
char	*file_keyboard = NULL;		/* キー設定ファイル名		   */

int	use_xdnd = TRUE;		/* XDnD に対応するかどうか	   */
int	use_joydevice = TRUE;		/* ジョイスティックデバイスを開く? */


static	int mouse_jumped = FALSE;


static	const char *debug_x11keysym(int code); /* デバッグ用 */
/*==========================================================================
 * キー配列について
 *
 *  一般キー(文字キー) は、
 *	106 キーボードの場合、PC-8801 と同じなので問題なし。
 *	101 キーボードの場合、一部配置が異なるし、キーが足りない。
 *	とりあえず、以下のように配置してみる。
 *		` → \、 = → ^、 [ ] はそのまま、 \ → @、' → :、右CTRL → _
 *
 *  特殊キー(機能キー) は、
 *	ホスト側のキー刻印と似た雰囲気の機能を、PC-8801のキーに割り当てよう。
 *	Pause は STOP、 PrintScreen は COPY など。個人的な主観で決める。
 *
 *  テンキーは、
 *	PC-8801と106キーでキー刻印が若干異なるが、そのままのキー刻印を使う。
 *
 *  最下段のキーの配置は、適当に割り振る。 (カッコのキーには割り振らない)
 *
 *	PC-8801        かな GRPH 決定  スペース 変換  PC    全角
 *	101キー   Ctrl      Alt        スペース             Alt          Ctrl
 *	104キー   Ctrl Win  Alt        スペース             Alt  Win App Ctrl
 *	109キー   Ctrl Win  Alt 無変換 スペース 変換 (ひら) Alt  Win App Ctrl
 *	mac ?     Ctrl      Opt (Cmd)  スペース      (cmd) (Opt)        (Ctrl)
 *
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


/* X11 の keysym を 0〜511 の範囲に丸める */

#define	LOCAL_KEYSYM(ks)						    \
	((((ks) & 0xff00) == 0xff00)			/* XK_MISCELLANY */ \
		? (((ks) & 0x00ff) | 0x100)				    \
		: ((((ks) & 0xff00) == 0x0000)		/* XK_LATIN1 */	    \
					? (ks) : 0))

/*----------------------------------------------------------------------
 * X11 の keysym を QUASI88 の キーコードに変換するテーブル
 *	XK_LATIN1 と XK_MISCELLANY のキーシンボルのみ。
 *
 *	キーシンボル XK_xxx が押されたら、 
 *	keysym2key88[ XK_xxx ] が押されたとする。
 *
 *	keysym2key88[] には、 KEY88_xxx をセットしておく。
 *	初期値は keysym2key88_default[] と同じ
 *----------------------------------------------------------------------*/
static int keysym2key88[ 256 + 256 ];



/*----------------------------------------------------------------------
 * X11 の scancode を QUASI88 の キーコードに変換するテーブル
 *
 *	スキャンコード code が押されたら、
 *	keycode2key88[ code ] が押されたとする。
 *
 *	keycode2key88[] には、 KEY88_xxx または -1 をセットしておく。
 *	これは keysym2key88[] に優先される。(ただし -1 の場合は無効)
 *	初期値は 全て -1、変換可能なスキャンコードは 0〜255までに制限。
 *----------------------------------------------------------------------*/
static int scancode2key88[ 256 ];

 

/*----------------------------------------------------------------------
 * ソフトウェア NumLock オン時の キーコード変換情報
 *
 *	binding[].code (X11 の keysym ないし keycode) が押されたら、
 *	binding[].new_key88 (KEY88_xxx) が押されたことにする。
 *
 *	ソフトウェア NumLock オン時は、この情報にしたがって、
 *	keysym2key88[] 、 keycode2key88[] を書き換える。
 *	変更できるキーの個数は、64個まで (これだけあればいいだろう)
 *----------------------------------------------------------------------*/
static T_BINDING binding[ 64 ];





/*----------------------------------------------------------------------
 * XK_xxx → KEY88_xxx 変換テーブル (デフォルト)
 *----------------------------------------------------------------------*/

static const int keysym2key88_default[ 256 + 256 ] =
{
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/
  /* from XK_LATIN1					0x0000〜0x00FF	*/
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  KEY88_SPACE,		/*	XK_space               0x020	*/
  KEY88_EXCLAM,		/*	XK_exclam              0x021	*/
  KEY88_QUOTEDBL,	/*	XK_quotedbl            0x022	*/
  KEY88_NUMBERSIGN,	/*	XK_numbersign          0x023	*/
  KEY88_DOLLAR,		/*	XK_dollar              0x024	*/
  KEY88_PERCENT,	/*	XK_percent             0x025	*/
  KEY88_AMPERSAND,	/*	XK_ampersand           0x026	*/
  KEY88_APOSTROPHE,	/*	XK_apostrophe          0x027	*/
  KEY88_PARENLEFT,	/*	XK_parenleft           0x028	*/
  KEY88_PARENRIGHT,	/*	XK_parenright          0x029	*/
  KEY88_ASTERISK,	/*	XK_asterisk            0x02a	*/
  KEY88_PLUS,		/*	XK_plus                0x02b	*/
  KEY88_COMMA,		/*	XK_comma               0x02c	*/
  KEY88_MINUS,		/*	XK_minus               0x02d	*/
  KEY88_PERIOD,		/*	XK_period              0x02e	*/
  KEY88_SLASH,		/*	XK_slash               0x02f	*/

  KEY88_0,		/*	XK_0                   0x030	*/
  KEY88_1,		/*	XK_1                   0x031	*/
  KEY88_2,		/*	XK_2                   0x032	*/
  KEY88_3,		/*	XK_3                   0x033	*/
  KEY88_4,		/*	XK_4                   0x034	*/
  KEY88_5,		/*	XK_5                   0x035	*/
  KEY88_6,		/*	XK_6                   0x036	*/
  KEY88_7,		/*	XK_7                   0x037	*/
  KEY88_8,		/*	XK_8                   0x038	*/
  KEY88_9,		/*	XK_9                   0x039	*/
  KEY88_COLON,		/*	XK_colon               0x03a	*/
  KEY88_SEMICOLON,	/*	XK_semicolon           0x03b	*/
  KEY88_LESS,		/*	XK_less                0x03c	*/
  KEY88_EQUAL,		/*	XK_equal               0x03d	*/
  KEY88_GREATER,	/*	XK_greater             0x03e	*/
  KEY88_QUESTION,	/*	XK_question            0x03f	*/

  KEY88_AT,		/*	XK_at                  0x040	*/
  KEY88_A,		/*	XK_A                   0x041	*/
  KEY88_B,		/*	XK_B                   0x042	*/
  KEY88_C,		/*	XK_C                   0x043	*/
  KEY88_D,		/*	XK_D                   0x044	*/
  KEY88_E,		/*	XK_E                   0x045	*/
  KEY88_F,		/*	XK_F                   0x046	*/
  KEY88_G,		/*	XK_G                   0x047	*/
  KEY88_H,		/*	XK_H                   0x048	*/
  KEY88_I,		/*	XK_I                   0x049	*/
  KEY88_J,		/*	XK_J                   0x04a	*/
  KEY88_K,		/*	XK_K                   0x04b	*/
  KEY88_L,		/*	XK_L                   0x04c	*/
  KEY88_M,		/*	XK_M                   0x04d	*/
  KEY88_N,		/*	XK_N                   0x04e	*/
  KEY88_O,		/*	XK_O                   0x04f	*/

  KEY88_P,		/*	XK_P                   0x050	*/
  KEY88_Q,		/*	XK_Q                   0x051	*/
  KEY88_R,		/*	XK_R                   0x052	*/
  KEY88_S,		/*	XK_S                   0x053	*/
  KEY88_T,		/*	XK_T                   0x054	*/
  KEY88_U,		/*	XK_U                   0x055	*/
  KEY88_V,		/*	XK_V                   0x056	*/
  KEY88_W,		/*	XK_W                   0x057	*/
  KEY88_X,		/*	XK_X                   0x058	*/
  KEY88_Y,		/*	XK_Y                   0x059	*/
  KEY88_Z,		/*	XK_Z                   0x05a	*/
  KEY88_BRACKETLEFT,	/*	XK_bracketleft         0x05b	*/
  KEY88_YEN,		/*	XK_backslash           0x05c	*/
  KEY88_BRACKETRIGHT,	/*	XK_bracketright        0x05d	*/
  KEY88_CARET,		/*	XK_asciicircum         0x05e	*/
  KEY88_UNDERSCORE,	/*	XK_underscore          0x05f	*/

  KEY88_BACKQUOTE,	/*	XK_grave               0x060	*/
  KEY88_a,		/*	XK_a                   0x061	*/
  KEY88_b,		/*	XK_b                   0x062	*/
  KEY88_c,		/*	XK_c                   0x063	*/
  KEY88_d,		/*	XK_d                   0x064	*/
  KEY88_e,		/*	XK_e                   0x065	*/
  KEY88_f,		/*	XK_f                   0x066	*/
  KEY88_g,		/*	XK_g                   0x067	*/
  KEY88_h,		/*	XK_h                   0x068	*/
  KEY88_i,		/*	XK_i                   0x069	*/
  KEY88_j,		/*	XK_j                   0x06a	*/
  KEY88_k,		/*	XK_k                   0x06b	*/
  KEY88_l,		/*	XK_l                   0x06c	*/
  KEY88_m,		/*	XK_m                   0x06d	*/
  KEY88_n,		/*	XK_n                   0x06e	*/
  KEY88_o,		/*	XK_o                   0x06f	*/

  KEY88_p,		/*	XK_p                   0x070	*/
  KEY88_q,		/*	XK_q                   0x071	*/
  KEY88_r,		/*	XK_r                   0x072	*/
  KEY88_s,		/*	XK_s                   0x073	*/
  KEY88_t,		/*	XK_t                   0x074	*/
  KEY88_u,		/*	XK_u                   0x075	*/
  KEY88_v,		/*	XK_v                   0x076	*/
  KEY88_w,		/*	XK_w                   0x077	*/
  KEY88_x,		/*	XK_x                   0x078	*/
  KEY88_y,		/*	XK_y                   0x079	*/
  KEY88_z,		/*	XK_z                   0x07a	*/
  KEY88_BRACELEFT,	/*	XK_braceleft           0x07b	*/
  KEY88_BAR,		/*	XK_bar                 0x07c	*/
  KEY88_BRACERIGHT,	/*	XK_braceright          0x07d	*/
  KEY88_TILDE,		/*	XK_asciitilde          0x07e	*/
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
  /* from XK_MISCELLANY					0xFF00〜0xFFFF	*/
  /* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -*/

  0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_BS,		/*	XK_BackSpace		0xFF08	   */
  KEY88_TAB,		/*	XK_Tab			0xFF09	   */
  KEY88_RETURN,		/*	XK_Linefeed		0xFF0A	無 */
  KEY88_HOME,		/*	XK_Clear		0xFF0B	無 */
  0,
  KEY88_RETURNL,	/*	XK_Return		0xFF0D	   */
  0, 0,

  0, 0, 0,
  KEY88_STOP,		/*	XK_Pause		0xFF13	   */
  KEY88_KANA,		/*	XK_Scroll_Lock		0xFF14	   */
  KEY88_COPY,		/*	XK_Sys_Req		0xFF15	   */
  0, 0,
  0, 0, 0,
  KEY88_ESC, 		/*	XK_Escape		0xFF1B	   */
  0, 0, 0, 0,

  0,			/*	XK_Multi_key		0xFF20	無 */
  0,			/*	XK_Kanji		0xFF21	   */
  KEY88_KETTEI,		/*	XK_Muhenkan		0xFF22	   */
  KEY88_HENKAN,		/*	XK_Henkan_Mode		0xFF23	   */
  KEY88_KANA,		/*	XK_Romaji		0xFF24     */
  0,			/*	XK_Hiragana		0xFF25  無 */
  0,			/*	XK_Katakana		0xFF26  無 */
  KEY88_KANA,		/*	XK_Hiragana_Katakana	0xFF27	   */
  0,			/*	XK_Zenkaku		0xFF28  無 */
  0,			/*	XK_Hankaku		0xFF29  無 */
  KEY88_ZENKAKU,	/*	XK_Zenkaku_Hankaku	0xFF2A	   */
  0,			/*	XK_Touroku		0xFF2B  無 */
  0,			/*	XK_Massyo		0xFF2C  無 */
  KEY88_KANA,		/*	XK_Kana_Lock		0xFF2D	無 */
  KEY88_KANA,		/*	XK_Kana_Shift		0xFF2E  無 */
  0,			/*	XK_Eisu_Shift		0xFF2F  無 */

  KEY88_CAPS,		/*	XK_Eisu_toggle		0xFF30     */
  0, 0, 0, 0, 0, 0,
  0,			/*	XK_Kanji_Bangou		0xFF37  無 */
  0, 0, 0, 0,
  0,			/*	XK_SingleCandidate	0xFF3C	無 */
  0,			/*	XK_Zen_Koho		0xFF3D	無 */
  0,			/*	XK_Mae_Koho		0xFF3E	無 */
  0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,

  KEY88_HOME,		/*	XK_Home			0xFF50	   */
  KEY88_LEFT,		/*	XK_Left			0xFF51	   */
  KEY88_UP,		/*	XK_Up			0xFF52	   */
  KEY88_RIGHT,		/*	XK_Right		0xFF53	   */
  KEY88_DOWN,		/*	XK_Down			0xFF54	   */
  KEY88_ROLLDOWN,	/*	XK_Prior		0xFF55	   */
  KEY88_ROLLUP,		/*	XK_Next			0xFF56	   */
  KEY88_HELP,		/*	XK_End			0xFF57	   */
  0,			/*	XK_Begin		0xFF58	無 */
  0, 0, 0, 0, 0, 0, 0,

  0,			/*	XK_Select		0xFF60	無 */
  KEY88_COPY,		/*	XK_Print		0xFF61	   */
  KEY88_COPY,		/*	XK_Execute		0xFF62	   */
  KEY88_INS, 		/*	XK_Insert		0xFF63	   */
  0,
  0,			/*	XK_Undo			0xFF65	無 */
  0,			/*	XK_Redo			0xFF66	無 */
  0,			/*	XK_Menu			0xFF67	   */
  0,			/*	XK_Find			0xFF68	無 */
  0,			/*	XK_Cancel		0xFF69	無 */
  KEY88_HELP,		/*	XK_Help			0xFF6A	無 */
  KEY88_STOP,		/*	XK_Break		0xFF6B	   */
  0, 0, 0, 0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_HENKAN,		/*	XK_Mode_switch		0xFF7E	   */
  0,			/*	XK_Num_Lock		0xFF7F	   */

  KEY88_SPACE,		/*	XK_KP_Space		0xFF80	無 */
  0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_TAB,		/*	XK_KP_Tab		0xFF89	無 */
  0, 0, 0,
  KEY88_RETURNR,	/*	XK_KP_Enter		0xFF8D	   */
  0, 0,

  0,
  KEY88_F1,		/*	XK_KP_F1		0xFF91	無 */
  KEY88_F2,		/*	XK_KP_F2		0xFF92	無 */
  KEY88_F3,		/*	XK_KP_F3		0xFF93	無 */
  KEY88_F4,		/*	XK_KP_F4		0xFF94	無 */
  KEY88_KP_7,		/*	XK_KP_Home		0xFF95	   */
  KEY88_KP_4,		/*	XK_KP_Left		0xFF96	   */
  KEY88_KP_8,		/*	XK_KP_Up		0xFF97	   */
  KEY88_KP_6,		/*	XK_KP_Right		0xFF98	   */
  KEY88_KP_2,		/*	XK_KP_Down		0xFF99	   */
  KEY88_KP_9,		/*	XK_KP_Page_Up		0xFF9A	   */
  KEY88_KP_3,		/*	XK_KP_Page_Down		0xFF9B	   */
  KEY88_KP_1,		/*	XK_KP_End		0xFF9C	   */
  KEY88_KP_5,		/*	XK_KP_Begin		0xFF9D	   */
  KEY88_KP_0,		/*	XK_KP_Insert		0xFF9E	   */
  KEY88_KP_PERIOD,	/*	XK_KP_Delete		0xFF9F	   */

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_KP_MULTIPLY,	/*	XK_KP_Multiply		0xFFAA	   */
  KEY88_KP_ADD,		/*	XK_KP_Add		0xFFAB	   */
  KEY88_KP_COMMA,	/*	XK_KP_Separator		0xFFAC	無 */
  KEY88_KP_SUB,		/*	XK_KP_Subtract		0xFFAD	   */
  KEY88_KP_PERIOD,	/*	XK_KP_Decimal		0xFFAE	   */
  KEY88_KP_DIVIDE,	/*	XK_KP_Divide		0xFFAF	   */

  KEY88_KP_0,		/*	XK_KP_0			0xFFB0	   */
  KEY88_KP_1,		/*	XK_KP_1			0xFFB1	   */
  KEY88_KP_2,		/*	XK_KP_2			0xFFB2	   */
  KEY88_KP_3,		/*	XK_KP_3			0xFFB3	   */
  KEY88_KP_4,		/*	XK_KP_4			0xFFB4	   */
  KEY88_KP_5,		/*	XK_KP_5			0xFFB5	   */
  KEY88_KP_6,		/*	XK_KP_6			0xFFB6	   */
  KEY88_KP_7,		/*	XK_KP_7			0xFFB7	   */
  KEY88_KP_8,		/*	XK_KP_8			0xFFB8	   */
  KEY88_KP_9,		/*	XK_KP_9			0xFFB9	   */
  0, 0, 0,
  KEY88_KP_EQUAL,	/*	XK_KP_Equal		0xFFBD	無 */
  KEY88_F1,		/*	XK_F1			0xFFBE	   */
  KEY88_F2,		/*	XK_F2			0xFFBF	   */

  KEY88_F3,		/*	XK_F3			0xFFC0	   */
  KEY88_F4,		/*	XK_F4			0xFFC1	   */
  KEY88_F5,		/*	XK_F5			0xFFC2	   */
  KEY88_F6,		/*	XK_F6			0xFFC3	   */
  KEY88_F7,		/*	XK_F7			0xFFC4	   */
  KEY88_F8,		/*	XK_F8			0xFFC5	   */
  KEY88_F9,		/*	XK_F9			0xFFC6	   */
  KEY88_F10,		/*	XK_F10			0xFFC7	   */
  KEY88_F11,		/*	XK_F11			0xFFC8	   */
  KEY88_F12,		/*	XK_F12			0xFFC9	   */
  KEY88_F13,		/*	XK_F13			0xFFCA	無 */
  KEY88_F14,		/*	XK_F14			0xFFCB	無 */
  KEY88_F15,		/*	XK_F15			0xFFCC	無 */
  KEY88_F16,		/*	XK_F16			0xFFCD	無 */
  KEY88_F17,		/*	XK_F17			0xFFCE	無 */
  KEY88_F18,		/*	XK_F18			0xFFCF	無 */

  KEY88_F19,		/*	XK_F19			0xFFD0	無 */
  KEY88_F20,		/*	XK_F20			0xFFD1	無 */
  0,			/*	XK_F21			0xFFD2	無 */
  0,			/*	XK_F22			0xFFD3	無 */
  0,			/*	XK_F23			0xFFD4	無 */
  0,			/*	XK_F24			0xFFD5	無 */
  0,			/*	XK_F25			0xFFD6	無 */
  0,			/*	XK_F26			0xFFD7	無 */
  0,			/*	XK_F27			0xFFD8	無 */
  0,			/*	XK_F28			0xFFD9	無 */
  0,			/*	XK_F29			0xFFDA	無 */
  0,			/*	XK_F30			0xFFDB	無 */
  0,			/*	XK_F31			0xFFDC	無 */
  0,			/*	XK_F32			0xFFDD	無 */
  0,			/*	XK_F33			0xFFDE	無 */
  0,			/*	XK_F34			0xFFDF	無 */

  0,			/*	XK_F35			0xFFE0	無 */
  KEY88_SHIFTL,		/*	XK_Shift_L		0xFFE1	   */
  KEY88_SHIFTR,		/*	XK_Shift_R		0xFFE2	   */
  KEY88_CTRL,		/*	XK_Control_L		0xFFE3	   */
  KEY88_CTRL,		/*	XK_Control_R		0xFFE4	   */
  KEY88_CAPS,		/*	XK_Caps_Lock		0xFFE5	   */
  KEY88_CAPS,		/*	XK_Shift_Lock		0xFFE6	無 */
  KEY88_GRAPH,		/*	XK_Meta_L		0xFFE7	   */
  KEY88_GRAPH,		/*	XK_Meta_R		0xFFE8	   */
  KEY88_GRAPH,		/*	XK_Alt_L		0xFFE9	   */
  KEY88_GRAPH,		/*	XK_Alt_R		0xFFEA	   */
  0,			/*	XK_Super_L		0xFFEB	   */
  0,			/*	XK_Super_R		0xFFEC	   */
  0,			/*	XK_Hyper_L		0xFFED	無 */
  0,			/*	XK_Hyper_R		0xFFEE	無 */
  0,

  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  KEY88_DEL,		/*	XK_Delete		0xFFFF	   */

};



/*----------------------------------------------------------------------
 * keysym2key88[]   の初期値は、keysym2key88_default[] と同じ、
 * scancode2key88[] の初期値は、全て -1 (未使用) であるが、
 * キーボードの種類に応じて、keysym2key88[] と scancode2key88[] の一部を
 * 変更することにする。以下は、その変更の情報。
 *----------------------------------------------------------------------*/

static const T_REMAPPING remapping_106[] =
{
    {	KEYCODE_SYM,  XK_Super_L,	KEY88_KANA,	    },
    {	KEYCODE_SYM,  XK_Alt_R,		KEY88_ZENKAKU,	    },
    {	KEYCODE_SYM,  XK_Meta_R,	KEY88_ZENKAKU,	    },
    {	KEYCODE_SYM,  XK_Menu,		KEY88_SYS_MENU,	    },
/*  {	KEYCODE_SYM,  XK_Control_R,	KEY88_UNDERSCORE,   },*/

    {	KEYCODE_SCAN,	  19,		KEY88_0,	    },   /* 0        */
    {	KEYCODE_SCAN,	 123,		KEY88_UNDERSCORE,   },   /* \ _ ロ   */

    {	KEYCODE_INVALID, 0,		0,		    },
};

static const T_REMAPPING remapping_101[] =
{
    {	KEYCODE_SYM,  XK_Super_L,	KEY88_KANA,	    },
    {	KEYCODE_SYM,  XK_Alt_R,		KEY88_ZENKAKU,	    },
    {	KEYCODE_SYM,  XK_Meta_R,	KEY88_ZENKAKU,	    },
    {	KEYCODE_SYM,  XK_Menu,		KEY88_SYS_MENU,	    },
    {	KEYCODE_SYM,  XK_Control_R,	KEY88_UNDERSCORE,   },

    {	KEYCODE_SYM,  XK_grave,		KEY88_YEN,	    },
    {	KEYCODE_SYM,  XK_asciitilde,	KEY88_BAR,	    },
    {	KEYCODE_SYM,  XK_at,		KEY88_QUOTEDBL,	    },
    {	KEYCODE_SYM,  XK_asciicircum,	KEY88_AMPERSAND,    },
    {	KEYCODE_SYM,  XK_ampersand,	KEY88_APOSTROPHE,   },
    {	KEYCODE_SYM,  XK_asterisk,	KEY88_PARENLEFT,    },
    {	KEYCODE_SYM,  XK_parenleft,	KEY88_PARENRIGHT,   },
    {	KEYCODE_SYM,  XK_parenright,	KEY88_0,	    },
    {	KEYCODE_SYM,  XK_underscore,	KEY88_EQUAL,	    },
    {	KEYCODE_SYM,  XK_equal,		KEY88_CARET,	    },
    {	KEYCODE_SYM,  XK_plus,		KEY88_TILDE,	    },
    {	KEYCODE_SYM,  XK_bracketleft,	KEY88_AT,	    },
    {	KEYCODE_SYM,  XK_braceleft,	KEY88_BACKQUOTE,    },
    {	KEYCODE_SYM,  XK_bracketright,	KEY88_BRACKETLEFT,  },
    {	KEYCODE_SYM,  XK_braceright,	KEY88_BRACELEFT,    },
    {	KEYCODE_SYM,  XK_backslash,	KEY88_BRACKETRIGHT, },
    {	KEYCODE_SYM,  XK_bar,		KEY88_BRACERIGHT,   },
    {	KEYCODE_SYM,  XK_colon,		KEY88_PLUS,	    },
    {	KEYCODE_SYM,  XK_apostrophe,	KEY88_COLON,	    },
    {	KEYCODE_SYM,  XK_quotedbl,	KEY88_ASTERISK,	    },
    {	KEYCODE_INVALID, 0,		0,		    },
};



/*----------------------------------------------------------------------
 * ソフトウェア NumLock オン時の キーコード変換情報 (デフォルト)
 *----------------------------------------------------------------------*/

static const T_BINDING binding_106[] =
{
    {	KEYCODE_SYM,	XK_5,		KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	XK_6,		KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	XK_7,		KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	XK_8,		KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	XK_9,		KEY88_KP_9,		0,	},
    {	KEYCODE_SCAN,	19,		KEY88_KP_MULTIPLY,	0,	},
    {	KEYCODE_SYM,	XK_minus,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	XK_asciicircum,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	XK_u,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	XK_i,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	XK_o,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	XK_p,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	XK_j,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	XK_k,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	XK_l,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	XK_semicolon,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	XK_m,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	XK_comma,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	XK_period,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	XK_slash,	KEY88_RETURNR,		0,	},

    {	KEYCODE_SYM,	XK_percent,	KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	XK_ampersand,	KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	XK_apostrophe,	KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	XK_parenleft,	KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	XK_parenright,	KEY88_KP_9,		0,	},
    {	KEYCODE_SYM,	XK_equal,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	XK_asciitilde,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	XK_U,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	XK_I,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	XK_O,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	XK_P,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	XK_J,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	XK_K,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	XK_L,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	XK_plus,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	XK_M,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	XK_less,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	XK_greater,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	XK_question,	KEY88_RETURNR,		0,	},

    {	KEYCODE_INVALID,0,		0,			0,	},
};

static const T_BINDING binding_101[] = 
{
    {	KEYCODE_SYM,	XK_5,		KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	XK_6,		KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	XK_7,		KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	XK_8,		KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	XK_9,		KEY88_KP_9,		0,	},
    {	KEYCODE_SCAN,	19,		KEY88_KP_MULTIPLY,	0,	},
    {	KEYCODE_SYM,	XK_minus,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	XK_equal,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	XK_u,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	XK_i,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	XK_o,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	XK_p,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	XK_j,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	XK_k,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	XK_l,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	XK_semicolon,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	XK_m,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	XK_comma,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	XK_period,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	XK_slash,	KEY88_RETURNR,		0,	},

    {	KEYCODE_SYM,	XK_percent,	KEY88_HOME,		0,	},
    {	KEYCODE_SYM,	XK_asciicircum,	KEY88_HELP,		0,	},
    {	KEYCODE_SYM,	XK_ampersand,	KEY88_KP_7,		0,	},
    {	KEYCODE_SYM,	XK_asterisk,	KEY88_KP_8,		0,	},
    {	KEYCODE_SYM,	XK_parenleft,	KEY88_KP_9,		0,	},
    {	KEYCODE_SYM,	XK_parenright,	KEY88_KP_MULTIPLY,	0,	},
    {	KEYCODE_SYM,	XK_underscore,	KEY88_KP_SUB,		0,	},
    {	KEYCODE_SYM,	XK_plus,	KEY88_KP_DIVIDE,	0,	},
    {	KEYCODE_SYM,	XK_U,		KEY88_KP_4,		0,	},
    {	KEYCODE_SYM,	XK_I,		KEY88_KP_5,		0,	},
    {	KEYCODE_SYM,	XK_O,		KEY88_KP_6,		0,	},
    {	KEYCODE_SYM,	XK_P,		KEY88_KP_ADD,		0,	},
    {	KEYCODE_SYM,	XK_J,		KEY88_KP_1,		0,	},
    {	KEYCODE_SYM,	XK_K,		KEY88_KP_2,		0,	},
    {	KEYCODE_SYM,	XK_L,		KEY88_KP_3,		0,	},
    {	KEYCODE_SYM,	XK_colon,	KEY88_KP_EQUAL,		0,	},
    {	KEYCODE_SYM,	XK_M,		KEY88_KP_0,		0,	},
    {	KEYCODE_SYM,	XK_less,	KEY88_KP_COMMA,		0,	},
    {	KEYCODE_SYM,	XK_greater,	KEY88_KP_PERIOD,	0,	},
    {	KEYCODE_SYM,	XK_question,	KEY88_RETURNR,		0,	},

    {	KEYCODE_INVALID,0,		0,			0,	},
};



/******************************************************************************
 * イベントハンドリング
 *
 *	1/60毎に呼び出される。
 *****************************************************************************/
static	int	analyze_keyconf_file(void);

static	void	xdnd_receive_drag(XClientMessageEvent *E);
static	void	xdnd_receive_drop(XSelectionEvent *E);

/*
 * これは 起動時に1回だけ呼ばれる
 */
void	event_init(void)
{
    const T_REMAPPING *map;
    const T_BINDING   *bin;
    int i;

    /* ジョイスティック初期化 */

    if (use_joydevice) {
	joystick_init();
    }


    /* キーマッピング初期化 */

    memset(keysym2key88, 0, sizeof(keysym2key88));
    for (i=0; i<COUNTOF(scancode2key88); i++) {
	scancode2key88[ i ] = -1;
    }
    memset(binding, 0, sizeof(binding));



    switch (keyboard_type) {

    case 0:					/* デフォルトキーボード */
	if (analyze_keyconf_file()) {
	    ;
	} else {
	    memcpy(keysym2key88,
		   keysym2key88_default, sizeof(keysym2key88_default));
	    memcpy(binding,
		   binding_106, sizeof(binding_106));
	}
	break;


    case 1:					/* 106日本語キーボード */
    case 2:					/* 101英語キーボード ? */
	memcpy(keysym2key88,
	       keysym2key88_default, sizeof(keysym2key88_default));

	if (keyboard_type == 1) {
	    map = remapping_106;
	    bin = binding_106;
	} else {
	    map = remapping_101;
	    bin = binding_101;
	}

	for ( ; map->type; map ++) {

	    if        (map->type == KEYCODE_SYM) {

		keysym2key88[ LOCAL_KEYSYM(map->code) ] = map->key88;

	    } else if (map->type == KEYCODE_SCAN) {

		scancode2key88[ map->code ] = map->key88;

	    }
	}

	for (i=0; i<COUNTOF(binding); i++) {
	    if (bin->type == KEYCODE_INVALID) break;

	    binding[ i ].type      = bin->type;
	    binding[ i ].code      = bin->code;
	    binding[ i ].org_key88 = bin->org_key88;
	    binding[ i ].new_key88 = bin->new_key88;
	    bin ++;
	}
	break;
    }



    /* ソフトウェアNumLock 時のキー差し替えの準備 */

    for (i=0; i<COUNTOF(binding); i++) {

	if        (binding[i].type == KEYCODE_SYM) {

	    binding[i].org_key88 = keysym2key88[LOCAL_KEYSYM(binding[i].code)];

	} else if (binding[i].type == KEYCODE_SCAN) {

	    binding[i].org_key88 = scancode2key88[ binding[i].code ];

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
    XEvent E;
    KeySym keysym;
    char   dummy[4];
    int    key88, x, y;

    while (XPending(x11_display)) {

	XNextEvent(x11_display, &E);

	switch (E.type) {

	case KeyPress:		/*------------------------------------------*/
	case KeyRelease:

	/* 以下のどっちを使うのが正解？ 取得出来る keysym が違うのだが …… */
#if 0
	    keysym = XLookupKeysym((XKeyEvent *)&E, 0);
	    /* ・Shift + TAB は XK_TAB が返る				*/
	    /* ・Shift + \ (シフトの左のキー) は XK_backslash が返る	*/
#else
	    XLookupString((XKeyEvent *)&E, dummy, 0, &keysym, NULL);
	    /* ・Shift + TAB は XK_ISO_Left_Tab が返る			*/
	    /* ・Shift + \ (シフトの左のキー) は XK_underscore が返る	*/
#endif

	    if (quasi88_is_exec()) {

		/* scancode2key88[] が定義済なら、そのキーコードを優先する */
		if (E.xkey.keycode < COUNTOF(scancode2key88) &&
		    scancode2key88[ E.xkey.keycode ] >= 0) {

		    key88 = scancode2key88[ E.xkey.keycode ];

		} else {
		    if ((keysym & 0xff00) == 0xff00) {	    /* XK_MISCELLANY */

			key88 = keysym2key88[ (keysym & 0x00ff) | 0x100 ];

		    } else if ((keysym & 0xff00) == 0x0000) {	/* XK_LATIN1 */

			key88 = keysym2key88[ (keysym) ];

		    } else if (keysym == XK_ISO_Left_Tab) {	/* 0xFE20 */

			key88 = KEY88_TAB;

		    } else {					/* else... */

			key88 = 0;

		    }
		}

	    } else {			/* (メニューなど) */

		if ((keysym & 0xff00) == 0xff00) {	    /* XK_MISCELLANY */

		    key88 = keysym2key88[ (keysym & 0x00ff) | 0x100 ];

		} else if ((keysym & 0xff00) == 0x0000) {	/* XK_LATIN1 */

		    key88 = keysym;

		} else if (keysym == XK_ISO_Left_Tab) {		/* 0xFE20 */

		    key88 = KEY88_TAB;

		} else {					/* else... */

		    key88 = 0;

		}

	    }

	    /*if (E.type==KeyPress)
		printf("scan=%3d : %04x=%-16s -> %d\n", E.xkey.keycode, keysym,
		       debug_x11keysym(keysym), key88);*/
	    /*printf("%d %d %d\n",key88,keysym,E.xkey.keycode);*/

	    if (key88) {
		quasi88_key(key88, (E.type == KeyPress));
	    }
	    break;

	case MotionNotify:	/*------------------------------------------*/
	    switch (x11_mouse_rel_move) {

	    case 1:
		/* DGA：マウス移動イベントは常に相対移動量がセットされる */

		x = E.xmotion.x;
		y = E.xmotion.y;

		if (x || y) {
		    quasi88_mouse_moved_rel(x, y);
		}
		break;

	    case -1:
		/* ウインドウグラブ時は、マウスがウインドウの端にたどり着くと
		   それ以上動けなくなる。
		   そこで、マウスを常にウインドウの中央にジャンプさせることで、
		   無限にマウスを動かすことができるかのようにする。
		   この時、マウスを非表示にしておかないと、無様な様子が見える
		   ので注意。
		   なお、XWarpPointer でポインタを移動させると、それに応じて
		   MotionNotify が発生するので、無視すること。*/

		if (mouse_jumped) { mouse_jumped = FALSE; break; }

		x = E.xmotion.x - x11_width  / 2;
		y = E.xmotion.y - x11_height / 2;

		if (x11_scaling) {
		    x = x * x11_scale_x_num / x11_scale_x_den;
		    y = y * x11_scale_y_num / x11_scale_y_den;
		}

		if (x || y) {
		    quasi88_mouse_moved_rel(x, y);
		}
      
		XWarpPointer(x11_display, None, x11_window, 0, 0, 0, 0,
			     x11_width / 2, x11_height / 2);
		mouse_jumped = TRUE;
		break;

	    default:
		/* 通常：マウス移動イベントは、ウインドウ座標がセットされる */

		x = E.xmotion.x;
		y = E.xmotion.y;

		if (x11_scaling) {
		    x = x * x11_scale_x_num / x11_scale_x_den;
		    y = y * x11_scale_y_num / x11_scale_y_den;
		}

		quasi88_mouse_moved_abs(x, y);
	    }
	    /*printf("%d %d -> (%d, %d)\n",E.xmotion.x,E.xmotion.y,x,y);*/
	    break;

	case ButtonPress:	/*------------------------------------------*/
	case ButtonRelease:
	    /* マウス移動イベントも同時に処理する必要があるなら、
	       quasi88_mouse_moved_abs/rel 関数をここで呼び出しておく */

	    if      (E.xbutton.button == Button1) key88 = KEY88_MOUSE_L;
	    else if (E.xbutton.button == Button2) key88 = KEY88_MOUSE_M;
	    else if (E.xbutton.button == Button3) key88 = KEY88_MOUSE_R;
	    else if (E.xbutton.button == Button4) key88 = KEY88_MOUSE_WUP;
	    else if (E.xbutton.button == Button5) key88 = KEY88_MOUSE_WDN;
	    else break;

	    quasi88_mouse(key88, (E.type == ButtonPress));
	    break;

	case ClientMessage:	/*------------------------------------------*/
	    /* ウインドウが破壊された場合は、ここで終了 */
	    if (E.xclient.message_type == x11_atom_kill_type &&
		(Atom)E.xclient.data.l[0] == x11_atom_kill_data) {

		if (verbose_proc) printf("Window Closed !\n");
		quasi88_quit();
		break;
	    }

	    /* Drag & Drop のチェック */
	    if (use_xdnd)
		xdnd_receive_drag((XClientMessageEvent *)&E);
	    break;

	case SelectionNotify:	/*------------------------------------------*/
	    /* Drag & Drop のチェック */
	    if (use_xdnd)
		xdnd_receive_drop((XSelectionEvent *)&E);
	    break; 

	case FocusIn:		/*------------------------------------------*/
	    quasi88_focus_in();
	    x11_get_focus = TRUE;
	    x11_set_attribute_focus_in();
	    break;

	case FocusOut:		/*------------------------------------------*/
	    quasi88_focus_out();
	    x11_get_focus = FALSE;
	    x11_set_attribute_focus_out();
	    break;

	case Expose:		/*------------------------------------------*/
	    quasi88_expose();
	    break;
	}
    }

    /* X11 には標準でジョイステックイベントが発生しない。
       なので、ここでジョイステック状態をポーリングして処理する */

    joystick_update();
}



/*
 * これは 終了時に1回だけ呼ばれる
 */
void	event_exit(void)
{
    joystick_exit();
}



/***********************************************************************
 * 現在のマウス座標取得関数
 *
 *	現在のマウスの絶対座標を *x, *y にセット
 ************************************************************************/

void	event_get_mouse_pos(int *x, int *y)
{
    if (x11_mouse_rel_move) {

	/* DGAおよびウインドウグラブ時は、マウスの絶対座標は存在しない。 */

    } else {

	Window root_return;			/* ルートウインドウID     */
	Window child_return;			/* 子ウインドウID(あれば) */
	int    root_x, root_y;			/* ルートウインドウの座標 */
	int    win_x,  win_y;			/* 指定ウインドウの座標   */
	unsigned int button;			/* マウスボタンの押下状況 */

	if (XQueryPointer(x11_display, x11_window, &root_return, &child_return,
			  &root_x, &root_y, 
			  &win_x,  &win_y, 
			  &button)) {

	    *x = win_x;
	    *y = win_y;

	    return;
	}

    }

    /* 絶対座標が無い場合は、とりあえず画面中央の座標を返すことにしよう */

    *x = x11_width  / 2;
    *y = x11_height / 2;
}




/******************************************************************************
 * ソフトウェア NumLock 有効／無効
 *
 *****************************************************************************/

static	void	numlock_setup(int enable)
{
    int i;
    int keysym;

    for (i=0; i<COUNTOF(binding); i++) {

	if        (binding[i].type == KEYCODE_SYM) {

	    keysym = LOCAL_KEYSYM(binding[i].code);

	    if (enable) {
		keysym2key88[ keysym ] = binding[i].new_key88;
	    } else {
		keysym2key88[ keysym ] = binding[i].org_key88;
	    }

	} else if (binding[i].type == KEYCODE_SCAN) {

	    if (enable) {
		scancode2key88[ binding[i].code ] = binding[i].new_key88;
	    } else {
		scancode2key88[ binding[i].code ] = binding[i].org_key88;
	    }

	} else {
	    break;
	}
    }
}

int	event_numlock_on (void) { numlock_setup(TRUE);  return TRUE; }
void	event_numlock_off(void) { numlock_setup(FALSE); }



/******************************************************************************
 * エミュレート／メニュー／ポーズ／モニターモード の 開始時の処理
 *
 *****************************************************************************/

void	event_switch(void)
{
#if 1
    /* 既存のイベントをすべて破棄 */
    XEvent E;
    while (XCheckWindowEvent(x11_display, x11_window,
			     ExposureMask|KeyPressMask|KeyReleaseMask|
			     ButtonPressMask|ButtonReleaseMask|
			     PointerMotionMask,
			     &E));
#endif


    /* ウインドウグラブ中は、マウスを画面中央へ移動 */
    if (x11_mouse_rel_move == -1) {
	XWarpPointer(x11_display, None, x11_window, 0, 0, 0, 0,
		     x11_width / 2, x11_height / 2);
	mouse_jumped = TRUE;
    }

    XFlush(x11_display);
}



#if 0
/***********************************************************************
 * 強制終了のイベントを生成
 ************************************************************************/
void	event_quit_signal(void)
{
    XClientMessageEvent E;

    E.type    = ClientMessage;
    E.display = x11_display;
    E.window  = x11_window;
    E.message_type = atom_sigint;
    E.format       = 32;

    if (XSendEvent(x11_display, x11_window, False, ExposureMask, (XEvent*)&E)){
	XFlush(x11_display);
    }
}
#endif


/****************************************************************************
 ****************************************************************************
 *
 *	XDnD を適当に実装してみた。実装にはあまり自信なし。
 *
 ****************************************************************************
 *****************************************************************************/

#include <X11/Xatom.h>

static int drop_filename(const char *buf, int size);

static const int my_xdnd_ver = 5;

static Atom XA_XdndAware;
static Atom XA_XdndProxy;
static Atom XA_XdndEnter;
static Atom XA_XdndPosition;
static Atom XA_XdndStatus;
static Atom XA_XdndLeave;
static Atom XA_XdndDrop;
static Atom XA_XdndFinished;
static Atom XA_XdndTypeList;
static Atom XA_XdndSelection;
static Atom XV_DND_SELECTION;
static Atom XA_XdndActionCopy;
static Atom XA_XdndActionMove;
static Atom XA_XdndActionLink;
static Atom XA_XdndActionPrivate;
static Atom XA_XdndActionAsk;
static Atom MIME_text_plain;
static Atom MIME_text_uri_list;

/*
 *	起動時に1回だけ呼び出す。(Atomを取得or設定する)
 */
void	xdnd_initialize(void)
{
    if (use_xdnd == FALSE) return;

    XA_XdndAware	= XInternAtom(x11_display, "XdndAware",		False);
    XA_XdndProxy	= XInternAtom(x11_display, "XdndProxy",		False);
    XA_XdndEnter	= XInternAtom(x11_display, "XdndEnter",		False);
    XA_XdndPosition	= XInternAtom(x11_display, "XdndPosition",	False);
    XA_XdndStatus	= XInternAtom(x11_display, "XdndStatus",	False);
    XA_XdndLeave	= XInternAtom(x11_display, "XdndLeave",		False);
    XA_XdndDrop		= XInternAtom(x11_display, "XdndDrop",		False);
    XA_XdndFinished	= XInternAtom(x11_display, "XdndFinished",	False);

    XA_XdndActionCopy	= XInternAtom(x11_display, "XdndActionCopy",	False);
    XA_XdndActionMove	= XInternAtom(x11_display, "XdndActionMove",	False);
    XA_XdndActionLink	= XInternAtom(x11_display, "XdndActionLink",	False);
    XA_XdndActionPrivate= XInternAtom(x11_display, "XdndActionPrivate",	False);
    XA_XdndActionAsk	= XInternAtom(x11_display, "XdndActionAsk",	False);

    XA_XdndTypeList	= XInternAtom(x11_display, "XdndTypeList",	False);
    XA_XdndSelection	= XInternAtom(x11_display, "XdndSelection",	False);
    XV_DND_SELECTION	= XInternAtom(x11_display, "XV_DND_SELECTION",	False);

    MIME_text_plain	= XInternAtom(x11_display, "text/plain",	False);
    MIME_text_uri_list	= XInternAtom(x11_display, "text/uri-list",	False);

#if 0
    printf("XA_XdndAware          %d\n", XA_XdndAware        );
    printf("XA_XdndProxy          %d\n", XA_XdndProxy        );
    printf("XA_XdndEnter          %d\n", XA_XdndEnter        );
    printf("XA_XdndPosition       %d\n", XA_XdndPosition     );
    printf("XA_XdndStatus         %d\n", XA_XdndStatus       );
    printf("XA_XdndLeave          %d\n", XA_XdndLeave        );
    printf("XA_XdndDrop           %d\n", XA_XdndDrop         );
    printf("XA_XdndFinished       %d\n", XA_XdndFinished     );
    printf("XA_XdndActionCopy     %d\n", XA_XdndActionCopy   );
    printf("XA_XdndActionMove     %d\n", XA_XdndActionMove   );
    printf("XA_XdndActionLink     %d\n", XA_XdndActionLink   );
    printf("XA_XdndActionPrivate  %d\n", XA_XdndActionPrivate);
    printf("XA_XdndActionAsk      %d\n", XA_XdndActionAsk    );
    printf("XA_XdndTypeList       %d\n", XA_XdndTypeList     );
    printf("XA_XdndSelection      %d\n", XA_XdndSelection    );
    printf("XV_DND_SELECTION      %d\n", XV_DND_SELECTION    );
    printf("MIME_text_plain       %d\n", MIME_text_plain     );
    printf("MIME_text_uri_list    %d\n", MIME_text_uri_list  );
#endif
}


/*
 *	ウインドウ生成時に呼び出す (XDndの使用を宣言する)
 */
void	xdnd_start(void)
{
    if (use_xdnd == FALSE) return;

    XChangeProperty(x11_display, x11_window, XA_XdndAware, XA_ATOM, 32,
		    PropModeReplace,(unsigned char*) &my_xdnd_ver, 1);
}



/*
 *	イベント処理
 */

static int	xdnd_ver;
static Window	xdnd_from;
static Window	xdnd_to;
static Atom	xdnd_type;
static int	xdnd_accept;
static enum { YET, ENTER, POS, DROP } xdnd_step = YET;

static	void	xdnd_receive_drag(XClientMessageEvent *E)
{
    if (E->message_type == XA_XdndEnter) {
	/*
	  ■■■■ 最初：マウスがウインドウに入ってきた時のイベント ■■■■

	  E->data.l[0]		ソースウィンドウの XID
	  E->data.l[1]		上位 8bit プロトコルバージョン
     				bit 0 が真なら3個以上のデータタイプあり
	  E->data.l[2]-[4]	データタイプ 1個目、2個目、3個目
	*/

	xdnd_from = (Window)E->data.l[0];
	xdnd_to   =         E->window;
	xdnd_ver  = E->data.l[1] >> 24;
	xdnd_step = ENTER;
	xdnd_type = None;

	if ((E->data.l[1] & 1) == 0) {	/* データタイプは3個以内 */

	    if        ((Atom)E->data.l[2] == MIME_text_uri_list ||
		       (Atom)E->data.l[3] == MIME_text_uri_list ||
		       (Atom)E->data.l[4] == MIME_text_uri_list) {

		xdnd_type = MIME_text_uri_list;
	    } else if ((Atom)E->data.l[2] == MIME_text_plain ||
		       (Atom)E->data.l[3] == MIME_text_plain ||
		       (Atom)E->data.l[4] == MIME_text_plain) {

		xdnd_type = MIME_text_plain;
	    }

	} else {			/* データタイプもっとある */
	    Atom type;
	    int format;
	    unsigned long nitems, bytes, i;
	    Atom* prop;

	    if (XGetWindowProperty(x11_display, E->data.l[0], XA_XdndTypeList,
				   0, 8192, False, XA_ATOM, &type, &format,
				   &nitems, &bytes, (unsigned char **) &prop)
		== Success) {

		for (i = 0; i < nitems; i ++) {
		    if (prop[i] == MIME_text_uri_list ||
			prop[i] == MIME_text_plain) {
			xdnd_type = prop[i];
			break;
		    }
		}
		XFree(prop);
	    }
	}

    } else if (E->message_type == XA_XdndPosition) {

	/*
	  ■■■■ 途中：マウスがウインドウ内を動く時のイベント ■■■■

	  E->data.l[0]	ソースウィンドウの XID
	  E->data.l[1]	予約
	  E->data.l[2]	マウスの座標 (x << 16) | y
	  E->data.l[3]	タイムスタンプ
	  E->data.l[4]	リクエストアクション
	*/

	if (xdnd_from != (Window)E->data.l[0] ||
	    xdnd_to   !=         E->window) {
	    /* ん？ 違うぞ？ */
	    ;
	} else if (xdnd_step == ENTER || xdnd_step == POS) {

	    xdnd_step   = POS;
	    xdnd_accept = 0;

	    if (quasi88_is_exec()) {
		if (xdnd_type != None) {
		    if ((Atom)E->data.l[4] == XA_XdndActionCopy) {
			xdnd_accept = 1;
		    }
		}
	    }

	    {
		XClientMessageEvent S;

		S.type		= ClientMessage;
		S.display	= x11_display;
		S.window	= xdnd_from;
		S.message_type	= XA_XdndStatus;
		S.format	= 32;
		S.data.l[0]	= xdnd_to;
		S.data.l[1]	= xdnd_accept;
		S.data.l[2]	= 0; /* (x << 16) | y; */
		S.data.l[3]	= 0; /* (w << 16) | h; */
		S.data.l[4]	= (xdnd_accept) ? E->data.l[4] : None;

		XSendEvent(x11_display, S.window, False, 0, (XEvent*)&S);
	    }
	}

    } else if (E->message_type == XA_XdndLeave) {
	/*
	  ■■■■ 最後：マウスがウインドウを出た時のイベント ■■■■

	  E->data.l[0]	ソースウィンドウの XID
	  E->data.l[1]	予約
	*/

	if (xdnd_from != (Window)E->data.l[0] ||
	    xdnd_to   !=         E->window) {
	    /* ん？ 違うぞ？ */
	    ;
	} else {
	    xdnd_step = YET;
	}

    } else if (E->message_type == XA_XdndDrop) {
	/*
	  ■■■■ 実行：ウインドウ内でドロップされた時のイベント ■■■■

	  E->data.l[0]	ソースウィンドウの XID
	  E->data.l[1]	予約
	*/

	if (xdnd_from != (Window)E->data.l[0] ||
	    xdnd_to   !=         E->window) {
	    /* ん？ 違うぞ？ */
	    ;
	} else {
	    if (xdnd_step == POS) {
		XConvertSelection(x11_display, XA_XdndSelection, xdnd_type,
				  XV_DND_SELECTION, xdnd_to, E->data.l[2]);

		xdnd_step = DROP;
	    } else {
		if (xdnd_ver >= 5) {
		    XClientMessageEvent S;

		    S.type	= ClientMessage;
		    S.display	= x11_display;
		    S.window	= xdnd_from;
		    S.message_type= XA_XdndFinished;
		    S.format	= 32;
		    S.data.l[0]	= xdnd_to;
		    S.data.l[1]	= 0;
		    S.data.l[2]	= None;
		    S.data.l[3]	= 0;
		    S.data.l[4]	= 0;

		    XSendEvent(x11_display, S.window, False, 0, (XEvent*)&S);
		}
		xdnd_step = YET;
	    }
	}

    } else {
	/* 困った。わからない……… */
	;
    }
}



static	void	xdnd_receive_drop(XSelectionEvent *E)
{
    unsigned long	bytes_after;
    XTextProperty	prop;
    int			ret;

    /*
      ■■■■ 結末：ドロップ後に通知されるイベント ■■■■
    */

    if (E->property == XV_DND_SELECTION) {
	if (xdnd_step == DROP) {

	    ret = XGetWindowProperty(x11_display, E->requestor,
				     E->property, 0L, 0x1fffffffL,
				     True, AnyPropertyType,
				     &prop.encoding, &prop.format,
				     &prop.nitems,
				     &bytes_after, &prop.value);

	    if (ret           == Success &&
		prop.encoding != None    &&
		prop.format   == 8       &&
		prop.nitems   != 0       &&
		prop.value    != NULL) {

		drop_filename((char *)prop.value, (int)prop.nitems);

		XFree(prop.value);
	    }

	    {
		XClientMessageEvent S;
		xdnd_accept = TRUE;

		S.type		= ClientMessage;
		S.display	= x11_display;
		S.window	= xdnd_from;
		S.message_type	= XA_XdndFinished;
		S.format	= 32;
		S.data.l[0]	= xdnd_to;
		S.data.l[1]	= xdnd_accept;
		S.data.l[2]	= xdnd_accept ? XA_XdndActionCopy : None;
		S.data.l[3]	= 0;
		S.data.l[4]	= 0;

		XSendEvent(x11_display, S.window, False, 0, (XEvent*)&S);
	    }

	    xdnd_step = YET;
	}
    }
}



static int drop_filename(const char *buf, int size)
{
    char filename[ OSD_MAX_FILENAME ];
    char *d;
    const char *s;
    int i, len;

#if 0
    const char *c = buf;
    int l = 0;
    
    fprintf(stderr, "XdndSelection: dump property %d bytes.", size);
    while (l < size) {
	if (!(l & 15))
	    fprintf(stderr, "\n  %04x|", l);
	if (*c > ' ' && *c <= '~')
	    fprintf(stderr, "  %c", *c);
	else 
	    fprintf(stderr, " %02x", *c);
	l ++;
	c ++;
    }
    fputc('\n', stderr);
#endif

    s = NULL;
    if        (strncmp(buf, "file://localhost/", 17) == 0) {
	s = &buf[ 16 ];
	size -= 16;
    } else if (strncmp(buf, "file:///",           8) == 0) {
	s = &buf[ 7 ];
	size -= 7;
    } else if (strncmp(buf, "file://",            7) == 0) {
	s = &buf[ 6 ];
	size -= 6;
    } else if (strncmp(buf, "file:/",             6) == 0) {
	s = &buf[ 5 ];
	size -= 5;
    }

    if (s) {
	d = filename;
	len = sizeof(filename)-1;

	while (len && size) {
	    if (*s == '\r' ||
		*s == '\n' ||
		*s == '\0') break;

	    if (*s == '%') {
		if (size <= 2) return 0;
		s ++;
		if      ('0' <= (*s) && (*s) <= '9') i  = (*s) - '0';
		else if ('A' <= (*s) && (*s) <= 'F') i  = (*s) - 'A' + 10;
		else if ('a' <= (*s) && (*s) <= 'f') i  = (*s) - 'a' + 10;
		else return 0;
		i *= 16;
		s ++;
		if      ('0' <= (*s) && (*s) <= '9') i += (*s) - '0';
		else if ('A' <= (*s) && (*s) <= 'F') i += (*s) - 'A' + 10;
		else if ('a' <= (*s) && (*s) <= 'f') i += (*s) - 'a' + 10;
		else return 0;
		size -= 2;
	    } else {
		i = *s;
	    }
	    *d ++ = i; len  --;
	    s ++;      size --;
	}
	*d = '\0';

	/*printf("%s\n", filename);*/

	quasi88_drag_and_drop(filename);

    }
    return 1;
}






/****************************************************************************
 * キー設定ファイルを読み込んで、設定する。
 *	設定ファイルが無ければ偽、あれば処理して真を返す
 *****************************************************************************/

/* X11 の keysym の文字列を int 値に変換するテーブル */

static const T_SYMBOL_TABLE x11keysym_list[] =
{
    {	"XK_space",		XK_space                }, /*	0x020	*/
    {	"XK_exclam",		XK_exclam               }, /*	0x021	*/
    {	"XK_quotedbl",		XK_quotedbl             }, /*	0x022	*/
    {	"XK_numbersign",	XK_numbersign           }, /*	0x023	*/
    {	"XK_dollar",		XK_dollar               }, /*	0x024	*/
    {	"XK_percent",		XK_percent              }, /*	0x025	*/
    {	"XK_ampersand",		XK_ampersand            }, /*	0x026	*/
    {	"XK_apostrophe",	XK_apostrophe           }, /*	0x027	*/
    {	"XK_quoteright",	XK_quoteright           }, /*	0x027	*/
    {	"XK_parenleft",		XK_parenleft            }, /*	0x028	*/
    {	"XK_parenright",	XK_parenright           }, /*	0x029	*/
    {	"XK_asterisk",		XK_asterisk             }, /*	0x02a	*/
    {	"XK_plus",		XK_plus                 }, /*	0x02b	*/
    {	"XK_comma",		XK_comma                }, /*	0x02c	*/
    {	"XK_minus",		XK_minus                }, /*	0x02d	*/
    {	"XK_period",		XK_period               }, /*	0x02e	*/
    {	"XK_slash",		XK_slash                }, /*	0x02f	*/
    {	"XK_0",			XK_0                    }, /*	0x030	*/
    {	"XK_1",			XK_1                    }, /*	0x031	*/
    {	"XK_2",			XK_2                    }, /*	0x032	*/
    {	"XK_3",			XK_3                    }, /*	0x033	*/
    {	"XK_4",			XK_4                    }, /*	0x034	*/
    {	"XK_5",			XK_5                    }, /*	0x035	*/
    {	"XK_6",			XK_6                    }, /*	0x036	*/
    {	"XK_7",			XK_7                    }, /*	0x037	*/
    {	"XK_8",			XK_8                    }, /*	0x038	*/
    {	"XK_9",			XK_9                    }, /*	0x039	*/
    {	"XK_colon",		XK_colon                }, /*	0x03a	*/
    {	"XK_semicolon",		XK_semicolon            }, /*	0x03b	*/
    {	"XK_less",		XK_less                 }, /*	0x03c	*/
    {	"XK_equal",		XK_equal                }, /*	0x03d	*/
    {	"XK_greater",		XK_greater              }, /*	0x03e	*/
    {	"XK_question",		XK_question             }, /*	0x03f	*/
    {	"XK_at",		XK_at                   }, /*	0x040	*/
    {	"XK_A",			XK_A                    }, /*	0x041	*/
    {	"XK_B",			XK_B                    }, /*	0x042	*/
    {	"XK_C",			XK_C                    }, /*	0x043	*/
    {	"XK_D",			XK_D                    }, /*	0x044	*/
    {	"XK_E",			XK_E                    }, /*	0x045	*/
    {	"XK_F",			XK_F                    }, /*	0x046	*/
    {	"XK_G",			XK_G                    }, /*	0x047	*/
    {	"XK_H",			XK_H                    }, /*	0x048	*/
    {	"XK_I",			XK_I                    }, /*	0x049	*/
    {	"XK_J",			XK_J                    }, /*	0x04a	*/
    {	"XK_K",			XK_K                    }, /*	0x04b	*/
    {	"XK_L",			XK_L                    }, /*	0x04c	*/
    {	"XK_M",			XK_M                    }, /*	0x04d	*/
    {	"XK_N",			XK_N                    }, /*	0x04e	*/
    {	"XK_O",			XK_O                    }, /*	0x04f	*/
    {	"XK_P",			XK_P                    }, /*	0x050	*/
    {	"XK_Q",			XK_Q                    }, /*	0x051	*/
    {	"XK_R",			XK_R                    }, /*	0x052	*/
    {	"XK_S",			XK_S                    }, /*	0x053	*/
    {	"XK_T",			XK_T                    }, /*	0x054	*/
    {	"XK_U",			XK_U                    }, /*	0x055	*/
    {	"XK_V",			XK_V                    }, /*	0x056	*/
    {	"XK_W",			XK_W                    }, /*	0x057	*/
    {	"XK_X",			XK_X                    }, /*	0x058	*/
    {	"XK_Y",			XK_Y                    }, /*	0x059	*/
    {	"XK_Z",			XK_Z                    }, /*	0x05a	*/
    {	"XK_bracketleft",	XK_bracketleft          }, /*	0x05b	*/
    {	"XK_backslash",		XK_backslash            }, /*	0x05c	*/
    {	"XK_bracketright",	XK_bracketright         }, /*	0x05d	*/
    {	"XK_asciicircum",	XK_asciicircum          }, /*	0x05e	*/
    {	"XK_underscore",	XK_underscore           }, /*	0x05f	*/
    {	"XK_grave",		XK_grave                }, /*	0x060	*/
    {	"XK_quoteleft",		XK_quoteleft            }, /*	0x060	*/
    {	"XK_a",			XK_a                    }, /*	0x061	*/
    {	"XK_b",			XK_b                    }, /*	0x062	*/
    {	"XK_c",			XK_c                    }, /*	0x063	*/
    {	"XK_d",			XK_d                    }, /*	0x064	*/
    {	"XK_e",			XK_e                    }, /*	0x065	*/
    {	"XK_f",			XK_f                    }, /*	0x066	*/
    {	"XK_g",			XK_g                    }, /*	0x067	*/
    {	"XK_h",			XK_h                    }, /*	0x068	*/
    {	"XK_i",			XK_i                    }, /*	0x069	*/
    {	"XK_j",			XK_j                    }, /*	0x06a	*/
    {	"XK_k",			XK_k                    }, /*	0x06b	*/
    {	"XK_l",			XK_l                    }, /*	0x06c	*/
    {	"XK_m",			XK_m                    }, /*	0x06d	*/
    {	"XK_n",			XK_n                    }, /*	0x06e	*/
    {	"XK_o",			XK_o                    }, /*	0x06f	*/
    {	"XK_p",			XK_p                    }, /*	0x070	*/
    {	"XK_q",			XK_q                    }, /*	0x071	*/
    {	"XK_r",			XK_r                    }, /*	0x072	*/
    {	"XK_s",			XK_s                    }, /*	0x073	*/
    {	"XK_t",			XK_t                    }, /*	0x074	*/
    {	"XK_u",			XK_u                    }, /*	0x075	*/
    {	"XK_v",			XK_v                    }, /*	0x076	*/
    {	"XK_w",			XK_w                    }, /*	0x077	*/
    {	"XK_x",			XK_x                    }, /*	0x078	*/
    {	"XK_y",			XK_y                    }, /*	0x079	*/
    {	"XK_z",			XK_z                    }, /*	0x07a	*/
    {	"XK_braceleft",		XK_braceleft            }, /*	0x07b	*/
    {	"XK_bar",		XK_bar                  }, /*	0x07c	*/
    {	"XK_braceright",	XK_braceright           }, /*	0x07d	*/
    {	"XK_asciitilde",	XK_asciitilde           }, /*	0x07e	*/
    {	"XK_nobreakspace",	XK_nobreakspace         }, /*	0x0a0	*/
    {	"XK_exclamdown",	XK_exclamdown           }, /*	0x0a1	*/
    {	"XK_cent",		XK_cent                 }, /*	0x0a2	*/
    {	"XK_sterling",		XK_sterling             }, /*	0x0a3	*/
    {	"XK_currency",		XK_currency             }, /*	0x0a4	*/
    {	"XK_yen",		XK_yen                  }, /*	0x0a5	*/
    {	"XK_brokenbar",		XK_brokenbar            }, /*	0x0a6	*/
    {	"XK_section",		XK_section              }, /*	0x0a7	*/
    {	"XK_diaeresis",		XK_diaeresis            }, /*	0x0a8	*/
    {	"XK_copyright",		XK_copyright            }, /*	0x0a9	*/
    {	"XK_ordfeminine",	XK_ordfeminine          }, /*	0x0aa	*/
    {	"XK_guillemotleft",	XK_guillemotleft        }, /*	0x0ab	*/
    {	"XK_notsign",		XK_notsign              }, /*	0x0ac	*/
    {	"XK_hyphen",		XK_hyphen               }, /*	0x0ad	*/
    {	"XK_registered",	XK_registered           }, /*	0x0ae	*/
    {	"XK_macron",		XK_macron               }, /*	0x0af	*/
    {	"XK_degree",		XK_degree               }, /*	0x0b0	*/
    {	"XK_plusminus",		XK_plusminus            }, /*	0x0b1	*/
    {	"XK_twosuperior",	XK_twosuperior          }, /*	0x0b2	*/
    {	"XK_threesuperior",	XK_threesuperior        }, /*	0x0b3	*/
    {	"XK_acute",		XK_acute                }, /*	0x0b4	*/
    {	"XK_mu",		XK_mu                   }, /*	0x0b5	*/
    {	"XK_paragraph",		XK_paragraph            }, /*	0x0b6	*/
    {	"XK_periodcentered",	XK_periodcentered       }, /*	0x0b7	*/
    {	"XK_cedilla",		XK_cedilla              }, /*	0x0b8	*/
    {	"XK_onesuperior",	XK_onesuperior          }, /*	0x0b9	*/
    {	"XK_masculine",		XK_masculine            }, /*	0x0ba	*/
    {	"XK_guillemotright",	XK_guillemotright       }, /*	0x0bb	*/
    {	"XK_onequarter",	XK_onequarter           }, /*	0x0bc	*/
    {	"XK_onehalf",		XK_onehalf              }, /*	0x0bd	*/
    {	"XK_threequarters",	XK_threequarters        }, /*	0x0be	*/
    {	"XK_questiondown",	XK_questiondown         }, /*	0x0bf	*/
    {	"XK_Agrave",		XK_Agrave               }, /*	0x0c0	*/
    {	"XK_Aacute",		XK_Aacute               }, /*	0x0c1	*/
    {	"XK_Acircumflex",	XK_Acircumflex          }, /*	0x0c2	*/
    {	"XK_Atilde",		XK_Atilde               }, /*	0x0c3	*/
    {	"XK_Adiaeresis",	XK_Adiaeresis           }, /*	0x0c4	*/
    {	"XK_Aring",		XK_Aring                }, /*	0x0c5	*/
    {	"XK_AE",		XK_AE                   }, /*	0x0c6	*/
    {	"XK_Ccedilla",		XK_Ccedilla             }, /*	0x0c7	*/
    {	"XK_Egrave",		XK_Egrave               }, /*	0x0c8	*/
    {	"XK_Eacute",		XK_Eacute               }, /*	0x0c9	*/
    {	"XK_Ecircumflex",	XK_Ecircumflex          }, /*	0x0ca	*/
    {	"XK_Ediaeresis",	XK_Ediaeresis           }, /*	0x0cb	*/
    {	"XK_Igrave",		XK_Igrave               }, /*	0x0cc	*/
    {	"XK_Iacute",		XK_Iacute               }, /*	0x0cd	*/
    {	"XK_Icircumflex",	XK_Icircumflex          }, /*	0x0ce	*/
    {	"XK_Idiaeresis",	XK_Idiaeresis           }, /*	0x0cf	*/
    {	"XK_ETH",		XK_ETH                  }, /*	0x0d0	*/
    {	"XK_Eth",		XK_Eth                  }, /*	0x0d0	*/
    {	"XK_Ntilde",		XK_Ntilde               }, /*	0x0d1	*/
    {	"XK_Ograve",		XK_Ograve               }, /*	0x0d2	*/
    {	"XK_Oacute",		XK_Oacute               }, /*	0x0d3	*/
    {	"XK_Ocircumflex",	XK_Ocircumflex          }, /*	0x0d4	*/
    {	"XK_Otilde",		XK_Otilde               }, /*	0x0d5	*/
    {	"XK_Odiaeresis",	XK_Odiaeresis           }, /*	0x0d6	*/
    {	"XK_multiply",		XK_multiply             }, /*	0x0d7	*/
    {	"XK_Ooblique",		XK_Ooblique             }, /*	0x0d8	*/
    {	"XK_Ugrave",		XK_Ugrave               }, /*	0x0d9	*/
    {	"XK_Uacute",		XK_Uacute               }, /*	0x0da	*/
    {	"XK_Ucircumflex",	XK_Ucircumflex          }, /*	0x0db	*/
    {	"XK_Udiaeresis",	XK_Udiaeresis           }, /*	0x0dc	*/
    {	"XK_Yacute",		XK_Yacute               }, /*	0x0dd	*/
    {	"XK_THORN",		XK_THORN                }, /*	0x0de	*/
    {	"XK_Thorn",		XK_Thorn                }, /*	0x0de	*/
    {	"XK_ssharp",		XK_ssharp               }, /*	0x0df	*/
    {	"XK_agrave",		XK_agrave               }, /*	0x0e0	*/
    {	"XK_aacute",		XK_aacute               }, /*	0x0e1	*/
    {	"XK_acircumflex",	XK_acircumflex          }, /*	0x0e2	*/
    {	"XK_atilde",		XK_atilde               }, /*	0x0e3	*/
    {	"XK_adiaeresis",	XK_adiaeresis           }, /*	0x0e4	*/
    {	"XK_aring",		XK_aring                }, /*	0x0e5	*/
    {	"XK_ae",		XK_ae                   }, /*	0x0e6	*/
    {	"XK_ccedilla",		XK_ccedilla             }, /*	0x0e7	*/
    {	"XK_egrave",		XK_egrave               }, /*	0x0e8	*/
    {	"XK_eacute",		XK_eacute               }, /*	0x0e9	*/
    {	"XK_ecircumflex",	XK_ecircumflex          }, /*	0x0ea	*/
    {	"XK_ediaeresis",	XK_ediaeresis           }, /*	0x0eb	*/
    {	"XK_igrave",		XK_igrave               }, /*	0x0ec	*/
    {	"XK_iacute",		XK_iacute               }, /*	0x0ed	*/
    {	"XK_icircumflex",	XK_icircumflex          }, /*	0x0ee	*/
    {	"XK_idiaeresis",	XK_idiaeresis           }, /*	0x0ef	*/
    {	"XK_eth",		XK_eth                  }, /*	0x0f0	*/
    {	"XK_ntilde",		XK_ntilde               }, /*	0x0f1	*/
    {	"XK_ograve",		XK_ograve               }, /*	0x0f2	*/
    {	"XK_oacute",		XK_oacute               }, /*	0x0f3	*/
    {	"XK_ocircumflex",	XK_ocircumflex          }, /*	0x0f4	*/
    {	"XK_otilde",		XK_otilde               }, /*	0x0f5	*/
    {	"XK_odiaeresis",	XK_odiaeresis           }, /*	0x0f6	*/
    {	"XK_division",		XK_division             }, /*	0x0f7	*/
    {	"XK_oslash",		XK_oslash               }, /*	0x0f8	*/
    {	"XK_ugrave",		XK_ugrave               }, /*	0x0f9	*/
    {	"XK_uacute",		XK_uacute               }, /*	0x0fa	*/
    {	"XK_ucircumflex",	XK_ucircumflex          }, /*	0x0fb	*/
    {	"XK_udiaeresis",	XK_udiaeresis           }, /*	0x0fc	*/
    {	"XK_yacute",		XK_yacute               }, /*	0x0fd	*/
    {	"XK_thorn",		XK_thorn                }, /*	0x0fe	*/
    {	"XK_ydiaeresis",	XK_ydiaeresis           }, /*	0x0ff	*/

    {	"XK_BackSpace",		XK_BackSpace            }, /*	0xFF08	*/
    {	"XK_Tab",		XK_Tab                  }, /*	0xFF09	*/
    {	"XK_Linefeed",		XK_Linefeed             }, /*	0xFF0A	*/
    {	"XK_Clear",		XK_Clear                }, /*	0xFF0B	*/
    {	"XK_Return",		XK_Return               }, /*	0xFF0D	*/
    {	"XK_Pause",		XK_Pause                }, /*	0xFF13	*/
    {	"XK_Scroll_Lock",	XK_Scroll_Lock          }, /*	0xFF14	*/
    {	"XK_Sys_Req",		XK_Sys_Req              }, /*	0xFF15	*/
    {	"XK_Escape",		XK_Escape               }, /*	0xFF1B	*/
    {	"XK_Delete",		XK_Delete               }, /*	0xFFFF	*/
    {	"XK_Multi_key",		XK_Multi_key            }, /*	0xFF20	*/
    {	"XK_Codeinput",		XK_Codeinput            }, /*	0xFF37	*/
    {	"XK_SingleCandidate",	XK_SingleCandidate      }, /*	0xFF3C	*/
    {	"XK_MultipleCandidate",	XK_MultipleCandidate    }, /*	0xFF3D	*/
    {	"XK_PreviousCandidate",	XK_PreviousCandidate    }, /*	0xFF3E	*/
    {	"XK_Kanji",		XK_Kanji                }, /*	0xFF21	*/
    {	"XK_Muhenkan",		XK_Muhenkan             }, /*	0xFF22	*/
    {	"XK_Henkan_Mode",	XK_Henkan_Mode          }, /*	0xFF23	*/
    {	"XK_Henkan",		XK_Henkan               }, /*	0xFF23	*/
    {	"XK_Romaji",		XK_Romaji               }, /*	0xFF24	*/
    {	"XK_Hiragana",		XK_Hiragana             }, /*	0xFF25	*/
    {	"XK_Katakana",		XK_Katakana             }, /*	0xFF26	*/
    {	"XK_Hiragana_Katakana",	XK_Hiragana_Katakana    }, /*	0xFF27	*/
    {	"XK_Zenkaku",		XK_Zenkaku              }, /*	0xFF28	*/
    {	"XK_Hankaku",		XK_Hankaku              }, /*	0xFF29	*/
    {	"XK_Zenkaku_Hankaku",	XK_Zenkaku_Hankaku      }, /*	0xFF2A	*/
    {	"XK_Touroku",		XK_Touroku              }, /*	0xFF2B	*/
    {	"XK_Massyo",		XK_Massyo               }, /*	0xFF2C	*/
    {	"XK_Kana_Lock",		XK_Kana_Lock            }, /*	0xFF2D	*/
    {	"XK_Kana_Shift",	XK_Kana_Shift           }, /*	0xFF2E	*/
    {	"XK_Eisu_Shift",	XK_Eisu_Shift           }, /*	0xFF2F	*/
    {	"XK_Eisu_toggle",	XK_Eisu_toggle          }, /*	0xFF30	*/
    {	"XK_Kanji_Bangou",	XK_Kanji_Bangou         }, /*	0xFF37	*/
    {	"XK_Zen_Koho",		XK_Zen_Koho             }, /*	0xFF3D	*/
    {	"XK_Mae_Koho",		XK_Mae_Koho             }, /*	0xFF3E	*/
    {	"XK_Home",		XK_Home                 }, /*	0xFF50	*/
    {	"XK_Left",		XK_Left                 }, /*	0xFF51	*/
    {	"XK_Up",		XK_Up                   }, /*	0xFF52	*/
    {	"XK_Right",		XK_Right                }, /*	0xFF53	*/
    {	"XK_Down",		XK_Down                 }, /*	0xFF54	*/
    {	"XK_Prior",		XK_Prior                }, /*	0xFF55	*/
    {	"XK_Page_Up",		XK_Page_Up              }, /*	0xFF55	*/
    {	"XK_Next",		XK_Next                 }, /*	0xFF56	*/
    {	"XK_Page_Down",		XK_Page_Down            }, /*	0xFF56	*/
    {	"XK_End",		XK_End                  }, /*	0xFF57	*/
    {	"XK_Begin",		XK_Begin                }, /*	0xFF58	*/
    {	"XK_Select",		XK_Select               }, /*	0xFF60	*/
    {	"XK_Print",		XK_Print                }, /*	0xFF61	*/
    {	"XK_Execute",		XK_Execute              }, /*	0xFF62	*/
    {	"XK_Insert",		XK_Insert               }, /*	0xFF63	*/
    {	"XK_Undo",		XK_Undo                 }, /*	0xFF65	*/
    {	"XK_Redo",		XK_Redo                 }, /*	0xFF66	*/
    {	"XK_Menu",		XK_Menu                 }, /*	0xFF67	*/
    {	"XK_Find",		XK_Find                 }, /*	0xFF68	*/
    {	"XK_Cancel",		XK_Cancel               }, /*	0xFF69	*/
    {	"XK_Help",		XK_Help                 }, /*	0xFF6A	*/
    {	"XK_Break",		XK_Break                }, /*	0xFF6B	*/
    {	"XK_Mode_switch",	XK_Mode_switch          }, /*	0xFF7E	*/
    {	"XK_script_switch",	XK_script_switch        }, /*	0xFF7E	*/
    {	"XK_Num_Lock",		XK_Num_Lock             }, /*	0xFF7F	*/
    {	"XK_KP_Space",		XK_KP_Space             }, /*	0xFF80	*/
    {	"XK_KP_Tab",		XK_KP_Tab               }, /*	0xFF89	*/
    {	"XK_KP_Enter",		XK_KP_Enter             }, /*	0xFF8D	*/
    {	"XK_KP_F1",		XK_KP_F1                }, /*	0xFF91	*/
    {	"XK_KP_F2",		XK_KP_F2                }, /*	0xFF92	*/
    {	"XK_KP_F3",		XK_KP_F3                }, /*	0xFF93	*/
    {	"XK_KP_F4",		XK_KP_F4                }, /*	0xFF94	*/
    {	"XK_KP_Home",		XK_KP_Home              }, /*	0xFF95	*/
    {	"XK_KP_Left",		XK_KP_Left              }, /*	0xFF96	*/
    {	"XK_KP_Up",		XK_KP_Up                }, /*	0xFF97	*/
    {	"XK_KP_Right",		XK_KP_Right             }, /*	0xFF98	*/
    {	"XK_KP_Down",		XK_KP_Down              }, /*	0xFF99	*/
    {	"XK_KP_Prior",		XK_KP_Prior             }, /*	0xFF9A	*/
    {	"XK_KP_Page_Up",	XK_KP_Page_Up           }, /*	0xFF9A	*/
    {	"XK_KP_Next",		XK_KP_Next              }, /*	0xFF9B	*/
    {	"XK_KP_Page_Down",	XK_KP_Page_Down         }, /*	0xFF9B	*/
    {	"XK_KP_End",		XK_KP_End               }, /*	0xFF9C	*/
    {	"XK_KP_Begin",		XK_KP_Begin             }, /*	0xFF9D	*/
    {	"XK_KP_Insert",		XK_KP_Insert            }, /*	0xFF9E	*/
    {	"XK_KP_Delete",		XK_KP_Delete            }, /*	0xFF9F	*/
    {	"XK_KP_Equal",		XK_KP_Equal             }, /*	0xFFBD	*/
    {	"XK_KP_Multiply",	XK_KP_Multiply          }, /*	0xFFAA	*/
    {	"XK_KP_Add",		XK_KP_Add               }, /*	0xFFAB	*/
    {	"XK_KP_Separator",	XK_KP_Separator         }, /*	0xFFAC	*/
    {	"XK_KP_Subtract",	XK_KP_Subtract          }, /*	0xFFAD	*/
    {	"XK_KP_Decimal",	XK_KP_Decimal           }, /*	0xFFAE	*/
    {	"XK_KP_Divide",		XK_KP_Divide            }, /*	0xFFAF	*/
    {	"XK_KP_0",		XK_KP_0                 }, /*	0xFFB0	*/
    {	"XK_KP_1",		XK_KP_1                 }, /*	0xFFB1	*/
    {	"XK_KP_2",		XK_KP_2                 }, /*	0xFFB2	*/
    {	"XK_KP_3",		XK_KP_3                 }, /*	0xFFB3	*/
    {	"XK_KP_4",		XK_KP_4                 }, /*	0xFFB4	*/
    {	"XK_KP_5",		XK_KP_5                 }, /*	0xFFB5	*/
    {	"XK_KP_6",		XK_KP_6                 }, /*	0xFFB6	*/
    {	"XK_KP_7",		XK_KP_7                 }, /*	0xFFB7	*/
    {	"XK_KP_8",		XK_KP_8                 }, /*	0xFFB8	*/
    {	"XK_KP_9",		XK_KP_9                 }, /*	0xFFB9	*/
    {	"XK_F1",		XK_F1                   }, /*	0xFFBE	*/
    {	"XK_F2",		XK_F2                   }, /*	0xFFBF	*/
    {	"XK_F3",		XK_F3                   }, /*	0xFFC0	*/
    {	"XK_F4",		XK_F4                   }, /*	0xFFC1	*/
    {	"XK_F5",		XK_F5                   }, /*	0xFFC2	*/
    {	"XK_F6",		XK_F6                   }, /*	0xFFC3	*/
    {	"XK_F7",		XK_F7                   }, /*	0xFFC4	*/
    {	"XK_F8",		XK_F8                   }, /*	0xFFC5	*/
    {	"XK_F9",		XK_F9                   }, /*	0xFFC6	*/
    {	"XK_F10",		XK_F10                  }, /*	0xFFC7	*/
    {	"XK_F11",		XK_F11                  }, /*	0xFFC8	*/
    {	"XK_L1",		XK_L1                   }, /*	0xFFC8	*/
    {	"XK_F12",		XK_F12                  }, /*	0xFFC9	*/
    {	"XK_L2",		XK_L2                   }, /*	0xFFC9	*/
    {	"XK_F13",		XK_F13                  }, /*	0xFFCA	*/
    {	"XK_L3",		XK_L3                   }, /*	0xFFCA	*/
    {	"XK_F14",		XK_F14                  }, /*	0xFFCB	*/
    {	"XK_L4",		XK_L4                   }, /*	0xFFCB	*/
    {	"XK_F15",		XK_F15                  }, /*	0xFFCC	*/
    {	"XK_L5",		XK_L5                   }, /*	0xFFCC	*/
    {	"XK_F16",		XK_F16                  }, /*	0xFFCD	*/
    {	"XK_L6",		XK_L6                   }, /*	0xFFCD	*/
    {	"XK_F17",		XK_F17                  }, /*	0xFFCE	*/
    {	"XK_L7",		XK_L7                   }, /*	0xFFCE	*/
    {	"XK_F18",		XK_F18                  }, /*	0xFFCF	*/
    {	"XK_L8",		XK_L8                   }, /*	0xFFCF	*/
    {	"XK_F19",		XK_F19                  }, /*	0xFFD0	*/
    {	"XK_L9",		XK_L9                   }, /*	0xFFD0	*/
    {	"XK_F20",		XK_F20                  }, /*	0xFFD1	*/
    {	"XK_L10",		XK_L10                  }, /*	0xFFD1	*/
    {	"XK_F21",		XK_F21                  }, /*	0xFFD2	*/
    {	"XK_R1",		XK_R1                   }, /*	0xFFD2	*/
    {	"XK_F22",		XK_F22                  }, /*	0xFFD3	*/
    {	"XK_R2",		XK_R2                   }, /*	0xFFD3	*/
    {	"XK_F23",		XK_F23                  }, /*	0xFFD4	*/
    {	"XK_R3",		XK_R3                   }, /*	0xFFD4	*/
    {	"XK_F24",		XK_F24                  }, /*	0xFFD5	*/
    {	"XK_R4",		XK_R4                   }, /*	0xFFD5	*/
    {	"XK_F25",		XK_F25                  }, /*	0xFFD6	*/
    {	"XK_R5",		XK_R5                   }, /*	0xFFD6	*/
    {	"XK_F26",		XK_F26                  }, /*	0xFFD7	*/
    {	"XK_R6",		XK_R6                   }, /*	0xFFD7	*/
    {	"XK_F27",		XK_F27                  }, /*	0xFFD8	*/
    {	"XK_R7",		XK_R7                   }, /*	0xFFD8	*/
    {	"XK_F28",		XK_F28                  }, /*	0xFFD9	*/
    {	"XK_R8",		XK_R8                   }, /*	0xFFD9	*/
    {	"XK_F29",		XK_F29                  }, /*	0xFFDA	*/
    {	"XK_R9",		XK_R9                   }, /*	0xFFDA	*/
    {	"XK_F30",		XK_F30                  }, /*	0xFFDB	*/
    {	"XK_R10",		XK_R10                  }, /*	0xFFDB	*/
    {	"XK_F31",		XK_F31                  }, /*	0xFFDC	*/
    {	"XK_R11",		XK_R11                  }, /*	0xFFDC	*/
    {	"XK_F32",		XK_F32                  }, /*	0xFFDD	*/
    {	"XK_R12",		XK_R12                  }, /*	0xFFDD	*/
    {	"XK_F33",		XK_F33                  }, /*	0xFFDE	*/
    {	"XK_R13",		XK_R13                  }, /*	0xFFDE	*/
    {	"XK_F34",		XK_F34                  }, /*	0xFFDF	*/
    {	"XK_R14",		XK_R14                  }, /*	0xFFDF	*/
    {	"XK_F35",		XK_F35                  }, /*	0xFFE0	*/
    {	"XK_R15",		XK_R15                  }, /*	0xFFE0	*/
    {	"XK_Shift_L",		XK_Shift_L              }, /*	0xFFE1	*/
    {	"XK_Shift_R",		XK_Shift_R              }, /*	0xFFE2	*/
    {	"XK_Control_L",		XK_Control_L            }, /*	0xFFE3	*/
    {	"XK_Control_R",		XK_Control_R            }, /*	0xFFE4	*/
    {	"XK_Caps_Lock",		XK_Caps_Lock            }, /*	0xFFE5	*/
    {	"XK_Shift_Lock",	XK_Shift_Lock           }, /*	0xFFE6	*/
    {	"XK_Meta_L",		XK_Meta_L               }, /*	0xFFE7	*/
    {	"XK_Meta_R",		XK_Meta_R               }, /*	0xFFE8	*/
    {	"XK_Alt_L",		XK_Alt_L                }, /*	0xFFE9	*/
    {	"XK_Alt_R",		XK_Alt_R                }, /*	0xFFEA	*/
    {	"XK_Super_L",		XK_Super_L              }, /*	0xFFEB	*/
    {	"XK_Super_R",		XK_Super_R              }, /*	0xFFEC	*/
    {	"XK_Hyper_L",		XK_Hyper_L              }, /*	0xFFED	*/
    {	"XK_Hyper_R",		XK_Hyper_R              }, /*	0xFFEE	*/
};
/* デバッグ用 */
static	const char *debug_x11keysym(int code)
{
    int i;
    for (i=0; i<COUNTOF(x11keysym_list); i++) {
	if (code == x11keysym_list[i].val)
	    return x11keysym_list[i].name;
    }
    return "invalid";
}


/* キー設定ファイルの、識別タグをチェックするコールバック関数 */

static const char *identify_callback(const char *parm1,
				     const char *parm2,
				     const char *parm3)
{
    if (my_strcmp(parm1, "[X11]") == 0) {
	return NULL;				/* 有効 */
    }

    return "";					/* 無効 */
}

/* キー設定ファイルの、設定を処理するコールバック関数 */

static const char *setting_callback(int type,
				    int code,
				    int key88,
				    int numlock_key88)
{
    static int binding_cnt = 0;

    if (type == KEYCODE_SCAN) {
	if (code >= COUNTOF(scancode2key88)) {
	    return "scancode too large";	/* 無効 */
	}
	scancode2key88[ code ] = key88;
    } else {
	keysym2key88[ LOCAL_KEYSYM(code) ] = key88;
    }

    if (numlock_key88 >= 0) {
	if (binding_cnt >= COUNTOF(binding)) {
	    return "too many NumLock-code";	/* 無効 */
	}
	binding[ binding_cnt ].type      = type;
	binding[ binding_cnt ].code      = code;
	binding[ binding_cnt ].new_key88 = numlock_key88;
	binding_cnt ++;
    }

    return NULL;				/* 有効 */
}

/* キー設定ファイルの処理関数 */

static	int	analyze_keyconf_file(void)
{
    return
	config_read_keyconf_file(file_keyboard,		  /* キー設定ファイル*/
				 identify_callback,	  /* 識別タグ行 関数 */
				 x11keysym_list,	  /* 変換テーブル    */
				 COUNTOF(x11keysym_list), /* テーブルサイズ  */
				 TRUE,			  /* 大小文字無視    */
				 setting_callback);	  /* 設定行 関数     */
}
