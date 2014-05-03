/******************************************************************************
Ootake
・PCEパッド1～5にキーボードのキーも含めて自由に割り当てできるようにした。
・Windows用16ボタンパッドとハットスイッチ（アナログ対応パッドでよく使われてい
  る）にも対応した。
・v0.54。ジョイパッド入力の判定部分をゲーム本番同様にDirectInputで行うようにし
  た。（0.53以前の２本以上ジョイパッドをつないでいたときの不具合を解消）
・連射専用ボタンの設定を追加。２ボタン，３ボタンパッド時に、ボタン５,６の割り
  当て領域を使用する。（６ボタンパッド時には設定できない仕様）
・早送り用ボタンの設定を追加。
・ステートセーブ用ボタンの設定を追加。
・スクリーンショット用ボタンの設定を追加。

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[PadConfig.c]

		Implements a pad configuration window.

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

#define DIRECTINPUT_VERSION	0x0800	//Kitao追加。環境にもよるかもしれないが、DirectInput5が軽い。7だとやや遅延あり。スペースハリアーがわかりやすい。

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "PadConfig.h"
#include "resource.h"
#include "WinMain.h"
#include "Input.h"
#include "JoyPad.h" //Kitao追加


#define PADCONFIG_CAPTION	"Pad Configuration"


#define LINE_LEN			70 //Kitao更新
#define N_LINES				20

#define N_MAXJOYSTICK		5


//Kitao更新。ハットスイッチと16ボタンパッドにも対応。
static const Sint32	button[INPUT_NUM_BUTTON] =
{
	INPUT_JOYSTICK_UP,       INPUT_JOYSTICK_RIGHT,    INPUT_JOYSTICK_DOWN,     INPUT_JOYSTICK_LEFT,
	INPUT_JOYSTICK_POVUP,    INPUT_JOYSTICK_POVRIGHT, INPUT_JOYSTICK_POVDOWN,  INPUT_JOYSTICK_POVLEFT,
	INPUT_JOYSTICK_BUTTON1,  INPUT_JOYSTICK_BUTTON2,  INPUT_JOYSTICK_BUTTON3,  INPUT_JOYSTICK_BUTTON4,
	INPUT_JOYSTICK_BUTTON5,  INPUT_JOYSTICK_BUTTON6,  INPUT_JOYSTICK_BUTTON7,  INPUT_JOYSTICK_BUTTON8,
	INPUT_JOYSTICK_BUTTON9,  INPUT_JOYSTICK_BUTTON10, INPUT_JOYSTICK_BUTTON11, INPUT_JOYSTICK_BUTTON12,
	INPUT_JOYSTICK_BUTTON13, INPUT_JOYSTICK_BUTTON14, INPUT_JOYSTICK_BUTTON15, INPUT_JOYSTICK_BUTTON16
};

typedef struct
{
	DIJOYSTATE2		joyState;
	Uint32			buttonState; //Kitao更新。16ボタンに対応。ハットスイッチぶんも入れたのでUint32に。
} JOYSTICK;


//Kitao追加。DirectInputを使用する。
static LPDIRECTINPUT			_pDI		= NULL;			// DirectInput インターフェースポインタ
static LPDIRECTINPUTDEVICE		_pDIDKey	= NULL;			// DirectInput Keyboard device
static LPDIRECTINPUTDEVICE2		_pDIDJoy[N_MAXJOYSTICK];	// DirectInput Joystick device

static Uint32		_FontWidth;
static Uint32		_FontHeight;
static const char*	_pCaption = PADCONFIG_CAPTION;
static HINSTANCE	_hInstance = NULL;
static HWND			_hWnd;
static BOOL			_bWindowCreated; //Kitao追加

static char*		_pText[N_LINES];
static char			_Text[N_LINES][LINE_LEN];
static Uint32		_Line = 0;


static Sint32		_nJoySticks;
static JOYSTICK		_Joystick[N_MAXJOYSTICK];

static Sint32		_Mode; //Kitao追加。0…通常の設定。1…連射専用ボタンの設定。2…早回し用ボタンの設定。3…ステートセーブ用ボタンの設定。4…ステートロード用ボタンの設定。
						   //			5…スクリーンショット用ボタンの設定。6…ファンクション用ボタンの設定。7…ファンクションボタン時のステートセーブ＆ロード用ボタンの設定。
						   //			8…ポーズ用ボタンの設定。
static Sint32		_PadID; //Kitao追加
static PCEPAD*		_pPad; //Kitao追加
static Sint32		_SetOk = -1; //Kitao追加。戻り値。設定完了なら1。キャンセルなら-1。未設定中は0。
static Sint32*		_pSetOk = 0; //Kitao追加
static char			_DIKeys[256]; //Kitao追加。キーボードの状態用


/* フォントの高さを取得する */
static Uint32
get_font_height(
	HWND			hWnd)
{
	HDC				hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC      = GetDC(hWnd);
	hFont    = (HFONT)GetStockObject(OEM_FIXED_FONT);   /* 固定ピッチフォント */
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
	HDC				hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	TEXTMETRIC		tm;

	hDC      = GetDC(hWnd);
	hFont    = (HFONT)GetStockObject(OEM_FIXED_FONT);   /* 固定ピッチフォント */
	hFontOld = (HFONT)SelectObject(hDC, hFont);

	GetTextMetrics(hDC, &tm);

	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);

	return (Uint32)tm.tmAveCharWidth;
}


//Kitao更新
static void
set_window_size(
	HWND			hWnd)
{
	RECT	rc;
	Uint32	wndW = _FontWidth  * LINE_LEN;
	Uint32	wndH = _FontHeight * N_LINES;
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


static void
update_window(
	HWND			hWnd)
{
	HDC				hDC;
	HFONT			hFont;
	HFONT			hFontOld;
	PAINTSTRUCT		ps;
	Uint32			i;

	/* 描画準備 */
	hDC      = BeginPaint(hWnd, &ps);
	hFont    = (HFONT)GetStockObject(OEM_FIXED_FONT);
	hFontOld = (HFONT)SelectObject(hDC, hFont);
	SetBkColor(hDC, RGB(0,0,0));
	SetTextColor(hDC, RGB(224, 224, 224));

	/* 文字の背景を塗りつぶす */
	SetBkMode(hDC, OPAQUE);

	for (i=0; i<_Line; i++)
	{
		TextOut(hDC, 0, i*_FontHeight, _pText[i], strlen(_pText[i]));
	}

	/* 終了処理 */
	EndPaint(hWnd, &ps);
	SelectObject(hDC, hFontOld);
	DeleteObject(hFont);
	ReleaseDC(hWnd, hDC);
}


static void
add_text(
	const char*		pText, ...)
{
	Uint32		i;
	va_list		ap;
	char*		p;

	va_start(ap, pText);
	vsprintf(_pText[_Line++], pText, ap);
	va_end(ap);

	// scroll a line
	if (_Line == N_LINES)
	{
		p = _pText[0];
		for (i = 1; i < N_LINES; ++i)
		{
			_pText[i-1] = _pText[i];
		}
		_pText[N_LINES-1] = p;
		*p = '\0';

		--_Line;
	}

	InvalidateRect(_hWnd, NULL, FALSE); //Kitao更新
	UpdateWindow(_hWnd);
}


/*-----------------------------------------------------------------------------
	[joypad_update_state]
		入力状況を更新します。
-----------------------------------------------------------------------------*/
static void
joypad_update_state()
{
	int		i;
	HRESULT hResult;

	for (i = 0; i < _nJoySticks; i++)
	{
		// ポーリングを行なう
		hResult = _pDIDJoy[i]->Poll();
		if (hResult != DI_OK) //失敗したときはアクセス権を取り直してやり直す
		{
			_pDIDJoy[i]->Acquire();
			_pDIDJoy[i]->Poll();
		}

		// ジョイスティックの状態を読む
		_pDIDJoy[i]->GetDeviceState(sizeof(DIJOYSTATE2), &_Joystick[i].joyState);

		// ボタンの状態を更新する (とりあえず 12 個のボタンに対応) Kitao更新。16個のボタンに対応
		_Joystick[i].buttonState = 0;
		if (_Joystick[i].joyState.rgbButtons[0] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON1;
		if (_Joystick[i].joyState.rgbButtons[1] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON2;
		if (_Joystick[i].joyState.rgbButtons[2] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON3;
		if (_Joystick[i].joyState.rgbButtons[3] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON4;
		if (_Joystick[i].joyState.rgbButtons[4] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON5;
		if (_Joystick[i].joyState.rgbButtons[5] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON6;
		if (_Joystick[i].joyState.rgbButtons[6] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON7;
		if (_Joystick[i].joyState.rgbButtons[7] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON8;
		if (_Joystick[i].joyState.rgbButtons[8] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON9;
		if (_Joystick[i].joyState.rgbButtons[9] & 0x80)		_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON10;
		if (_Joystick[i].joyState.rgbButtons[10] & 0x80)	_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON11;
		if (_Joystick[i].joyState.rgbButtons[11] & 0x80)	_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON12;
		if (_Joystick[i].joyState.rgbButtons[12] & 0x80)	_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON13;
		if (_Joystick[i].joyState.rgbButtons[13] & 0x80)	_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON14;
		if (_Joystick[i].joyState.rgbButtons[14] & 0x80)	_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON15;
		if (_Joystick[i].joyState.rgbButtons[15] & 0x80)	_Joystick[i].buttonState |= INPUT_JOYSTICK_BUTTON16;
		if (_Joystick[i].joyState.lY < -250)	_Joystick[i].buttonState |= INPUT_JOYSTICK_UP;
		if (_Joystick[i].joyState.lX > +250)	_Joystick[i].buttonState |= INPUT_JOYSTICK_RIGHT;
		if (_Joystick[i].joyState.lY > +250)	_Joystick[i].buttonState |= INPUT_JOYSTICK_DOWN;
		if (_Joystick[i].joyState.lX < -250)	_Joystick[i].buttonState |= INPUT_JOYSTICK_LEFT;
		//Kitao更新。ハットスイッチ（アナログ対応コントローラの十字ボタンでよく使われる）にも対応。
		if (_Joystick[i].joyState.rgdwPOV[0] == 0)				_Joystick[i].buttonState |= INPUT_JOYSTICK_POVUP;
		if (_Joystick[i].joyState.rgdwPOV[0] == 45*DI_DEGREES)
		{
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVUP;
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVRIGHT;
		}
		if (_Joystick[i].joyState.rgdwPOV[0] == 90*DI_DEGREES)	_Joystick[i].buttonState |= INPUT_JOYSTICK_POVRIGHT;
		if (_Joystick[i].joyState.rgdwPOV[0] == 135*DI_DEGREES)
		{
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVRIGHT;
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVDOWN;
		}
		if (_Joystick[i].joyState.rgdwPOV[0] == 180*DI_DEGREES)		_Joystick[i].buttonState |= INPUT_JOYSTICK_POVDOWN;
		if (_Joystick[i].joyState.rgdwPOV[0] == 225*DI_DEGREES)
		{
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVDOWN;
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVLEFT;
		}
		if (_Joystick[i].joyState.rgdwPOV[0] == 270*DI_DEGREES)	_Joystick[i].buttonState |= INPUT_JOYSTICK_POVLEFT;
		if (_Joystick[i].joyState.rgdwPOV[0] == 315*DI_DEGREES)
		{
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVLEFT;
			_Joystick[i].buttonState |= INPUT_JOYSTICK_POVUP;
		}
	}
}


//Kitao追加。キーボードの状態を読み取って更新する。
static void
keyborad_update_state()
{
	HRESULT hResult;

	// キーボードの状態を読む
	hResult =_pDIDKey->GetDeviceState(256, &_DIKeys);

	// 読み取りに失敗した時の処理
	if (hResult != DI_OK) //失敗したときはアクセス権を取り直してやり直す
	{
		_pDIDKey->Acquire();
		_pDIDKey->GetDeviceState(256, &_DIKeys);
	}
}


static BOOL
pump_message()
{
	MSG			msg;

	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	if (_hInstance == NULL)
		return FALSE;

	return TRUE;
}


static BOOL
get_button(
	Sint16*		pButton)
{
	Sint16		q,i; //Kitao追加
	Sint16		theJoystick = -1; //Kitao追加
	Sint16		theButton = -1;

	while (theButton < 0)
	{
		Sleep(1);

		if (!pump_message())
			return FALSE;

		//ジョイスティック
		joypad_update_state(); //ジョイスティックの状態を読む
		for (q=0; q<=_nJoySticks-1; q++) //Kitao追加。つながれている全てのジョイスティックのボタンを受け付けるようにした。
			for (i=0; i<=INPUT_NUM_BUTTON-1; i++)
				if (_Joystick[q].buttonState & button[i]) //ボタンが押されていたら
				{
					//押されたボタンが離されるまで待つ
					while (_Joystick[q].buttonState & button[i])
					{
						Sleep(1);
						if (!pump_message())
							return FALSE;
						joypad_update_state();
					}
					
					theJoystick = q+1; //Kitao追加。どのジョイスティックのボタンか
					theButton = i;
					if (theButton <=7) //十字キーorハットスイッチ
						*pButton = (200 + theJoystick*100) + theButton;
					else //ボタン
						*pButton = (200 + theJoystick*100 +50) + (theButton -8);
					
					return TRUE;
				}

		//Kitao追加。キーボード
		keyborad_update_state(); //キーボードの状態を読む
		for (i=0; i<=255; i++)
			if ((_DIKeys[i] & 0x80)&&(i != DIK_ESCAPE)&&(i != DIK_CAPITAL)&&(i != DIK_KANJI)&& //何かキーが押されていたら（Esc＆CAPS＆漢字(半角/全角)キー以外）。v0.83更新(Kiさん修正ありがとうございます)
				(i != DIK_CAPITAL)&&(i != DIK_KANA)) //「CapsLockキー」と「かなキー」も無効に。(押してしまうとキーを受け付けなくなる)。v1.33追加
			{
				//押されたボタンが離されるまで待つ
				while (_DIKeys[i] & 0x80)
				{
					Sleep(1);
					if (!pump_message())
						return FALSE;
					keyborad_update_state();
				}
				
				if (i == DIK_F1) //F1キーなら設定をクリア
					i = -1;
				*pButton = i;
				
				return TRUE;
			}
	}
	return FALSE;
}


//Kitao更新
static BOOL
configure(
	Sint32		mode, //Kitao追加。0…通常の設定。1…連射専用ボタンの設定。2～…特殊ボタンの設定
	Sint32		padID, //Kitao更新。joyID→padID(1～5)へ。PCエンジンのパッドナンバーで管理するようにした。
	PCEPAD*		pPad)
{
	Sint16		up        = -1;
	Sint16		right     = -1;
	Sint16		down      = -1;
	Sint16		left      = -1;
	Sint16		select    = -1;
	Sint16		run       = -1;
	Sint16		buttonI   = -1;
	Sint16		buttonII  = -1;
	Sint16		buttonIII = -1;
	Sint16		buttonIV  = -1;
	Sint16		buttonV   = -1;
	Sint16		buttonVI  = -1;

	add_text("\n");

	switch (mode)
	{
		case 0:
		case 1:	//通常の設定or連射専用ボタンの設定
			if (JOYPAD_GetConnectSixButton()) //Kitao追加
				add_text("Setting of Player#%ld for \"6 Button Pad\": (\"Esc\"=Abort \"F1\"=Clear)", padID);
			else if (JOYPAD_GetConnectThreeButton()) //Kitao追加
				add_text("Setting of Player#%ld for \"3 Button Pad\": (\"Esc\"=Abort \"F1\"=Clear)", padID);
			else
				add_text("Setting of Player#%ld for \"2 Button Pad\": (\"Esc\"=Abort \"F1\"=Clear)", padID);
			if (mode == 1)
			{	//連射ボタンの設定時。ボタン５,６に連射専用ボタンを設定。
				up		= pPad->buttonU;
				right	= pPad->buttonR;
				down	= pPad->buttonD;
				left	= pPad->buttonL;
				select	= pPad->buttonSel;
				run		= pPad->buttonRun;
				buttonI	= pPad->button1;
				buttonII= pPad->button2;
				buttonIII=pPad->button3;
				buttonIV= pPad->button4;
				add_text("Press a button for \"turbo button I\"..."); if (!get_button(&buttonV)) return FALSE;
				add_text("Press a button for \"turbo button II\"...");  if (!get_button(&buttonVI))  return FALSE;
			}
			else 
			{	//通常時
				add_text("Press a button for \"UP\"...");         if (!get_button(&up))        return FALSE;
				add_text("Press a button for \"RIGHT\"...");      if (!get_button(&right))     return FALSE;
				add_text("Press a button for \"DOWN\"...");       if (!get_button(&down))      return FALSE;
				add_text("Press a button for \"LEFT\"...");       if (!get_button(&left))      return FALSE;
				add_text("Press a button for \"SELECT\"...");     if (!get_button(&select))    return FALSE;
				add_text("Press a button for \"RUN\"...");        if (!get_button(&run))       return FALSE;
				add_text("Press a button for \"button I\"...");   if (!get_button(&buttonI))   return FALSE;
				add_text("Press a button for \"button II\"...");  if (!get_button(&buttonII))  return FALSE;
				if (JOYPAD_GetConnectSixButton()) //Kitao追加
				{
					add_text("Press a button for \"button III\"..."); if (!get_button(&buttonIII)) return FALSE;
					add_text("Press a button for \"button IV\"...");  if (!get_button(&buttonIV))  return FALSE;
					add_text("Press a button for \"button V\"...");   if (!get_button(&buttonV))   return FALSE;
					add_text("Press a button for \"button VI\"...");  if (!get_button(&buttonVI))  return FALSE;
				}
				else if (JOYPAD_GetConnectThreeButton()) //Kitao追加
				{
					add_text("Press a button for \"button III[=RUN]\"..."); if (!get_button(&buttonIII)) return FALSE;
					add_text("Press a button for \"button IV[=SELECT]\"...");  if (!get_button(&buttonIV)) return FALSE;
				}
			}
			break;
		case 2: //早回し用ボタンの設定
			add_text("Setting of \"Video Speed Up Button\": (\"Esc\"=Abort \"F1\"=Clear)");
			add_text("Press a button for \"Video Speed Up Button\"...");   if (!get_button(&buttonI)) return FALSE;
			break;
		case 3: //ステートセーブ用ボタンの設定
			add_text("Setting of \"Save State Button\": (\"Esc\"=Abort \"F1\"=Default[S])");
			add_text("Press a button for \"Save State Button\"...");   if (!get_button(&buttonI)) return FALSE;
			break;
		case 4: //ステートロード用ボタンの設定
			add_text("Setting of \"Load State Button\": (\"Esc\"=Abort \"F1\"=Default[L])");
			add_text("Press a button for \"Load State Button\"...");   if (!get_button(&buttonI)) return FALSE;
			break;
		case 5: //スクリーンショット用ボタンの設定
			add_text("Setting of \"Screenshot Button\": (\"Esc\"=Abort \"F1\"=Default[PrintScr])");
			add_text("Press a button for \"Screenshot Button\"...");   if (!get_button(&buttonI)) return FALSE;
			break;
		case 6: //ファンクション用ボタンの設定
			add_text("Setting of \"Function Button\": (\"Esc\"=Abort \"F1\"=Clear)");
			add_text("Press a button for \"Function Button\"...");   if (!get_button(&buttonI)) return FALSE;
			break;
		case 7: //ファンクションボタン時のセーブ＆ロード用ボタンの設定
			add_text("Setting of \"Function SaveState Button\": (\"Esc\"=Abort \"F1\"=Clear)");
			add_text("Press a button for \"Function SaveState Button\"...");   if (!get_button(&buttonI)) return FALSE;
			add_text("Setting of \"Function LoadState Button\": (\"Esc\"=Abort \"F1\"=Clear)");
			add_text("Press a button for \"Function LoadState Button\"...");   if (!get_button(&buttonII)) return FALSE;
			break;
		case 8: //ポーズ用ボタンの設定
			add_text("Setting of \"Pause Button\": (\"Esc\"=Abort \"F1\"=Clear)");
			add_text("Press a button for \"Pause Button\"...");   if (!get_button(&buttonI)) return FALSE;
			break;
	}

	//Kitao更新
	pPad->buttonU	= up;		//上キーの設定。アスキーコード(0～255)。ジョイパッド１(300～399。100ボタンまで対応)。ジョイパッド２(400～499)。以下ジョイパッド５本まで同様。
	pPad->buttonR	= right;	//右キーの設定
	pPad->buttonD	= down;		//下キーの設定
	pPad->buttonL	= left;		//左キーの設定
	pPad->buttonSel	= select;	//Selectボタンの設定
	pPad->buttonRun	= run;		//Runボタンの設定
	pPad->button1	= buttonI;	//Iボタンの設定
	pPad->button2	= buttonII;	//IIボタンの設定
	pPad->button3	= buttonIII;//IIIボタンの設定
	pPad->button4	= buttonIV;	//IVボタンの設定
	pPad->button5	= buttonV;	//Vボタンの設定
	pPad->button6	= buttonVI;	//VIボタンの設定

	return TRUE;
}


static LRESULT CALLBACK
padconfig_wnd_proc(
	HWND		hWnd,
	UINT		uMsg,
	WPARAM		wParam,
	LPARAM		lParam)
{
	int		i;

	switch(uMsg)
	{
	case WM_CREATE:
		EnableWindow(WINMAIN_GetHwnd(), FALSE);//Kitao追加。メインウインドウを無効化してモーダルに。
		_FontWidth  = get_font_width(hWnd);
		_FontHeight = get_font_height(hWnd);
		set_window_size(hWnd);
		break;

	case WM_PAINT:
		update_window(hWnd);
		_bWindowCreated = TRUE; //v0.74追加
		break;

	case WM_ACTIVATE: //Kitao追加
		if (wParam != 0) //アクティブになるとき
		{
			//DirectInputのアクセス権利を得る
			//キーボード
			if (_pDIDKey != NULL)
				_pDIDKey->Acquire();
			//ジョイスティック
			for (i = 0; i < _nJoySticks; ++i)
				if (_pDIDJoy[i] != NULL)
					_pDIDJoy[i]->Acquire();
		}
		break;

	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE)
			PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;

	case WM_CLOSE:
		PADCONFIG_Deinit();
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


//Kitao追加。JoyStickデバイスの軸を取得するコールバック関数
static BOOL
CALLBACK
DIEnumAxisCallback(
	LPCDIDEVICEOBJECTINSTANCE	lpddi,
	LPVOID						pvRef)
{
	DIPROPRANGE				diDR;
	DIPROPDWORD				diDD;

	//見つかった軸の設定をする。
	ZeroMemory(&diDR, sizeof(diDR));
	diDR.diph.dwSize       = sizeof(diDR);
	diDR.diph.dwHeaderSize = sizeof(diDR.diph);
	diDR.diph.dwHow        = DIPH_BYID;
	diDR.diph.dwObj        = lpddi->dwType;
	diDR.lMin = -1000;
	diDR.lMax = +1000;
	_pDIDJoy[_nJoySticks]->SetProperty(DIPROP_RANGE, &diDR.diph);

	//絶対軸に設定。
	ZeroMemory(&diDD, sizeof(diDD));
	diDD.diph.dwSize       = sizeof(diDD);
	diDD.diph.dwHeaderSize = sizeof(diDD.diph);
	diDD.diph.dwHow        = DIPH_DEVICE;
	diDD.dwData		       = DIPROPAXISMODE_ABS;
	_pDIDJoy[_nJoySticks]->SetProperty(DIPROP_AXISMODE, &diDD.diph);

	//デッドゾーンを0に設定。デジタルパッドでも有効。DirectX7以降でもこれでだいぶ反応が良くなる。Kitao追加
	ZeroMemory(&diDD, sizeof(diDD));
	diDD.diph.dwSize       = sizeof(diDD);
	diDD.diph.dwHeaderSize = sizeof(diDD.diph);
	diDD.diph.dwHow        = DIPH_DEVICE;
	diDD.dwData		       = 0;
	_pDIDJoy[_nJoySticks]->SetProperty(DIPROP_DEADZONE, &diDD.diph);

	//飽和ゾーンを最大に。Kitao追加
	ZeroMemory(&diDD, sizeof(diDD));
	diDD.diph.dwSize       = sizeof(diDD);
	diDD.diph.dwHeaderSize = sizeof(diDD.diph);
	diDD.diph.dwHow        = DIPH_DEVICE;
	diDD.dwData		       = 10000;
	_pDIDJoy[_nJoySticks]->SetProperty(DIPROP_SATURATION, &diDD.diph);

	return DIENUM_CONTINUE; //次の軸を探す
}

static BOOL
CALLBACK
DIEnumDevicesCallback(
	LPCDIDEVICEINSTANCE	lpddi,
	LPVOID				pvRef)
{
	HRESULT					hResult;
	LPDIRECTINPUTDEVICE		pDIDJoy; //Kitao追加
	DIPROPDWORD				diDD;

	//見つかったジョイスティックデバイスを作成する
	hResult = _pDI->CreateDevice(lpddi->guidInstance, &pDIDJoy, NULL);
	if(hResult != DI_OK)
		return DIENUM_CONTINUE; //失敗したら中止して次を探す
	
	//Kitao追加。ここでDirectInputDevice2に拡張しておくようにした（高速化）。
	pDIDJoy->QueryInterface(IID_IDirectInputDevice8, (LPVOID*)&_pDIDJoy[_nJoySticks]);
	pDIDJoy->Release();
	
	//データフォーマットを設定する
	_pDIDJoy[_nJoySticks]->SetDataFormat(&c_dfDIJoystick2);

	//バッファを最小に。反応が良くなる。Kitao追加
	ZeroMemory(&diDD, sizeof(diDD));
	diDD.diph.dwSize       = sizeof(diDD);
	diDD.diph.dwHeaderSize = sizeof(diDD.diph);
	diDD.diph.dwHow        = DIPH_DEVICE;
	diDD.dwData		       = 1;
	_pDIDJoy[_nJoySticks]->SetProperty(DIPROP_BUFFERSIZE, &diDD.diph);

	//協調レベルを指定する  Kitao更新。協調設定をDISCL_NONEXCLUSIVEに＆バックグラウンドでも操作可能にした。
	_pDIDJoy[_nJoySticks]->SetCooperativeLevel(_hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

	//JoyStickデバイスの軸を列挙する。Kitao追加
	_pDIDJoy[_nJoySticks]->EnumObjects(DIEnumAxisCallback, NULL, DIDFT_AXIS);

	_nJoySticks++;

	//コントローラが5つ検出されたら終了する
	if(_nJoySticks==N_MAXJOYSTICK)
		return DIENUM_STOP; //列挙を終了させる
	else
		return DIENUM_CONTINUE; //次のデバイスを探す
}

//Kitao更新。キーボードにも対応。ジョイスティックも好きなジョイスティックを好きな番号で使えるようにした。
static BOOL
padconfig_main()
{
	WNDCLASS	wc;
	HWND		hWnd;
	BOOL		bOk;
	HRESULT 	hResult;
	int			i;
	RECT		rc;

	ZeroMemory(&wc, sizeof(wc));
	wc.style		 = 0;
	wc.lpfnWndProc	 = padconfig_wnd_proc;
	wc.cbClsExtra	 = 0;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = _hInstance;
	wc.hIcon		 = LoadIcon(_hInstance, MAKEINTRESOURCE(OOTAKEICON)); //アイコンを読み込み。v2.00更新
	wc.hCursor		 = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszMenuName	 = "";
	wc.lpszClassName = _pCaption;

	if (RegisterClass(&wc) == 0)
		return FALSE;

	_bWindowCreated = FALSE; //Kitao追加

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
	);

	if (hWnd == NULL)
		return FALSE;

	_hWnd	   = hWnd;

	CloseWindow(WINMAIN_GetHwnd());//メインウィンドウを最小化
	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);
	GetWindowRect(hWnd, &rc);
	SetWindowPos(hWnd, HWND_TOP, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, SWP_FRAMECHANGED); //このウィンドウを「常に手前に表示」
	while (_bWindowCreated == FALSE) //ウィンドウの生成完了まで待つ。v0.74
		Sleep(1);
	ImmAssociateContext(hWnd, 0); //Kitao追加。IMEを無効にする。v0.79

	//以下Kitao追加。
	//Windowsジョイスティック用の変数を初期化
	_nJoySticks = 0;
	ZeroMemory(_Joystick, sizeof(_Joystick));
	// DirectInputインターフェースを取得する
	if (DirectInput8Create(WINMAIN_GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&_pDI, NULL) != DI_OK)
		return FALSE;

	//キーボードデバイスを取得する
	if (_pDI->CreateDevice(GUID_SysKeyboard, &_pDIDKey, NULL) != DI_OK)
		return FALSE;
	//データフォーマットを設定する
	if (_pDIDKey->SetDataFormat(&c_dfDIKeyboard) != DI_OK)
		return FALSE;
	//協調レベルを指定する
	hResult=_pDIDKey->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	//キーボードのアクセス権を得ておく。
	_pDIDKey->Acquire();
	//JoyStickデバイスを列挙する
	_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, DIEnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY);
	if (_nJoySticks == 0)
		add_text("PADCONFIG: No supported joystick found.");
	else
		add_text("PADCONFIG: %ld joystick(s) found.", _nJoySticks);
	//最初にアクセス権を得ておく。
	for (i = 0; i < _nJoySticks; i++)
		_pDIDJoy[i]->Acquire();

	//入力設定
	bOk = configure(_Mode, _PadID, _pPad);
	if (bOk)
		_SetOk = 1; //設定完了の印

	PostMessage(hWnd, WM_CLOSE, 0, 0);
	return bOk;
}


//Kitao更新。複数人プレイのときもキーボードのボタンを使えるようにした。
BOOL
PADCONFIG_Init(
	HINSTANCE	hInstance,
	Sint32	 	mode, //Kitao追加。0…通常の設定。1…連射専用ボタンの設定。2…早回し用ボタンの設定。3…ステートセーブ用ボタンの設定。4…ステートロード用ボタンの設定。5…スクリーンショット用ボタンの設定。
					  //		   6…ファンクション用ボタンの設定。7…ファンクションボタン時のステートセーブ＆ロード用ボタンの設定。8…ポーズ用ボタンの設定。
	Sint32	 	padID,
	PCEPAD* 	pPad,
	Sint32*		setOk)
{
	int				i;

	_hInstance = hInstance;
	_Line      = 0;

	_Mode	= mode;
	_PadID	= padID;
	_pPad	= pPad;
	_pSetOk	= setOk;
	_SetOk	= -1; //キャンセル

	for (i=0; i<N_LINES; i++)
		_pText[i] = _Text[i];

	return padconfig_main();
}


void
PADCONFIG_Deinit()
{
	int	i;

	if (_hInstance != NULL)
	{
		if (_pDIDKey != NULL)
		{
			_pDIDKey->Unacquire();
			_pDIDKey->Release();
			_pDIDKey = NULL;
		}
		for (i = 0; i < N_MAXJOYSTICK; ++i)
		{
			if (_pDIDJoy[i] != NULL)
			{
				_pDIDJoy[i]->Unacquire();
				_pDIDJoy[i]->Release();
				_pDIDJoy[i] = NULL;
			}
		}
		if (_pDI != NULL)
		{
			_pDI->Release();
			_pDI = NULL;
		}
		DestroyWindow(_hWnd);
		_hWnd = NULL;
		UnregisterClass(_pCaption, _hInstance);
		_hInstance = NULL;
		
		//メインウィンドウにフォーカスを戻し元の大きさに。
		EnableWindow(WINMAIN_GetHwnd(), TRUE);
		OpenIcon(WINMAIN_GetHwnd());
		
		*_pSetOk = _SetOk; //戻り値を設定。この瞬間にメインウィンドウは動き出す。
	}
}

