/******************************************************************************
Ootake
・マウスを大きく動かしたときは加速するようにした。
・マウスのリセット頻度を増やして、動作の遅延をできる限り減らした。
・マウス使用中のときは、「Windowsのマウスカーソル」を常にエミュレータウィンドウ
  の真ん中に位置させて隠すようにした。
・画面の表示倍率によってマウスの感度を最適化した。
・負の動きの限界を-128ではなく-127（正の限界と同じ値）にした。
・マウスホイールの回転操作でセレクトボタン(上回転)・ランボタン(した回転)を機能
  させるようにした。ホイール(真ん中ボタン)をクリックすることでも、ランボタンを
  機能させるようにした。

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[Mouse.c]
		マウスを実装します。

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
#define DIRECTINPUT_VERSION	0x0800	//Kitao追加。環境にもよるかもしれないが、DirectInput5が軽い。7だとやや遅延あり。スペースハリアーがわかりやすい。

#include "Mouse.h"
#include "WinMain.h"
#include "Input.h"
#include "JoyPad.h"
#include "MainBoard.h" //Kitao追加


static Uint32	_ReadCount;
static Sint8	_DeltaX;
static Sint8	_DeltaY;
static Sint32	_PrevX;
static Sint32	_PrevY;
static BOOL		_bMoveX = TRUE; //Kitao追加。v1.23
static BOOL		_bMsConnected = FALSE;
static Uint8	_ButtonState;
static Sint32	_ClockCounter;
static Uint32	_MouseWheelFlg = 0; //Kitao追加。マウスホイールの回転でセレクト・ランボタンを押された場合、セレクト・ランボタンを離す処理のために必要。


void
MOUSE_RButtonDown(
	BOOL		bDown)
{
	_ButtonState &= ~1;
	_ButtonState |= bDown;
}

void
MOUSE_LButtonDown(
	BOOL		bDown)
{
	_ButtonState &= ~2;
	_ButtonState |= bDown << 1;
}

void
MOUSE_SelButtonDown(
	BOOL		bDown)
{
	_ButtonState &= ~4;
	_ButtonState |= bDown << 2;
}

void
MOUSE_RunButtonDown(
	BOOL		bDown)
{
	_ButtonState &= ~8;
	_ButtonState |= bDown << 3;
}


void
MOUSE_Reset()
{
	if (_ClockCounter >= 7159090.0/60.0/16.0) //Kitao更新。遅延を減らすため、リセット頻度は誤動作しない範囲でできるだけ多くした。
	{
		_ReadCount = 0;
		_ClockCounter = 0;
	}
}


//Kitao追加。VBlank期間に座標をアップデートするようにした。v1.23
void
MOUSE_UpdateDelta()
{
	int		mag; //Kitao追加
	RECT	rc; //Kitao追加
	POINT	p; //Kitao追加
	Sint32	dx;
	Sint32	dy;

	if (!_bMsConnected) return; //高速化のためマウスをつないでいないときは処理しない

	//Kitao追加。画面の表示倍率によって感度を最適化した。
	mag = MAINBOARD_GetMagnification();
	GetCursorPos(&p);
	dx = _PrevX - p.x;
	dy = _PrevY - p.y;
	if ((dx % mag) != 0) //マウスを少しでも動かしていたときは、0にせず最低ぶんのカーソルを動かす。
	{ 
		if (dx > 0)
			dx = (dx / mag) + 1; //移動量は小数点以下を切り上げるようにした。"初恋物語"でマウスカーソルの動きが悪い問題が解消。v2.39
		else
			dx = (dx / mag) - 1;
	}
	else
		dx = dx / mag;
	if ((dy % mag) != 0) //マウスを少しでも動かしていたときは、0にせず最低ぶんのカーソルを動かす。
	{ 
		if (dy > 0)
			dy = (dy / mag) + 1; //移動量は小数点以下を切り上げるようにした。"初恋物語"でマウスカーソルの動きが悪い問題が解消。v2.39
		else
			dy = (dy / mag) - 1;
	}
	else
		dy = dy / mag;

	//Kitao追加。大きく動かしたときは加速するようにした。
	if (dx >=  64)
	{
		dx += dx/2;
		if (dx >  127)	dx =  127;
	}
	if (dy >=  64)
	{
		dy += dy/2;
		if (dy >  127)	dy =  127;
	}
	if (dx <= -64)
	{
		dx -= dx/2;
		if (dx < -127)	dx = -127; //Kitao更新。-128まで許すとマウスを大きく回したときにだんだん左に寄ってくる感があるので-127止まりで。
	}
	if (dy <= -64)
	{
		dy -= dy/2;
		if (dy < -127)	dy = -127; //Kitao更新
	}

	//Kitao追加。斜め移動の時に移動量1を返すと、"同級生"(7MHz)で画面が乱れるので、その場合は縦横２回に分けて移動させるようにした。v1.23
	//			 こうすると斜めの移動スピードが落ちて、細かくマウスを動かしたときに微調整もしやすくなった感じで、一石二鳥。"ときめきメモリアル"のゲームシーン(7MHz)でも、処理したほうが操作しやすい。
	if (VDC_GetTvWidth() >= 336) //VDCが高クロック(7MHz以上)で動いているときだけ、このタイミング調整処理をするようにした。この条件がないと、"初恋物語"(5MHz)でマウスカーソルの動きが悪い。たいていのゲームでは処理しないほうが動きがスムーズ。v2.39更新
		if (((dx == -1)||(dx ==  1))&&((dy == -1)||(dy ==  1)))
		{
			if (_bMoveX)
			{
				dy = 0;
				_bMoveX = FALSE;
			}
			else
			{
				dx = 0;
				_bMoveX = TRUE;
			}
		}

	//Kitao更新。マウス使用中のときは、「Windowsのマウスカーソル」を常にエミュレータウィンドウの真ん中に位置させるようにした。
	GetWindowRect(WINMAIN_GetHwnd(), &rc);
	_PrevX = rc.left + (rc.right-rc.left)/2;
	_PrevY = rc.top + (rc.bottom-rc.top)/2;
	SetCursorPos(_PrevX, _PrevY);

	_DeltaX = (Sint8)dx;
	_DeltaY = (Sint8)dy;
}


Uint8
MOUSE_ReadDelta()
{
	switch (_ReadCount++)
	{
		case 0: /* read delta x hi */
			//PRINTF("dx hi = %d\n", ((Uint8)_DeltaX) >> 4);
			return ((Uint8)_DeltaX) >> 4;

		case 1: /* read delta x lo */
			//PRINTF("dx lo = %d\n", ((Uint8)_DeltaX) & 0xF);
			return ((Uint8)_DeltaX) & 0xF;

		case 2: /* read delta y hi */
			//PRINTF("dy hi = %d\n", ((Uint8)_DeltaY) >> 4);
			return ((Uint8)_DeltaY) >> 4;

		case 3: /* read delta y lo */
			//PRINTF("dy lo = %d\n", ((Uint8)_DeltaY) & 0xF);
			return ((Uint8)_DeltaY) & 0xF;

		case 4: //Kitao追加。この状態でリセットされるまで待つ。こうすると一定のリズムでカーソルが動く。実機でも同様かは未確認。v1.23
			_DeltaX = 0;
			_DeltaY = 0;
			_ReadCount = 4;
			return 0;
	}

	return 0;
}


Uint8
MOUSE_ReadButtons()
{
	Uint8	ret = _ButtonState;

	if ((ret & 0xC) == 0xC) //RUN+SELECT同時押しになってしまっている場合
		ret &= ~0xC; //誤操作によるソフトリセットを避ける。ジョイパッドからはリセットOK。v1.43

	//ジョイパッド１のSelectボタン,Runボタン,Iボタン,IIボタンも受け付ける
	if (INPUT_IsPressed(1, JOYPAD_BUTTON_SELECT))	ret |= 4;
	if (INPUT_IsPressed(1, JOYPAD_BUTTON_RUN))		ret |= 8;
	if (INPUT_IsPressed(1, JOYPAD_BUTTON_I))		ret |= 1;
	if (INPUT_IsPressed(1, JOYPAD_BUTTON_II))		ret |= 2;

	//Kitao追加。SelectボタンとRunボタンをスワップする設定（バルンバ等のための機能）なら。
	if (JOYPAD_GetSwapSelRun())
	{
		Uint8	a = ret;
		ret &= ~12;
		ret |= (a & 4)<<1;
		ret |= (a & 8)>>1;
	}

	//Kitao追加。IボタンとIIボタンをスワップする設定（レミングス等のための機能）なら。
	if (JOYPAD_GetSwapIandII())
	{
		Uint8	a = ret;
		ret &= ~3;
		ret |= (a & 1)<<1;
		ret |= (a & 2)>>1;
	}

	return ret ^0xF;
}


void
MOUSE_Connect(
	BOOL		bConnect)
{
	_bMsConnected = bConnect;
}


BOOL
MOUSE_IsConnected()
{
	return _bMsConnected;
}


//Kitao更新。進めるクロック数を1クロック固定にした。
void
MOUSE_AdvanceClock()
{
	_ClockCounter++;

	if (_MouseWheelFlg > 0) //Kitao追加。マウスホイールの回転でセレクトボタンとランボタンが押された場合は、ボタンステートをクリアする。
		if (VDC_GetOverClockNow() == FALSE) //オーバークロックぶんの処理ではなく通常のCPU処理パワー内の動作なら
			if (--_MouseWheelFlg == 0) //約0.33秒経過したら、セレクトボタン・ランボタンが離されたことにする。
				_ButtonState &= ~12;
}


//Kitao追加。ホイール回転でセレクト・ランボタンを押したあとに、セレクト・ランボタンを離す処理のために必要。
void
MOUSE_SetMouseWheelFlg()
{
	_MouseWheelFlg = 7159090/3; //約0.33秒経過したら、自動的にセレクトボタン・ランボタンが離されるようににする。
}

//Kitao追加
Uint32
MOUSE_GetMouseWheelFlg()
{
	return _MouseWheelFlg;
}


// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
MOUSE_SaveState(
	FILE*		p)
{
	Sint32	currY; //Kitao更新。_CurrXと_CurrYは非使用にしたが過去バージョンのステートと互換性を取るために残してある

	if (p == NULL)
		return FALSE;

	SAVE_V(_ReadCount);
	SAVE_V(_DeltaX);
	SAVE_V(_DeltaY);
	SAVE_V(_bMoveX); //Kitao追加。v1.23。元はcurrXが保存されていたが非使用になったので、ここにbMoveX保存。
	SAVE_V(currY);
	SAVE_V(_PrevX);
	SAVE_V(_PrevY);
	SAVE_V(_bMsConnected);
	SAVE_V(_ButtonState);
	SAVE_V(_MouseWheelFlg); //Kitao追加

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
MOUSE_LoadState(
	FILE*		p)
{
	Sint32	currY; //Kitao更新。_CurrXと_CurrYは非使用にしたが過去バージョンのステートと互換性を取るために残してある

	if (p == NULL)
		return FALSE;

	LOAD_V(_ReadCount);
	LOAD_V(_DeltaX);
	LOAD_V(_DeltaY);
	LOAD_V(_bMoveX); //Kitao追加。v1.23。元はcurrXが保存されていたが非使用になったので、ここにbMoveX保存。古いステートでcurrXをbMoveXとして読み込んでしまうが、bMoveXはどんな値でも不都合は無いので問題なし。
	LOAD_V(currY); //Kitao更新。現在非使用
	LOAD_V(_PrevX);
	LOAD_V(_PrevY);
	LOAD_V(_bMsConnected);
	LOAD_V(_ButtonState);
	LOAD_V(_MouseWheelFlg); //Kitao追加

	return TRUE;
}

#undef SAVE_V
#undef LOAD_V
