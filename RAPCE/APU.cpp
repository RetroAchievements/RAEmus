/******************************************************************************
Ootake
・PSG,ADPCM,CDDAそれぞれにDirectSoundバッファを用意することで、各サウンドのダイ
  ナミックレンジを最大限に取り、音質を向上させた。
・バッファのブロックを４つに分けることで、音の遅延とパソコンへの負荷を軽減させ
  た。
・マスターボリュームの調整は100%を基準とし絞るのみとした。(音質向上)
・初回の再生時にはウェイトを入れることで、初回再生に起こるノイズを解消した。
・現状は再生サンプルレートは44.1KHz固定とした。(CD-DA再生時の速度アップのため)
・クリティカルセクションは必要ない(書き込みが同時に行われるわけではない)ような
  ので、省略し高速化した。v1.09

Copyright(C)2006-2010 Kitao Nakamura.
	改造版・後継版を公開なさるときは必ずソースコードを添付してください。
	その際に事後でかまいませんので、ひとことお知らせいただけると幸いです。
	商的な利用は禁じます。
	あとは「GNU General Public License(一般公衆利用許諾契約書)」に準じます。

*******************************************************************************
	[APU.c]
		ＡＰＵを実装します。

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
#include <math.h>
#include "APU.h"
#include "AudioOut.h"
#include "PSG.h"
#include "ADPCM.h"
#include "CDROM.h"
#include "App.h"
#include "MainBoard.h"
#include "PRINTF.h"


static Sint32			_SampleRate = 44100;	// [Hz]
static Sint32			_BufSize = 1664; //Kitao更新。この値はDirectXで一度に再生(転送)するサイズ。負担を軽くするため＆遅延を少なくするために、この大きさのAOUT_BUFFERRATE(現状4にした)倍のバッファを用意してDirectX側にその部分部分を渡す方式にする。単位[sample]
static Sint32			_BufSizeAll = 1664*AOUT_BUFFERRATE; //Kitao追加。全てのブロックのバッファ容量。高速化のため用意。v0.92。
static Sint32			_nSamplesPerFrame;

static Sint16*			_pMixBuf1 = NULL; //Kitao更新。PSG専用バッファ。PSG,ADPCM,CDDAそれぞれにバッファを用意してダイナミックレンジを最大限に取る。(音質大幅アップ)
static Sint16*			_pMixBuf2 = NULL; //Kitao更新。ADPCM専用バッファ
static Sint16*			_pMixBuf3 = NULL; //Kitao更新。CDDA専用バッファ
static volatile Sint32	_MixBufPos1; //Kitao更新。PSG。PSGの場合、タイミングを取るためにメインスレッドからも参照するのでvolatileに。
static volatile Sint32	_MixBufPos2; //Kitao更新。ADPCM。ADPCMも細やかなタイミングを必要とするソフトもある(ドラゴンスレイヤー英雄伝説のドラムなど)ので、PSGと同様の処理をする。v2.18
static Sint32			_MixBufPos3; //Kitao更新。CDDA
static volatile double	_MixBufEndPos1; //Kitao更新。PSG
static volatile double	_MixBufEndPos2; //Kitao更新。ADPCM
static Sint32			_MixBufEndPos3; //Kitao更新。CDDA。CDDAは細かいタイミングを取る必要が無いので整数演算でOK。v2.29更新
static volatile Sint32	_NextPlayPos1; //Kitao追加。PSG用。次に再生(DirectSoundに転送)する位置。0かBufSize*n。
static volatile Sint32	_NextPlayPos2; //Kitao追加。ADPCM用
static Sint32			_NextPlayPos3; //Kitao追加。CDDA用
static volatile BOOL	_bPosStop1 = FALSE; //Kitao追加。バッファが満杯のときTRUEに。
static volatile BOOL	_bPosStop2 = FALSE; //Kitao追加。バッファが満杯のときTRUEに。

static Uint32		_ClockCount;
static Sint32		_Volume; // APU volume (0-65535) Kitao更新。音質優先のためここではボリュームダウンのみを行う。

static HANDLE	_hMutex; //v2.18
static volatile BOOL	_bApuBusy = FALSE; //Kitao追加


/*-----------------------------------------------------------------------------
	[callback_mixer]
		この関数は AudioInterface のコールバックとして登録される。
	この関数が呼び出されるたびに、それぞれのチャネルの出力を
	Sint16 に変換して pDst に書き出す。チャネルの出力（サンプル）
	が足りない場合は、その分だけ補充する。
-----------------------------------------------------------------------------*/
//Kitao追記：ここはAudioOut.cの音再生専用スレッド上で実行される。
static void
callback_mixer(
	int 			ch,					// Kitao追加。1=PSG, 2=ADPCM, 3=CDDA
	Sint16*			pDst,				// 出力先バッファ //Kitao更新。各チャンネル専用バッファに分けたためSint16に。
	Sint32			nSamples)			// 書き出すサンプル数 
{
	Sint32			n, n2;
	Sint32			a;
	Sint16*			pSrc; //Kitao更新。Sint16に。
	int				i;

	_bApuBusy = TRUE;

	WaitForSingleObject(_hMutex, INFINITE); //メインスレッドからも同時に共有変数が書き換えられる可能性があるので、排他処理。v2.18
	switch (ch)
	{
		case 1: //PSG
			pSrc = _pMixBuf1 + (_NextPlayPos1 << 1);
			
			//Kitao更新。MixBufをAOUT_BUFFERRATE倍取るようにした。a＝バッファに溜まっているサンプル数
			if ((!_bPosStop1)&&(_MixBufPos1 >= _NextPlayPos1))
				a = _MixBufPos1 - _NextPlayPos1;
			else
				a = _BufSizeAll - _NextPlayPos1 + _MixBufPos1;
			
			if (a < nSamples) //溜まっているサンプル数が足りない場合
			{
				// 足りない分を補充する。
				n = nSamples - a;
				PSG_Mix(_pMixBuf1+(_MixBufPos1 << 1), n);
				_MixBufPos1 += n;
				if (_MixBufPos1 >= _BufSizeAll)
					_MixBufPos1 -= _BufSizeAll;
				_MixBufEndPos1 = _MixBufPos1;
			}
			break;
			
		case 2: //ADPCM
			pSrc = _pMixBuf2 + (_NextPlayPos2 << 1);
			
			//Kitao更新。MixBufをAOUT_BUFFERRATE倍取るようにした。a＝バッファに溜まっているサンプル数
			if ((!_bPosStop2)&&(_MixBufPos2 >= _NextPlayPos2))
				a = _MixBufPos2 - _NextPlayPos2;
			else
				a = _BufSizeAll - _NextPlayPos2 + _MixBufPos2;
			
			if (a < nSamples) //溜まっているサンプル数が足りない場合
			{
				// 足りない分を補充する。
				n = nSamples - a;
				ADPCM_Mix(_pMixBuf2+(_MixBufPos2 << 1), n);
				_MixBufPos2 += n;
				if (_MixBufPos2 >= _BufSizeAll)
					_MixBufPos2 -= _BufSizeAll;
				_MixBufEndPos2 = _MixBufPos2;
			}
			break;
		
		case 3: //CDDA
			pSrc = _pMixBuf3 + (_NextPlayPos3 << 1);
			
			//Kitao更新。MixBufをAOUT_BUFFERRATE倍取るようにした。a＝バッファに溜まっているサンプル数
			if (_MixBufPos3 >= _NextPlayPos3)
				a = _MixBufPos3 - _NextPlayPos3;
			else
				a = _BufSizeAll - _NextPlayPos3 + _MixBufPos3;
			
			if (a < nSamples) //溜まっているサンプル数が足りない場合
			{
				// 足りない分を補充する。
				n = nSamples - a;
				CDROM_Mix(_pMixBuf3+(_MixBufPos3 << 1), n);
				_MixBufPos3 += n;
				if (_MixBufPos3 >= _BufSizeAll)
					_MixBufPos3 -= _BufSizeAll;
				_MixBufEndPos3 = _MixBufPos3;
			}
			break;
		default:
			pSrc = 0; //コンパイルエラーを回避
	}
	ReleaseMutex(_hMutex); //v2.18。排他処理

	//DirectSoundのバッファに書き込み
	//Kitao更新。ここではボリュームダウンのみ行うようにしたためサチュレーションチェックは不要に。
	for (i = 0; i < nSamples; i++)
	{
		*pDst++ = (Sint16)((Sint32)(*pSrc) * _Volume / 65535); //Kitao更新。pSrcのほうもSint16にしたのでそのまま書き込む。
		*pSrc++ = 0; //使い終わったバッファは0にクリアしておく
		*pDst++ = (Sint16)((Sint32)(*pSrc) * _Volume / 65535);
		*pSrc++ = 0; //使い終わったバッファは0にクリアしておく
	}

	//Kitao追加。次の１バッファが埋まるぶんまでバッファデータを補充しておく。v2.17更新
	//			 ここで行うことで、直前のDirectSoundへの書き込み中に溜まったバッファも使えることになり、バッファを少なく設定しても音質が保てる。
	//			 また、PSGの音解像度も上がり(上のDirectSoundのバッファ書き込み処理中にメインスレッドでCPUからPSGレジスタが書き換わることが多々あるので)、ビブラート処理などが実機の音に近づく。
	//				   CPUエミュレートの動作を4ブロックに分けて行っている(MainBorad.cpp上)理由は、これを最大限に生かすため。(＆もうひとつの理由はジョイパッド入力の１フレーム遅延削除)
	//				   バッファの量によって再現性が若干変わったり、処理があまりに速いPCになったときにはこの効果が出にくくなる可能性があるので、将来そのタイミング取りも実装したい。v2.17記
	WaitForSingleObject(_hMutex, INFINITE); //メインスレッドからも同時に共有変数が書き換えられる可能性があるので、排他処理。v2.18
	switch (ch)
	{
		case 1: //PSG
			_NextPlayPos1 += _BufSize;
			if (_NextPlayPos1 == _BufSizeAll) _NextPlayPos1 = 0;
			//次の１バッファが埋まるぶんまで補充
			if ((_MixBufPos1 >= _NextPlayPos1))
				a = _MixBufPos1 - _NextPlayPos1;
			else
				a = _BufSizeAll - _NextPlayPos1 + _MixBufPos1;
			n = nSamples - a;// n＝足りない分。a＝バッファに溜まっているサンプル数。
			if (n > 0) //溜まっているサンプル数が足りない場合
			{
				// 足りない分を補充する。
				if (_MixBufPos1 + n > _BufSizeAll) //バッファ最後尾からあふれてしまう場合は、まずバッファ最後尾までMixして残りを先頭からMix。
				{
					n2 = _BufSizeAll - _MixBufPos1;
					PSG_Mix(_pMixBuf1+(_MixBufPos1 << 1), n2);
					_MixBufPos1 = 0;
					n -= n2;
				}
				PSG_Mix(_pMixBuf1+(_MixBufPos1 << 1), n);
				_MixBufPos1 += n;
				if (_MixBufPos1 >= _BufSizeAll)
					_MixBufPos1 -= _BufSizeAll;
				_MixBufEndPos1 = _MixBufPos1;
			}
			_bPosStop1 = FALSE;
			break;
			
		case 2: //ADPCM
			_NextPlayPos2 += _BufSize;
			if (_NextPlayPos2 == _BufSizeAll) _NextPlayPos2 = 0;
			//次の１バッファが埋まるぶんまで補充
			if ((_MixBufPos2 >= _NextPlayPos2))
				a = _MixBufPos2 - _NextPlayPos2;
			else
				a = _BufSizeAll - _NextPlayPos2 + _MixBufPos2;
			n = nSamples - a;// n＝足りない分。a＝バッファに溜まっているサンプル数。
			if (n > 0) //溜まっているサンプル数が足りない場合
			{
				// 足りない分を補充する。
				if (_MixBufPos2 + n > _BufSizeAll) //バッファ最後尾からあふれてしまう場合は、まずバッファ最後尾までMixして残りを先頭からMix。
				{
					n2 = _BufSizeAll - _MixBufPos2;
					ADPCM_Mix(_pMixBuf2+(_MixBufPos2 << 1), n2);
					_MixBufPos2 = 0;
					n -= n2;
				}
				ADPCM_Mix(_pMixBuf2+(_MixBufPos2 << 1), n);
				_MixBufPos2 += n;
				if (_MixBufPos2 >= _BufSizeAll)
					_MixBufPos2 -= _BufSizeAll;
				_MixBufEndPos2 = _MixBufPos2;
			}
			_bPosStop2 = FALSE;
			break;
		
		case 3: //CDDA
			_NextPlayPos3 += _BufSize;
			if (_NextPlayPos3 == _BufSizeAll) _NextPlayPos3 = 0;
			//次の１バッファが埋まるぶんまで補充
			if ((_MixBufPos3 >= _NextPlayPos3))
				a = _MixBufPos3 - _NextPlayPos3;
			else
				a = _BufSizeAll - _NextPlayPos3 + _MixBufPos3;
			n = nSamples - a;// n＝足りない分。a＝バッファに溜まっているサンプル数。
			if (n > 0) //溜まっているサンプル数が足りない場合
			{
				// 足りない分を補充する。
				if (_MixBufPos3 + n > _BufSizeAll) //バッファ最後尾からあふれてしまう場合は、まずバッファ最後尾までMixして残りを先頭からMix。
				{
					n2 = _BufSizeAll - _MixBufPos3;
					CDROM_Mix(_pMixBuf3+(_MixBufPos3 << 1), n2);
					_MixBufPos3 = 0;
					n -= n2;
				}
				CDROM_Mix(_pMixBuf3+(_MixBufPos3 << 1), n);
				_MixBufPos3 += n;
				if (_MixBufPos3 >= _BufSizeAll)
					_MixBufPos3 -= _BufSizeAll;
				_MixBufEndPos3 = _MixBufPos3;
			}
			break;
	}
	ReleaseMutex(_hMutex); //v2.18。排他処理

	_bApuBusy = FALSE; //Kitao追加
}


//Kitao追加。コールバックスレッドを実行中ならTRUEを返す。ステートセーブする際に使用。
BOOL
APU_GetApuBusy()
{
	return _bApuBusy;
}


//Kitao追加
static
void
apu_resetBuffer()
{
	_BufSizeAll		= _BufSize * AOUT_BUFFERRATE; //Kitao追加
	_ClockCount		= 0;
	_MixBufPos1		= 0;
	_MixBufEndPos1	= 0.0;
	_NextPlayPos1	= 0; //Kitao追加
	_bPosStop1		= FALSE; //Kitao追加
	_bPosStop2		= FALSE; //Kitao追加
	_MixBufPos2		= 0;
	_MixBufEndPos2	= 0.0;
	_NextPlayPos2	= 0; //Kitao追加
	_MixBufPos3		= 0;
	_MixBufEndPos3	= 0;
	_NextPlayPos3	= 0; //Kitao追加
	AOUT_SetPlayStart(); //Kitao追加。初回再生の合図を設定する。※初回はまだバッファにする材用（PSGキュー等）が溜まっていないためAOUTのほうでウェイトするようにした。
}

/*-----------------------------------------------------------------------------
	[Init]
		APUを初期化します。
-----------------------------------------------------------------------------*/
BOOL
APU_Init(
	Uint32		sampleRate,
	Uint32		bufSize)		// in samples
{
	_hMutex = CreateMutex(NULL,FALSE,NULL); //v2.18

	_Volume				= 65535;

	_SampleRate			= sampleRate;
	_nSamplesPerFrame	= _SampleRate / 60;

	//バッファを初期化
	if (!APU_SetBufferSize(bufSize))
		return FALSE;

	PSG_Init(sampleRate);
	ADPCM_Init();

	return TRUE;
}


void
APU_Pause(
	BOOL		bPause)
{
	if (bPause)
		AOUT_Play(FALSE);
	else
		AOUT_Play(TRUE);
}


void
APU_Deinit()
{
	AOUT_Deinit(); //この呼び出しでAOUTのスレッドは終了する。
	PSG_Deinit();
	ADPCM_Deinit();
	if (_pMixBuf1 != NULL)
		free(_pMixBuf1);
	if (_pMixBuf2 != NULL)
		free(_pMixBuf2);
	if (_pMixBuf3 != NULL)
		free(_pMixBuf3);
	_pMixBuf1 = NULL;
	_pMixBuf2 = NULL;
	_pMixBuf3 = NULL;
	CloseHandle(_hMutex); //v2.18
}


BOOL
APU_Reset()
{
	//Kitao更新。APU関連の変数も初期化が必要（初期化しないとバッファのポインタが途中から始まるので音が遅れる場合がある）
	APU_Deinit();
	return APU_Init(_SampleRate, _BufSize);
}


/*-----------------------------------------------------------------------------
	[SetSampleRate]
-----------------------------------------------------------------------------*/
BOOL
APU_SetSampleRate(
	Uint32		sampleRate)
{
	AOUT_Deinit();

	ZeroMemory(_pMixBuf1, _BufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE); //Kitao更新。PSG用。内部バッファをAOUT_BUFFERRATE倍ぶん用意。
	ZeroMemory(_pMixBuf2, _BufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE); //Kitao更新。ADPCM用
	ZeroMemory(_pMixBuf3, _BufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE); //Kitao更新。CDDA用

	if (!AOUT_Init(APP_GetSoundType(), _BufSize, sampleRate, callback_mixer))
	{
		// 元の設定に戻す 
		if (!AOUT_Init(APP_GetSoundType(), _BufSize, _SampleRate, callback_mixer))
			return FALSE;	// それでもダメならあきらめる。
	}
	_SampleRate = sampleRate;
	_nSamplesPerFrame = _SampleRate / 60;

	PSG_SetSampleRate(sampleRate);

	return TRUE;
}


/*-----------------------------------------------------------------------------
	[SetBufferSize]
-----------------------------------------------------------------------------*/
//Kitao更新。通常失敗することはないのでシンプルにした。
BOOL
APU_SetBufferSize(
	Uint32		bufSize)
{
	AOUT_Deinit();

	if (_pMixBuf1 != NULL)
		free(_pMixBuf1);
	if (_pMixBuf2 != NULL)
		free(_pMixBuf2);
	if (_pMixBuf3 != NULL)
		free(_pMixBuf3);

	// 8 [byte/sample] for Sint32 stereo buffer 
	if ((_pMixBuf1 = (Sint16*)malloc(bufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE)) == NULL) //Kitao更新。PSG用。内部バッファをAOUT_BUFFERRATE倍ぶん用意。
	{
		AOUT_Deinit();
		return FALSE;
	}
	if ((_pMixBuf2 = (Sint16*)malloc(bufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE)) == NULL) //Kitao更新。ADPCM用
	{
		AOUT_Deinit();
		return FALSE;
	}
	if ((_pMixBuf3 = (Sint16*)malloc(bufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE)) == NULL) //Kitao更新。CDDA用
	{
		AOUT_Deinit();
		return FALSE;
	}

	ZeroMemory(_pMixBuf1, bufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE); //Kitao更新。PSG用。内部バッファをAOUT_BUFFERRATE倍ぶん用意。
	ZeroMemory(_pMixBuf2, bufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE); //Kitao更新。ADPCM用
	ZeroMemory(_pMixBuf3, bufSize*sizeof(Sint16)*2*AOUT_BUFFERRATE); //Kitao更新。CDDA用

	if (!AOUT_Init(APP_GetSoundType(), bufSize, _SampleRate, callback_mixer))
		return FALSE;

	_BufSize = bufSize;
	apu_resetBuffer();

	return TRUE;
}


void
APU_SetVolume(
	Uint32		volume)		// 0 - 65535
{
	_Volume = volume;
}


#define DIV				256.0
#define DELTA_POS		44100.0 / 60.0 / DIV //Kitao追加。定数にした。
#define DELTA_CLOCK		(Sint32)(2.0 * PSG_FRQ / 60.0 / DIV - 3.0) //Kitao追加。定数にした。音途切れのプチノイズを防ぐために-3.0（DELTA_POSの切り上げ値ぶん。大きく引きすぎると連続性は良くなるがのっぺりした音になる）。v1.09更新

//Kitao更新。v2.32
void
APU_AdvanceClock()
{
	Sint32		nSamples;
	Sint32		nSamples2;//Kitao追加
//	Sint32		deltaClock; //Kitao更新。定数にして速度アップ。
//	double		deltaPos; //Kitao更新。現状は44100Hz固定のため定数にして速度アップ。

	ADPCM_AdvanceFadeClock();
	CDROM_AdvanceFadeClock(); //Kitao追加

	if (++_ClockCount < DELTA_CLOCK)
		return;

	//WriteBuffer処理
	//  Kitao更新。バッファを４つに分けてバッファに途切れる位置がないようにした。遅延はそのままでバッファが多く溜められるようになり結果的に音質も上がった。
	//			   進めるクロック数を1クロック固定にした。戻り値は廃止した。
	//			   v2.17更新。ここではPSGチャンネルのみをMixすることにした。CD-ROMはストリームなのでここの細かいタイミングでMixする必要がないため。高速化。

	_ClockCount = 0;

	WaitForSingleObject(_hMutex, INFINITE); //オーディオスレッドからも同時に共有変数が書き換えられる可能性があるので、排他処理。v2.18

	// Kitao更新。PSG, ADPCM, CDDA それぞれを別バッファで再生するようにした。
	//（ソフトでPSG,ADPCM,CDDAをミックスして音を鳴らした場合、ダイナミックレンジを1/3に下げなくてはいけないので音質が大きく落ちてしまうため）

	// PSGチャンネル
	if (!_bPosStop1)
	{
		_MixBufEndPos1 += DELTA_POS;
		nSamples = (Sint32)(_MixBufEndPos1 + 0.5) - _MixBufPos1; //小数点以下は四捨五入。PSGの場合、Mixするタイミングが正確なほど、切り捨て・切り上げよりも音質が良くなる。
		if (_MixBufPos1 + nSamples >= _BufSizeAll) //最終点を過ぎるなら
		{
			nSamples2 = _BufSizeAll - _MixBufPos1;
			//限界点まで書き込む
			PSG_Mix(_pMixBuf1 + (_MixBufPos1 << 1), nSamples2);// ステレオだから _MixBufPos*2
			_MixBufPos1 = 0;
			if (_NextPlayPos1 != 0) //次の再生が先頭からじゃない場合
			{
				//残りを先頭から書き込む
				nSamples -=	nSamples2;
				PSG_Mix(_pMixBuf1 + (_MixBufPos1 << 1), nSamples);// ステレオだから _MixBufPos*2 
				_MixBufPos1 = nSamples;
				_MixBufEndPos1 -= (double)_BufSizeAll;
			}
			else
			{
				//バッファの限界まで書き込んだ。バッファが使われるまで次回はバッファに追加しない。
				_MixBufEndPos1 = 0.0;
				_bPosStop1 = TRUE;
			}
		}
		else if	((_MixBufPos1 < _NextPlayPos1)&&(_MixBufPos1 + nSamples >= _NextPlayPos1)) //現在分岐点前で、足すと分岐点を過ぎるなら
		{
			nSamples = _NextPlayPos1 - _MixBufPos1;
			//限界点まで書き込む
			PSG_Mix(_pMixBuf1 + (_MixBufPos1 << 1), nSamples);// ステレオだから _MixBufPos*2 
			//バッファの限界まで書き込んだ。バッファが使われるまで次回はバッファに追加しない。
			_MixBufPos1 = _NextPlayPos1;
			_MixBufEndPos1 = _MixBufPos1;
			_bPosStop1 = TRUE;
		}
		else //通常
		{
			PSG_Mix(_pMixBuf1 + (_MixBufPos1 << 1), nSamples);// ステレオだから _MixBufPos*2 
			_MixBufPos1 += nSamples;
		}
	}

	// ADPCMチャンネル
	if (!_bPosStop2)
	{
		_MixBufEndPos2 += DELTA_POS;
		nSamples = (Sint32)(_MixBufEndPos2 + 0.5) - _MixBufPos2; //小数点以下は四捨五入。ドラゴンスレイヤー英雄伝説のドラム等で細かなタイミングを要求されることがあるので、切捨て切り上げでなく四捨五入とした。
		if (_MixBufPos2 + nSamples >= _BufSizeAll) //最終点を過ぎるなら
		{
			nSamples2 = _BufSizeAll - _MixBufPos2;
			//限界点まで書き込む
			ADPCM_Mix(_pMixBuf2 + (_MixBufPos2 << 1), nSamples2);// ステレオだから _MixBufPos*2
			_MixBufPos2 = 0;
			if (_NextPlayPos2 != 0) //次の再生が先頭からじゃない場合
			{
				//残りを先頭から書き込む
				nSamples -=	nSamples2;
				ADPCM_Mix(_pMixBuf2 + (_MixBufPos2 << 1), nSamples);// ステレオだから _MixBufPos*2 
				_MixBufPos2 = nSamples;
				_MixBufEndPos2 -= (double)_BufSizeAll;
			}
			else
			{
				//バッファの限界まで書き込んだ。バッファが使われるまで次回はバッファに追加しない。
				_MixBufEndPos2 = 0.0;
				_bPosStop2 = TRUE;
			}
		}
		else if	((_MixBufPos2 < _NextPlayPos2)&&(_MixBufPos2 + nSamples >= _NextPlayPos2)) //現在分岐点前で、足すと分岐点を過ぎるなら
		{
			nSamples = _NextPlayPos2 - _MixBufPos2;
			//限界点まで書き込む
			ADPCM_Mix(_pMixBuf2 + (_MixBufPos2 << 1), nSamples);// ステレオだから _MixBufPos*2 
			//バッファの限界まで書き込んだ。バッファが使われるまで次回はバッファに追加しない。
			_MixBufPos2 = _NextPlayPos2;
			_MixBufEndPos2 = _MixBufPos2;
			_bPosStop2 = TRUE;
		}
		else //通常
		{
			ADPCM_Mix(_pMixBuf2 + (_MixBufPos2 << 1), nSamples);// ステレオだから _MixBufPos*2 
			_MixBufPos2 += nSamples;
		}
	}

	ReleaseMutex(_hMutex); //v2.18。排他処理
}


// save variable
#define SAVE_V(V)	if (fwrite(&V, sizeof(V), 1, p) != 1)	return FALSE
#define LOAD_V(V)	if (fread(&V, sizeof(V), 1, p) != 1)	return FALSE
/*-----------------------------------------------------------------------------
	[SaveState]
		状態をファイルに保存します。 
-----------------------------------------------------------------------------*/
BOOL
APU_SaveState(
	FILE*		p)
{
	BOOL	ret;
	Sint32	windowsVolume;

	if (p == NULL)
		return FALSE;

	SAVE_V(_ClockCount);
	windowsVolume = APP_GetWindowsVolume();
	SAVE_V(windowsVolume); //v1.61追加。ゲーム毎のボリューム調整値。レジュームモードのときのみ有効。非レジューム時にもSAVEはしておく。

	ret =  PSG_SaveState(p);
	ret |= ADPCM_SaveState(p);
	
	return ret;
}


/*-----------------------------------------------------------------------------
	[LoadState]
		状態をファイルから読み込みます。 
-----------------------------------------------------------------------------*/
BOOL
APU_LoadState(
	FILE*		p)
{
	BOOL	ret;
	Sint32	windowsVolume;

	if (p == NULL)
		return FALSE;

	LOAD_V(_ClockCount);
	if (MAINBOARD_GetStateVersion() >= 33) //Kitao追加。v1.61beta以降のセーブファイルなら
	{
		LOAD_V(windowsVolume);
		APP_ResumeWindowsVolume(windowsVolume); //レジュームファイルを読み出した場合のみ、音量を復元する。
	}

	ret =  PSG_LoadState(p);
	ret |= ADPCM_LoadState(p);

	return TRUE;
}

#undef SAVE_V
#undef LOAD_V
