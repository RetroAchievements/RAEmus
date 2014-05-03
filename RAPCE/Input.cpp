/******************************************************************************
Ootake
・PCEパッド1～5にキーボードのキーも含めて自由に割り当てできるようにした。
・Windows用16ボタンパッドとハットスイッチ（アナログ対応パッドでよく使われてい
  る）にも対応した。
・高速化のためにDirectInputのアクセス権の取得はアクティブになったとき１回だけ行
  うようにした。同様にアクセス権の引渡しもディアクテブ時に１回だけ行うようにし
  た。
・プレイレコード時に、Windowsパッドの状態ではなく、「PCEパッドの状態」として記
  録するようにした。現状は容量と対応ソフト数を考え、２ボタンコントローラ１の
  み記録。今後、設定で６ボタンやマルチタップでの記録にも対応したい。
・６ボタン＆連射専用ボタンと２プレイヤーパッドのプレイレコードにも対応した。
  v2.15

Copyright(C)2006-2009 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[Input.c]
		入力の実装を行ないます。

	Copyright (C) 2004 Ki

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
******************************************************************************/
#define DIRECTINPUT_VERSION	0x0800	//Kitao追加。環境にもよるかもしれないが、DirectInput5が軽い。7,8だとやや遅延あり。スペースハリアーがわかりやすい。

#include <windows.h>
#include <string.h>
#include "Input.h"
#include "WinMain.h"
#include "Printf.h"
#include "JoyPad.h"
#include "MainBoard.h"
#include "App.h"


#define INPUT_BUFFERSIZE		8+60*60*120*10*7			// 8heder + 60frame * 60sec * 120min * 10回 * 7byte(5player)  //Kitao更新。1frameのキーステータス更新回数を不定にしたため、バッファも増やした。１フレームで10回呼ばれるとして５人プレイ時で２時間撮れる大きさ(約30MB)にした。ヘッダも付けるようにした。
#define N_MAXJOYSTICK			5


static LPDIRECTINPUT			_pDI		= NULL;			// DirectInput インターフェースポインタ
static LPDIRECTINPUTDEVICE		_pDIDKey	= NULL;			// DirectInput Keyboard device
static LPDIRECTINPUTDEVICE2		_pDIDJoy[N_MAXJOYSTICK];	// DirectInput Joystick device  Kitao更新。LPDIRECTINPUTDEVICE2にした。


typedef struct
{
	DIJOYSTATE2					joyState;
	Uint32						buttonState; //Kitao更新。16ボタンに対応。ハットスイッチぶんも入れたのでUint32に。
} JOYSTICK; //Kitao更新


static char						_DIKeys[256];					// キーボードのステータスを代入する配列

static Sint32					_nJoySticks = 0;

static Uint8					_InputBuffer[INPUT_BUFFERSIZE];	// record pad #1&#2 //Kitao更新。PCEパッドの状態として記録するようにした。
static Sint32					_InputBufferIndex;
static Sint32					_InputBufferIndexEnd = INPUT_BUFFERSIZE;
static Sint32					_InputPlayRecordPad; //Kitao追加。読み込み中プレイレコードファイルの記録パッド数。v2.15

static BOOL						_bRecord;
static BOOL						_bPlayRecord;
static BOOL						_bInit;

//Kitao追加。v1.21
static BOOL						_bGameSetting;
static BOOL						_bKonamiStereo;
static BOOL						_bGradiusII;

static JOYSTICK					_Joystick[N_MAXJOYSTICK];

//Kitao追加
static PCEPAD	_PcePad[6]; //現在の各PCEパッド[1]～[5]のボタン設定([0]は非使用)


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
	diDR.lMin = -100;
	diDR.lMax = +100;
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

/*-----------------------------------------------------------------------------
	[DIEnumDevicesCallback]
	JoyStickデバイスを取得するコールバック関数
-----------------------------------------------------------------------------*/
static BOOL
CALLBACK
DIEnumDevicesCallback(
	LPCDIDEVICEINSTANCE	lpddi,
	LPVOID				pvRef)
{
	HWND					hWnd;
	HRESULT					hResult;
	LPDIRECTINPUTDEVICE		pDIDJoy; //Kitao追加
	DIPROPDWORD				diDD;

	hWnd = WINMAIN_GetHwnd();

	//見つかったジョイスティックデバイスを作成する
	hResult = _pDI->CreateDevice(lpddi->guidInstance, &pDIDJoy, NULL);
	if (hResult != DI_OK)
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
	if (APP_GetJoypadBackground())
		_pDIDJoy[_nJoySticks]->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	else
		_pDIDJoy[_nJoySticks]->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	//JoyStickデバイスの軸を列挙する。Kitao追加
	_pDIDJoy[_nJoySticks]->EnumObjects(DIEnumAxisCallback, NULL, DIDFT_AXIS);

	_nJoySticks++;

	//コントローラが5つ検出されたら終了する
	if(_nJoySticks==N_MAXJOYSTICK)
		return DIENUM_STOP; //列挙を終了させる
	else
		return DIENUM_CONTINUE; //次のデバイスを探す
}


/*-----------------------------------------------------------------------------
	[Init]
		初期化します。
-----------------------------------------------------------------------------*/
BOOL
INPUT_Init()
{
	HRESULT		hResult;
	HANDLE		hWnd;
	int			i;

	if (_bInit)
		INPUT_Deinit();

	memset(_PcePad, 0xFF, sizeof(_PcePad)); //0xFFFF(-1)で初期化
	ZeroMemory(_Joystick, sizeof(_Joystick));
	_InputBufferIndex = 0;

	hWnd = WINMAIN_GetHwnd();

	// DirectInputインターフェースを取得する
	if (DirectInput8Create(WINMAIN_GetHInstance(), DIRECTINPUT_VERSION, IID_IDirectInput8A, (void**)&_pDI, NULL) != DI_OK)
		return FALSE;

	//キーボードデバイスを取得する
	if (_pDI->CreateDevice(GUID_SysKeyboard, &_pDIDKey, NULL) != DI_OK)
		return FALSE;

	//データフォーマットを設定する
	if (_pDIDKey->SetDataFormat(&c_dfDIKeyboard) != DI_OK)
		return FALSE;

	//協調レベルを指定する  Kitao更新。協調設定をDISCL_NONEXCLUSIVEに＆バックグラウンドでも操作可能にした。
	if (APP_GetKeyboardBackground())
		hResult = _pDIDKey->SetCooperativeLevel((HWND)hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	else
		hResult = _pDIDKey->SetCooperativeLevel((HWND)hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);

	//キーボードのアクセス権を得ておく。
	_pDIDKey->Acquire();

	//JoyStickデバイスを列挙する
	//_pDI->EnumDevices(DIDEVTYPE_JOYSTICK, DIEnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY);
	_pDI->EnumDevices(DI8DEVCLASS_GAMECTRL, DIEnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY);

	//Kitao追加。最初にアクセス権を得ておく。
	for (i = 0; i < _nJoySticks; i++)
		_pDIDJoy[i]->Acquire();

	_bInit = TRUE;

	return _bInit;
}


/*-----------------------------------------------------------------------------
	[Deinit]
		終了します。
-----------------------------------------------------------------------------*/
void
INPUT_Deinit()
{
	Uint32		i;

	if (!_bInit)
		return;

	if (_pDIDKey != NULL)
	{
		_pDIDKey->Unacquire(); //Kitao追記。UnacquireはRelease前に１回だけ行うようにする。Unacquireを繰り返すと、環境によっては不安定になるらしい。
		_pDIDKey->Release();
		_pDIDKey = NULL;
	}

	for (i = 0; i < N_MAXJOYSTICK; ++i)
	{
		if (_pDIDJoy[i] != NULL)
		{
			_pDIDJoy[i]->Unacquire(); //Kitao追記。UnacquireはRelease前に１回だけ行うようにする。Unacquireを繰り返すと、環境によっては不安定になるらしい。
			_pDIDJoy[i]->Release();
			_pDIDJoy[i] = NULL;
		}
	}

	if (_pDI != NULL)
	{
		_pDI->Release();
		_pDI = NULL;
	}

	_nJoySticks = 0;
	_bInit = FALSE;
}


/*-----------------------------------------------------------------------------
	[ConnectButton]
		ユーザー定義のボタンと入力ボタンを接続します。
-----------------------------------------------------------------------------*/
//Kitao更新。App.cで読み込んだ_PcePad2[]または_PcePad6[]の内容を、こちらの_PcePad[]にも反映させる関数とした。
void
INPUT_ConnectButton(
	Sint32		padID,	//padID…PCEパッドナンバー(1～5)
	PCEPAD*		pcepad) //pcepad…_PcePad2[]か_PcePad6[]のアドレス
{
	_PcePad[padID] = *pcepad;
	return;
}


//Kitao追加。ボタンのステータス（押されていたかどうか）をチェックするサブ。
//a…キーナンバーorジョイパッドのボタンナンバー。戻り値は、そのキー,ボタンが押されていた場合にTRUE。
BOOL
INPUT_CheckButtonState(
	Sint32	a)
{
	Sint32	winJoyID;

	if (a == -1) //未設定の場合
		return FALSE;
	
	if (a <= 255) //キーボードのキーが設定されていた場合
		return (_DIKeys[a] & 0x80);
	else //ジョイパッドのキー・ボタンが設定されていた場合
	{
		a -= 300;
		winJoyID = 0;
		while (a >= 100)
		{
			winJoyID++;
			a -= 100;
		}
		if (a < 50) //十字キー・ハットスイッチ類なら
		{
			switch (a)
			{
				case 0: //0～3 十字キー
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_UP) != 0;
				case 1:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_RIGHT) != 0;
				case 2:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_DOWN) != 0;
				case 3:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_LEFT) != 0;
				case 4: //4～7 ハットスイッチ
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_POVUP) != 0;
				case 5:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_POVRIGHT) != 0;
				case 6:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_POVDOWN) != 0;
				case 7:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_POVLEFT) != 0;
			}
		}
		else //ボタン類なら
		{
			switch (a - 50)
			{
				case 0:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON1) != 0;
				case 1:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON2) != 0;
				case 2:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON3) != 0;
				case 3:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON4) != 0;
				case 4:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON5) != 0;
				case 5:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON6) != 0;
				case 6:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON7) != 0;
				case 7:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON8) != 0;
				case 8:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON9) != 0;
				case 9:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON10) != 0;
				case 10:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON11) != 0;
				case 11:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON12) != 0;
				case 12:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON13) != 0;
				case 13:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON14) != 0;
				case 14:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON15) != 0;
				case 15:
					return (_Joystick[winJoyID].buttonState & INPUT_JOYSTICK_BUTTON16) != 0;
			}
		}
	}
	return FALSE;
}

//Kitao追加。指定のボタンのステータス（押されていたかどうか）をオンにする。プレイバック時に使用
//a…キーナンバーorジョイパッドのボタンナンバー
static void
set_button_state(
	int	a)
{
	Sint32	winJoyID;

	if (a == -1) //未設定の場合
		return;
	
	if (a <= 255) //キーボードのキーが設定されていた場合
	{
		_DIKeys[a] |= 0x80;
		return;
	}
	else //ジョイパッドのキー・ボタンが設定されていた場合
	{
		a -= 300;
		winJoyID = 0;
		while (a >= 100)
		{
			winJoyID++;
			a -= 100;
		}
		if (a < 50) //十字キー・ハットスイッチ類なら
		{
			switch (a)
			{
				case 0: //0～3 十字キー
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_UP;
					return;
				case 1:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_RIGHT;
					return;
				case 2:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_DOWN;
					return;
				case 3:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_LEFT;
					return;
				case 4: //4～7 ハットスイッチ
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_POVUP;
					return;
				case 5:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_POVRIGHT;
					return;
				case 6:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_POVDOWN;
					return;
				case 7:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_POVLEFT;
					return;
			}
		}
		else //ボタン類なら
		{
			switch (a - 50)
			{
				case 0:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON1;
					return;
				case 1:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON2;
					return;
				case 2:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON3;
					return;
				case 3:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON4;
					return;
				case 4:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON5;
					return;
				case 5:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON6;
					return;
				case 6:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON7;
					return;
				case 7:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON8;
					return;
				case 8:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON9;
					return;
				case 9:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON10;
					return;
				case 10:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON11;
					return;
				case 11:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON12;
					return;
				case 12:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON13;
					return;
				case 13:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON14;
					return;
				case 14:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON15;
					return;
				case 15:
					_Joystick[winJoyID].buttonState |= INPUT_JOYSTICK_BUTTON16;
					return;
			}
		}
	}
}

//Kitao追加。指定のボタンのステータス（押されていたかどうか）をオフにする。コナミのオートステレオ時に使用
//a…キーナンバーorジョイパッドのボタンナンバー
static void
reset_button_state(
	int	a)
{
	Sint32	winJoyID;

	if (a == -1) //未設定の場合
		return;
	
	if (a <= 255) //キーボードのキーが設定されていた場合
	{
		_DIKeys[a] &= ~0x80;
		return;
	}
	else //ジョイパッドのキー・ボタンが設定されていた場合
	{
		a -= 300;
		winJoyID = 0;
		while (a >= 100)
		{
			winJoyID++;
			a -= 100;
		}
		if (a < 50) //十字キー・ハットスイッチ類なら
		{
			switch (a)
			{
				case 0: //0～3 十字キー
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_UP;
					return;
				case 1:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_RIGHT;
					return;
				case 2:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_DOWN;
					return;
				case 3:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_LEFT;
					return;
				case 4: //4～7 ハットスイッチ
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_POVUP;
					return;
				case 5:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_POVRIGHT;
					return;
				case 6:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_POVDOWN;
					return;
				case 7:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_POVLEFT;
					return;
			}
		}
		else //ボタン類なら
		{
			switch (a - 50)
			{
				case 0:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON1;
					return;
				case 1:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON2;
					return;
				case 2:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON3;
					return;
				case 3:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON4;
					return;
				case 4:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON5;
					return;
				case 5:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON6;
					return;
				case 6:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON7;
					return;
				case 7:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON8;
					return;
				case 8:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON9;
					return;
				case 9:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON10;
					return;
				case 10:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON11;
					return;
				case 11:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON12;
					return;
				case 12:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON13;
					return;
				case 13:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON14;
					return;
				case 14:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON15;
					return;
				case 15:
					_Joystick[winJoyID].buttonState &= ~INPUT_JOYSTICK_BUTTON16;
					return;
			}
		}
	}
}

/*-----------------------------------------------------------------------------
	[UpdateState]
		入力状況を更新します。
-----------------------------------------------------------------------------*/
void
INPUT_UpdateState(
	BOOL	bRecord) //Kitao追加。記録・再生を行うならTRUE。
{
	int			i;
	Uint8		rc; //v2.15更新
	HRESULT		hResult;

	//Kitao更新。Windowsパッドではなく、PCEパッドの状態として記録するようにした。現状は容量と対応ソフト数を考え、２ボタンコントローラ１のみ。今後、設定で６ボタンやマルチタップにも対応したい。
	//           INPUT_UpdateState()はJOYPAD_Readが呼ばれるたびにやるようにしたため、記録容量は不定。1記録は8ビットにしたのでそのぶんは半減。
	if ((bRecord)&&(_bPlayRecord))
	{
		memset(_DIKeys, 0, sizeof(_DIKeys)); //キーボードのステータスをクリア
		for (i = 0; i < _nJoySticks; ++i)
			_Joystick[i].buttonState = 0; //ジョイスティックのステータスをクリア
		
		rc = _InputBuffer[_InputBufferIndex++];
		if (rc & 0x01)
			set_button_state(_PcePad[1].buttonU);
		if (rc & 0x02)
			set_button_state(_PcePad[1].buttonR);
		if (rc & 0x04)
			set_button_state(_PcePad[1].buttonD);
		if (rc & 0x08)
			set_button_state(_PcePad[1].buttonL);
		if (rc & 0x10)
			set_button_state(_PcePad[1].buttonSel);
		if (rc & 0x20)
			set_button_state(_PcePad[1].buttonRun);
		if (rc & 0x40)
			set_button_state(_PcePad[1].button1);
		if (rc & 0x80)
			set_button_state(_PcePad[1].button2);

		if (_InputPlayRecordPad == 2)
		{
			rc = _InputBuffer[_InputBufferIndex++];
			if (rc & 0x01)
				set_button_state(_PcePad[2].buttonU);
			if (rc & 0x02)
				set_button_state(_PcePad[2].buttonR);
			if (rc & 0x04)
				set_button_state(_PcePad[2].buttonD);
			if (rc & 0x08)
				set_button_state(_PcePad[2].buttonL);
			if (rc & 0x10)
				set_button_state(_PcePad[2].buttonSel);
			if (rc & 0x20)
				set_button_state(_PcePad[2].buttonRun);
			if (rc & 0x40)
				set_button_state(_PcePad[2].button1);
			if (rc & 0x80)
				set_button_state(_PcePad[2].button2);
			//６ボタンor連射専用ボタン(ボタン5,6を使用)の状態を読み出し
			rc = _InputBuffer[_InputBufferIndex++];
			if (rc & 0x01)
				set_button_state(_PcePad[1].button3);
			if (rc & 0x02)
				set_button_state(_PcePad[1].button4);
			if (rc & 0x04)
				set_button_state(_PcePad[1].button5);
			if (rc & 0x08)
				set_button_state(_PcePad[1].button6);
			if (rc & 0x10)
				set_button_state(_PcePad[2].button3);
			if (rc & 0x20)
				set_button_state(_PcePad[2].button4);
			if (rc & 0x40)
				set_button_state(_PcePad[2].button5);
			if (rc & 0x80)
				set_button_state(_PcePad[2].button6);
		}
		
		if (_InputBufferIndex == _InputBufferIndexEnd)
		{
			i = APP_GetInputPlayRecordNumber();
			APP_EndPlayRecord();
			if (i == 0)
				PRINTF("End of Record Data.");
			else
				PRINTF("End of Record Data #%d.", i);
			MAINBOARD_ScreenUpdate(TRUE); //すぐにメッセージ表示を更新
			APP_RunEmulator(FALSE);
		}
	}
	else
	{
		// キーボードの状態を読む
		hResult =_pDIDKey->GetDeviceState(256, _DIKeys);

		// 読み取りに失敗した時の処理
		if (hResult != DI_OK) //失敗したときはアクセス権を取り直してやり直す
		{
			_pDIDKey->Acquire();
			_pDIDKey->GetDeviceState(256, _DIKeys);
		}

		// ジョイスティックが存在する時はジョイスティックの状態も読む
		for (i=0; i<_nJoySticks; i++)
		{
			// ポーリングを行なう。Kitao更新。DirectInputDevice2への拡張は初期化時に１度だけ行うようにした(高速化)。
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
			if (_Joystick[i].joyState.lY < -25)	_Joystick[i].buttonState |= INPUT_JOYSTICK_UP;
			if (_Joystick[i].joyState.lX > +25)	_Joystick[i].buttonState |= INPUT_JOYSTICK_RIGHT;
			if (_Joystick[i].joyState.lY > +25)	_Joystick[i].buttonState |= INPUT_JOYSTICK_DOWN;
			if (_Joystick[i].joyState.lX < -25)	_Joystick[i].buttonState |= INPUT_JOYSTICK_LEFT;
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

		//Kitao更新。Windowsパッドではなく、PCEパッドの状態として記録するようにした。現状は容量と対応ソフト数を考え、２ボタンコントローラ１のみ。今後、設定で６ボタンやマルチタップにも対応したい。
		//           INPUT_UpdateState()はJOYPAD_Readが呼ばれるたびにやるようにしたため、記録容量は不定。1記録は8ビットにしたのでそのぶんは半減。
		if ((bRecord)&&(_bRecord))
		{
			rc = 0;
			if (INPUT_CheckButtonState(_PcePad[1].buttonU))
				rc |= 0x01;
			if (INPUT_CheckButtonState(_PcePad[1].buttonR))
				rc |= 0x02;
			if (INPUT_CheckButtonState(_PcePad[1].buttonD))
				rc |= 0x04;
			if (INPUT_CheckButtonState(_PcePad[1].buttonL))
				rc |= 0x08;
			if (INPUT_CheckButtonState(_PcePad[1].buttonSel))
				rc |= 0x10;
			if (INPUT_CheckButtonState(_PcePad[1].buttonRun))
				rc |= 0x20;
			if (INPUT_CheckButtonState(_PcePad[1].button1))
				rc |= 0x40;
			if (INPUT_CheckButtonState(_PcePad[1].button2))
				rc |= 0x80;
			_InputBuffer[_InputBufferIndex++] = rc;

			if (APP_GetInputRecordMode() == 2)
			{
				rc = 0;
				if (INPUT_CheckButtonState(_PcePad[2].buttonU))
					rc |= 0x01;
				if (INPUT_CheckButtonState(_PcePad[2].buttonR))
					rc |= 0x02;
				if (INPUT_CheckButtonState(_PcePad[2].buttonD))
					rc |= 0x04;
				if (INPUT_CheckButtonState(_PcePad[2].buttonL))
					rc |= 0x08;
				if (INPUT_CheckButtonState(_PcePad[2].buttonSel))
					rc |= 0x10;
				if (INPUT_CheckButtonState(_PcePad[2].buttonRun))
					rc |= 0x20;
				if (INPUT_CheckButtonState(_PcePad[2].button1))
					rc |= 0x40;
				if (INPUT_CheckButtonState(_PcePad[2].button2))
					rc |= 0x80;
				_InputBuffer[_InputBufferIndex++] = rc;
				//６ボタンor連射専用ボタン(ボタン5,6を使用)の状態を書き込み
				rc = 0;
				if (INPUT_CheckButtonState(_PcePad[1].button3))
					rc |= 0x01;
				if (INPUT_CheckButtonState(_PcePad[1].button4))
					rc |= 0x02;
				if (INPUT_CheckButtonState(_PcePad[1].button5))
					rc |= 0x04;
				if (INPUT_CheckButtonState(_PcePad[1].button6))
					rc |= 0x08;
				if (INPUT_CheckButtonState(_PcePad[2].button3))
					rc |= 0x10;
				if (INPUT_CheckButtonState(_PcePad[2].button4))
					rc |= 0x20;
				if (INPUT_CheckButtonState(_PcePad[2].button5))
					rc |= 0x40;
				if (INPUT_CheckButtonState(_PcePad[2].button6))
					rc |= 0x80;
				_InputBuffer[_InputBufferIndex++] = rc;
			}

			if (_InputBufferIndex == INPUT_BUFFERSIZE)
			{
				APP_EndRecording();
				PRINTF("End of input buffer. Recording stopped.");
			}
		}
	}

	//Kitao追加。ゲームごとの自動操作処理。※Record終了後のこの位置で。v1.15
	if (_bGameSetting) //速度を落とさないためこのフラグで、処理を行うかどうかを先に判定する。
	{
		//グラディウス，沙羅曼蛇，パロディウスだ！を自動的にステレオ起動にする。
		if (_bKonamiStereo)
		{
			reset_button_state(_PcePad[1].buttonU);  //十字キーを離した状態にする。沙羅曼蛇で必要。
			reset_button_state(_PcePad[1].buttonR);  //
			reset_button_state(_PcePad[1].buttonD);  //
			reset_button_state(_PcePad[1].buttonL);  //
			reset_button_state(_PcePad[1].buttonSel);//SELボタンを離した状態にする
			reset_button_state(_PcePad[1].buttonRun);//RUNボタンを離した状態にする
			set_button_state(_PcePad[1].button1);	 //１ボタンが押しっぱなしにされた状態にする
		}
		//グラディウスIIで、自動的にレーザー・スプレッドボムをちらつかない設定にする。
		if (_bGradiusII)
		{
			//ちらつかない設定
			reset_button_state(_PcePad[1].buttonSel);//SELボタンを離した状態にする
			reset_button_state(_PcePad[1].buttonRun);//RUNボタンを離した状態にする
			if (_PcePad[2].buttonD == -1) //コントローラ２の下キーの設定がリセットされている場合
				_PcePad[2].buttonD = 902; //仮想的に設定(902は実際の入力機器では入力できない値)しておく
			set_button_state(_PcePad[2].buttonD); //コントローラー２の下キーが押しっぱなしにされた状態にする
			//アーケードモード(オリジナル面とポーズ機能無し)
			//set_button_state(_PcePad[1].buttonU);
			//set_button_state(_PcePad[1].button1);
			//set_button_state(_PcePad[1].button2);
		}
	}
}


/*-----------------------------------------------------------------------------
	[IsPressed]
		指定のボタンが押されているかどうかを返します．
-----------------------------------------------------------------------------*/
//Kitao更新。複数人プレイのときもキーボードのボタンを使えるようにした。
BOOL
INPUT_IsPressed(
	Sint32	padID, //Kitao更新。joyID→padID(1～5)へ。PCエンジンのパッドナンバーで管理するようにした。
	Sint32	userButtonID)
{
	Sint32	a = -1;

	switch (userButtonID)
	{
		case JOYPAD_BUTTON_UP:
			a = _PcePad[padID].buttonU;
			break;
		case JOYPAD_BUTTON_RIGHT:
			a = _PcePad[padID].buttonR;
			break;
		case JOYPAD_BUTTON_DOWN:
			a = _PcePad[padID].buttonD;
			break;
		case JOYPAD_BUTTON_LEFT:
			a = _PcePad[padID].buttonL;
			break;
		case JOYPAD_BUTTON_SELECT:
			a = _PcePad[padID].buttonSel;
			break;
		case JOYPAD_BUTTON_RUN:
			a = _PcePad[padID].buttonRun;
			break;
		case JOYPAD_BUTTON_I:
			a = _PcePad[padID].button1;
			break;
		case JOYPAD_BUTTON_II:
			a = _PcePad[padID].button2;
			break;
		case JOYPAD_BUTTON_III:
			a = _PcePad[padID].button3;
			break;
		case JOYPAD_BUTTON_IV:
			a = _PcePad[padID].button4;
			break;
		case JOYPAD_BUTTON_V:
			a = _PcePad[padID].button5;
			break;
		case JOYPAD_BUTTON_VI:
			a = _PcePad[padID].button6;
			break;
	}

	return INPUT_CheckButtonState(a);
}


void
INPUT_Record(
	BOOL		bRecord)
{
	Sint32	softVersion;

	_bRecord = bRecord;

	if (_bRecord)
	{
		memset(_InputBuffer, 0, sizeof(_InputBuffer));
		_InputBufferIndex = 0;
		//Kitao追加。ヘッダーを付ける。
		_InputBuffer[_InputBufferIndex++] = 1; //レコードファイルのバージョン
		switch (APP_GetInputRecordMode())
		{
			case 2:
				_InputBuffer[_InputBufferIndex++] = 2; //パッド数 1～5。
				_InputBuffer[_InputBufferIndex++] = 6; //ボタン数 2or6。
				break;
			default: //1
				_InputBuffer[_InputBufferIndex++] = 1; //パッド数 1～5。
				_InputBuffer[_InputBufferIndex++] = 2; //ボタン数 2or6。
				break;
		}
		softVersion = APP_GetSoftVersion(); //Ootake本体のバージョン(小数点をカットして３桁整数に)
		_InputBuffer[_InputBufferIndex++] = (Uint8)(softVersion % 256); //Ootake本体のバージョン
		_InputBuffer[_InputBufferIndex++] = (Uint8)(softVersion / 256); //Ootake本体のバージョン
		_InputBuffer[_InputBufferIndex++] = 0; //予備
		_InputBuffer[_InputBufferIndex++] = 0; //予備
		_InputBuffer[_InputBufferIndex++] = 0; //予備

		if (_bPlayRecord)
			_bPlayRecord = FALSE;
	}
}

void
INPUT_PlayRecord(
	BOOL		bPlayrecord)
{
	_bPlayRecord = bPlayrecord;

	if (_bRecord)
		_bRecord = FALSE;

	_InputBufferIndex = 0;
	//Kitao追加。ヘッダーを読む。※現バージョンでは読み飛ばすだけ
	_InputBufferIndex++; //レコードファイルのバージョン
	_InputPlayRecordPad = _InputBuffer[_InputBufferIndex++]; //パッド数 1～5。現バージョンは1か2。
	_InputBufferIndex++; //ボタン数 2or6。現バージョンは2ボタン専用
	_InputBufferIndex++; //Ootake本体のバージョン
	_InputBufferIndex++; //Ootake本体のバージョン
	_InputBufferIndex++; //予備
	_InputBufferIndex++; //予備
	_InputBufferIndex++; //予備
}


//Kitao更新
BOOL
INPUT_WriteBuffer(
	FILE*		fp)
{
	if (fwrite(&_InputBufferIndex, sizeof(_InputBufferIndex), 1, fp) != 1) return FALSE; //バッファに溜まったバイト数を書き込み
	return fwrite(_InputBuffer, sizeof(Uint8)*_InputBufferIndex, 1, fp) == 1; //バッファに溜まった内容を書き込み
}

//Kitao更新
BOOL
INPUT_ReadBuffer(
	FILE*		fp)
{
	if (fread(&_InputBufferIndexEnd, sizeof(_InputBufferIndexEnd), 1, fp) != 1)	return FALSE; //バイト数を読み込み
	return fread(_InputBuffer, sizeof(Uint8)*_InputBufferIndexEnd, 1, fp) == 1; //内容を読み込み
}


Sint32
INPUT_GetNumJoystick()
{
	return _nJoySticks;
}


/*-----------------------------------------------------------------------------
	[IsTriggered]
	ボタンがトリガされたかどうかを返します．
-----------------------------------------------------------------------------*/
BOOL
INPUT_IsTriggered(
	Sint32		userButtonID)
{
	/* 未実装 */
	return FALSE;
}


//Kitao追加。高速化のためにDirectInputのアクセス権の取得はアクティブになったとき１回だけ行うようにした。
void
INPUT_Acquire()
{
	int i;

	//キーボード
	if (_pDIDKey != NULL)
		_pDIDKey->Acquire();

	//ジョイスティック
	for (i = 0; i < _nJoySticks; ++i)
		if (_pDIDJoy[i] != NULL)
			_pDIDJoy[i]->Acquire();
}


//Kitao追加。各ゲームごとの自動操作設定をリセット(オフに)する。
void
INPUT_ResetGameSetting()
{
	_bGameSetting = FALSE;
	_bKonamiStereo = FALSE;
	_bGradiusII = FALSE;
}

//Kitao追加。グラディウス，沙羅曼蛇，パロディウスだ！を自動的にステレオ起動にする。
void
INPUT_SetKonamiStereo(
	BOOL	bKonamiStereo)
{
	_bKonamiStereo = bKonamiStereo;
	_bGameSetting = _bKonamiStereo;
}

//Kitao追加。グラディウスIIで、レーザー・スプレッドボムをちらつかない設定に。
void
INPUT_SetGradiusII(
	BOOL	bGradiusII)
{
	_bGradiusII = bGradiusII;
	_bGameSetting = _bGradiusII;
}
