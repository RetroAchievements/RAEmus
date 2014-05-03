/******************************************************************************
Ootake
・CPU.cから呼び出してクロックを進めるのではなく、VDC.cからAPUなどと並列して1ク
  ロックずつ進めるようにした。(音質の向上とオーバークロック機能搭載のため)
・早回しモードを付けた。その際に音まで早回しになってしまわないように、タイマー
  割り込みを遅らせることができるようにした。v0.92。
・Kiさんから情報をご連絡いただき実機により近い動作を実装。プリスケーラの動きの
  特徴(カウンタが0を下回ったときではなく512を下回った瞬間に割り込みが起こる)か
  ら、初回の割り込みは512クロックぶん早く発生するようにした。←※クラックスの
  サンプリング音声が早口になってしまったので、元に戻した（v0.56）。私の実装の
  仕方が悪かったのか、実際の実機では「512クロックの遊びが他にある」可能性も。
  とりあえず「シンプルに時間通り」で良さそう。
・ReloadRegisterが未設定の場合、タイマー割り込みはカウントダウンしないようにし
  た。(暗黒伝説ポーズ解除時にモッサリする問題を解消)
・DownCounterのリセットは、コントロールレジスタに0が書き込まれたときにおこなう
  ようにした。v0.69。
・TimerRunningがオフでもReloadRegisterが設定されていれば、カウントダウンをおこ
  なうようにした。v0.69。←この実装でクラックスのサンプリング音が実機に近くな
  った。以前にKiさんから情報をご連絡いただいた「初回の割り込みは早めに起こる」
  現象にも一致した。
・ReloadRegisterは-1しないほうが実機に近い割り込み間隔になるようだ。クラックス
  やダンジョンエクスプローラーなどの音色が実機に近づいたと思う。v1.62
・タイマー割り込み発生の回数が実機より少なかったようなので、TimerRunningがFALSE
  のときの処理を改良した。真女神転生のオートマッピング表示時の乱れがほぼ解消。
  v2.20

Copyright(C)2006-2012 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[Timer.c]
		タイマーを実装します．

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
#include <stdio.h>
#include "Timer.h"
#include "IntCtrl.h"
#include "MAINBOARD.h"
#include "APP.h"
#include "Printf.h"

/*
	タイマーの割り込み周期：
	21.47727 / 3 / 1024 = 6991.298828125 [Hz] --> 1.430349e-4 [s]
	21.47727 / 3 / 1024 / 128 = 54.6195220947265625 [Hz] --> 0.01830847 [s]

	タイマはＣＰＵの動作クロック 21.47727 / 3 [MHz] を
	1024分周して動作する．ということは，CPUの1024サイクル
	がダウンカウンタの1サイクルと等しい．よって，ダウンカウンタは
	内部で本来の値を1024倍(<<10)して保持することにする．
*/
static Sint32	_ReloadRegister;
static Sint32	_DownCounter;
static BOOL		_bTimerRunning;


/*-----------------------------------------------------------------------------
** [TIMER_Init]
**   タイマーを初期化します．
**---------------------------------------------------------------------------*/
Sint32
TIMER_Init()
{
	_ReloadRegister = 1024; //Kitao追加。v2.20
	_DownCounter = 1; //Kitao追加。v1.61
	_bTimerRunning = FALSE;
	return 0;
}


/*-----------------------------------------------------------------------------
** [TIMER_Destroy]
**   タイマーの終了処理を行ないます．
**---------------------------------------------------------------------------*/
void
TIMER_Deinit()
{
	INTCTRL_Cancel(INTCTRL_TIRQ);
}


/*-----------------------------------------------------------------------------
** [TIMER_Read]
**   タイマーからの読み出し動作です．
**---------------------------------------------------------------------------*/
Uint8
TIMER_Read() //Kitao更新
{
	return (Uint8)((_DownCounter >> 10) & 0x7F);
}


/*-----------------------------------------------------------------------------
** [TIMER_Write]
**   タイマーへの書き込み動作です．
**---------------------------------------------------------------------------*/
void
TIMER_Write(
	Uint32		regNum,
	Uint8		data)
{
	switch (regNum & 1)
	{
		case 0:
			//PRINTF("TimerTest %X, %X",regNum,data);//test用
			//Kitao更新。シンプルに0がセットされると1024カウンタ待ち。v1.62
			//クラックスなど多くのソフトで音色が実機に近づいたと思う。イース４のビジュアルシーンの口パクと音もピッタリに。
			_ReloadRegister = ((data & 0x7F) + 1) * 1024;
			//このタイミングでカウンタを初期化するようにした。真女神転生のオートマッピング画面の乱れがほぼ解消。稀に乱れた場合でも乱れ具合が最小限。※実機でも稀に乱れる。v2.20更新
			if (!_bTimerRunning) //タイマーが動いていなかった場合
				_DownCounter = _ReloadRegister + 1; //TVスポーツバスケでフリーズ、マジカルチェイスの面スタート時などでの、動作安定のためシンプルに。v2.65更新
			return;

		case 1:
			//PRINTF("TimerTest %X, %X",regNum,data);//test用
			//ダウンカウンタを設定
			//カウント開始時は、この直後のTIMER_AdvanceClockですぐ-1引かれてしまうので、+1しておく。
			//そうするとおそらく実機と同じタイミング。+1しないとROCK-ONで音が鳴らない。v1.62更新
			if (!_bTimerRunning) //Kitao更新。タイマーが動いていなかった場合。動いているときにDownCounterをリセットすると"レギオン"でBGMのテンポが遅くなってしまう。v2.26更新
			{
				if (_DownCounter == 1)
					_DownCounter = _ReloadRegister + 1; //クラックスでリセットが必要。
			}
			//タイマー割り込みオンオフ設定
			_bTimerRunning = (data & 1);
			//タイマー割り込みをオフにする場合の処理
			if (!_bTimerRunning)
				_DownCounter = 1; //クラックスで必要。v2.20
			return;
	}
}


/*-----------------------------------------------------------------------------
	[TIMER_AdvanceClock]
		指定のＣＰＵクロック数分だけタイマー処理を進めます．
-----------------------------------------------------------------------------*/
//Kitao更新。進めるクロック数を1クロック固定にした。
void
TIMER_AdvanceClock()
{
	//Kitao更新。v2.20
	if (--_DownCounter == 0)  //v1.62更新。_ReloadRegisterを1024ジャストにしたのでここもジャスト1024サイクルに。1サイクルの違いで音色に大きな違いが出る。
	{
		if (_bTimerRunning) //Kitao追加
		{
			INTCTRL_Request(INTCTRL_TIRQ); //タイマ割り込み発生
			_DownCounter = _ReloadRegister;
		}
		else //タイマーランニングがオフのときはカウンタを最短値に設定して待つようにした。v2.20
			_DownCounter = 1;
	}
}


#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
TIMER_SaveState(
	FILE*		p)
{
	if (p == NULL)
		return FALSE;

	SAVE_V(_DownCounter);
	SAVE_V(_ReloadRegister);
	SAVE_V(_bTimerRunning);

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
TIMER_LoadState(
	FILE*		p)
{
	Sint32	fastForwardingR; //v0.95から廃止。ダミー
	BOOL	bSoundAjust; //v0.95から廃止。ダミー
	BOOL	bUseVideoSpeedUpButton; //v0.95から廃止。ダミー

	if (p == NULL)
		return FALSE;

	LOAD_V(_DownCounter);
	LOAD_V(_ReloadRegister);
	LOAD_V(_bTimerRunning);

	if ((MAINBOARD_GetStateVersion() >= 19)&&
		(MAINBOARD_GetStateVersion() <= 20)) //Kitao追加。v0.92～v0.94のセーブファイルなら
	{
		LOAD_V(fastForwardingR); //ダミー
		LOAD_V(bSoundAjust); //ダミー
		LOAD_V(bUseVideoSpeedUpButton); //ダミー
	}

	return TRUE;
}

#undef SAVE_V
#undef LOAD_V
