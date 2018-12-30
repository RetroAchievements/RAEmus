/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/

/*----------------------------------------------------------------------*
 * Classicバージョンのソースコードの大部分は、                          *
 * Koichi NISHIDA 氏の Classic iP6 PC-6001/mk2/6601 emulator のソースを *
 * 参考にさせていただきました。                                         *
 *                                                   (c) Koichi NISHIDA *
 *----------------------------------------------------------------------*/

#include "device.h"
#include <stdio.h>

#include "quasi88.h"
#include "keyboard.h"

#include "emu.h"
#include "graph.h"
#include "event.h"
#include "screen.h"
#include "snddrv.h"


/*
                    +---------+
                    | Power   |
                    +---------+
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ +---+---+---+---+
|ESC|1  |2  |3  |4  |5  |6  |7  |8  |9  |0  |-  |^  |\  |DEL| Clear=  |/  |*  |
+----+---+---+---+---+---+---+---+---+---+---+---+---+------+ +---+---+---+---+
|TAB |Q  |W  |E  |R  |T  |Y  |U  |I  |O  |P  |@  |[  |  Ret | |7  |8  |9  |-  |
+-----+---+---+---+---+---+---+---+---+---+---+---+---+ urn | +---+---+---+---+
|CTRL |A  |S  |D  |F  |G  |H  |J  |K  |L  |;  |:  |]  |     | |4  |5  |6  |+  |
+------+---+---+---+---+---+---+---+---+---+---+---+--------+ +---+---+---+---+
|SHIFT |Z  |X  |C  |V  |B  |N  |M  |,  |.  |/  |_  |   SHIFT| |1  |2  |3  |Ent|
+----+------+---+----+-----------+----+-----+---+---+---+---+ +---+---+---+ er|
|Caps|Option|Cmd|英数|           |カナ|Enter|← |→ |↓ |↑ | |0  |,  |.  |   |
+----+------+---+----+-----------+----+-----+---+---+---+---+ +---+---+---+---+

                    +---------+
                    |       7F|
                    +---------+
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ +---+---+---+---+
| 35| 12| 13| 14| 15| 17| 16| 1A| 1C| 19| 1D| 1B| 18| 5D| 33| | 47| 51| 4B| 43|
+----+---+---+---+---+---+---+---+---+---+---+---+---+------+ +---+---+---+---+
|  30| 0C| 0D| 0E| 0F| 11| 10| 20| 22| 1F| 23| 21| 1E|    24| | 59| 5B| 5C| 4E|
+-----+---+---+---+---+---+---+---+---+---+---+---+---+     | +---+---+---+---+
|   36| 00| 01| 02| 03| 05| 04| 26| 28| 25| 29| 27| 2A|     | | 56| 57| 58| 45|
+------+---+---+---+---+---+---+---+---+---+---+---+--------+ +---+---+---+---+
|    38| 06| 07| 08| 09| 0B| 2D| 2E| 2B| 2F| 2C| 5E|      38| | 53| 54| 55| 4C|
+----+------+---+----+-----------+----+-----+---+---+---+---+ +---+---+---+   |
|  39|    3A| 37|  66|         31|  68|   4C| 3B| 3C| 3D| 3E| | 52| 5F| 41|   |
+----+------+---+----+-----------+----+-----+---+---+---+---+ +---+---+---+---+

Assign? ... help, stop, copy, rollup, rolldown, f1..f5


<extended type>
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ +---+---+---+
| 7A| 78| 63| 76| 60| 61| 62| 64| 65| 6D| 67| 6F|   |   |   | | 69| 6B| 71|
|F1 |F2 |F3 |F4 |F5 |F6 |F7 |F8 |F9 |F10|F11|F12|F13|F14|F15| |Prn|SLk|Pus|
+---+---+---+---+---+---+---+---+---+---+---+---+---+---+---+ +---+---+---+
+---+ +----+ +-----+                                +---+     +---+---+---+
| 32| |  3B| |   34|                                | 7E|     | 72| 73| 74|
|`  | |CTRL| |Enter|                                |↑ |     |Ins|Hom|Pup|
+---+ +----+ +-----+                            +---+---+---+ +---+---+---+
                                                | 7B| 7D| 7C| | 75| 77| 79|
                                                |← |↓ |→ | |Del|End|Pdn|
                                                +---+---+---+ +---+---+---+
*/							 


/*----------------------------------------------------------------------
 * MAC のキーコードを QUASI88 の キーコードに変換するテーブル
 *	mac2keycode[][0] はエミュレート時
 *	mac2keycode[][1] はメニュー時 かつ SHIFT押下中
 *----------------------------------------------------------------------*/
static const int mac2keycode[128][2] =
{
  { KEY88_a,		KEY88_A,	},	/* 0x00			*/
  { KEY88_s,		KEY88_S,	},	/* 0x01			*/
  { KEY88_d,		KEY88_D,	},	/* 0x02			*/
  { KEY88_f,		KEY88_F,	},	/* 0x03			*/
  { KEY88_h,		KEY88_H,	},	/* 0x04			*/
  { KEY88_g,		KEY88_G,	},	/* 0x05			*/
  { KEY88_z,		KEY88_Z,	},	/* 0x06			*/
  { KEY88_x,		KEY88_X,	},	/* 0x07			*/
  { KEY88_c,		KEY88_C,	},	/* 0x08			*/
  { KEY88_v,		KEY88_V,	},	/* 0x09			*/
  { 0,			0,		},	/*			*/
  { KEY88_b,		KEY88_B,	},	/* 0x0B			*/
  { KEY88_q,		KEY88_Q,	},	/* 0x0C			*/
  { KEY88_w,		KEY88_W,	},	/* 0x0D			*/
  { KEY88_e,		KEY88_E,	},	/* 0x0E			*/
  { KEY88_r,		KEY88_R,	},	/* 0x0F			*/
  { KEY88_y,		KEY88_Y,	},	/* 0x10			*/
  { KEY88_t,		KEY88_T,	},	/* 0x11			*/
  { KEY88_1,		KEY88_EXCLAM,	},	/* 0x12			*/
  { KEY88_2,		KEY88_QUOTEDBL,	},	/* 0x13			*/
  { KEY88_3,		KEY88_NUMBERSIGN},	/* 0x14			*/
  { KEY88_4,		KEY88_DOLLAR,	},	/* 0x15			*/
  { KEY88_6,		KEY88_AMPERSAND,},	/* 0x16			*/
  { KEY88_5,		KEY88_PERCENT,	},	/* 0x17			*/
  { KEY88_CARET,	KEY88_TILDE,	},	/* 0x18			*/
  { KEY88_9,		KEY88_PARENRIGHT},	/* 0x19			*/
  { KEY88_7,		KEY88_APOSTROPHE},	/* 0x1A			*/
  { KEY88_MINUS,	KEY88_EQUAL,	},	/* 0x1B			*/
  { KEY88_8,		KEY88_PARENLEFT,},	/* 0x1C			*/
  { KEY88_0,		KEY88_0,	},	/* 0x1D			*/
  { KEY88_BRACKETLEFT,	KEY88_BRACELEFT	},	/* 0x1E			*/
  { KEY88_o,		KEY88_O,	},	/* 0x1F			*/
  { KEY88_u,		KEY88_U,	},	/* 0x20			*/
  { KEY88_AT,		KEY88_BACKQUOTE	},	/* 0x21			*/
  { KEY88_i,		KEY88_I,	},	/* 0x22			*/
  { KEY88_p,		KEY88_P,	},	/* 0x23			*/
  { KEY88_RETURN,	KEY88_RETURN,	},	/* 0x24			*/
  { KEY88_l,		KEY88_L,	},	/* 0x25			*/
  { KEY88_j,		KEY88_J,	},	/* 0x26			*/
  { KEY88_COLON,	KEY88_ASTERISK,	},	/* 0x27			*/
  { KEY88_k,		KEY88_K,	},	/* 0x28			*/
  { KEY88_SEMICOLON,	KEY88_PLUS,	},	/* 0x29			*/
  { KEY88_BRACKETRIGHT,	KEY88_BRACERIGHT},	/* 0x2A			*/
  { KEY88_COMMA,	KEY88_LESS,	},	/* 0x2B			*/
  { KEY88_SLASH,	KEY88_QUESTION,	},	/* 0x2C			*/
  { KEY88_n,		KEY88_N,	},	/* 0x2D			*/
  { KEY88_m,		KEY88_M,	},	/* 0x2E			*/
  { KEY88_PERIOD,	KEY88_GREATER,	},	/* 0x2F			*/
  { KEY88_TAB,		KEY88_TAB,	},	/* 0x30			*/
  { KEY88_SPACE,	KEY88_SPACE,	},	/* 0x31			*/
  { 0,			0,		},	/* 0x32	` ~		*/
  { KEY88_INS_DEL,	KEY88_INS_DEL,	},	/* 0x33			*/
  { KEY88_RETURNR,	KEY88_RETURNR,	},	/* 0x34	enter ?		*/
  { KEY88_ESC,		KEY88_ESC,	},	/* 0x35			*/
  { KEY88_CTRL,		0,		},	/* 0x36	ctrl		*/
  { 0,			0,		},	/* 0x37	cmd		*/
  { KEY88_SHIFT,	0,		},	/* 0x38	shift		*/
  { 0 /*KEY88_CAPS*/,	0,		},	/* 0x39	capslock	*/
  { KEY88_GRAPH,	0,		},	/* 0x3A	option		*/
  { KEY88_CTRL,		0,		},	/* 0x3B left ?		*/
  { KEY88_RIGHT,	KEY88_RIGHT,	},	/* 0x3C	right ?		*/
  { KEY88_DOWN,		KEY88_DOWN,	},	/* 0x3D	down ?		*/
  { KEY88_UP,		KEY88_UP,	},	/* 0x3E	up ?		*/
  { 0,			0,		},	/*			*/
  { 0,			0,		},	/*			*/
  { KEY88_KP_PERIOD,	KEY88_KP_PERIOD,},	/* 0x41			*/
  { 0,			0,		},	/*			*/
  { KEY88_KP_MULTIPLY,	KEY88_KP_MULTIPLY},	/* 0x43			*/
  { 0,			0,		},	/*			*/
  { KEY88_KP_ADD,	KEY88_KP_ADD,	},	/* 0x45			*/
  { 0,			0,		},	/*			*/
  { 0,			0,		},	/* 0x47	clear		*/
  { 0,			0,		},	/*			*/
  { 0,			0,		},	/*			*/
  { 0,			0,		},	/*			*/
  { KEY88_KP_DIVIDE,	KEY88_KP_DIVIDE,},	/* 0x4B			*/
  { KEY88_RETURNR,	KEY88_RETURNR,	},	/* 0x4C	enter		*/
  { 0,			0,		},	/*			*/
  { KEY88_KP_SUB,	KEY88_KP_SUB,	},	/* 0x4E			*/
  { 0,			0,		},	/*			*/
  { 0,			0,		},	/*			*/
  { KEY88_KP_EQUAL,	KEY88_KP_EQUAL,	},	/* 0x51			*/
  { KEY88_KP_0,		KEY88_KP_0,	},	/* 0x52			*/
  { KEY88_KP_1,		KEY88_KP_1,	},	/* 0x53			*/
  { KEY88_KP_2,		KEY88_KP_2,	},	/* 0x54			*/
  { KEY88_KP_3,		KEY88_KP_3,	},	/* 0x55			*/
  { KEY88_KP_4,		KEY88_KP_4,	},	/* 0x56			*/
  { KEY88_KP_5,		KEY88_KP_5,	},	/* 0x57			*/
  { KEY88_KP_6,		KEY88_KP_6,	},	/* 0x58			*/
  { KEY88_KP_7,		KEY88_KP_7,	},	/* 0x59			*/
  { 0,			0,		},	/*			*/
  { KEY88_KP_8,		KEY88_KP_8,	},	/* 0x5B			*/
  { KEY88_KP_9,		KEY88_KP_9,	},	/* 0x5C			*/
  { KEY88_YEN,		KEY88_BAR,	},	/* 0x5D			*/
  { KEY88_UNDERSCORE,	KEY88_UNDERSCORE},	/* 0x5E			*/
  { KEY88_KP_COMMA,	KEY88_KP_COMMA,	},	/* 0x5F			*/
  { KEY88_F5,		KEY88_F5,	},	/* 0x60			*/
  { KEY88_F6,		KEY88_F6,	},	/* 0x61			*/
  { KEY88_F7,		KEY88_F7,	},	/* 0x62			*/
  { KEY88_F3,		KEY88_F3,	},	/* 0x63			*/
  { KEY88_F8,		KEY88_F8,	},	/* 0x64			*/
  { KEY88_F9,		KEY88_F9,	},	/* 0x65			*/
  { 0,			0,		},	/* 0x66 eusii		*/
  { KEY88_F11,		KEY88_F11,	},	/* 0x67			*/
  { KEY88_KANA,		0,		},	/* 0x68 kana		*/
  { KEY88_COPY,		0,		},	/* 0x69	printscreen	*/
  { 0,			0,		},	/*			*/
  { 0,			0,		},	/* 0x6B	scrlllock	*/
  { 0,			0,		},	/*			*/
  { KEY88_F10,		KEY88_F10,	},	/* 0x6D			*/
  { 0,			0,		},	/*			*/
  { KEY88_F12,		KEY88_F12,	},	/* 0x6F			*/
  { 0,			0,		},	/*			*/
  { KEY88_STOP,		0,		},	/* 0x71	pause		*/
  { KEY88_INS,		0,		},	/* 0x72			*/
  { KEY88_HOME,		0,		},	/* 0x73			*/
  { KEY88_ROLLDOWN,	KEY88_ROLLDOWN,	},	/* 0x74	pageup		*/
  { KEY88_DEL,		KEY88_DEL,	},	/* 0x75			*/
  { KEY88_F4,		KEY88_F4,	},	/* 0x76			*/
  { KEY88_HELP,		0,		},	/* 0x77	end		*/
  { KEY88_F2,		KEY88_F2,	},	/* 0x78			*/
  { KEY88_ROLLUP,	KEY88_ROLLUP,	},	/* 0x79	pagedown	*/
  { KEY88_F1,		KEY88_F1,	},	/* 0x7A			*/
  { KEY88_LEFT,		KEY88_LEFT,	},	/* 0x7B			*/
  { KEY88_RIGHT,	KEY88_RIGHT,	},	/* 0x7C			*/
  { KEY88_DOWN,		KEY88_DOWN,	},	/* 0x7D			*/
  { KEY88_UP,		KEY88_UP,	},	/* 0x7E			*/
  { 0,			0,		},	/* 0x7F power		*/
};














//void toWindowMode(void);
//void toFullscreenMode(void);



// whether currently in the background
static	Boolean		inBackground;



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
}



/*
 * 約 1/60 毎に呼ばれる
 */
void	event_update(void)
{
    static char kon[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    EventRecord event;
    Boolean loop = TRUE;
    short part;
    WindowPtr window;
    char key;
    unsigned int keyCode;

    // event loop
    while (loop) {
	if (WaitNextEvent(everyEvent, &event, 0, NULL) == false) break;

	/* SHIFTキーなどの押下はイベントがないらしいので、キーは全て常時監視する */
	{
	    static EventModifiers last_mods;
	    static KeyMap         last_keys;

	    int i;
	    EventModifiers mods;
	    KeyMap         keys;

				/* CAPSLOCK(alphaLock) は modifiers で監視 */
	    mods = event.modifiers & alphaLock;
	    if (mods != last_mods) {
		quasi88_key(KEY88_CAPS, (mods));
		last_mods = mods;
	    }

				/* その他装飾キー、文字キーは GetKeys で監視 */
	    GetKeys(keys);
	    if ((keys[0] != last_keys[0]) || (keys[1] != last_keys[1]) ||
		(keys[2] != last_keys[2]) || (keys[3] != last_keys[3])) {
		int old_bit, new_bit;

		for (i=0; i<128; ++i) {
		    old_bit = (((unsigned char *)last_keys)[i/8]>>(i%8)) & 0x01;
		    new_bit = (((unsigned char *)keys)[i/8]>>(i%8)) & 0x01;

		    if (old_bit != new_bit) {

			if ((event.modifiers & cmdKey) && new_bit) {

			    /* cmdKey かつ ON はとりあえず無視 */ ;

			} else {

			    if (quasi88_is_exec()) {
				quasi88_key(mac2keycode[i][0], new_bit);
			    } else {
				quasi88_key(mac2keycode[i][ (event.modifiers & shiftKey) ? 1 : 0 ],
					    new_bit);

				/* KeyTranslate() を使う？ */
			    }

			}
		    }
		}
		last_keys[0] = keys[0];
		last_keys[1] = keys[1];
		last_keys[2] = keys[2];
		last_keys[3] = keys[3];
	    }
	}

	/* マウスの単純移動はイベントがないらしいので、常時監視する */
	{
	    ;		/* 面倒そうなのでやめよう・・・ */
	}


	/* ここから、イベントに応じた処理を行う */
	switch (event.what) {
	case mouseDown:		/* ------------------------------------------*/
	    if (macWin != FrontWindow()) {
		SelectWindow(macWin);
	    }

	    part = FindWindow(event.where, &window);
	    switch (part) {
	    case inMenuBar:
		if (quasi88_is_exec()) xmame_sound_suspend();
		{
		    doMenuCommand(MenuSelect(event.where));
		}
		if (quasi88_is_exec()) xmame_sound_resume();
		break;
	    case inContent:
		if (quasi88_is_menu()) {
		    int x, y;

		    /* マウスのクリック座標を、ウインドウ座標に変換 */
		    GrafPtr saveport;
		    GetPort(&saveport);
		    {
			SetPort(macWin);
			GlobalToLocal(&event.where);
		    }
		    SetPort(saveport);

		    /* この座標に 移動 & クリック ということにする */
		    x = event.where.h;
		    y = event.where.v;

		    quasi88_mouse_moved_abs(x, y);
		    quasi88_mouse_pressed(KEY88_MOUSE_L);
		}
		break;
	    case inDrag:
		if (quasi88_is_exec()) xmame_sound_suspend();
		{
		    DragWindow(macWin, event.where, &macQd.screenBits.bounds);
		}
		if (quasi88_is_exec()) xmame_sound_resume();
		break;
	    case inGrow:
		break;
	    case inZoomIn:
	    case inZoomOut:
		break;
	    }
	    break;

	case mouseUp:		/* ------------------------------------------*/
	    if (quasi88_is_menu()) {
		int x, y;
		GrafPtr saveport;
		GetPort(&saveport);
		{
		    SetPort(macWin);
		    GlobalToLocal(&event.where);
		}
		SetPort(saveport);
		x = event.where.h;
		y = event.where.v;

		quasi88_mouse_moved_abs(x, y);
		quasi88_mouse_released(KEY88_MOUSE_L);
	    }
	    break;

	case keyDown:		/* ------------------------------------------*/
	case autoKey:		/* ------------------------------------------*/
	    key = event.message & charCodeMask;
	    if (event.modifiers & cmdKey) {
		if (event.what == keyDown) {
		    doMenuCommand(MenuKey(key));
		}
		keyCode = (event.message & keyCodeMask) >> 8;
		switch (keyCode) {
		case 0x12:  kon[0] = 1; quasi88_key_pressed(KEY88_F1);		break;	/* Cmd + 1 */
		case 0x13:  kon[1] = 1; quasi88_key_pressed(KEY88_F2);		break;	/* Cmd + 2 */
		case 0x14:  kon[2] = 1; quasi88_key_pressed(KEY88_F3);		break;	/* Cmd + 3 */
		case 0x15:  kon[3] = 1; quasi88_key_pressed(KEY88_F4);		break;	/* Cmd + 4 */
		case 0x17:  kon[4] = 1; quasi88_key_pressed(KEY88_F5);		break;	/* Cmd + 5 */
		case 0x08:  kon[5] = 1; quasi88_key_pressed(KEY88_COPY);	break;	/* Cmd + C */
		case 0x01:  kon[6] = 1; quasi88_key_pressed(KEY88_STOP);	break;	/* Cmd + S */
		case 0x04:  kon[7] = 1; quasi88_key_pressed(KEY88_HELP);	break;	/* Cmd + H */
		case 0x7e:
		case 0x3e:  kon[8] = 1; quasi88_key_pressed(KEY88_ROLLUP);	break;	/* Cmd +↑ */
		case 0x7d:
		case 0x3d:  kon[9] = 1; quasi88_key_pressed(KEY88_ROLLDOWN);	break;	/* Cmd +↓ */
		case 0x2e:  kon[10]= 1; quasi88_key_pressed(KEY88_F12);		break;	/* Cmd + M */
		}
	    }
	    break;

	case keyUp:		/* ------------------------------------------*/
	    {
		keyCode = (event.message & keyCodeMask) >> 8;
		switch (keyCode) {
		case 0x12: if (kon[0]) { kon[0] = 0; quasi88_key_released(KEY88_F1); }		break;	/* Cmd + 1 */
		case 0x13: if (kon[1]) { kon[1] = 0; quasi88_key_released(KEY88_F2); }		break;	/* Cmd + 2 */
		case 0x14: if (kon[2]) { kon[2] = 0; quasi88_key_released(KEY88_F3); }		break;	/* Cmd + 3 */
		case 0x15: if (kon[3]) { kon[3] = 0; quasi88_key_released(KEY88_F4); }		break;	/* Cmd + 4 */
		case 0x17: if (kon[4]) { kon[4] = 0; quasi88_key_released(KEY88_F5); }		break;	/* Cmd + 5 */
		case 0x08: if (kon[5]) { kon[5] = 0; quasi88_key_released(KEY88_COPY); }	break;	/* Cmd + C */
		case 0x01: if (kon[6]) { kon[6] = 0; quasi88_key_released(KEY88_STOP); }	break;	/* Cmd + S */
		case 0x04: if (kon[7]) { kon[7] = 0; quasi88_key_released(KEY88_HELP); }	break;	/* Cmd + H */
		case 0x7e:
		case 0x3e: if (kon[8]) { kon[8] = 0; quasi88_key_released(KEY88_ROLLUP); }	break;	/* Cmd +↑ */
		case 0x7d:
		case 0x3d: if (kon[9]) { kon[9] = 0; quasi88_key_released(KEY88_ROLLDOWN); }	break;	/* Cmd +↓ */
		case 0x2e: if (kon[10]){ kon[10]= 0; quasi88_key_released(KEY88_F12); }		break;	/* Cmd + M */
		}
	    }
	    break;

	case osEvt:		/* ------------------------------------------*/
	    switch ((event.message >> 24) & 0xFF) {
	    case suspendResumeMessage:
		inBackground = (event.message & resumeFlag) == 0;
		break;
	    }
	    break;

	case activateEvt:	/* ------------------------------------------*/
	    break;

	case updateEvt:		/* ------------------------------------------*/
	    if (mac_is_fullscreen() == FALSE) {
		quasi88_expose();
	    }
	    BeginUpdate((WindowRef)event.message);
	    EndUpdate((WindowRef)event.message);
	    break;

	case nullEvent:		/* ------------------------------------------*/
	    loop = FALSE;
	    break;
	}
    }
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
    *x = 0;
    *y = 0;
}


/******************************************************************************
 * ソフトウェア NumLock 有効／無効
 *
 *****************************************************************************/

int	event_numlock_on (void){ return FALSE; }
void	event_numlock_off(void){ }



/******************************************************************************
 * エミュレート／メニュー／ポーズ／モニターモード の 開始時の処理
 *
 *****************************************************************************/

void	event_switch(void)
{
    if (quasi88_is_exec()) {		/* エミュモードなら */

	menubar_setup(TRUE);			/* メニューバー有効 */

    } else {				/* メニューモードなどなら */

	menubar_setup(FALSE);			/* メニューバー無効 */
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



/******************************************************************************
 ******************************************************************************
 *
 *				MAC のメニュー
 *
 ******************************************************************************
 *****************************************************************************/

#include "menubar.c"
