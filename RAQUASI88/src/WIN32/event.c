/***********************************************************************
 * イベント処理 (システム依存)
 *
 *	詳細は、 event.h 参照
 ************************************************************************/

#include "quasi88.h"
#include "device.h"

#include "event.h"
#include "keyboard.h"



static	int	now_charcode = FALSE;


/* 仮想キーと、KEY88 の対応 */
static	int	keysym2key88[ 256 ];

/* 仮想キーと、KEY88 の対応 (デフォルト) */
static const int keysym2key88_default[256] =
{
    0,
    0,				/* VK_LBUTTON        0x01 */
    0,				/* VK_RBUTTON        0x02 */
    KEY88_STOP,			/* VK_CANCEL         0x03 */
    0,				/* VK_MBUTTON        0x04 */
    0,
    0,
    0,

    KEY88_INS_DEL,		/* VK_BACK           0x08 */
    KEY88_TAB,			/* VK_TAB            0x09 */
    0,
    0,
    KEY88_KP_5,			/* VK_CLEAR          0x0C */
    KEY88_RETURNL,		/* VK_RETURN         0x0D */
    0,
    0,

    KEY88_SHIFTL,		/* VK_SHIFT          0x10 */
    KEY88_CTRL,			/* VK_CONTROL        0x11 */
    KEY88_GRAPH,		/* VK_MENU           0x12 */
    KEY88_STOP,			/* VK_PAUSE          0x13 */
    KEY88_CAPS,			/* VK_CAPITAL        0x14 */
    0,				/* VK_KANA           0x15 */
    0,
    0,				/* VK_JUNJA          0x17 */

    0,				/* VK_FINAL          0x18 */
    0,				/* VK_KANJI          0x19 */
    0,
    KEY88_ESC,			/* VK_ESCAPE         0x1B */
    0,				/* VK_CONVERT        0x1C */ /* 変換 */
    0,				/* VK_NONCONVERT     0x1D */ /* 無変換 */
    0,				/* VK_ACCEPT         0x1E */
    0,				/* VK_MODECHANGE     0x1F */

    KEY88_SPACE,		/* VK_SPACE          0x20 */
    KEY88_ROLLDOWN,		/* VK_PRIOR          0x21 */
    KEY88_ROLLUP,		/* VK_NEXT           0x22 */
    KEY88_HELP,			/* VK_END            0x23 */
    KEY88_HOME,			/* VK_HOME           0x24 */
    KEY88_LEFT,			/* VK_LEFT           0x25 */
    KEY88_UP,			/* VK_UP             0x26 */
    KEY88_RIGHT,		/* VK_RIGHT          0x27 */

    KEY88_DOWN,			/* VK_DOWN           0x28 */
    0,				/* VK_SELECT         0x29 */
    0,				/* VK_PRINT          0x2A */
    0,				/* VK_EXECUTE        0x2B */
    KEY88_COPY,			/* VK_SNAPSHOT       0x2C */
    KEY88_INS,			/* VK_INSERT         0x2D */
    KEY88_DEL,			/* VK_DELETE         0x2E */
    0,				/* VK_HELP           0x2F */

/* VK_0 thru VK_9 are the same as ASCII '0' thru '9' (0x30 - 0x39) */
/* VK_A thru VK_Z are the same as ASCII 'A' thru 'Z' (0x41 - 0x5A) */

    KEY88_0,
    KEY88_1,
    KEY88_2,
    KEY88_3,
    KEY88_4,
    KEY88_5,
    KEY88_6,
    KEY88_7,

    KEY88_8,
    KEY88_9,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    KEY88_A,
    KEY88_B,
    KEY88_C,
    KEY88_D,
    KEY88_E,
    KEY88_F,
    KEY88_G,

    KEY88_H,
    KEY88_I,
    KEY88_J,
    KEY88_K,
    KEY88_L,
    KEY88_M,
    KEY88_N,
    KEY88_O,

    KEY88_P,
    KEY88_Q,
    KEY88_R,
    KEY88_S,
    KEY88_T,
    KEY88_U,
    KEY88_V,
    KEY88_W,

    KEY88_X,
    KEY88_Y,
    KEY88_Z,
    0,				/* VK_LWIN           0x5B */
    0,				/* VK_RWIN           0x5C */
    0,				/* VK_APPS           0x5D */
    0,
    0,

    KEY88_KP_0,			/* VK_NUMPAD0        0x60 */
    KEY88_KP_1,			/* VK_NUMPAD1        0x61 */
    KEY88_KP_2,			/* VK_NUMPAD2        0x62 */
    KEY88_KP_3,			/* VK_NUMPAD3        0x63 */
    KEY88_KP_4,			/* VK_NUMPAD4        0x64 */
    KEY88_KP_5,			/* VK_NUMPAD5        0x65 */
    KEY88_KP_6,			/* VK_NUMPAD6        0x66 */
    KEY88_KP_7,			/* VK_NUMPAD7        0x67 */

    KEY88_KP_8,			/* VK_NUMPAD8        0x68 */
    KEY88_KP_9,			/* VK_NUMPAD9        0x69 */
    KEY88_KP_MULTIPLY,		/* VK_MULTIPLY       0x6A */
    KEY88_KP_ADD,		/* VK_ADD            0x6B */
    0,				/* VK_SEPARATOR      0x6C */
    KEY88_KP_SUB,		/* VK_SUBTRACT       0x6D */
    KEY88_KP_PERIOD,		/* VK_DECIMAL        0x6E */
    KEY88_KP_DIVIDE,		/* VK_DIVIDE         0x6F */

    KEY88_F1,			/* VK_F1             0x70 */
    KEY88_F2,			/* VK_F2             0x71 */
    KEY88_F3,			/* VK_F3             0x72 */
    KEY88_F4,			/* VK_F4             0x73 */
    KEY88_F5,			/* VK_F5             0x74 */
    KEY88_F6,			/* VK_F6             0x75 */
    KEY88_F7,			/* VK_F7             0x76 */
    KEY88_F8,			/* VK_F8             0x77 */

    KEY88_F9,			/* VK_F9             0x78 */
    KEY88_F10,			/* VK_F10            0x79 */
    KEY88_F11,			/* VK_F11            0x7A */
    KEY88_F12,			/* VK_F12            0x7B */
    KEY88_F13,			/* VK_F13            0x7C */
    KEY88_F14,			/* VK_F14            0x7D */
    KEY88_F15,			/* VK_F15            0x7E */
    0,				/* VK_F16            0x7F */

    0,				/* VK_F17            0x80 */
    0,				/* VK_F18            0x81 */
    0,				/* VK_F19            0x82 */
    0,				/* VK_F20            0x83 */
    0,				/* VK_F21            0x84 */
    0,				/* VK_F22            0x85 */
    0,				/* VK_F23            0x86 */
    0,				/* VK_F24            0x87 */

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,				/* VK_NUMLOCK        0x90 */
    KEY88_KANA,			/* VK_SCROLL         0x91 */
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

/*
 * VK_L* & VK_R* - left and right Alt, Ctrl and Shift virtual keys.
 * Used only as parameters to GetAsyncKeyState() and GetKeyState().
 * No other API or message will distinguish left and right keys in this way.
 */

    KEY88_SHIFTL,		/* VK_LSHIFT         0xA0 */
    KEY88_SHIFTR,		/* VK_RSHIFT         0xA1 */
    KEY88_CTRL,			/* VK_LCONTROL       0xA2 */
    KEY88_CTRL,			/* VK_RCONTROL       0xA3 */
    KEY88_GRAPH,		/* VK_LMENU          0xA4 */
    KEY88_GRAPH,		/* VK_RMENU          0xA5 */
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    KEY88_COLON,
    KEY88_SEMICOLON,
    KEY88_COMMA,
    KEY88_MINUS,
    KEY88_PERIOD,
    KEY88_SLASH,

    KEY88_AT,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    KEY88_BRACKETLEFT,
    KEY88_YEN,
    KEY88_BRACKETRIGHT,
    KEY88_CARET,
    0,

    0,
    0,
    KEY88_UNDERSCORE,
    0,
    0,
    0,				/* VK_PROCESSKEY     0xE5 */
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,				/* VK_ATTN           0xF6 */
    0,				/* VK_CRSEL          0xF7 */

    0,				/* VK_EXSEL          0xF8 */
    0,				/* VK_EREOF          0xF9 */
    0,				/* VK_PLAY           0xFA */
    0,				/* VK_ZOOM           0xFB */
    0,				/* VK_NONAME         0xFC */
    0,				/* VK_PA1            0xFD */
    0,				/* VK_OEM_CLEAR      0xFE */
    0,
};


/******************************************************************************
 * ウィンドウプロシージャ
 *****************************************************************************/
static void key_event_debug(UINT msg, WPARAM wp, LPARAM lp);

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp)
{
    int on, key88;
    int x, y;

    switch(msg) {

    case WM_ERASEBKGND:		/* WM_PAINTの直前に来る、画面クリア要求 */
	return 0;

    case WM_PAINT:		/* 描画すべきタイミングで送られてくる */
	if (graph_update_WM_PAINT() == FALSE) {
/*
	    fprintf(debugfp, "Expose\n");
*/
	    quasi88_expose();
	}
	return 0;


    case WM_MOUSEMOVE:		/* マウスカーソル移動 */
	x = LOWORD(lp);
	y = HIWORD(lp);
/*
	fprintf(debugfp, "Move %d %d\n",x,y);
*/
	quasi88_mouse_moved_abs(x, y);
	return 0;


    case WM_LBUTTONDOWN:	/* マウス左ボタン押下 */
    case WM_LBUTTONUP:		/* マウス左ボタン解放 */
    case WM_RBUTTONDOWN:	/* マウス右ボタン押下 */
    case WM_RBUTTONUP:		/* マウス右ボタン解放 */

	if (msg == WM_LBUTTONDOWN ||
	    msg == WM_LBUTTONUP)    { key88 = KEY88_MOUSE_L; }
	else                        { key88 = KEY88_MOUSE_R; }
	if (msg == WM_LBUTTONDOWN ||
	    msg == WM_RBUTTONDOWN) { on = TRUE;   SetCapture(hWnd); }
	else                       { on = FALSE;  ReleaseCapture(); }
/*
	fprintf(debugfp, "%c:%s %d %d\n", (key88 == KEY88_MOUSE_L) ? 'L' : 'R',
		(on ? "On " : "Off"), LOWORD(lp), HIWORD(lp));
*/
	quasi88_mouse(key88, on);
	return 0;


    case WM_KEYDOWN:		/* キー押下			*/
    case WM_KEYUP:		/* キー解放			*/
    case WM_SYSKEYDOWN:		/* システムキー押下 (Alt, F10)	*/
    case WM_SYSKEYUP:		/* システムキー解放 (Alt, F10)	*/
    case WM_CHAR:		/* 文字入力			*/

#if 0
	key_event_debug(msg, wp, lp);	/* デバッグ用の、キーチェック */
	return 0;
#endif

	if (now_charcode) {		/* WM_CHAR を拾う (メニュー中) */

	    if (msg == WM_CHAR) {

		if (isprint(wp)) {
		    /* 表示できる文字は、処理する */
		} else {
		    /* 表示できない文字は、無視 */
		    return 0;
		}

	    } else {
		if ((              wp == ' ')  ||		/* 空白 */
		    ('0'  <= wp && wp <= '9')  ||		/* 数字 */
		    ('A'  <= wp && wp <= 'Z')  ||		/* 英字 */
		    (0x60 <= wp && wp <= 0x6f) ||		/* テンキー */
		    (0xb0 <= wp && wp <= 0xef)) {		/* 記号キー */

		    /* これらのキーは WM_CHAR を待つ */
		    return 0;
		} else {
		    /* 特殊キー、制御キーは処理する */
		}
	    }

	} else {			/* WM_CHAR を捨てる (エミュ中) */

	    if (msg == WM_CHAR) {
		/* 文字イベントは、無視 */
		return 0;
	    } else {
		/* キーオン・オフは処理する */
	    }
	}

	if (msg == WM_KEYUP || msg == WM_SYSKEYUP) { on = FALSE; }
	else                                       { on = TRUE;  }
    

	/* bit30 は、直前の押下 (つまり連続押下)を表す ↓*/
	if (on && (g_keyrepeat == FALSE) && (lp & 0x40000000UL)) {
	    /* キーリピートの場合は、条件により無視 */
	    return 0;
	}


	if (msg == WM_CHAR) {
	    key88 = wp;
	} else {
	    /* bit24 は拡張キーフラグ。右Alt、右Ctrl、
			↓      テンキー以外の方向キー押下時は 1 */
	    if (((lp & (1UL<<24)) == 0) && (now_charcode == FALSE)) {
		/* NumLock なしでテンキー押下なら、キーを読み替える */
		if      (wp == VK_INSERT) wp = VK_NUMPAD0;
		else if (wp == VK_DELETE) wp = VK_DECIMAL;
		else if (wp == VK_END)    wp = VK_NUMPAD1;
		else if (wp == VK_DOWN)   wp = VK_NUMPAD2;
		else if (wp == VK_NEXT)   wp = VK_NUMPAD3;
		else if (wp == VK_LEFT)   wp = VK_NUMPAD4;
		else if (wp == VK_CLEAR)  wp = VK_NUMPAD5;
		else if (wp == VK_RIGHT)  wp = VK_NUMPAD6;
		else if (wp == VK_HOME)   wp = VK_NUMPAD7;
		else if (wp == VK_UP)     wp = VK_NUMPAD8;
		else if (wp == VK_PRIOR)  wp = VK_NUMPAD9;
	    }

	    if (wp == VK_CAPITAL ||
		wp == VK_SCROLL) {
		/* 最下位ビットはトグル状態を表す ↓ */
		int toggle = GetKeyState(wp) & 0x01;

		if ((on && toggle) ||
		    (on == FALSE && toggle == FALSE)) {
		    /* キー押下とトグル状態が一致してればOK */
		} else {
		    /* 一致してなければ、キーを無視する */
		    return 0;
		}
	    }

	    key88 = keysym2key88[ wp ];
	}
/*
	fprintf(debugfp,
		"%s <%s>\n", (on) ? "ON " : "OFF", keyboard_key882str(key88));
*/
	quasi88_key(key88, on);

	/* WM_CHAR の場合、キーオフイベントがないけど…まあいいか */
	return 0;


    case WM_SETFOCUS:
/*
	fprintf(debugfp, "Focus In\n");
*/
	quasi88_focus_in();
	return 0;

    case WM_KILLFOCUS:
/*
	fprintf(debugfp, "Focus Out\n");
*/
	quasi88_focus_out();
	return 0;

#ifdef	USE_SOUND
    /* サウンド系のイベント */
    case MM_WOM_OPEN:
	wave_event_open((HWAVEOUT)wp);
	return 0;

    case MM_WOM_DONE:
	wave_event_done((HWAVEOUT)wp, (LPWAVEHDR)lp);
	return 0;

    case MM_WOM_CLOSE:
	wave_event_close((HWAVEOUT)wp);
	return 0;
#endif

    case WM_DROPFILES:
	{
	    HDROP hDrop;
	    int i, file_num;
	    char filename[ OSD_MAX_FILENAME ];

	    hDrop = (HDROP)wp;
	    file_num = (int)DragQueryFile(hDrop, 0xffffffff, NULL, 0);
/*
	    for (i = 0; i < file_num; i++) {
		DragQueryFile(hDrop, i, filename, sizeof(filename));
		fprintf(debugfp, "%d %s\n", i, filename);
	    }
*/
	    if (file_num > 0) {
		DragQueryFile(hDrop, 0, filename, sizeof(filename));

		if (quasi88_drag_and_drop(filename)) {
		    menubar_setup(TRUE); /* メニューバーを全て更新 */
		}
	    }

	    DragFinish(hDrop);
	}
	return 0;

    case WM_COMMAND:
	if (menubar_event(LOWORD(wp))) {
	    return 0;
	}
	break;

    case WM_CLOSE:
	/* ウインドウを閉じようとした場合、確認させるならここで */
/*
	fprintf(debugfp, "Close\n");
*/
	/*return 0;*/
	break;

    case WM_DESTROY:
/*
	fprintf(debugfp, "Destroy\n");
*/
	PostQuitMessage(0);
	return 0;
    }

    return DefWindowProc(hWnd, msg, wp, lp);
}

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
    memcpy(keysym2key88, keysym2key88_default, sizeof(keysym2key88_default));
}



/*
 * 約 1/60 毎に呼ばれる
 */
void	event_update(void)
{
    MSG msg;

#if 0 /* 基本形 */
    while (GetMessage(&msg, NULL, 0, 0)) {	/* メッセージを取得 */
	TranslateMessage(&msg);			/* キーコードなら手直し */
	DispatchMessage(&msg);			/* プロシージャに送る */
    }
#elif 0

    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
	if (GetMessage(&msg, NULL, 0, 0)) {	/* メッセージを取得 */
	    TranslateMessage(&msg);		/* キーコードなら手直し */
	    DispatchMessage(&msg);		/* プロシージャに送る */
	}
    }

#elif 1

    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
	if (GetMessage(&msg, NULL, 0, 0)) {	/* メッセージを取得 */

	    TranslateMessage(&msg);		/* キーコードなら手直し */
	    DispatchMessage(&msg);		/* プロシージャに送る */

	} else {				/* 取得失敗 */
	    quasi88_quit();
	    break;
	}
    }


#elif 0 /* メニューがある場合？ */

    HACCEL hAccel= LoadAccelerators(hAppModule, MAKEINTRESOURCE(IDR_ACCEL1));
    while (GetMessage(&msg, NULL, 0, 0)) {
	if (!TranslateAccelerator(hWnd, hAccel, &msg)) { /* メニュー処理 */
	    TranslateMessage(&msg);
	    DispatchMessage(&msg);
	}
    }

#elif 0 /* これは？ */
    while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) {
	if (GetMessage(&msg, NULL, 0, 0)) {

	    if (((msg.message == WM_SYSKEYDOWN) ||
		 (msg.message == WM_SYSKEYUP)) &&
		(msg.wParam == VK_F10)) {
		/* DO NOTHING */
	    } else
	    {
		if (TranslateAccelerator(GetWndHandle(), m_haccel, &msg) == FALSE) {
		    TranslateMessage(&msg);
		    DispatchMessage(&msg);
		}
	    }

	} else {	/* エラー！ */
	    quasi88_quit();
	    break;
	}
    }
#endif
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
    POINT pos;

    GetCursorPos(&pos);

    if (ScreenToClient(g_hWnd, &pos)) {
	*x = pos.x;
	*y = pos.y;
    } else {
	*x = 0;
	*y = 0;
    }
}


/******************************************************************************
 * ソフトウェア NumLock 有効／無効
 *
 *****************************************************************************/

int	event_numlock_on (void)
{
    /* ソフトウェア NumLock 有効なら、メインキーの一部をテンキーにする */

    keysym2key88[ '5' ]		= KEY88_HOME;
    keysym2key88[ '6' ]		= KEY88_HELP;
    keysym2key88[ '7' ]		= KEY88_KP_7;
    keysym2key88[ '8' ]		= KEY88_KP_8;
    keysym2key88[ '9' ]		= KEY88_KP_9;
    keysym2key88[ '0' ]		= KEY88_KP_MULTIPLY;
    keysym2key88[ 0xbd ]/* - */	= KEY88_KP_SUB;
    keysym2key88[ 0xde ]/* ^ */	= KEY88_KP_DIVIDE;
    keysym2key88[ 'U' ]		= KEY88_KP_4;
    keysym2key88[ 'I' ]		= KEY88_KP_5;
    keysym2key88[ 'O' ]		= KEY88_KP_6;
    keysym2key88[ 'P' ]		= KEY88_KP_ADD;
    keysym2key88[ 'J' ]		= KEY88_KP_1;
    keysym2key88[ 'K' ]		= KEY88_KP_2;
    keysym2key88[ 'L' ]		= KEY88_KP_3;
    keysym2key88[ 0xbb ]/* ; */	= KEY88_KP_EQUAL;
    keysym2key88[ 'M' ]		= KEY88_KP_0;
    keysym2key88[ 0xbc ]/* , */	= KEY88_KP_COMMA;
    keysym2key88[ 0xbe ]/* . */	= KEY88_KP_PERIOD;
    keysym2key88[ 0xbf ]/* / */	= KEY88_RETURNR;

    return TRUE;
}
void	event_numlock_off(void)
{
    /* ソフトウェア NumLock 無効なら、デフォルトに戻す */

    memcpy(keysym2key88, keysym2key88_default, sizeof(keysym2key88_default));
}



/******************************************************************************
 * エミュレート／メニュー／ポーズ／モニターモード の 開始時の処理
 *
 *****************************************************************************/

void	event_switch(void)
{
    if (quasi88_is_exec()) {		/* エミュモードなら */

	now_charcode = FALSE;			/* WM_CHAR イベント無視 */

	menubar_setup(TRUE);			/* メニューバー有効 */

    } else {				/* メニューモードなどなら */

	now_charcode = TRUE;			/* WM_CHAR イベント処理 */

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






/*---------------------------------------------------------------------------
 *
 * デバッグ
 *
 *---------------------------------------------------------------------------*/

static const char *vk_list[256] =
{
    0,
    "VK_LBUTTON",      /* 0x01 */
    "VK_RBUTTON",      /* 0x02 */
    "VK_CANCEL",       /* 0x03 */ /* Ctrl + Pause, Ctrl + ScrollLock */
    "VK_MBUTTON",      /* 0x04 */
    0,
    0,
    0,

    "VK_BACK",         /* 0x08 */ /* BackSpace */
    "VK_TAB",          /* 0x09 */
    0,
    0,
    "VK_CLEAR",        /* 0x0C */ /* テンキー 5 */
    "VK_RETURN",       /* 0x0D */ /* Enter */
    0,
    0,

    "VK_SHIFT",        /* 0x10 */
    "VK_CONTROL",      /* 0x11 */
    "VK_MENU",         /* 0x12 */ /* Alt */
    "VK_PAUSE",        /* 0x13 */
    "VK_CAPITAL",      /* 0x14 */ /* CapsLock */
    "VK_KANA",         /* 0x15 */
    0,
    "VK_JUNJA",        /* 0x17 */

    "VK_FINAL",        /* 0x18 */
    "VK_KANJI",        /* 0x19 */ /* Alt + 全角半角 */
    0,
    "VK_ESCAPE",       /* 0x1B */
    "VK_CONVERT",      /* 0x1C */ /* 変換 */
    "VK_NONCONVERT",   /* 0x1D */ /* 無変換 */
    "VK_ACCEPT",       /* 0x1E */
    "VK_MODECHANGE",   /* 0x1F */

    "VK_SPACE",        /* 0x20 */
    "VK_PRIOR",        /* 0x21 */ /* PageUp */
    "VK_NEXT",         /* 0x22 */ /* PageDown */
    "VK_END",          /* 0x23 */
    "VK_HOME",         /* 0x24 */
    "VK_LEFT",         /* 0x25 */
    "VK_UP",           /* 0x26 */
    "VK_RIGHT",        /* 0x27 */

    "VK_DOWN",         /* 0x28 */
    "VK_SELECT",       /* 0x29 */
    "VK_PRINT",        /* 0x2A */
    "VK_EXECUTE",      /* 0x2B */
    "VK_SNAPSHOT",     /* 0x2C */ /* PrintScreen */
    "VK_INSERT",       /* 0x2D */
    "VK_DELETE",       /* 0x2E */
    "VK_HELP",         /* 0x2F */

    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",

    "8",
    "9", 
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",

    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",

    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",

    "X",
    "Y",
    "Z",
    "VK_LWIN",         /* 0x5B */
    "VK_RWIN",         /* 0x5C */
    "VK_APPS",         /* 0x5D */
    0,
    0,

    "VK_NUMPAD0",      /* 0x60 */
    "VK_NUMPAD1",      /* 0x61 */
    "VK_NUMPAD2",      /* 0x62 */
    "VK_NUMPAD3",      /* 0x63 */
    "VK_NUMPAD4",      /* 0x64 */
    "VK_NUMPAD5",      /* 0x65 */
    "VK_NUMPAD6",      /* 0x66 */
    "VK_NUMPAD7",      /* 0x67 */

    "VK_NUMPAD8",      /* 0x68 */
    "VK_NUMPAD9",      /* 0x69 */
    "VK_MULTIPLY",     /* 0x6A */
    "VK_ADD",          /* 0x6B */
    "VK_SEPARATOR",    /* 0x6C */
    "VK_SUBTRACT",     /* 0x6D */
    "VK_DECIMAL",      /* 0x6E */	/* テンキー . */
    "VK_DIVIDE",       /* 0x6F */

    "VK_F1",           /* 0x70 */
    "VK_F2",           /* 0x71 */
    "VK_F3",           /* 0x72 */
    "VK_F4",           /* 0x73 */
    "VK_F5",           /* 0x74 */
    "VK_F6",           /* 0x75 */
    "VK_F7",           /* 0x76 */
    "VK_F8",           /* 0x77 */

    "VK_F9",           /* 0x78 */
    "VK_F10",          /* 0x79 */
    "VK_F11",          /* 0x7A */
    "VK_F12",          /* 0x7B */
    "VK_F13",          /* 0x7C */
    "VK_F14",          /* 0x7D */
    "VK_F15",          /* 0x7E */
    "VK_F16",          /* 0x7F */

    "VK_F17",          /* 0x80 */
    "VK_F18",          /* 0x81 */
    "VK_F19",          /* 0x82 */
    "VK_F20",          /* 0x83 */
    "VK_F21",          /* 0x84 */
    "VK_F22",          /* 0x85 */
    "VK_F23",          /* 0x86 */
    "VK_F24",          /* 0x87 */

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    "VK_NUMLOCK",      /* 0x90 */
    "VK_SCROLL",       /* 0x91 */ /* ScrollLock */
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    "VK_LSHIFT",       /* 0xA0 */
    "VK_RSHIFT",       /* 0xA1 */
    "VK_LCONTROL",     /* 0xA2 */
    "VK_RCONTROL",     /* 0xA3 */
    "VK_LMENU",        /* 0xA4 */
    "VK_RMENU",        /* 0xA5 */
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
					  /* 0xB0 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    ":",
    ";",
    ",",
    "-",
    ".",
    "/",
					  /* 0xC0 */
    "@",
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
					  /* 0xD0 */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,
    0,
    "[",
    "\\",
    "]",
    "^",
    0,

    0,
    0,
    "_",
    0,
    0,
    "VK_PROCESSKEY",   /* 0xE5 */ /* Ctrl + F10, 全角半角? */
    0,
    0,

    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,

    0,
    0,					/* Shift + ひらがな */
    0,					/* Shift + ひらがな */
    0,					/* 全角半角 */
    0,					/* 全角半角 */
    0,					/* Alt + ひらがな */
    "VK_ATTN",         /* 0xF6 */	/* Alt + ひらがな */
    "VK_CRSEL",        /* 0xF7 */

    "VK_EXSEL",        /* 0xF8 */
    "VK_EREOF",        /* 0xF9 */
    "VK_PLAY",         /* 0xFA */
    "VK_ZOOM",         /* 0xFB */
    "VK_NONAME",       /* 0xFC */
    "VK_PA1",          /* 0xFD */
    "VK_OEM_CLEAR",    /* 0xFE */
    0,

    /*
	怪しいキー

	Ctrl + ESC	スタートメニュー
	Alt  + ESC	タスクスイッチ
	Alt + Space	ウインドウメニュー
	ひらがな	DOWN/UP 反応なし?
	Pause		DOWN/UP連続発生
	PrintScreen	DOWNなし？ UPあり

	VK_INSERT とか VK_UP とか
			拡張フラグが 0 なら テンキー (NumLockオフ)
			拡張フラグが 1 なら メインキー


	〜 IME有効にするとさらに怪しくなる。意味不明 〜

	Ctrl + F10	0xE7	IMEメニュー

	全角半角	押下すると、KEYDOWN (0xe5) が発生し、IME がオン。
			KEYUPは発生しない。
			  IME がオンの間は、どのキーを押しても、KEYDOWN (0xe5)
			  が発生する。(KEYUPは、そのキーのコード)。
			再度押下すると、KEYUP (0xf4 or 0xf3) が発生し、IME が
			オフ。(0xf3/0xf4 の違いは不明)
			同時に、KEYDOWN (0xe5) が発生

	変換		全角半角に同じ。
			オフは、KEYDOWN (0x1c)

	ひらがな	全角半角に似てるが、なんか違う。
    */

};

static void key_event_debug(UINT msg, WPARAM wp, LPARAM lp)
{

    int on = FALSE;
    const char *s;

    switch (msg) {
    case WM_SYSKEYDOWN:		/* Alt / F10 オン */
	s = "Sys*On ";
	on = TRUE;
	goto KEY_COMMON;

    case WM_SYSKEYUP:		/*           オフ */
	s = "Sys*Off";
	goto KEY_COMMON;

    case WM_KEYDOWN:		/* その他キーオン */ 
	s = "KEY On ";
	on = TRUE;
	goto KEY_COMMON;

    case WM_KEYUP:		/*           オフ */
	s = "KEY Off";
	goto KEY_COMMON;

    KEY_COMMON:

	/* lp   bit  0-15 キーリピート回数 (DOWNのみ、まとめて通知時)
		bit 16-23 スキャンコード
		bit 24    拡張キー
		bit 25-26 na
		bit 27-28 OS
		bit 29    Alt
		bit 30    repeat
		bit 31    1:DOWN / 0:UP
	*/

	if (on && (lp & 0x40000000UL)) {

	    ;	/* オートリピート中 */

	} else {
	    /*
	    fprintf(debugfp, "%s %02x [%08X] ", s, wp, lp);
	    */
	    fprintf(debugfp, "%s %02x <%x> ", s, wp, (lp&(1UL<<24))?1:0);
	    s = vk_list[wp];
	    if (s) {
		fprintf(debugfp, "%s\n", s);
	    } else {
		fprintf(debugfp, "0x%02X\n", wp);
	    }
	}
	break;


    case WM_CHAR:
	/* カーソル、INS/DEL、F1 などは来ない */
	/* BackSpace = 0x08 */
	/* Tab       = 0x09 */
	/* Enter     = 0x0d */
	/* Esc r     = 0x1b */
	fprintf(debugfp, "Chr %c [%02x]\n", wp, wp);
	break;
    }
}
