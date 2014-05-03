/******************************************************************************
Ootake
・他のアプリへの影響を考慮して、Windowsのボリュームミキサーは不使用にした。
・PSGのボリュームを個別に設定できるようにした。
・65535（最大音量）のときはスクロールバーのポジションを65600にするようにした。
・0（最小音量）も表現するようにした。

Copyright(C)2006-2009 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[AudioConfig.c]

		Implements an audio configuration window.

	Copyright (C) 2005 Ki

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
******************************************************************************/
#define _CRT_SECURE_NO_DEPRECATE

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "AudioConfig.h"
#include "resource.h"
#include "WinMain.h"
#include "APU.h"
#include "ADPCM.h"
#include "PSG.h"
#include "CDROM.h"
#include "MainBoard.h"
#include "App.h"
#include "AppEvent.h"


#define AUDIOCONFIG_CAPTION 	"Volume Balance Setting"

#define LINE_LEN	42
#define N_LINES 	9


static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = AUDIOCONFIG_CAPTION;
static HINSTANCE	_hInstance = NULL;
static HWND 		_hWnd;


static HWND 		_hApuSB;
static HWND 		_hPsgSB;
static HWND 		_hAdpcmSB;
static HWND 		_hCdSB;

static Uint32*		_pApu = 0;
static Uint32*		_pPsg = 0;
static Uint32*		_pAdpcm = 0;
static Uint32*		_pCd = 0;


/* audio config thread */
static HANDLE	_hThread = INVALID_HANDLE_VALUE;
static DWORD	_dwThreadID;


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
	Uint32		wndW = _FontWidth  * LINE_LEN;
	Uint32		wndH = _FontHeight * N_LINES;

	SetRect(&rc, 0, 0, wndW, wndH);
	AdjustWindowRectEx(&rc, GetWindowLong(hWnd, GWL_STYLE),
						GetMenu(hWnd) != NULL, GetWindowLong(hWnd, GWL_EXSTYLE));
	wndW = rc.right - rc.left;
	wndH = rc.bottom - rc.top;
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	MoveWindow(hWnd, rc.left, rc.top, wndW, wndH, TRUE); //Kitao更新。座標を変えた。
}


static void
update_scrollbars()
{
	SCROLLINFO		si;

	si.cbSize = sizeof(si);
	si.fMask  = SIF_RANGE | SIF_POS | SIF_PAGE;
	si.nMin   = 0;
	si.nMax   = 66256; //Kitao更新。nPageで割り切れる数に変更。65536～65600は65535にする。最大値のところでスクロールバーのドラッグが終わったときに「この値マイナスnPage」に戻されてしまうようなので65560+656の値に設定した。
	si.nPage  = 656;

	if (*_pApu == 65535) //Kitao追加。65535（最大音量）のときはスクロールバーのポジションを65600にする。
		si.nPos = si.nMax;
	else
		si.nPos = *_pApu;
	SetScrollInfo(_hApuSB, SB_CTL, &si, TRUE);
	
	if (*_pPsg == 65535) //Kitao追加。65535（最大音量）のときはスクロールバーのポジションを65600にする。
		si.nPos = si.nMax;
	else
		si.nPos = *_pPsg;
	SetScrollInfo(_hPsgSB, SB_CTL, &si, TRUE);
	
	if (*_pAdpcm == 65535) //Kitao追加。65535（最大音量）のときはスクロールバーのポジションを65600にする。
		si.nPos = si.nMax;
	else
		si.nPos = *_pAdpcm;
	SetScrollInfo(_hAdpcmSB, SB_CTL, &si, TRUE);
	
	if (*_pCd == 65535) //Kitao追加。65535（最大音量）のときはスクロールバーのポジションを65600にする。
		si.nPos = si.nMax;
	else
		si.nPos = *_pCd;
	SetScrollInfo(_hCdSB, SB_CTL, &si, TRUE);
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
	char			buf[256];
	int 			percent;

	/* 描画準備 */
	hDC 	 = BeginPaint(hWnd, &ps);
	hFont	 = (HFONT)GetStockObject(OEM_FIXED_FONT);
	hFontOld = (HFONT)SelectObject(hDC, hFont);
	SetBkColor(hDC, RGB(0,0,0));
	SetTextColor(hDC, RGB(224, 224, 224));

	/* 文字の背景を塗りつぶす */
	SetBkMode(hDC, OPAQUE);

	x = _FontWidth*2;
	y = _FontHeight + 1;
	TextOut(hDC, x, y, "MASTER",	strlen("MASTER"));	y += 2*_FontHeight;
	TextOut(hDC, x, y, "PSG",		strlen("PSG"));	y += 2*_FontHeight; //Kitao追加
	TextOut(hDC, x, y, "ADPCM",		strlen("ADPCM"));	y += 2*_FontHeight;
	TextOut(hDC, x, y, "CD-DA",		strlen("CD-DA"));	y += 2*_FontHeight;

	x = _FontWidth + _FontWidth*8 + _FontWidth*25; //Kitao更新
	y = _FontHeight + 1;

	if (*_pApu == 65535) //Kitao更新。100%を表現
		percent = 100;
	else
		percent = *_pApu *100 /65600; //Kitao変更。+1.0をやめてゼロも表現した。
	sprintf(buf, "%3d[%%]", percent);	TextOut(hDC, x, y, buf, strlen(buf));	y += 2*_FontHeight;

	if (*_pPsg == 65535) //Kitao更新。100%を表現
		percent = 100;
	else
		percent = *_pPsg *100 /65600; //Kitao変更
	sprintf(buf, "%3d[%%]", percent);	TextOut(hDC, x, y, buf, strlen(buf));	y += 2*_FontHeight;

	if (*_pAdpcm == 65535) //Kitao更新。100%を表現
		percent = 100;
	else
		percent = *_pAdpcm *100 /65600; //Kitao変更
	sprintf(buf, "%3d[%%]", percent);	TextOut(hDC, x, y, buf, strlen(buf));	y += 2*_FontHeight;

	if (*_pCd == 65535) //Kitao更新。100%を表現
		percent = 100;
	else
		percent = *_pCd *100 / 65600; //Kitao変更
	sprintf(buf, "%3d[%%]", percent);	TextOut(hDC, x, y, buf, strlen(buf));	y += 2*_FontHeight;

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}


static Uint32
on_scroll(
	HWND		hWnd,
	Uint32		code)
{
	SCROLLINFO		si;

	si.cbSize = sizeof(si);

	switch (code)
	{
	case SB_LINEDOWN:
		si.fMask  = SIF_RANGE | SIF_POS | SIF_PAGE;
		GetScrollInfo(hWnd, SB_CTL, &si);
		si.nPos += si.nPage;
		if (si.nPos > si.nMax)
			si.nPos = si.nMax;
		break;

	case SB_LINEUP:
		si.fMask  = SIF_POS | SIF_PAGE;
		GetScrollInfo(hWnd, SB_CTL, &si);
		if (si.nPos >= (int)si.nPage)
			si.nPos -= si.nPage;
		else
			si.nPos = 0;
		break;

	case SB_PAGEDOWN:
		si.fMask  = SIF_RANGE | SIF_POS | SIF_PAGE;
		GetScrollInfo(hWnd, SB_CTL, &si);
		si.nPos += si.nPage*10;
		if (si.nPos > si.nMax)
			si.nPos = si.nMax;
		break;

	case SB_PAGEUP:
		si.fMask  = SIF_POS | SIF_PAGE;
		GetScrollInfo(hWnd, SB_CTL, &si);
		if (si.nPos >= (int)si.nPage*10)
			si.nPos -= si.nPage*10;
		else
			si.nPos = 0;
		break;

	case SB_THUMBTRACK:
		si.fMask = SIF_POS | SIF_TRACKPOS | SIF_PAGE;
		GetScrollInfo(hWnd, SB_CTL, &si);
		si.nPos = (int)(ceil((double)si.nTrackPos / (double)si.nPage)) * si.nPage; //Kitao更新。100%が表現できるように端数を切り上げ。
		break;

	case SB_THUMBPOSITION:
	case SB_ENDSCROLL:
		si.fMask = SIF_POS;
		GetScrollInfo(hWnd, SB_CTL, &si);
		break;
	}

	si.cbSize = sizeof(si);
	si.fMask = SIF_POS;
	SetScrollInfo(hWnd, SB_CTL, &si, TRUE);

	if (si.nPos > 65535)
		return 65535; //Kitao追加。スクロールバーのポジションが65535を超えていた場合、返す値は65535にする。

	return si.nPos;
}


static LRESULT CALLBACK
audioconfig_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	Uint32		val;

	switch(uMsg)
	{
	case WM_CREATE:
		EnableWindow(WINMAIN_GetHwnd(), FALSE);//Kitao追加。メインウィンドウを無効化してモーダルに。
		_FontWidth	= get_font_width(hWnd);
		_FontHeight = get_font_height(hWnd);
		set_window_size(hWnd);
		break;

	case WM_PAINT:
		update_scrollbars();
		update_window(hWnd);
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case WM_HSCROLL:
		val = on_scroll((HWND)lParam, LOWORD(wParam));
		
		if ((HWND)lParam == _hApuSB)
		{
			*_pApu		 = val;
			APU_SetVolume(*_pApu);
		}
		else if ((HWND)lParam == _hPsgSB)
		{
			*_pPsg		 = val;
			PSG_SetVolume(*_pPsg);
		}
		else if ((HWND)lParam == _hAdpcmSB)
		{
			*_pAdpcm	 = val;
			ADPCM_SetVolume(*_pAdpcm);
		}
		else if ((HWND)lParam == _hCdSB)
		{
			*_pCd	 = val;
			CDROM_SetCdVolume(*_pCd);
		}
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
		break;

	case WM_DESTROY:
		//メインウィンドウにフォーカスを戻し元の大きさに。
		EnableWindow(WINMAIN_GetHwnd(), TRUE);
		OpenIcon(WINMAIN_GetHwnd());
		if (MAINBOARD_GetPause()) //ポーズ中ならポーズ解除
			APPEVENT_Set(APPEVENT_RUN, NULL);
		
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static HWND
create_scrollbar(
	const char* 	pText,
	HWND			hWnd,
	HINSTANCE		hInstance,
	Uint32			x,
	Uint32			y,
	Uint32			w,
	Uint32			h)
{
	return CreateWindow(
							TEXT("SCROLLBAR"),
							TEXT(pText),
							WS_CHILD | WS_VISIBLE,
							x,
							y,
							w,
							h,
							hWnd,
							NULL,
							hInstance,
							NULL);
}



static DWORD WINAPI
audio_config_thread(
	LPVOID	param)
{
	WNDCLASS	wc;
	HWND		hWnd;
	Uint32		y;
	RECT		rc;//Kitao追加
	MSG 		msg;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = audioconfig_wnd_proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = _hInstance;
	wc.hIcon		 = LoadIcon(_hInstance, MAKEINTRESOURCE(OOTAKEICON)); //アイコンを読み込み。v2.00更新
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName  = "";
	wc.lpszClassName = _pCaption;

	if (RegisterClass(&wc) == 0)
		return 0;

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
	); //新スレッド内でCreateすることでWindow操作時にメインスレッドが止まってしまうことがない

	if (hWnd == NULL)
		return FALSE;

	_hWnd	   = hWnd;

	y = _FontHeight;
	_hApuSB = create_scrollbar(
					"APU",
					hWnd,
					_hInstance,
					_FontWidth*9,
					y,
					_FontWidth*25,
					_FontHeight+2);

	y += 2*_FontHeight;
	_hPsgSB = create_scrollbar(
					"PSG", //Kitao追加。PSGも個別に調整できるように。
					hWnd,
					_hInstance,
					_FontWidth*9,
					y,
					_FontWidth*25,
					_FontHeight+2);

	y += 2*_FontHeight;
	_hAdpcmSB = create_scrollbar(
					"ADPCM",
					hWnd,
					_hInstance,
					_FontWidth*9,
					y,
					_FontWidth*25,
					_FontHeight+2);

	y += 2*_FontHeight;
	_hCdSB = create_scrollbar(
					"CD-DA", //Kitao更新。Windowsミキサーの音量ではなく、エミュレータ上で音量を調節。
					hWnd,
					_hInstance,
					_FontWidth*9,
					y,
					_FontWidth*25,
					_FontHeight+2);

	update_scrollbars();

	CloseWindow(WINMAIN_GetHwnd());//メインウィンドウを最小化
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	GetWindowRect(hWnd, &rc);
	SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //手前に表示
	ImmAssociateContext(hWnd, 0); //Kitao追加。IMEを無効にする。v0.79

	// メッセージループ 
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}


BOOL
AUDIOCONFIG_Init(
	HINSTANCE		hInstance,
	Uint32* 		apuVal,
	Uint32* 		psgVal,
	Uint32* 		adpcmVal,
	Uint32* 		cdVal)
{
	if (_hInstance != NULL)
		AUDIOCONFIG_Deinit();

	_hInstance = hInstance;

	_pApu		= apuVal;
	_pPsg		= psgVal;
	_pAdpcm 	= adpcmVal;
	_pCd 		= cdVal;

	// スレッドを作成して実行する	
	_hThread = CreateThread(NULL, 0, audio_config_thread, NULL, 0, &_dwThreadID);
	if (_hThread == NULL)
		return FALSE;

	return TRUE;
}


void
AUDIOCONFIG_Deinit()
{
	if (_hInstance != NULL)
	{
		PostThreadMessage(_dwThreadID, WM_QUIT, 0, 0);

		// スレッドの終了を待つ 
		if (_hThread != INVALID_HANDLE_VALUE)
		{
			WaitForSingleObject(_hThread, INFINITE);
			CloseHandle(_hThread);
			_hThread = INVALID_HANDLE_VALUE;
		}
		UnregisterClass(_pCaption, _hInstance);
		_dwThreadID = 0;
		_hInstance = NULL;
		_hWnd = NULL;
	}
}

