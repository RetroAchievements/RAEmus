/******************************************************************************
Ootake

 [RecentRom.cpp]
	最近起動したゲームの一覧を表示するためのフォーム

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "RecentRom.h"
#include "resource.h"
#include "WinMain.h"
#include "App.h"

#define LINE_LEN	48
#define N_LINES 	22

enum RecentComponent
{
	LISTBOX_SELECTEDROM = 1
};
static HWND			_hWndC[1+1]; //各コンポーネントのハンドル

static WNDPROC		_WPListBox; //キーフックのため
static BOOL			_bKeyRepeatDown;
static BOOL			_bKeyRepeatUp;
static BOOL			_bKeyRepeatSel;
static BOOL			_bKeyRepeatRun;

static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFontB; //リスト用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "\"Ootake\" Game Select";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;
static BOOL 		_bToggle;

static Sint32		_SelectedRom = -1; //戻り値(決定した場合1～9999が返される)。キャンセルなら-1。未設定中は0。
static Sint32*		_pSelectedRom;


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
	Uint32	wndW = _FontWidth  * LINE_LEN - 3;
	Uint32	wndH = _FontHeight * N_LINES - 5;
	int		y;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	y = rc.top;
	if (y + (int)wndH > GetSystemMetrics(SM_CYSCREEN))
	{
		y = GetSystemMetrics(SM_CYSCREEN) - wndH ;
		if (y<0) y=0;
	}
	MoveWindow(hWnd, rc.left, y, wndW, wndH, TRUE);
}


//LISTBOXコンポーネント上でのキー入力を検知するためのフック
static LRESULT CALLBACK
ListBoxProc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	if (uMsg == WM_CHAR) //キー入力を処理済にするためWM_KEYDOWNでなくWM_CHARを使用
	{
		if (wParam == VK_ESCAPE) //Escキーが押されていたら閉じる
		{
			PostMessage(_hWnd, WM_CLOSE, 0, 0);
			return 0; //0を返すことでWM_CHARを処理済に。ビープ音を鳴らさずに済む。
		}
		if (wParam == 'O')
		{
			_SelectedRom = -2; //Open Rom
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0; //0を返すことでWM_CHARを処理済に。ビープ音を鳴らさずに済む。
		}
		if (wParam == 'P')
		{
			_SelectedRom = -3; //Open Cue
			PostMessage(hWnd, WM_CLOSE, 0, 0);
			return 0; //0を返すことでWM_CHARを処理済に。ビープ音を鳴らさずに済む。
		}
	}
	return CallWindowProc(_WPListBox, hWnd, uMsg, wParam, lParam);
} 


static LRESULT CALLBACK
recent_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	switch(uMsg)
	{
	case WM_CREATE:
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
		_hFontB = (HFONT)GetStockObject(SYSTEM_FONT);
		_FontWidth	= get_font_width(hWnd);
		_FontHeight = get_font_height(hWnd);
		set_window_size(hWnd);
		break;

	case WM_SIZE:
		if (_bToggle) //DirectDrawでフルスクリーンだった場合、切替後にウィンドウのアクティブ化が必要
		{
			SetForegroundWindow(WINMAIN_GetHwnd());
			SetForegroundWindow(hWnd);
		}
		break;

	case WM_PAINT:
		SetFocus(_hWnd);//リストボックスからフォーカスを外す
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
		{
			_SelectedRom = -1; //キャンセル
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		if (wParam == 'O')
		{
			_SelectedRom = -2; //Open Rom
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		if (wParam == 'P')
		{
			_SelectedRom = -3; //Open Cue
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;

	case WM_COMMAND:
		switch(HIWORD(wParam))
		{
			case LBN_DBLCLK:
				//戻り値を設定
				_SelectedRom = SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_GETCURSEL, 0, 0) + 1;
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
		}
		break;

	case WM_CLOSE:
		RECENT_Deinit();
		if (_bToggle) //DirectDrawでフルスクリーンだった場合
		{
			APP_RunEmulator(FALSE); //ウィンドウをアクティブにするためにメニューを表示してのポーズ。
			APP_ToggleFullscreen(); //フルスクリーンに戻す。
			APP_RunEmulator(TRUE); //ここでポーズ解除しておかないと、なぜか後ではポーズ解除がうまくいかない。DirectDrawは細かな動作が不安定で難しい。Direct3DはOK。
		}
		if (_SelectedRom == 0)
			_SelectedRom = -1; //キャンセル
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static BOOL
recent_main()
{
	WNDCLASS	wc;
	HWND		hWnd;
	RECT		rc;
	Uint32		x;
	Uint32		y;
	Sint32		i;
	MSG			msg;
	Sint32		a;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = recent_wnd_proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = _hInstance;
	wc.hIcon		 = LoadIcon(_hInstance, MAKEINTRESOURCE(OOTAKEICON)); //アイコンを読み込み
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

	//ROMセレクト用リストボックスを作成
	x = _FontWidth*2 - 2;
	y = _FontHeight - 5;
	_hWndC[1] = CreateWindow(
		"LISTBOX", "",
		WS_CHILD | WS_VISIBLE | LBS_NOTIFY,
		x, y, _FontWidth*44, _FontHeight*20,
		_hWnd, (HMENU)LISTBOX_SELECTEDROM, _hInstance, NULL
	);
	_WPListBox = (WNDPROC)GetWindowLong(_hWndC[1], GWL_WNDPROC); //キーフックするために元のプロシージャアドレスを退避
	SetWindowLong(_hWndC[1], GWL_WNDPROC, (LONG)ListBoxProc); //自前のフックに書き換え
	SendMessage(_hWndC[1], WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//フォントを設定
	for (i=1; i<=20; i++)
		SendMessage(_hWndC[1], LB_ADDSTRING, 0, (LPARAM)APP_GetRecentRom(i));
	SendMessage(_hWndC[1], LB_SETCURSEL, 0, 0);

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED);
	ImmAssociateContext(_hWnd, 0); //IMEを無効にする

	//ループ処理
	while (_SelectedRom == 0)
	{
		Sleep(1);

		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		if (APP_CheckButtonState(11)) //Iボタンを押した場合
		{
			//戻り値を設定
			_SelectedRom = SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_GETCURSEL, 0, 0) + 1;
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		if (APP_CheckButtonState(18)) //RUNボタンを押した場合
		{
			if (_bKeyRepeatRun)
			{	//前回押して、まだ押しっぱなしの場合
				if (!APP_CheckButtonState(18))
					_bKeyRepeatRun = FALSE;
			}
			else
			{
				//戻り値を設定
				_SelectedRom = SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_GETCURSEL, 0, 0) + 1;
				PostMessage(hWnd, WM_CLOSE, 0, 0);
			}
		}
		else
			_bKeyRepeatRun = FALSE;

		if (APP_CheckButtonState(12)) //IIボタンを押した場合
		{
			_SelectedRom = -1; //キャンセル
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}

		if (APP_CheckButtonState(2)) //下ボタンを押した場合
		{
			if (_bKeyRepeatDown)
			{	//前回押して、まだ押しっぱなしの場合
				if (!APP_CheckButtonState(2))
					_bKeyRepeatDown = FALSE;
			}
			else
			{
				a = SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_GETCURSEL, 0, 0) + 1;
				if (a == 20)
					a = 0;
				SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_SETCURSEL, (WPARAM)a, 0);
				_bKeyRepeatDown = TRUE;
			}
		}
		else
			_bKeyRepeatDown = FALSE;
		if ((APP_CheckButtonState(17))&&(APP_CheckFuncAndSelConflict())) //SELECTボタンを押した場合
		{
			if (_bKeyRepeatSel)
			{	//前回押して、まだ押しっぱなしの場合
				if (!APP_CheckButtonState(17))
					_bKeyRepeatSel = FALSE;
			}
			else
			{
				a = SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_GETCURSEL, 0, 0) + 1;
				if (a == 20)
					a = 0;
				SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_SETCURSEL, (WPARAM)a, 0);
				_bKeyRepeatSel = TRUE;
			}
		}
		else
			_bKeyRepeatSel = FALSE;

		if (APP_CheckButtonState(8)) //上ボタンを押した場合
		{
			if (_bKeyRepeatUp)
			{	//前回押して、まだ押しっぱなしの場合
				if (!APP_CheckButtonState(8))
					_bKeyRepeatUp = FALSE;
			}
			else
			{
				a = SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_GETCURSEL, 0, 0) - 1;
				if (a == -1)
					a = 19;
				SendMessage(GetDlgItem(hWnd, LISTBOX_SELECTEDROM), LB_SETCURSEL, (WPARAM)a, 0);
				_bKeyRepeatUp = TRUE;
			}
		}
		else
			_bKeyRepeatUp = FALSE;
	}

	//決定キーを離すまで待つ。これがないとゲーム復帰時に誤操作の可能性あり。
	while ((APP_CheckButtonState(11))||(APP_CheckButtonState(12))||(APP_CheckButtonState(18))) //I,II,RUNキーが全て離されるまで待つ。
		Sleep(1);
	
	//戻り値を設定
	*_pSelectedRom = _SelectedRom; //この瞬間にApp.c に制御が戻る。

	return TRUE;
}


BOOL
RECENT_Init(
	HINSTANCE	hInstance,
	Sint32*		selectedRom)
{
	if (_hInstance != NULL)
		RECENT_Deinit();

	_hInstance = hInstance;

	_bKeyRepeatDown	= FALSE;
	_bKeyRepeatUp	= FALSE;
	_bKeyRepeatSel	= FALSE;
	_bKeyRepeatRun	= TRUE; //Runボタンは押された状態でここが呼ばれる

	_pSelectedRom = selectedRom;
	_SelectedRom = 0;

	return recent_main();
}


void
RECENT_Deinit()
{
	if (_hInstance != NULL)
	{
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放
		
		//メインウィンドウをEnableに戻し、前面に。
		if (APP_GetFullScreen())
			ShowWindow(WINMAIN_GetHwnd(), SW_SHOWNORMAL);
		EnableWindow(WINMAIN_GetHwnd(), TRUE);
		SetForegroundWindow(WINMAIN_GetHwnd());
	}
}

