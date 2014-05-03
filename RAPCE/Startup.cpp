/******************************************************************************
Ootake

 [Srartup.c]
	スタート時のメッセージ表示＆CD-ROM・Huカードの選択

Copyright(C)2006-2009 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

******************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Startup.h"
#include "resource.h"
#include "WinMain.h"
#include "Option.h"
#include "App.h"

#define LINE_LEN	92
#define N_LINES 	25

#define BUTTON_CDROM	1
#define BUTTON_CUE		2
#define BUTTON_HUCARD	3
#define BUTTON_REPLAY	4
#define BUTTON_OPTION	5
#define BUTTON_FAVORITE	6


static HBRUSH		_hMyb; //自作ブラシ色
static HFONT		_hFontB; //ボタン用フォント

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = "To Enjoy \"Ootake\" Truly.";
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;


static Sint32		_CDorHu = -1; //戻り値。CDなら1。CDをCUE起動なら2。Huカードなら3。リプレイなら4。キャンセルなら-1。未設定中は0。
static Sint32*		_pCDorHu;


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
	Uint32		wndW = _FontWidth  * LINE_LEN +2;
	Uint32		wndH = _FontHeight * N_LINES -2;
	Uint32		a;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	a = rc.bottom - rc.top - 16 -(rc.right - rc.left - 598)/2/*横枠＝下枠の幅*/; //a=メインウィンドウの上枠の高さ。16はメインウィンドウ(WinMain)のクライアントの高さ
	MoveWindow(hWnd, rc.left, rc.top + a -1, wndW, wndH, TRUE);
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

	x = _FontWidth*2 +1;
	y = _FontHeight;

	TextOut(hDC, x, y, "In English:", strlen("In English:") );	y += _FontHeight;
	y += _FontHeight;
	TextOut(hDC, x, y, "Using ROMs of games which you do not own is illegal.", strlen("Using ROMs of games which you do not own is illegal."));	y += _FontHeight;
	TextOut(hDC, x, y, "You must only use ROMs for games which you have backed up yourself.", strlen("You must only use ROMs for games which you have backed up yourself."));	y += _FontHeight;
	TextOut(hDC, x, y, "Do not use this emulator to play illegally downloaded ROMs (this is Piracy!)", strlen("Do not use this emulator to play illegally downloaded ROMs (this is Piracy!)."));		y += _FontHeight;
	y += _FontHeight;
	TextOut(hDC, x, y, "Piracy damages publishers, developers and the games industry as a whole.", strlen("Piracy damages publishers, developers and the games industry as a whole."));	y += _FontHeight;
	y += _FontHeight;
	TextOut(hDC, x, y, "Ootake/RAPCE allows PC Engine games to be reborn. Enjoy responsibly.", strlen("Ootake/RAPCE allows PC Engine games to be reborn. Enjoy responsibly."));	y += _FontHeight;
	y += _FontHeight;
	TextOut(hDC, x, y, "In Japanese: 起動前に少し堅いお話を。", 37);	y += _FontHeight;
	TextOut(hDC, x, y, "自身で所有していないソフトのＲＯＭイメージを使用することは、汗水流して心を込めてそのソフ", 88);	y += _FontHeight;
	TextOut(hDC, x, y, "トをを制作された方に対しての礼儀を欠いた違法な行為になりますので、必ず自身で所有している", 88);	y += _FontHeight;
	TextOut(hDC, x, y, "ソフトで利用なさってください。またこのことは、自身で所有しているソフトのＲＯＭイメージを", 88);	y += _FontHeight;
	TextOut(hDC, x, y, "違法にダウンロードすることを推奨する意味ではありません。", 56);	y += _FontHeight;
	y += _FontHeight;
	TextOut(hDC, x, y, "残念なことですが、歳には勝てず、ＰＣエンジン実機の部品寿命も年々尽きつつある状況です…。", 88);	y += _FontHeight;
	TextOut(hDC, x, y, "このエミュレーターによって、少しでもハッピーな気持ちが生まれてくれると幸いに思います。", 86);	y += _FontHeight;
	y += _FontHeight;

	x = _FontWidth*46 + _FontWidth/2;
	y += 11;
	TextOut(hDC, x  , y, "<- Please choose.", 17);							y += _FontHeight;
	TextOut(hDC, x-1, y, "   [Add CD-ROM before pressing button]", 39);		y += _FontHeight;
	TextOut(hDC, x-1, y, "   [CD-ROMをセット後にボタンを押して下さい]", 43);

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}


static LRESULT CALLBACK
startup_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	MSG				msg;
	Sint32			screenType;
	Sint32			drawMode;
	Sint32			drawMethod;
	Sint32			vsync;
	Sint32			psgType;
	Sint32			soundBuffer;

	switch (uMsg)
	{
	case WM_CREATE:
		EnableWindow(WINMAIN_GetHwnd(), FALSE);//メインウィンドウを無効化してモーダルに。
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

	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
			case BUTTON_CDROM:
				_CDorHu = 1; //戻り値を設定
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			case BUTTON_CUE:
				_CDorHu = 2; //戻り値を設定
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			case BUTTON_HUCARD:
				_CDorHu = 3; //戻り値を設定
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			case BUTTON_REPLAY:
				_CDorHu = 4; //戻り値を設定
				PostMessage(hWnd, WM_CLOSE, 0, 0);
				break;
			case BUTTON_OPTION:
				soundBuffer = 0; //全項目の値が設定されたかどうかの判断にも使用。
				OPTION_Init(_hInstance, &screenType, &drawMode, &drawMethod, &vsync, &psgType, &soundBuffer); //Optionウィンドウを表示
				//メッセージループ
				while (soundBuffer == 0) //ダイアログが結果を返すまでループ
				{ 
					GetMessage(&msg ,NULL, 0, 0);
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				//Ok(SET)ボタンが押されていた場合、スクリーン設定を変更する。
				if (screenType > 0)
				{
					switch (screenType)
					{
						case 1:	APP_SetStartFullScreen(FALSE); break;
						case 2:
							APP_SetMagnification(2); //x2倍に設定
							APP_SetStartFullScreen(FALSE);
							break;
						case 3:
							APP_SetMagnification(3); //x3倍に設定
							APP_SetStartFullScreen(FALSE);
							break;
						case 4:
							APP_SetMagnification(4); //x4倍に設定
							APP_SetStartFullScreen(FALSE);
							break;
						case 5:	APP_SetStartFullScreen(TRUE); break;
						case 6:
							APP_SetFullScreen640(); //640x480に設定
							APP_SetStartFullScreen(TRUE);
							break;
					}
				}
				//Ok(SET)ボタンが押されていた場合、DrawMode設定を変更する。
				if (drawMode > 0)
				{
					switch (drawMode)
					{
						case 1: APP_SetScanLineType(1,TRUE); break;
						case 2: APP_SetScanLineType(4,TRUE); break;
						case 3: APP_SetScanLineType(2,TRUE); break;
						case 4: APP_SetScanLineType(0,TRUE); break;
					}
				}
				//Ok(SET)ボタンが押されていた場合、DrawMethod設定を変更する。
				if (drawMethod > 0)
				{
					APP_SetDrawMethod(drawMethod);
				}
				//Ok(SET)ボタンが押されていた場合、V-Sync設定を変更する。
				if (screenType > 0)
				{
					if (vsync == 2)
						APP_SetSyncTo60HzScreen(FALSE);
					else //==1
						APP_SetSyncTo60HzScreen(TRUE);
				}
				//Ok(SET)ボタンが押されていた場合、PSGクォリティ設定を変更する。
				if (psgType > 0)
				{
					if (psgType == 3)
						APP_SetPsgQuality(1);
					else if (psgType == 2)
						APP_SetPsgQuality(2);
					else //==1
						APP_SetPsgQuality(4);
				}
				//Ok(SET)ボタンが押されていた場合、PSGクォリティ設定を変更する。
				if (soundBuffer > 0)
				{
					switch (soundBuffer)
					{
						case  1: APP_SetSoundBufferSize(1024); break;
						case  2: APP_SetSoundBufferSize(1152); break;
						case  3: APP_SetSoundBufferSize(1280); break;
						case  4: APP_SetSoundBufferSize(1408); break;
						case  5: APP_SetSoundBufferSize(1536); break;
						case  7: APP_SetSoundBufferSize(1792); break;
						case  8: APP_SetSoundBufferSize(2048); break;
						case  9: APP_SetSoundBufferSize(2176); break;
						case 10: APP_SetSoundBufferSize(2304); break;
						case 11: APP_SetSoundBufferSize(2560); break;
						case 12: APP_SetSoundBufferSize(3072); break;
						default: //==6
							APP_SetSoundBufferSize(1664); break;
					}
				}
				break;
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		if ((wParam == 'C')||(wParam == VK_RETURN)) //「C」キーまたは 「Enter」キー
		{
			_CDorHu = 1; //戻り値を設定
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		if (wParam == 'P') //「P」キー
		{
			_CDorHu = 2; //戻り値を設定
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		if ((wParam == 'H')||(wParam == 'O')||(wParam == VK_SPACE)) //「H」キー「O」キーまたは スペースキー
		{
			_CDorHu = 3; //戻り値を設定
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		if (wParam == 'R') //「R」キー
		{
			_CDorHu = 4; //戻り値を設定
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		}
		break;

	case WM_CLOSE:
		STARTUP_Deinit();
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static BOOL
startup_main()
{
	WNDCLASS	wc;
	HWND		hWnd;
	RECT		rc;
	Uint32		x;
	Uint32		y;
	HWND		hWndTmp;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = startup_wnd_proc;
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

	//ボタンを作成
	x = _FontWidth*2 +1;
	y = _FontHeight*20 +3;
	hWndTmp = CreateWindow(
		"BUTTON", "Load &CD-ROM Game",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*21, _FontHeight*2-4,
		_hWnd, (HMENU)BUTTON_CDROM, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定
	
	x += _FontWidth*21 +3;
	hWndTmp = CreateWindow(
		"BUTTON", "Load &Hu-CARD Game",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*22, _FontHeight*2-4,
		_hWnd, (HMENU)BUTTON_HUCARD, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定
	
	x = _FontWidth*2 +1;
	y += _FontHeight*2 -2;
	hWndTmp = CreateWindow(
		"BUTTON", "Option",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*9, _FontHeight + _FontHeight/2+2,
		_hWnd, (HMENU)BUTTON_OPTION, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定
/*
	x = _FontWidth*2;
	hWndTmp = CreateWindow(
		"BUTTON", "&Favorite",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*11, _FontHeight + _FontHeight/2+2,
		_hWnd, (HMENU)BUTTON_FAVORITE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定
*/
	x += _FontWidth*9 +3;
	hWndTmp = CreateWindow(
		"BUTTON", "&Play Installed CD",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, _FontWidth*21, _FontHeight + _FontHeight/2+2,
		_hWnd, (HMENU)BUTTON_CUE, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定

	x += _FontWidth*21 +3;
	hWndTmp = CreateWindow(
		"BUTTON", "&Replay",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		x, y, (_FontWidth*30 + 4 + _FontWidth*15)-x, _FontHeight + _FontHeight/2+2,
		_hWnd, (HMENU)BUTTON_REPLAY, _hInstance, NULL
	);
	SendMessage(hWndTmp, WM_SETFONT, (WPARAM)_hFontB, MAKELPARAM(TRUE, 0));//文字のフォントを設定

	ShowWindow(_hWnd, SW_SHOWNORMAL);
	UpdateWindow(_hWnd);
	GetWindowRect(_hWnd, &rc);
	SetWindowPos(_hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //手前に表示
	ImmAssociateContext(_hWnd, 0); //Kitao追加。IMEを無効にする。v0.79

	return TRUE;
}


BOOL
STARTUP_Init(
	HINSTANCE	hInstance,
	Sint32*		CDorHu)
{
	if (_hInstance != NULL)
		STARTUP_Deinit();

	_hInstance = hInstance;

	_pCDorHu = CDorHu;
	_CDorHu = -1;//キャンセル

	return startup_main();
}


void
STARTUP_Deinit()
{
	if (_hInstance != NULL)
	{
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		DeleteObject(_hFontB); //ボタン用フォントを開放
		DeleteObject(_hMyb); //ブラシを開放
		
		//メインウィンドウにフォーカスを戻し前面に。
		EnableWindow(WINMAIN_GetHwnd(), TRUE);
		APP_SetForegroundWindowOotake(); //確実にアクティブにする
		
		*_pCDorHu = _CDorHu; //戻り値を設定。この瞬間にメインウィンドウは動き出す。
	}
}


const HWND
STARTUP_GetHwnd()
{
	return _hWnd;
}

