/******************************************************************************
Ootake

 [CDInstall.c]
	CDのデータをインストールすることでアクセスの高速化を図る

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <process.h>
#include "CDInstall.h"
#include "resource.h"
#include "WinMain.h"
#include "CDROM.h"
#include "TocDB.h"
#include "App.h"
#include "CDInterface.h"


#define CDINSTALL_CAPTION 	"Ootake CD Install"

#define LINE_LEN	52
#define N_LINES 	7

#define BUTTON_ABORT	0


static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFont; //通常文字用フォント
static HFONT		_hFontB; //ボタン用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = CDINSTALL_CAPTION;
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;
static BOOL			_bFullInstall; //v2.24追加


//CD Install スレッド
static HANDLE	_hThread;
static DWORD	_dwThreadID;

static Sint32	_Result;


/* フォントの高さを取得する */
static Uint32
get_font_height(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFontOld = (HFONT)SelectObject(hDC, _hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	ReleaseDC(hWnd, hDC);

	return (Uint32)(tm.tmHeight);
}

/* フォントの横幅を取得する */
static Uint32
get_font_width(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC 	 = GetDC(hWnd);
	hFontOld = (HFONT)SelectObject(hDC, _hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	ReleaseDC(hWnd, hDC);

	return (Uint32)tm.tmAveCharWidth;
}


static void
set_window_size(
	HWND			hWnd)
{
	RECT		rc;
	Uint32		wndW = _FontWidth  * LINE_LEN;
	Uint32		wndH = _FontHeight * N_LINES;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	MoveWindow(hWnd, rc.left, rc.top, wndW, wndH, TRUE);
}


static void
update_window(
	HWND			hWnd)
{
	HDC 			hDC;
	HFONT			hFontOld;
	PAINTSTRUCT 	ps;
	Uint32			x;
	Uint32			y;
	char			buf[100];

	/* 描画準備 */
	hDC = BeginPaint(hWnd, &ps);
	SetBkMode(hDC, OPAQUE);	//文字の背景を塗りつぶす
	SetBkColor(hDC, RGB(64,128,64));
	SetTextColor(hDC, RGB(240,240,240));
	hFontOld = (HFONT)SelectObject(hDC, _hFont);

	x = _FontWidth*2 +1;
	y = _FontHeight;

	sprintf(buf, "\"%s\"", TOCDB_GetGameTitle());
	TextOut(hDC, x, y, buf, strlen(buf)); y+=_FontHeight;
	sprintf(buf, " Required HDD Space - %dMB.", (int)CDROM_GetInstallRequiredHDD());
	TextOut(hDC, x, y, buf, strlen(buf)); y+=_FontHeight;
	y+=_FontHeight;

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	ReleaseDC(hWnd, hDC);
}


static LRESULT CALLBACK
cdinstall_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	LOGFONT		logFont;

	switch(uMsg)
	{
	case WM_CREATE:
		EnableWindow(WINMAIN_GetHwnd(), FALSE);//メインウィンドウを無効化してモーダルに。
		_hFont = (HFONT)GetStockObject(SYSTEM_FONT);
		GetObject(_hFont, sizeof(LOGFONT), &logFont);
		logFont.lfWeight = FW_BOLD;
		_hFont = CreateFontIndirect(&logFont);
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
			case BUTTON_ABORT:
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
		}
		break;

	case WM_CLOSE:
		if (MessageBox(hWnd, "Abort Install?", "Ootake", MB_YESNO) == IDYES)
		{
			if (_Result == 0) //Yesボタンを押すまでの間にインストールが完了した場合も考慮
				_Result = -1; //中断
		}
		return 0; //閉じずに続ける
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static DWORD WINAPI
cdinstall_thread()
{
	Sint32			installRequiredHDD = CDROM_GetInstallRequiredHDD();
	ULARGE_INTEGER	ulFreeBytesAvailableToCaller;
	ULARGE_INTEGER	ulTotalNumberOfFreeBytes;
	ULARGE_INTEGER	ulTotalNumberOfBytes;
	Sint32			check;

	//取り込めるトラックがなかった場合。CDを入れてないときも含む。
	if (installRequiredHDD == 0)
	{
		MessageBox(_hWnd, "There is no data that can be installed. Please put CD-ROM of the \"PC Engine\".    ", "Ootake", MB_OK);
		_Result = -1; //失敗,中断
		PostMessage(_hWnd, WM_QUIT, 0, 0);
		return 0;
	}

	//HDDの空き容量が足りない場合、メッセージを表示して中止する。
	if(GetDiskFreeSpaceEx(APP_GetAppPath(), &ulFreeBytesAvailableToCaller, &ulTotalNumberOfBytes, &ulTotalNumberOfFreeBytes))
		if ((__int64)ulTotalNumberOfFreeBytes.QuadPart/1024/1024 < installRequiredHDD+100)
		{
			MessageBox(_hWnd, "Free space of HDD doesn't suffice.    ", "Ootake", MB_OK);
			_Result = -1; //失敗,中断
			PostMessage(_hWnd, WM_QUIT, 0, 0);
			return 0;
		}

	//すでにインストールが完了済みの場合、メッセージを表示して終了。
	check = CDROM_CheckCDInstall(); //CDIF_GetBadInstalled()の値も設定される。
	if (((!_bFullInstall)&&(check != 0))||
		((_bFullInstall)&&(check == 2)&&((!CDIF_GetBadInstalled())||(APP_GetCueFile())))) //フルインストール時、バッドインストール(古いOotakeでのインストール)でかつCUEファイル起動ではない場合は、フルインストールやり直しを許可する。
	{
		if (check == 2)
		{
			if ((_bFullInstall)&&(CDIF_GetBadInstalled())&&(APP_GetCueFile())) //バッドインストール(古いOotakeでのインストール)でcue起動の場合
				MessageBox(_hWnd, "It is started with \"Installed Play\" now.    \nPlease start with \"CD-ROM drive\" to FullInstall.    ", "Ootake", MB_OK);
			else
				MessageBox(_hWnd, "\"CD FullInstall\" of this game is already completed.    ", "Ootake", MB_OK);
		}
		else
			MessageBox(_hWnd, "\"CD Install\" of this game is already completed.    ", "Ootake", MB_OK);
		_Result = 1; //完了
		PostMessage(_hWnd, WM_QUIT, 0, 0);
		return 0;
	}

	//インストール処理
	if (CDROM_CDInstall(_hWnd, _bFullInstall))
	{
        if (_bFullInstall)
		{
			MessageBox(_hWnd, "\"CD FullInstall\" of this game was completed.    \n\n"
							  "- Please use \"Image File(CUE+ISO+WAV)\" carefully.    \n"
							  "  Because distributing \"Image File(CUE+ISO+WAV)\" is illegal.    \n"
							  "  Please avoid direct sunshine and moisture, and keep    \n"
							  "  \"Mastering CD-ROM\" importantly.\n\n"
							  "In Japanese language\n"
							  "※イメージファイル(CUE+ISO+WAV)は、他の方へ配布すると著作権    \n"
							  "法違反になりますので、必ず慎重に扱ってください。また、オリジナル    \n"
							  "の実CD盤も所有し続ける必要があります。実CD盤は、直射日光や    \n"
							  "湿気を避け大切に保管なさってください。", "Ootake", MB_OK);
		}
		else
			MessageBox(_hWnd, "\"CD Install\" of this game was completed.    ", "Ootake", MB_OK);
		_Result = 1; //完了
	}
	else
		_Result = -1; //失敗,中断

	PostMessage(_hWnd, WM_QUIT, 0, 0);
	return 0;
}


BOOL
CDINSTALL_Init(
	HINSTANCE		hInstance,
	BOOL			bFullInstall)
{
	WNDCLASS		wc;
	HWND			hWnd;
	Uint32			x;
	Uint32			y;
	HWND			hWndTmp;
	MSG				msg;

	if (_hInstance != NULL)
		CDINSTALL_Deinit();

	//フルインストールかどうかの変数を設定
	_bFullInstall = bFullInstall;

	//HDDの必要容量を計算しておく
	CDROM_SetInstallRequiredHDD(_bFullInstall);

	_hInstance = hInstance;
	_Result = 0;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = cdinstall_wnd_proc;
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
		WS_MINIMIZEBOX | WS_SYSMENU | WS_CAPTION,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		0,
		0,
		NULL,
		NULL,
		_hInstance,
		NULL
	); //メインスレッドでCreateすることでCD取り込み中でもボタンの受付等ができる。

	if (hWnd == NULL)
		return 0;

	_hWnd = hWnd;

	//ボタンを作成
	x = _FontWidth*16;
	y = _FontHeight*4 +3;
	hWndTmp = CreateWindow(
		"BUTTON", "&Abort (Esc key)",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*21, _FontHeight*2-3,
		_hWnd, (HMENU)BUTTON_ABORT, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定

	CloseWindow(WINMAIN_GetHwnd());//メインウィンドウを最小化
	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	ImmAssociateContext(_hWnd, 0); //IMEを無効にする

	//CD読み取り処理専用スレッドを作成して実行する	
	_hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)cdinstall_thread, NULL, 0, &_dwThreadID);

	//メッセージループ 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	CDINSTALL_Deinit();
	return TRUE;
}


void
CDINSTALL_Deinit()
{
	if (_hInstance != NULL)
	{
		CloseHandle(_hThread);
		
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		
		DeleteObject(_hFont); //通常文字用フォントを開放
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放
		
		//メインウィンドウにフォーカスを戻し元の大きさに。
		EnableWindow(WINMAIN_GetHwnd(), TRUE);
		OpenIcon(WINMAIN_GetHwnd());
	}
}


Sint32
CDINSTALL_GetResult()
{
	return _Result;
}

