/******************************************************************************
Ootake
・入力の読み込み処理が行われるそのつど、Windowsパッドの状態を最新状態にして読み
  取るようにした。アクションゲームでの反応が良くなり、実機の操作感に近づけた。
・マルチタップを付けていない時は、実機同様に、１Ｐと同じ入力が２Ｐ～５Ｐにも出
  力されるようにした。(超兄貴でオプションが弾を撃たない問題も解消)
・カウンターの進め方＆セカンドバイトフラグの更新の仕方を変更し、リンダキューブ
  餓狼SPなどいくつかの６ボタンパッドで誤動作する問題を解消した。
・_bClearFlgが立っているときは、すぐ_bHighNibbleもクリアするようにした。真・女
  神転生で正常にセーブが(MB128と通常BRAM共に)できるようになった。v1.53
・セレクトボタンとランボタンを入れ替える機能を付けた。（バルンバ等でプレイしや
  すくするため）
・連射機能をつけた。
・アベニューパッド３のように、３ボタン用の設定をできるようにした。v0.79

Copyright(C)2006-2011 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[JoyPad.c]
		パッドを記述します。

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

#include "JoyPad.h"
#include "Input.h"
#include "MB128.h"
#include "Mouse.h"
#include "WinMain.h"
#include "MainBoard.h"
#include "App.h"

extern Sint32 gCPU_ClockCount; //v2.00追加。高速化のためカウンタをグローバル化。

//Kitao追加。連射スピード。現バージョンでは全パッド共用のスピード。
#define RENSHA_HIGH		2
#define RENSHA_MIDDLE	4 //ドラゴンスピリット，妖怪道中記，レジェンドオブヒーロートンマOK
#define RENSHA_LOW		7 //v0.87更新。奇数にすることでイース４のファイヤー連射OK

static BOOL		_bMultiTap;
static BOOL		_bSixButtonPad;
static BOOL		_bThreeButtonPad; //Kitao追加
static BOOL		_bSwapSelRun; //Kitao追加
static BOOL		_bSwapIandII; //Kitao追加。v0.80

static Uint32	_Counter;
static BOOL		_bClearFlg;
static BOOL		_bHighNibble;
static BOOL		_bSecondByte;

static Sint32	_PrevUpdateStateLine = -1; //Kitao追加
static BOOL		_bMakeHesNoSound = FALSE; //Kitao追加
static BOOL		_bMakeHesNoSoundExecute = FALSE; //Kitao追加

static PCEPAD	_RenshaCount[6]; //各PCEパッド[1]～[5]の連射設定([0]は非使用)。0なら連射オフ。1以上なら連射オン。カウンタとしても使う。
static PCEPAD	_RenshaSpeed[6]; //各PCEパッド[1]～[5]の連射速度設定([0]は非使用)
static Sint32	_RenshaSpeedSel; //連射設定の選択値。
static Sint16	_RenshaSpeedMax; //連射スピードが速すぎると弾が出ないソフトの場合、許容範囲値をここに設定する。

static Uint8	_PrevCursorKeyLR[6]; //v2.59追加
static Uint8	_PrevCursorKeyUD[6]; //v2.59追加


//Kitao更新
static
Uint8
read_two_button_pad()
{
	Uint8	ret = 0x0F; //TG16準拠(bit6=0)
	Uint8	b1, b2;
	Uint32	padNumber;
	BOOL	bThreeButtonPushed = FALSE; //Kitao追加

	if (_Counter == 0)
		return 0x00; //v1.10更新。エメラルドドラゴンでMB128ユーティリティが動作。ガンヘッドの面セレクト裏技もOK。

	if (_bMultiTap)
	{
		if (_Counter > 5)
			return 0x0F; //v1.10更新。ここで0x00を返すと真怨霊戦記で入力不具合。
		padNumber = _Counter;
	}
	else
		padNumber = 1; //Kitao追加。マルチタップを付けていないときは2P～5Pの信号も1Pの操作で出力される。

	if (_bHighNibble)
	{
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_UP))		ret &= ~0x1;
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_RIGHT))	ret &= ~0x2;
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_DOWN))		ret &= ~0x4;
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_LEFT))		ret &= ~0x8;

		//左右同時押し、上下同時押しの入力を抑制。イメージファイト２で必要。v2.59	
		if (APP_GetCheckSimultaneouslyPush())
		{
			if ((ret & 0xA) == 0) //左右同時押し
				ret |= _PrevCursorKeyLR[padNumber]; //一番最後に押したキーのみを有効にする
			else
				_PrevCursorKeyLR[padNumber] = ~ret & 0xA;
			if ((ret & 0x5) == 0) //上下同時押し
				ret |= _PrevCursorKeyUD[padNumber]; //一番最後に押したキーのみを有効にする
			else
				_PrevCursorKeyUD[padNumber] = ~ret & 0x5;
		}
	}
	else
	{
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_I))
		{
			if ((_RenshaCount[padNumber].button1 > 0)&&
				(_RenshaSpeed[1].button1 > 0)) //連射スピードがオフではない場合
			{
				if (_RenshaCount[padNumber].button1 <= (_RenshaSpeed[1].button1 >> 1)) //カウンタの値が半分以下のときは押されたことにする。半分より上のときは押されていないことにする。
					ret &= ~0x1; //押された
				if (--_RenshaCount[padNumber].button1 == 0)
					_RenshaCount[padNumber].button1 = _RenshaSpeed[1].button1; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
			}
			else
				ret &= ~0x1; //押された
		}
		else if ((_RenshaCount[padNumber].button1 > 0)&&
				 (_RenshaSpeed[1].button1 > 0)) //連射スピードがオフではない場合
					_RenshaCount[padNumber].button1 = (_RenshaSpeed[1].button1 >> 1); //ボタンを押していなかった場合、連射カウンタをリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。

		if (_RenshaSpeed[1].button5 > 0) //連射スピードがオフではない場合
		{
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_V)) //JOYPAD_BUTTON_V…2ボタン,3ボタンパッドのときは、連射専用ボタンとして機能。
			{
				if (_RenshaCount[padNumber].button5 > 0)
				{
					if (_RenshaCount[padNumber].button5 <= (_RenshaSpeed[1].button5 >> 1)) //カウンタの値が半分以下のときは押されたことにする。半分より上のときは押されていないことにする。
						ret &= ~0x1; //押された
					if (--_RenshaCount[padNumber].button5 == 0)
						_RenshaCount[padNumber].button5 = _RenshaSpeed[1].button5; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
				}
				else
					ret &= ~0x1; //押された
			}
			else if (_RenshaCount[padNumber].button5 > 0)
					_RenshaCount[padNumber].button5 = (_RenshaSpeed[1].button5 >> 1); //ボタンを押していなかった場合、連射カウンタをリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。
		}

		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_II))
		{
			if ((_RenshaCount[padNumber].button2 > 0)&&
				(_RenshaSpeed[1].button2 > 0)) //連射スピードがオフではない場合
			{
				if (_RenshaCount[padNumber].button2 <= (_RenshaSpeed[1].button2 >> 1)) //カウンタの値が半分以下のときは押されたことにする。半分より上のときは押されていないことにする。
					ret &= ~0x2; //押された
				if (--_RenshaCount[padNumber].button2 == 0)
					_RenshaCount[padNumber].button2 = _RenshaSpeed[1].button2; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
			}
			else
				ret &= ~0x2; //押された
		}
		else if ((_RenshaCount[padNumber].button2 > 0)&&
				 (_RenshaSpeed[1].button2 > 0)) //連射スピードがオフではない場合
					_RenshaCount[padNumber].button2 = (_RenshaSpeed[1].button2 >> 1); //ボタンを押していなかった場合、連射カウンタを1にリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。

		if (_RenshaSpeed[1].button6 > 0) //連射スピードがオフではない場合
		{
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_VI)) //JOYPAD_BUTTON_VI…2ボタン,3ボタンパッドのときは、連射専用ボタンとして機能。
			{
				if (_RenshaCount[padNumber].button6 > 0)
				{
					if (_RenshaCount[padNumber].button6 <= (_RenshaSpeed[1].button6 >> 1)) //カウンタの値が半分以下のときは押されたことにする。半分より上のときは押されていないことにする。
						ret &= ~0x2; //押された
					if (--_RenshaCount[padNumber].button6 == 0)
						_RenshaCount[padNumber].button6 = _RenshaSpeed[1].button6; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
				}
				else
					ret &= ~0x2; //押された
			}
			else if (_RenshaCount[padNumber].button6 > 0)
					_RenshaCount[padNumber].button6 = (_RenshaSpeed[1].button6 >> 1); //ボタンを押していなかった場合、連射カウンタを1にリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。
		}

		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_SELECT))
			ret &= ~0x4;
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_IV)) //３ボタンパッド設定の時用
		{
			ret &= ~0x4;
			bThreeButtonPushed = TRUE;
		}
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_RUN))
			ret &= ~0x8;
		if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_III)) //３ボタンパッド設定の時用
		{
			ret &= ~0x8;
			bThreeButtonPushed = TRUE;
		}
		if ((ret & 0x8)==0) //RUNボタンが押されたなら
		{
			//RUNボタンの連射処理
			if ((_RenshaCount[padNumber].buttonRun > 0)&&
				(_RenshaSpeed[1].buttonRun > 0)) //連射スピードがオフではない場合
			{
				if (_RenshaCount[padNumber].buttonRun > (_RenshaSpeed[1].buttonRun >> 1)) //カウンタの値が半分より上のときは押されていないことにする。半分以下のときは押されたことにする。
					ret |= 0x8; //押されていないことにする
				if (--_RenshaCount[padNumber].buttonRun == 0)
					_RenshaCount[padNumber].buttonRun = _RenshaSpeed[1].buttonRun; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
			}
		}
		else if ((_RenshaCount[padNumber].buttonRun > 0)&&
				 (_RenshaSpeed[1].buttonRun > 0)) //連射スピードがオフではない場合
					_RenshaCount[padNumber].buttonRun = (_RenshaSpeed[1].buttonRun >> 1); //ボタンを押していなかった場合、連射カウンタを1にリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。

		if (_bSwapSelRun)
			if ((!_bThreeButtonPad)||(bThreeButtonPushed)) //３ボタンパッドの場合、IIIかIVボタンが押されたときのみ入れ替えを行う(通常のSELECT,RUNボタンが押されたときは入れ換えない)。
			{	//RUNボタンとSELECTボタンを入れ替え
				b1 = (ret & 0x8) >> 1;
				b2 = (ret & 0x4) << 1;
				ret &= ~0xC;
				ret |= b1 | b2;
			}
		if (_bSwapIandII)
		{	//IボタンとIIボタンを入れ替え
			b1 = (ret & 0x2) >> 1;
			b2 = (ret & 0x1) << 1;
			ret &= ~0x3;
			ret |= b1 | b2;
		}
	}

	if ((padNumber == 1)&&(INPUT_CheckButtonState(APP_GetFunctionButton())))
		return 0x0F; //ファンクションボタンを押している間はPCE側への入力を受け付けないようにする。v2.38

	return ret;
}


//Kitao更新。動きをシンプルにした。セカンドバイトフラグの更新の仕方を変更し、餓狼SPなどでの不具合を直した。
static
Uint8
read_six_button_pad()
{
	Uint8	ret = 0x0F; //TG16準拠(bit6=0)
	Uint8	b1, b2;
	Uint32	padNumber;

	if (_Counter == 0)
		return 0x00; //v1.10更新。エメラルドドラゴンでMB128ユーティリティが動作。ガンヘッドの面セレクト裏技もOK。

	if (_bMultiTap)
	{
		if (_Counter > 5)
			return 0x0F; //v1.10更新。ここで0x00を返すと真怨霊戦記で入力不具合。
		padNumber = _Counter;
	}
	else
		padNumber = 1; //Kitao追加。マルチタップを付けていないときは2P～5Pの信号も1Pの操作で出力される。

	if (_bSecondByte)
	{
		if (_bHighNibble)
		{
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_UP))		ret &= ~0x1;
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_RIGHT))	ret &= ~0x2;
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_DOWN))		ret &= ~0x4;
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_LEFT))		ret &= ~0x8;

			//左右同時押し、上下同時押しの入力を抑制。イメージファイト２で必要。v2.59	
			if (APP_GetCheckSimultaneouslyPush())
			{
				if ((ret & 0xA) == 0) //左右同時押し
					ret |= _PrevCursorKeyLR[padNumber]; //一番最後に押したキーのみを有効にする
				else
					_PrevCursorKeyLR[padNumber] = ~ret & 0xA;
				if ((ret & 0x5) == 0) //上下同時押し
					ret |= _PrevCursorKeyUD[padNumber]; //一番最後に押したキーのみを有効にする
				else
					_PrevCursorKeyUD[padNumber] = ~ret & 0x5;
			}
		}
		else
		{
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_I))
			{
				if ((_RenshaCount[padNumber].button1 > 0)&&
					(_RenshaSpeed[1].button1 > 0)) //連射スピードがオフではない場合
				{
					if (_RenshaCount[padNumber].button1 <= (_RenshaSpeed[1].button1 >> 1)) //カウンタの値が半分以下のときは押されたことにする。半分より上のときは押されていないことにする。
						ret &= ~0x1; //押された
					if (--_RenshaCount[padNumber].button1 == 0)
						_RenshaCount[padNumber].button1 = _RenshaSpeed[1].button1; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
				}
				else
					ret &= ~0x1; //押された
			}
			else if ((_RenshaCount[padNumber].button1 > 0)&&
					 (_RenshaSpeed[1].button1 > 0)) //連射スピードがオフではない場合
						_RenshaCount[padNumber].button1 = (_RenshaSpeed[1].button1 >> 1); //ボタンを押していなかった場合、連射カウンタをリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。

			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_II))
			{
				if ((_RenshaCount[padNumber].button2 > 0)&&
					(_RenshaSpeed[1].button2 > 0)) //連射スピードがオフではない場合
				{
					if (_RenshaCount[padNumber].button2 <= (_RenshaSpeed[1].button2 >> 1)) //カウンタの値が半分以下のときは押されたことにする。半分より上のときは押されていないことにする。
						ret &= ~0x2; //押された
					if (--_RenshaCount[padNumber].button2 == 0)
						_RenshaCount[padNumber].button2 = _RenshaSpeed[1].button2; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
				}
				else
					ret &= ~0x2; //押された
			}
			else if ((_RenshaCount[padNumber].button2 > 0)&&
					 (_RenshaSpeed[1].button2 > 0)) //連射スピードがオフではない場合
						_RenshaCount[padNumber].button2 = (_RenshaSpeed[1].button2 >> 1); //ボタンを押していなかった場合、連射カウンタを1にリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。

			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_SELECT))
				ret &= ~0x4;
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_RUN))
				ret &= ~0x8;
			if ((ret & 0x8)==0) //RUNボタンが押されたなら
			{
				//RUNボタンの連射処理
				if ((_RenshaCount[padNumber].buttonRun > 0)&&
					(_RenshaSpeed[1].buttonRun > 0)) //連射スピードがオフではない場合
				{
					if (_RenshaCount[padNumber].buttonRun > (_RenshaSpeed[1].buttonRun >> 1)) //カウンタの値が半分より上のときは押されていないことにする。半分以下のときは押されたことにする。
						ret |= 0x8; //押されていないことにする
					if (--_RenshaCount[padNumber].buttonRun == 0)
						_RenshaCount[padNumber].buttonRun = _RenshaSpeed[1].buttonRun; //連射スピードは、現バージョンではパッド１の設定を全パッドで共用する。
				}
			}
			else if ((_RenshaCount[padNumber].buttonRun > 0)&&
					 (_RenshaSpeed[1].buttonRun > 0)) //連射スピードがオフではない場合
						_RenshaCount[padNumber].buttonRun = (_RenshaSpeed[1].buttonRun >> 1); //ボタンを押していなかった場合、連射カウンタを1にリセットする。次にこのボタンを押したときに「すぐ押したと判定される」ために必要。

			if (_bSwapSelRun)
			{	//RUNボタンとSELECTボタンを入れ替え
				b1 = (ret & 0x8) >> 1;
				b2 = (ret & 0x4) << 1;
				ret &= ~0xC;
				ret |= b1 | b2;
			}
			if (_bSwapIandII)
			{	//IボタンとIIボタンを入れ替え
				b1 = (ret & 0x2) >> 1;
				b2 = (ret & 0x1) << 1;
				ret &= ~0x3;
				ret |= b1 | b2;
			}
		}
	}
	else
	{
		if (_bHighNibble)
			ret = 0x00;
		else
		{
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_III))	ret &= ~0x1;
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_IV))	ret &= ~0x2;
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_V))	ret &= ~0x4;
			if (INPUT_IsPressed(padNumber, JOYPAD_BUTTON_VI))	ret &= ~0x8;
		}
	}

	//Kitao追加。マルチタップ非使用時は、パッド５ぶんを読み出し終えたらセカンドバイトフラグをクリア。リンダキューブのタイトル画面で必要。
	if (!_bMultiTap)
		if (_Counter == 5)
			_bSecondByte = FALSE;

	if ((padNumber == 1)&&(INPUT_CheckButtonState(APP_GetFunctionButton())))
		return 0x0F; //ファンクションボタンを押している間はPCE側への入力を受け付けないようにする。v2.38

	return ret;
}


//Kitao追加。MainBoard.cからも使用。
void
JOYPAD_Reset()
{
	_Counter = 0;
	_bClearFlg = FALSE;
	_bHighNibble = FALSE;
	_bSecondByte = FALSE;
}


/*-----------------------------------------------------------------------------
	初期化 
-----------------------------------------------------------------------------*/
Sint32
JOYPAD_Init()
{
	int	i;

	INPUT_Init();

	_bSixButtonPad = FALSE;
	_bMultiTap = TRUE;
	_bSwapSelRun = FALSE; //Kitao追加
	_bSwapIandII = FALSE; //Kitao追加
	JOYPAD_ConnectMouse(FALSE); //Kitao追加
	JOYPAD_ConnectMB128(TRUE);

	//Kitao追加。十字キー同時押し抑制用変数を初期化
	for (i=1; i<=5; i++)
	{
		_PrevCursorKeyLR[i] = 0;
		_PrevCursorKeyUD[i] = 0;
	}

	//Kitao追加。連射用変数を初期化
	ZeroMemory(_RenshaCount, sizeof(_RenshaCount));
	JOYPAD_SetRenshaButton5(TRUE);
	JOYPAD_SetRenshaButton6(TRUE);
	_RenshaSpeedSel = APP_GetRenshaSpeedSel();
	_RenshaSpeedMax = 0;
	JOYPAD_SetRenshaSpeedButton1(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton2(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButtonRun(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton5(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton6(_RenshaSpeedSel);

	JOYPAD_Reset();//Kitao追加

	return 0;
}


void
JOYPAD_Deinit()
{
	INPUT_Deinit();
}


//Kitao更新。戻り値をなくし設定専用にした。
void
JOYPAD_ConnectMultiTap(
	BOOL	bConnect)
{
	_bMultiTap = bConnect;
}

//Kitao更新。戻り値をなくし設定専用にした。
void
JOYPAD_UseSixButton(
	BOOL	bSixButton)
{
	_bSixButtonPad = bSixButton;
	//Kitao追加
	if (bSixButton)
	{	//ボタン５,６(２,３ボタンパッド使用時の連射用ボタン)の連射設定を解除する。
		JOYPAD_SetRenshaButton5(FALSE);
		JOYPAD_SetRenshaButton6(FALSE);
	}
	else
	{	//ボタン５,６(２,３ボタンパッド使用時の連射用ボタン)の連射設定を復帰する。
		if (JOYPAD_GetRenshaButton1())
			JOYPAD_SetRenshaButton5(FALSE);
		else
			JOYPAD_SetRenshaButton5(TRUE);
		if (JOYPAD_GetRenshaButton2())
			JOYPAD_SetRenshaButton6(FALSE);
		else
			JOYPAD_SetRenshaButton6(TRUE);
	}
}

//Kitao追加
void
JOYPAD_UseThreeButton(
	BOOL	bThreeButton)
{
	_bThreeButtonPad = bThreeButton;
}

//Kitao更新。戻り値をなくし設定専用にした。
void
JOYPAD_ConnectMouse(
	BOOL	bConnect)
{
	MOUSE_Connect(bConnect);
	WINMAIN_ShowCursor(~bConnect); //Kitao追加
}

//Kitao更新。戻り値をなくし設定専用にした。
void
JOYPAD_ConnectMB128(
	BOOL	bConnect)
{
	MB128_Connect(bConnect);
}


//Kitao追加
BOOL
JOYPAD_GetConnectMultiTap()
{
	return _bMultiTap;
}

//Kitao追加
BOOL
JOYPAD_GetConnectSixButton()
{
	return _bSixButtonPad;
}

//Kitao追加
BOOL
JOYPAD_GetConnectThreeButton()
{
	return _bThreeButtonPad;
}


//Kitao追加
void
JOYPAD_SetSwapSelRun(
	BOOL	bSwapSelRun)
{
	_bSwapSelRun = bSwapSelRun;
}

//Kitao追加
void
JOYPAD_SetSwapIandII(
	BOOL	bSwapIandII)
{
	_bSwapIandII = bSwapIandII;
}

//Kitao追加。App.cとMouse.cから使用。
BOOL
JOYPAD_GetSwapSelRun()
{
	return _bSwapSelRun;
}

//Kitao追加。App.cとMouse.cから使用。
BOOL
JOYPAD_GetSwapIandII()
{
	return _bSwapIandII;
}


/*	$1000 (R) status */
Uint8
JOYPAD_Read(
	Uint32	regNum)
{
	Uint8	ret;

	if (MB128_IsActive())
		return MB128_Read();

	//Kitao追加。カウンターはRead時に進めるようにした。餓狼SPなどいくつかの６ボタンパッド対応ソフトでの問題を解消。
	if (_bHighNibble)
	{
		if (++_Counter == 16) //4bitと仮定。16なら0に戻す。3bitだとＦ１サーカスで車がスタートできず。v1.53
			_Counter = 0;
	}

	//Kitao追加。ここでWindowsジョイスティックの状態をアップデートすることで遅延感なく操作できる。スペースハリアー(最小の円を描きながら連射)で違いがわかる
 	if (MAINBOARD_GetScanLine() != _PrevUpdateStateLine) //前回アップデートしてから１ライン描画以上の時間が立っていたらアップデート。アップデートのし過ぎで重くなることを回避。
	{
		INPUT_UpdateState(TRUE);
		_PrevUpdateStateLine = MAINBOARD_GetScanLine();
	}

	if (MOUSE_IsConnected())
	{
		if ((_Counter == 1)&&(_bHighNibble))
			return MOUSE_ReadDelta();
		else
			return MOUSE_ReadButtons();
	}

	if (_bSixButtonPad)
		return read_six_button_pad();

	//Kitao追加。hesファイルの曲間に無音を入れる。v1.29
	if (_bMakeHesNoSound)
	{
		ret = read_two_button_pad();
		if (_bMakeHesNoSoundExecute)
		{
			if ((_Counter == 1)&&(!_bHighNibble)&&((ret & 0x3) == 3)) //IボタンかIIボタンを離していたら
				_bMakeHesNoSoundExecute = FALSE;
		}
		else
		{
			if ((_Counter == 1)&&(!_bHighNibble)&&((ret & 0x3) != 3)) //IボタンかIIボタンを押していたら
			{
				PSG_ResetVolumeReg(); //PSGのボリュームレジスタを0に。
				gCPU_ClockCount -= (Sint32)(7159090*0.5); //0.5秒間。無音期間を作る。
				_bMakeHesNoSoundExecute = TRUE; //２回連続して動作させないための策
			}
		}
		return ret;
	}

	return read_two_button_pad();
}

//Kitao追加
void
JOYPAD_ClearPrevUpdateStateLine()
{
	_PrevUpdateStateLine = -1;
}


/*	$1000 (W) select */
//Kitao更新。動きをシンプルにした。いくつかの６ボタンパッド対応ソフトの不具合をなくした。
void
JOYPAD_Write(
	Uint32	regNum,
	Uint8	data)
{
	MB128_Write(data);

	_bClearFlg = data & 0x2;
	_bHighNibble = data & 0x1;

	if (_bClearFlg)
	{
		_Counter = 0;
		_bHighNibble = FALSE; //Kitao追加。真・女神転生のセーブ時に必要。v1.53
		_bSecondByte ^= TRUE; //Kitao追加。セカンドバイトが反転するようだ。餓狼SPなどでの不具合をなくした。
		MOUSE_Reset();
	}
}


//Kitao追加。連射スピードをセットする。現バージョンでは全パッド共用のスピード。
void
JOYPAD_SetRenshaSpeedButton1(
	Sint32	n)
{
	_RenshaSpeedSel = n;

	switch (n)
	{
		case 0: //Off
			_RenshaSpeed[1].button1 = 0;
			break;
		case 1: //High
			_RenshaSpeed[1].button1 = RENSHA_HIGH;
			break;
		case 2: //Middle
			_RenshaSpeed[1].button1 = RENSHA_MIDDLE;
			break;
		case 3: //Low
			_RenshaSpeed[1].button1 = RENSHA_LOW;
			break;
	}

	//連射スピードが速すぎると弾が出ないソフトの場合、自動的に速度を下げて調整する。v1.43追加
	if ((_RenshaSpeed[1].button1 != 0)&&(_RenshaSpeed[1].button1 < _RenshaSpeedMax))
		_RenshaSpeed[1].button1 = _RenshaSpeedMax;
}
//Kitao追加。連射スピードをセットする。現バージョンでは全パッド共用のスピード。
void
JOYPAD_SetRenshaSpeedButton2(
	Sint32	n)
{
	_RenshaSpeedSel = n;

	switch (n)
	{
		case 0: //Off
			_RenshaSpeed[1].button2 = 0;
			break;
		case 1: //High
			_RenshaSpeed[1].button2 = RENSHA_HIGH;
			break;
		case 2: //Middle
			_RenshaSpeed[1].button2 = RENSHA_MIDDLE;
			break;
		case 3: //Low
			_RenshaSpeed[1].button2 = RENSHA_LOW;
			break;
	}

	//連射スピードが速すぎると弾が出ないソフトの場合、自動的に速度を下げて調整する。v1.43追加
	if ((_RenshaSpeed[1].button2 != 0)&&(_RenshaSpeed[1].button2 < _RenshaSpeedMax))
		_RenshaSpeed[1].button2 = _RenshaSpeedMax;
}
//Kitao追加。連射スピードをセットする。現バージョンでは全パッド共用のスピード。
void
JOYPAD_SetRenshaSpeedButtonRun(
	Sint32	n)
{
	_RenshaSpeedSel = n;

	switch (n)
	{
		case 0: //Off
			_RenshaSpeed[1].buttonRun = 0;
			break;
		case 1: //High
			_RenshaSpeed[1].buttonRun = RENSHA_HIGH;
			break;
		case 2: //Middle
			_RenshaSpeed[1].buttonRun = RENSHA_MIDDLE;
			break;
		case 3: //Low
			_RenshaSpeed[1].buttonRun = RENSHA_LOW;
			break;
	}

	//連射スピードが速すぎると弾が出ないソフトの場合、自動的に速度を下げて調整する。v1.43追加
	if ((_RenshaSpeed[1].buttonRun != 0)&&(_RenshaSpeed[1].buttonRun < _RenshaSpeedMax))
		_RenshaSpeed[1].buttonRun = _RenshaSpeedMax;
}
//Kitao追加。連射スピードをセットする。現バージョンでは全パッド共用のスピード。
void
JOYPAD_SetRenshaSpeedButton5(
	Sint32	n)
{
	_RenshaSpeedSel = n;

	switch (n)
	{
		case 0: //Off
			_RenshaSpeed[1].button5 = 0;
			break;
		case 1: //High
			_RenshaSpeed[1].button5 = RENSHA_HIGH;
			break;
		case 2: //Middle
			_RenshaSpeed[1].button5 = RENSHA_MIDDLE;
			break;
		case 3: //Low
			_RenshaSpeed[1].button5 = RENSHA_LOW;
			break;
	}

	//連射スピードが速すぎると弾が出ないソフトの場合、自動的に速度を下げて調整する。v1.43追加
	if ((_RenshaSpeed[1].button5 != 0)&&(_RenshaSpeed[1].button5 < _RenshaSpeedMax))
		_RenshaSpeed[1].button5 = _RenshaSpeedMax;
}
//Kitao追加。連射スピードをセットする。現バージョンでは全パッド共用のスピード。
void
JOYPAD_SetRenshaSpeedButton6(
	Sint32	n)
{
	_RenshaSpeedSel = n;

	switch (n)
	{
		case 0: //Off
			_RenshaSpeed[1].button6 = 0;
			break;
		case 1: //High
			_RenshaSpeed[1].button6 = RENSHA_HIGH;
			break;
		case 2: //Middle
			_RenshaSpeed[1].button6 = RENSHA_MIDDLE;
			break;
		case 3: //Low
			_RenshaSpeed[1].button6 = RENSHA_LOW;
			break;
	}

	//連射スピードが速すぎると弾が出ないソフトの場合、自動的に速度を下げて調整する。v1.43追加
	if ((_RenshaSpeed[1].button6 != 0)&&(_RenshaSpeed[1].button6 < _RenshaSpeedMax))
		_RenshaSpeed[1].button6 = _RenshaSpeedMax;
}


//Kitao追加。連射のオンオフをセットする。
void
JOYPAD_SetRenshaButton1(
	BOOL	bSet)
{
	int	i;

	if (bSet)
		for (i=1; i<=5; i++)
			_RenshaCount[i].button1 = 1;
	else
		for (i=1; i<=5; i++)
			_RenshaCount[i].button1 = 0;
}
//Kitao追加。連射のオンオフをセットする。
void
JOYPAD_SetRenshaButton2(
	BOOL	set)
{
	int	i;

	if (set)
		for (i=1; i<=5; i++)
			_RenshaCount[i].button2 = 1;
	else
		for (i=1; i<=5; i++)
			_RenshaCount[i].button2 = 0;
}
//Kitao追加。連射のオンオフをセットする。
void
JOYPAD_SetRenshaButtonRun(
	BOOL	set)
{
	int	i;

	if (set)
		for (i=1; i<=5; i++)
			_RenshaCount[i].buttonRun = 1;
	else
		for (i=1; i<=5; i++)
			_RenshaCount[i].buttonRun = 0;
}
//Kitao追加。連射のオンオフをセットする。
void
JOYPAD_SetRenshaButton5(
	BOOL	bSet)
{
	int	i;

	if (bSet)
		for (i=1; i<=5; i++)
			_RenshaCount[i].button5 = 1;
	else
		for (i=1; i<=5; i++)
			_RenshaCount[i].button5 = 0;
}
//Kitao追加。連射のオンオフをセットする。
void
JOYPAD_SetRenshaButton6(
	BOOL	set)
{
	int	i;

	if (set)
		for (i=1; i<=5; i++)
			_RenshaCount[i].button6 = 1;
	else
		for (i=1; i<=5; i++)
			_RenshaCount[i].button6 = 0;
}


//Kitao追加。連射スピードが速すぎると弾が出ないソフトの場合、許容範囲値を設定する。
void
JOYPAD_SetRenshaSpeedMax(
	Sint32	n)	//n=1…High，2=Middle，3=Low。
{
	switch (n)
	{
		case 0: //Off
			_RenshaSpeedMax = 0;
			break;
		case 1: //High
			_RenshaSpeedMax = RENSHA_HIGH;
			break;
		case 2: //Middle
			_RenshaSpeedMax = RENSHA_MIDDLE;
			break;
		case 3: //Low
			_RenshaSpeedMax = RENSHA_LOW;
			break;
	}
	JOYPAD_SetRenshaSpeedButton1(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton2(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButtonRun(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton5(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton6(_RenshaSpeedSel);
}


//Kitao追加。現在連射モードなのかを返す。
BOOL
JOYPAD_GetRenshaButton1()
{
	return (_RenshaCount[1].button1 != 0);
}
//Kitao追加。現在連射モードなのかを返す。
BOOL
JOYPAD_GetRenshaButton2()
{
	return (_RenshaCount[1].button2 != 0);
}
//Kitao追加。現在連射モードなのかを返す。
BOOL
JOYPAD_GetRenshaButtonRun()
{
	return (_RenshaCount[1].buttonRun != 0);
}


//Kitao追加。曲間無音作成機能を有効にするかどうかを設定する。v1.29
void
JOYPAD_SetMakeHesNoSound(
	BOOL	bMakeHesNoSound)
{
	_bMakeHesNoSound = bMakeHesNoSound;
	_bMakeHesNoSoundExecute = FALSE;
}

//Kitao追加
BOOL
JOYPAD_GetMakeHesNoSound()
{
	return _bMakeHesNoSound;
}


// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
// save array
#define SAVE_A(A)	if (fwrite(A, sizeof(A), 1, p) != 1)	return FALSE
#define LOAD_A(A)	if (fread(A, sizeof(A), 1, p) != 1)		return FALSE
/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
JOYPAD_SaveState(
	FILE*		p)
{
	if (p == NULL)
		return FALSE;

	SAVE_V(_bSixButtonPad);
	SAVE_V(_bThreeButtonPad); //Kitao追加。v0.79

	SAVE_V(_bMultiTap);
	SAVE_V(_bSwapSelRun); //Kitao追加
	SAVE_V(_bSwapIandII); //Kitao追加
	SAVE_V(_Counter);
	SAVE_V(_bClearFlg);
	SAVE_V(_bHighNibble);
	SAVE_V(_bSecondByte);

	MOUSE_SaveState(p);
	MB128_SaveState(p);

	SAVE_A(_RenshaCount);
	SAVE_A(_RenshaSpeed);
	SAVE_V(_RenshaSpeedSel); //v1.43

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
JOYPAD_LoadState(
	FILE*		p)
{
	if (p == NULL)
		return FALSE;

	LOAD_V(_bSixButtonPad);
	if (MAINBOARD_GetStateVersion() >= 13) //Kitao追加。v0.79以降のセーブファイルなら
		LOAD_V(_bThreeButtonPad);
	APP_SetInputConfiguration(); //Kitao追加

	LOAD_V(_bMultiTap);
	LOAD_V(_bSwapSelRun); //Kitao追加
	if (MAINBOARD_GetStateVersion() >= 14) //Kitao追加。v0.80以降のセーブファイルなら
		LOAD_V(_bSwapIandII);
	LOAD_V(_Counter);
	LOAD_V(_bClearFlg);
	LOAD_V(_bHighNibble);
	LOAD_V(_bSecondByte);

	MOUSE_LoadState(p);
	MB128_LoadState(p);

	if (MAINBOARD_GetStateVersion() >= 4) //Kitao追加。v0.58以降のセーブファイルなら
	{
		LOAD_A(_RenshaCount);
		LOAD_A(_RenshaSpeed);
	}
	else
		ZeroMemory(_RenshaCount, sizeof(_RenshaCount));
	if (MAINBOARD_GetStateVersion() >= 32) //Kitao追加。v1.43以降のセーブファイルなら
	{
		LOAD_V(_RenshaSpeedSel);
	}
	else
	{
		switch (_RenshaSpeed[1].button1)
		{
			case RENSHA_HIGH:
				_RenshaSpeedSel = 1;
				break;
			case RENSHA_MIDDLE:
				_RenshaSpeedSel = 2;
				break;
			case RENSHA_LOW:
				_RenshaSpeedSel = 3;
				break;
			default:
				_RenshaSpeedSel = 0;
				break;
		}
	}
	APP_SetRenshaSpeedSel(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton1(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton2(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButtonRun(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton5(_RenshaSpeedSel);
	JOYPAD_SetRenshaSpeedButton6(_RenshaSpeedSel);
	JOYPAD_UseSixButton(_bSixButtonPad); //v0.91追加。v0.91より前のセーブファイルのために、連射専用ボタンの設定を更新する。

	return TRUE;
}

#undef SAVE_V
#undef LOAD_V
#undef SAVE_A
#undef LOAD_A
