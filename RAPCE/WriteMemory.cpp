/******************************************************************************
Ootake

 [WriteMemory.cpp]
	メモリ内容書き換えのコマンドを入力するためのフォーム

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#include "WriteMemory.h"
#include "resource.h"
#include "WinMain.h"
#include "App.h"
#include "Printf.h"
#include "MainBoard.h"

#define LINE_LEN	59
#define N_LINES 	6

enum WriteMemoryComponent
{
	EDIT_CODE = 1,
	BUTTON_CLEAR,
	BUTTON_SET,
};
static HWND			_hWndC[1+1]; //各コンポーネントのハンドル

static WNDPROC		_WPEdit; //キーフックのため

static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFontB; //ボタン用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "\"Ootake\" Write Memory";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;
static HWND 		_hParentWnd;
static BOOL 		_bToggle;

static char			_Code[32] = "";	//前回入力したコードを保存用
static Sint32		_SetOk = -1;	//戻り値。設定完了なら1。EDIT空欄で設定完了(Continuous解除)なら2。キャンセルなら-1。未設定中は0。
static Uint32*		_pMpr;			//戻り値設定用
static Uint32*		_pAddr;			//
static Uint8*		_pData;			//
static BOOL*		_pbContinuous;	//
static Sint32*		_pSetOk = 0;	//


/* フォントの高さを取得する */
static Uint32
get_font_height(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);	 /* 固定ピッチフォント */
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)(tm.tmHeight);
}

/* フォントの横幅を取得する */
static Uint32
get_font_width(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);	 /* 固定ピッチフォント */
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)tm.tmAveCharWidth;
}


static void
set_window_size(
	HWND			hWnd)
{
	RECT	rc;
	Uint32	wndW = _FontWidth  * LINE_LEN + 3;
	Uint32	wndH = _FontHeight * N_LINES - 2;
	int		y;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(_hParentWnd, &rc);
	y = rc.top;
	if (y + (int)wndH > GetSystemMetrics(SM_CYSCREEN))
	{
		y = GetSystemMetrics(SM_CYSCREEN) - wndH ;
		if (y<0) y=0;
	}
	MoveWindow(hWnd, rc.left, y, wndW, wndH, TRUE);
}


static void
update_window(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	PAINTSTRUCT 	ps;
	Uint32			x;
	Uint32			y;

	/* 描画準備 */
	hDC = BeginPaint(hWnd, &ps);
	SetBkMode(hDC, OPAQUE);	//文字の背景を塗りつぶす
	SetBkColor(hDC, RGB(64,128,64));
	SetTextColor(hDC, RGB(240,240,240));
	hFont = (HFONT)GetStockObject(OEM_FIXED_FONT);
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	x = _FontWidth*3 -4;
	y = _FontHeight -4;
	TextOut(hDC, x, y, "Input \"<mpr><addr>:<value><cont>\"  e.g.)\"F62000:255+\"", 53);

	x = _FontWidth*3 -6;
	y += _FontHeight;
	TextOut(hDC, x, y, "( <value>=decimal. If <cont>='+' then keeping value. )", 54);

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}


static void
SetButtonPushed()
{
	char	code[32];
	char	buf[32];
	char*	pBuf;
	char	buf2[32];

	GetWindowText(_hWndC[EDIT_CODE], code, 32);
	if (code[0] == 0) //空欄でSETボタンを押した場合
	{
		_SetOk = 2; //空欄で設定完了の印
		strcpy(_Code, code); //入力したコードを保管
		PostMessage(_hWnd, WM_CLOSE, 0, 0);
		return;
	}

	strcpy(buf, code);
	pBuf =strchr(buf, ':');
	if (pBuf != NULL)
	{
		*pBuf = 0;
		if (strlen(buf) == 6)
		{
			strncpy(buf2, buf, 2);
			*(buf2 + 2) = 0;
			*_pMpr = (Uint32)strtol(buf2, NULL, 16);
			strncpy(buf2, buf+2, 4);
			*(buf2 + 4) = 0;
			*_pAddr = (Uint32)strtol(buf2, NULL, 16);
			strcpy(buf2, pBuf+1);
			pBuf =strchr(buf2, '+');
			if (pBuf != NULL)
			{
				*_pbContinuous = TRUE;
				*pBuf = 0;
			}
			else
				*_pbContinuous = FALSE;
			pBuf =strchr(buf2, 'X');
			if (pBuf != NULL) //16進数指定の場合
			{
				strcpy(buf2, pBuf+1);
				*_pData = (Uint8)strtol(buf2, NULL, 16);
				_SetOk = 1; //設定完了の印
				strcpy(_Code, code); //入力したコードを保管
				PostMessage(_hWnd, WM_CLOSE, 0, 0);
				return;
			}
			if (strlen(buf2) <= 3)
			{
				buf[0] = buf2[0];
				buf[1] = 0;
				if (strtol(buf, NULL, 16) < 10) //一桁目に16進数の文字(A-F)が入っていなければ
				{
					buf[0] = buf2[1];
					buf[1] = 0;
					if (strtol(buf, NULL, 16) < 10) //二桁目に16進数の文字(A-F)が入っていなければ
					{
						*_pData = (Uint8)atoi(buf2);
						_SetOk = 1; //設定完了の印
						strcpy(_Code, code); //入力したコードを保管
						PostMessage(_hWnd, WM_CLOSE, 0, 0);
						return;
					}
					else
						MessageBox(_hWnd, "Error. \"<value>\" should be 1-3 Digits (Decimal).    \nIf Hexadecimal is used, add \"0x\".    ", "Ootake", MB_OK);
				}
				else
					MessageBox(_hWnd, "Error. \"<value>\" should be 1-3 Digits (Decimal).    \nIf Hexadecimal is used, add \"0x\".    ", "Ootake", MB_OK);
			}
			else
				MessageBox(_hWnd, "Error. \"<value>\" should be 1-3 Digits (Decimal).    \nIf Hexadecimal is used, add \"0x\".    ", "Ootake", MB_OK);
		}
		else
			MessageBox(_hWnd, "Error. \"<mpr><addr>\" should be 6 Digits (Hexadecimal).    ", "Ootake", MB_OK);
	}
	else
		MessageBox(_hWnd, "Error. Separator[ : ] is not found.    ", "Ootake", MB_OK);

	//エラーダイアログを出した後の処理
	//MAINBOARD_ScreenUpdate(TRUE); //エラーダイアログ表示した場合、メイン画面が汚れるので更新。※これをやるとWriteMemoryフォーム自体が一瞬消えるのでカット
	SetFocus(GetDlgItem(_hWnd, EDIT_CODE));//エディットボックスにフォーカス
}


//EDITコンポーネント上でのキー入力を検知するためのフック
static LRESULT CALLBACK
EditProc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	if (uMsg == WM_CHAR) //キー入力を処理済にするためWM_KEYDOWNでなくWM_CHARを使用
	{
		//Escキーが押されていたら閉じる
		if (wParam == VK_ESCAPE)
		{
			PostMessage(_hWnd, WM_CLOSE, 0, 0);
			return 0; //0を返すことでWM_CHARを処理済に。ビープ音を鳴らさずに済む。
		}
		//Enterキーが押されていたら[SET]ボタンを押したことにする
		if (wParam == VK_RETURN)
		{
			SetButtonPushed();
			return 0; //0を返すことでWM_CHARを処理済に。ビープ音を鳴らさずに済む。
		}
	}
	return CallWindowProc(_WPEdit, hWnd, uMsg, wParam, lParam);
} 


static LRESULT CALLBACK
writeMemory_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		EnableWindow(_hParentWnd, FALSE);//Kitao追加。親ウインドウを無効化してモーダルに。
		EnableWindow(WINMAIN_GetHwnd(), FALSE);//Kitao追加。メインウインドウを無効化してモーダルに。
		_bToggle = FALSE;
		if (APP_GetFullScreen())
		{
			if (APP_GetDrawMethod() == 2)
			{
				APP_ToggleFullscreen(); //DirectDrawの場合、画面とフォーカスが乱れるので、ウィンドウモードに強制。
				_bToggle = TRUE;
			}
			else
				ShowWindow(WINMAIN_GetHwnd(), SW_HIDE); //フルスクリーンの場合、メインウィンドウを隠す。
		}
		_hFontB = CreateFont(  0,						// 高さ。0 = デフォルト
		                       0,						// 幅。0なら高さに合った幅
    		                   0,						// 角度
        		               0,						// ベースラインとの角度
            		           FW_NORMAL,				// 太さ
                		       FALSE,					// イタリック
	                    	   FALSE,					// アンダーライン
		                       FALSE,					// 打ち消し線
    		                   0,						// 日本語を取り扱うときはSHIFTJIS_CHARSETにする。
        		               0,						// 出力精度
            		           0,						// クリッピング精度
                		       0,						// 出力品質
                    		   0,						// ピッチとファミリー
		                       ""						// 書体名
							); //英語のデフォルトフォントに設定
		_FontWidth	= get_font_width(hWnd);
		_FontHeight = get_font_height(hWnd);
		set_window_size(hWnd);
		break;

	case WM_SIZE:
		if (_bToggle) //DirectDrawでフルスクリーンだった場合、切替後にウィンドウのアクティブ化が必要
		{
			SetForegroundWindow(_hParentWnd);
			SetForegroundWindow(hWnd);
		}
		break;

	case WM_PAINT:
		update_window(hWnd);
		SetFocus(GetDlgItem(hWnd, EDIT_CODE));//エディットボックスにフォーカス
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case BUTTON_CLEAR:
				SetWindowText(_hWndC[EDIT_CODE], "");//クリア
				SetFocus(GetDlgItem(hWnd, EDIT_CODE));//エディットボックスにフォーカス
				break;

			case BUTTON_SET:
				//戻り値を設定
				SetButtonPushed();
				break;
		}
		break;

	case WM_CLOSE:
		WRITEMEM_Deinit();
		if (_bToggle)
			APP_ToggleFullscreen(); //フルスクリーンに戻す。
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static BOOL
writeMemory_main()
{
	WNDCLASS	wc;
	HWND		hWnd;
	RECT		rc;
	Uint32		x;
	Uint32		y;
	HWND		hWndTmp;
	
	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = writeMemory_wnd_proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = _hInstance;
	wc.hIcon		 = LoadIcon(_hInstance, MAKEINTRESOURCE(OOTAKEICON)); //アイコンを読み込み。v2.00更新
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	_hMyb = CreateSolidBrush(RGB(64,128,64)); //ブラシを作る
	wc.hbrBackground = _hMyb;
	wc.lpszMenuName  = "";
	wc.lpszClassName = _pCaption;

	if (RegisterClass(&wc) == 0)
		return FALSE;

	hWnd = CreateWindow(
		_pCaption,
		_pCaption,
		WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		NULL,
		NULL,
		_hInstance,
		NULL
	);

	if (hWnd == NULL)
		return FALSE;

	_hWnd = hWnd;

	//エディットボックスを作成
	x = _FontWidth*11;
	y = _FontHeight*3+_FontHeight/2-2;
	_hWndC[1] = CreateWindow(
		"EDIT", "",
		WS_CHILD | WS_VISIBLE | ES_UPPERCASE,
		x, y+4, _FontWidth*18+_FontWidth/2, _FontHeight+2,
		_hWnd, (HMENU)EDIT_CODE, _hInstance, NULL
	);
	_WPEdit = (WNDPROC)GetWindowLong(_hWndC[1], GWL_WNDPROC); //キーフックするために元のプロシージャアドレスを退避
	SetWindowLong(_hWndC[1], GWL_WNDPROC, (LONG)EditProc);	  //自前のフックに書き換え
	SetWindowText(_hWndC[1], _Code);//前回の入力文字列をデフォルトにする
	SendMessage(_hWndC[1], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	//SendMessage(_hWndC[1], EM_SETSEL, (WPARAM)0, (LPARAM)-1);//文字列選択状態にする
	SendMessage(_hWndC[1], EM_SETSEL, (WPARAM)32, (LPARAM)32);//カーソルを右端へ持ってくる

	//Clearボタンを作成
	x = _FontWidth*30+2;
	y = _FontHeight*3+_FontHeight/2-2;
	hWndTmp = CreateWindow(
		"BUTTON", "Clear",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*11, _FontHeight+_FontHeight/2,
		_hWnd, (HMENU)BUTTON_CLEAR, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//SETボタンを作成
	x = _FontWidth*41+_FontWidth/2;
	y = _FontHeight*3+_FontHeight/2-2;
	hWndTmp = CreateWindow(
		"BUTTON", "Set",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*11, _FontHeight+_FontHeight/2,
		_hWnd, (HMENU)BUTTON_SET, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
	ImmAssociateContext(_hWnd, 0); //IMEを無効にする

	return TRUE;
}


BOOL
WRITEMEM_Init(
	HWND		hWnd,
	HINSTANCE	hInstance,
	Uint32*		mpr,
	Uint32*		addr,
	Uint8*		data,
	BOOL*		bContinuous,
	Sint32*		setOk)
{
	if (_hInstance != NULL)
		WRITEMEM_Deinit();

	_hParentWnd = hWnd;
	_hInstance = hInstance;

	_pMpr = mpr;
	_pAddr = addr;
	_pData = data;
	_pbContinuous = bContinuous;
	_pSetOk	= setOk;
	_SetOk	= -1; //キャンセル

	return writeMemory_main();
}


void
WRITEMEM_Deinit()
{
	if (_hInstance != NULL)
	{
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放
		
		//メインウィンドウをEnableに戻す。
		if (APP_GetFullScreen())
			ShowWindow(WINMAIN_GetHwnd(), SW_SHOWNORMAL);
		EnableWindow(WINMAIN_GetHwnd(), TRUE);
		//親ウィンドウにフォーカスを戻し前面に。
		EnableWindow(_hParentWnd, TRUE);
		SetForegroundWindow(_hParentWnd);
		
		*_pSetOk = _SetOk; //戻り値を設定。この瞬間に親ウィンドウのスレッドは動き出す。
	}
}


char*
WRITEMEM_GetCode()
{
	return _Code;
}

void
WRITEMEM_ClearCode()
{
	strcpy(_Code, "");
}
