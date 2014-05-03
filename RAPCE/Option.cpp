/******************************************************************************
Ootake

 [Option.cpp]
	スタート時に設定を変更するためのフォーム

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Option.h"
#include "resource.h"
#include "Startup.h"
#include "WinMain.h"
#include "App.h"

#define LINE_LEN	48
#define N_LINES 	21

enum OptionComponent
{
	COMBOBOX_SCREENTYPE = 1,
	COMBOBOX_DRAWMODE,
	COMBOBOX_DRAWMETHOD,
	COMBOBOX_VSYNC,
	COMBOBOX_PSGTYPE,
	COMBOBOX_SOUNDBUFFER,
	BUTTON_LIGHT,
	BUTTON_MIDDLE,
	BUTTON_DEFAULT,
	BUTTON_SET,
};
static HWND			_hWndC[10+1]; //各コンポーネントのハンドル

static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFontB; //ボタン用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "\"Ootake\" Option";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;


static Sint32		_ScreenType = -1; //戻り値(決定した場合1～5が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pScreenType;
static Sint32		_DrawMode = -1; //戻り値(決定した場合1～4が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pDrawMode;
static Sint32		_DrawMethod = -1; //戻り値(決定した場合1～2が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pDrawMethod;
static Sint32		_VSync = -1; //戻り値(決定した場合1～2が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pVSync;
static Sint32		_PSGType = -1; //戻り値(決定した場合1～3が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pPSGType;
static Sint32		_SoundBuffer = -1; //戻り値(決定した場合1～9が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pSoundBuffer;


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
	RECT		rc;
	Uint32		wndW = _FontWidth  * LINE_LEN +_FontWidth/2 +2;
	Uint32		wndH = _FontHeight * N_LINES +_FontHeight/2 -2;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(STARTUP_GetHwnd(), &rc);
	MoveWindow(hWnd, rc.left, rc.bottom - wndH, wndW, wndH, TRUE);
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

	x = _FontWidth*2 +2;
	y = _FontHeight -2;

	TextOut(hDC, x, y, "Video Setting  ビデオ環境の設定", 31);	y += _FontHeight*9+_FontHeight/2;
	TextOut(hDC, x, y, "Sound Setting  サウンド環境の設定", 33);

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}


static LRESULT CALLBACK
option_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
		EnableWindow(STARTUP_GetHwnd(), FALSE);//スタートアップウィンドウを無効化してモーダルに。
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

	case WM_PAINT:
		update_window(hWnd);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case COMBOBOX_SCREENTYPE:
			case COMBOBOX_DRAWMODE:
			case COMBOBOX_DRAWMETHOD:
			case COMBOBOX_VSYNC:
			case COMBOBOX_PSGTYPE:
			case COMBOBOX_SOUNDBUFFER:
				if (HIWORD(wParam) == CBN_CLOSEUP)
					SetFocus(GetDlgItem(hWnd, BUTTON_SET));//OKボタンにフォーカス
				break;

			case BUTTON_LIGHT:
				SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_SETCURSEL, 5, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 1, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMETHOD], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_VSYNC], CB_SETCURSEL, 1, 0);
				SendMessage(_hWndC[COMBOBOX_PSGTYPE], CB_SETCURSEL, 2, 0);
				SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 7, 0);
				break;

			case BUTTON_MIDDLE:
				SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_SETCURSEL, 1, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 1, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMETHOD], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_VSYNC], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_PSGTYPE], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 5, 0);
				break;

			case BUTTON_DEFAULT:
				SendMessage(_hWndC[COMBOBOX_SCREENTYPE], CB_SETCURSEL, 2, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMODE], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_DRAWMETHOD], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_VSYNC], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_PSGTYPE], CB_SETCURSEL, 0, 0);
				SendMessage(_hWndC[COMBOBOX_SOUNDBUFFER], CB_SETCURSEL, 5, 0);
				break;

			case BUTTON_SET:
				//戻り値を設定
				_ScreenType = SendMessage(GetDlgItem(hWnd, COMBOBOX_SCREENTYPE), CB_GETCURSEL, 0, 0) + 1;
				_DrawMode = SendMessage(GetDlgItem(hWnd, COMBOBOX_DRAWMODE), CB_GETCURSEL, 0, 0) + 1;
				_DrawMethod = SendMessage(GetDlgItem(hWnd, COMBOBOX_DRAWMETHOD), CB_GETCURSEL, 0, 0) + 1;
				_VSync = SendMessage(GetDlgItem(hWnd, COMBOBOX_VSYNC), CB_GETCURSEL, 0, 0) + 1;
				_PSGType = SendMessage(GetDlgItem(hWnd, COMBOBOX_PSGTYPE), CB_GETCURSEL, 0, 0) + 1;
				_SoundBuffer = SendMessage(GetDlgItem(hWnd, COMBOBOX_SOUNDBUFFER), CB_GETCURSEL, 0, 0) + 1;
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
		}
		break;

	case WM_CLOSE:
		OPTION_Deinit();
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static BOOL
option_main()
{
	WNDCLASS	wc;
	HWND		hWnd;
	RECT		rc;
	Uint32		x;
	Uint32		y;
	HWND		hWndTmp;
	
	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = option_wnd_proc;
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

	//フルスクリーンorウィンドウモード切替用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*2;
	_hWndC[1] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*10,
		_hWnd, (HMENU)COMBOBOX_SCREENTYPE, _hInstance, NULL
	);
	SendMessage(_hWndC[1], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	SendMessage(_hWndC[1], CB_ADDSTRING, 0, (LPARAM)"Start Window mode");
	SendMessage(_hWndC[1], CB_ADDSTRING, 0, (LPARAM)"Start Window (x2)");
	SendMessage(_hWndC[1], CB_ADDSTRING, 0, (LPARAM)"Start Window (x3)");
	SendMessage(_hWndC[1], CB_ADDSTRING, 0, (LPARAM)"Start Window (x4)");
	SendMessage(_hWndC[1], CB_ADDSTRING, 0, (LPARAM)"Start FullScreen mode");
	SendMessage(_hWndC[1], CB_ADDSTRING, 0, (LPARAM)"Start FullScreen (640x480)");
	if (APP_GetStartFullScreen())
		SendMessage(_hWndC[1], CB_SETCURSEL, 4, 0);
	else
		SendMessage(_hWndC[1], CB_SETCURSEL, 0, 0);

	//TV or RGB切替用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*4;
	_hWndC[2] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*8,
		_hWnd, (HMENU)COMBOBOX_DRAWMODE, _hInstance, NULL
	);
	SendMessage(_hWndC[2], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	SendMessage(_hWndC[2], CB_ADDSTRING, 0, (LPARAM)"Special Scanlined (Sharp&Gentle)");
	SendMessage(_hWndC[2], CB_ADDSTRING, 0, (LPARAM)"TV Mode (Fast)");
	SendMessage(_hWndC[2], CB_ADDSTRING, 0, (LPARAM)"Horizontal Scanlined");
	SendMessage(_hWndC[2], CB_ADDSTRING, 0, (LPARAM)"Non-Scanlined (Fast)");
	switch (APP_GetScanLineType())
	{
		case 0: SendMessage(_hWndC[2], CB_SETCURSEL, 3, 0); break;
		case 1: SendMessage(_hWndC[2], CB_SETCURSEL, 0, 0); break;
		case 2: SendMessage(_hWndC[2], CB_SETCURSEL, 2, 0); break;
		//case 3: 縦スキャンライン。現在未実装。
		case 4: SendMessage(_hWndC[2], CB_SETCURSEL, 1, 0); break;
	}

	//Direct3D or DirectDraw切替用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*6;
	_hWndC[3] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*8,
		_hWnd, (HMENU)COMBOBOX_DRAWMETHOD, _hInstance, NULL
	);
	SendMessage(_hWndC[3], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	SendMessage(_hWndC[3], CB_ADDSTRING, 0, (LPARAM)"Use Direct3D (Default)");
	SendMessage(_hWndC[3], CB_ADDSTRING, 0, (LPARAM)"Use DirectDraw (Old)");
	if (APP_GetDrawMethod() == 2)
		SendMessage(_hWndC[3], CB_SETCURSEL, 1, 0);
	else //1
		SendMessage(_hWndC[3], CB_SETCURSEL, 0, 0);

	//V-Sync切替用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*8;
	_hWndC[4] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*8,
		_hWnd, (HMENU)COMBOBOX_VSYNC, _hInstance, NULL
	);
	SendMessage(_hWndC[4], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	SendMessage(_hWndC[4], CB_ADDSTRING, 0, (LPARAM)"V-Sync 59-61Hz (Beauty,Default)");
	SendMessage(_hWndC[4], CB_ADDSTRING, 0, (LPARAM)"Non-Use V-Sync (Fast,Flicker)");
	if (APP_GetSyncTo60HzScreen())
		SendMessage(_hWndC[4], CB_SETCURSEL, 0, 0);
	else
		SendMessage(_hWndC[4], CB_SETCURSEL, 1, 0);

	//PSGサウンドタイプ用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*11+_FontHeight/2;
	_hWndC[5] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*8,
		_hWnd, (HMENU)COMBOBOX_PSGTYPE, _hInstance, NULL
	);
	SendMessage(_hWndC[5], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	SendMessage(_hWndC[5], CB_ADDSTRING, 0, (LPARAM)"High Quality PSG (Default)");
	SendMessage(_hWndC[5], CB_ADDSTRING, 0, (LPARAM)"A Little Light PSG");
	SendMessage(_hWndC[5], CB_ADDSTRING, 0, (LPARAM)"Light PSG (Fast)");
	switch (APP_GetPsgQuality())
	{
		case 1: SendMessage(_hWndC[5], CB_SETCURSEL, 2, 0); break;
		case 2: SendMessage(_hWndC[5], CB_SETCURSEL, 1, 0); break;
		default: //4
			SendMessage(_hWndC[5], CB_SETCURSEL, 0, 0); break;
	}

	//サウンドバッファ用コンボボックスを作成
	x = _FontWidth*2;
	y = _FontHeight*13+_FontHeight/2;
	_hWndC[6] = CreateWindow(
		"COMBOBOX", "",
		WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWNLIST,
		x, y, _FontWidth*42, _FontHeight*16,
		_hWnd, (HMENU)COMBOBOX_SOUNDBUFFER, _hInstance, NULL
	);
	SendMessage(_hWndC[6], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 1024 (Not Delay)");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 1152");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 1280");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 1408");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 1536");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 1664 (Default)");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 1792");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 2048");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 2176");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 2304");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 2560");
	SendMessage(_hWndC[6], CB_ADDSTRING, 0, (LPARAM)"Buffer 3072 (Beauty,Delay)");
	switch (APP_GetSoundBufferSize())
	{
		case 1024: SendMessage(_hWndC[6], CB_SETCURSEL,  0, 0); break;
		case 1152: SendMessage(_hWndC[6], CB_SETCURSEL,  1, 0); break;
		case 1280: SendMessage(_hWndC[6], CB_SETCURSEL,  2, 0); break;
		case 1408: SendMessage(_hWndC[6], CB_SETCURSEL,  3, 0); break;
		case 1536: SendMessage(_hWndC[6], CB_SETCURSEL,  4, 0); break;
		case 1792: SendMessage(_hWndC[6], CB_SETCURSEL,  6, 0); break;
		case 2048: SendMessage(_hWndC[6], CB_SETCURSEL,  7, 0); break;
		case 2176: SendMessage(_hWndC[6], CB_SETCURSEL,  8, 0); break;
		case 2304: SendMessage(_hWndC[6], CB_SETCURSEL,  9, 0); break;
		case 2560: SendMessage(_hWndC[6], CB_SETCURSEL, 10, 0); break;
		case 3072: SendMessage(_hWndC[6], CB_SETCURSEL, 11, 0); break;
		default: //1664
			SendMessage(_hWndC[6], CB_SETCURSEL, 5, 0); break;
	}

	//Lightボタンを作成
	x = _FontWidth*2;
	y = _FontHeight*16;
	hWndTmp = CreateWindow(
		"BUTTON", "Light(Fast)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*13, _FontHeight*2-6,
		_hWnd, (HMENU)BUTTON_LIGHT, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//MiddleLightボタンを作成
	x = _FontWidth*16;
	y = _FontHeight*16;
	hWndTmp = CreateWindow(
		"BUTTON", "MiddleLight",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*14, _FontHeight*2-6,
		_hWnd, (HMENU)BUTTON_MIDDLE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//Defaultボタンを作成
	x = _FontWidth*31;
	y = _FontHeight*16;
	hWndTmp = CreateWindow(
		"BUTTON", "Default(HQ)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*14, _FontHeight*2-6,
		_hWnd, (HMENU)BUTTON_DEFAULT, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定

	//SETボタンを作成
	x = _FontWidth*36;
	y = _FontHeight*19;
	hWndTmp = CreateWindow(
		"BUTTON", "Set",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*10, _FontHeight*2-6,
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
OPTION_Init(
	HINSTANCE	hInstance,
	Sint32*		screenType,
	Sint32*		drawMode,
	Sint32*		drawMethod,
	Sint32*		vsync,
	Sint32*		psgType,
	Sint32*		soundBuffer)
{
	if (_hInstance != NULL)
		OPTION_Deinit();

	_hInstance = hInstance;

	_pScreenType = screenType;
	_ScreenType = -1;//キャンセル
	_pDrawMode = drawMode;
	_DrawMode = -1;//キャンセル
	_pDrawMethod = drawMethod;
	_DrawMethod = -1;//キャンセル
	_pVSync = vsync;
	_VSync = -1;//キャンセル
	_pPSGType = psgType;
	_PSGType = -1;//キャンセル
	_pSoundBuffer = soundBuffer;
	_SoundBuffer = -1;//キャンセル

	return option_main();
}


void
OPTION_Deinit()
{
	if (_hInstance != NULL)
	{
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放
		
		//スタートアップウィンドウにフォーカスを戻し前面に。
		EnableWindow(STARTUP_GetHwnd(), TRUE);
		SetForegroundWindow(STARTUP_GetHwnd());
		
		//戻り値を設定
		*_pScreenType = _ScreenType;
		*_pDrawMode = _DrawMode;
		*_pDrawMethod = _DrawMethod;
		*_pVSync = _VSync;
		*_pPSGType = _PSGType;
		*_pSoundBuffer = _SoundBuffer; //この瞬間にStartup.c に制御が戻る。
	}
}

